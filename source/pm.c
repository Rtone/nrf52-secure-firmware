#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "ble_advertising.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "fds.h"
#include "peer_manager.h"
#include "bsp_btn_ble.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_log.h"
#include "nrf_crypto.h"
#include "nrf_crypto_keys.h"
#include "le_secure.h"
#include "pm.h"
#include "app.h"



static void
_deleteBonds (void)
{
  ret_code_t err_code;

  NRF_LOG_INFO ("Erase bonds!");

  err_code = pm_peers_delete ();
  APP_ERROR_CHECK (err_code);
}


static void
_pmEvtHandler (pm_evt_t const *p_evt)
{
  ret_code_t err_code;
  pm_conn_sec_status_t status2;
  pm_conn_sec_status_t status3;

  switch (p_evt->evt_id)
    {
    case PM_EVT_BONDED_PEER_CONNECTED:
      {
	NRF_LOG_INFO ("Connected to a previously bonded device.");
	_deleteBonds ();
      }
      break;

    case PM_EVT_CONN_SEC_SUCCEEDED:
      {
	NRF_LOG_INFO
	  ("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.",
	   ble_conn_state_role (p_evt->conn_handle), p_evt->conn_handle,
	   p_evt->params.conn_sec_succeeded.procedure);

	err_code = pm_conn_sec_status_get (m_conn_handle, &status2);
	NRF_LOG_INFO ("Status %d", status2.encrypted);
	/* On checke la valeur de "p_evt->params.conn_sec_succeeded.procedure" pour connaitre la procedure (pairing, bonding, encryption)
	 * PM_LINK_SECURED_PROCEDURE_BONDING == 1
	 * PM_LINK_SECURED_PROCEDURE_PAIRING == 2
	 * PM_LINK_SECURED_PROCEDURE_ENCRYPTION == 0
	 */
      }
      break;

    case PM_EVT_CONN_SEC_FAILED:
      {
	NRF_LOG_INFO ("PM_EVT_CONN_SEC_FAILED");
	//_deleteBonds();
	/* Often, when securing fails, it shouldn't be restarted, for security reasons.
	 * Other times, it can be restarted directly.
	 * Sometimes it can be restarted, but only after changing some Security Parameters.
	 * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
	 * Sometimes it is impossible, to secure the link, or the peer device does not support it.
	 * How to handle this error is highly application dependent. */
      }
      break;

    case PM_EVT_CONN_SEC_CONFIG_REQ:
      {
	// Reject pairing request from an already bonded peer.
	pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false };
	pm_conn_sec_config_reply (p_evt->conn_handle, &conn_sec_config);
      }
      break;

    case PM_EVT_STORAGE_FULL:
      {
	NRF_LOG_INFO ("PM_EVT_STORAGE_FULL");
	// Run garbage collection on the flash.
	err_code = fds_gc ();
	if (err_code == FDS_ERR_BUSY
	    || err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
	  {
	    // Retry.
	  }
	else
	  {
	    APP_ERROR_CHECK (err_code);
	  }
      }
      break;

    case PM_EVT_PEERS_DELETE_SUCCEEDED:
      {
	NRF_LOG_INFO ("PM_EVT_PEERS_DELETE_SUCCEEDED");
	err_code = pm_conn_sec_status_get (m_conn_handle, &status3);
	NRF_LOG_INFO ("Status %d", status3.encrypted);
      }
      break;

    case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
      {
	NRF_LOG_INFO ("PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED");
	// The local database has likely changed, send service changed indications.
	pm_local_database_has_changed ();
      }
      break;

    case PM_EVT_PEER_DATA_UPDATE_FAILED:
      {
	// Assert.
	APP_ERROR_CHECK (p_evt->params.peer_data_update_failed.error);
      }
      break;

    case PM_EVT_PEER_DELETE_FAILED:
      {
	NRF_LOG_INFO ("PM_EVT_PEER_DELETE_FAILED");
	// Assert.
	APP_ERROR_CHECK (p_evt->params.peer_delete_failed.error);
      }
      break;

    case PM_EVT_PEERS_DELETE_FAILED:
      {
	NRF_LOG_INFO ("PM_EVT_PEERS_DELETE_FAILED");
	// Assert.
	APP_ERROR_CHECK (p_evt->params.peers_delete_failed_evt.error);
      }
      break;

    case PM_EVT_ERROR_UNEXPECTED:
      {
	NRF_LOG_INFO ("PM_EVT_ERROR_UNEXPECTED");
	// Assert.
	APP_ERROR_CHECK (p_evt->params.error_unexpected.error);
      }
      break;

    case PM_EVT_CONN_SEC_START:
      {
	NRF_LOG_INFO ("PM_EVT_CONN_SEC_START");
      }
      break;
    case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
    case PM_EVT_PEER_DELETE_SUCCEEDED:
    case PM_EVT_LOCAL_DB_CACHE_APPLIED:
    case PM_EVT_SERVICE_CHANGED_IND_SENT:
    case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
    default:
      break;
    }
}


void
leSecureConn (void)
{
  ble_gap_sec_params_t sec_param;
  ret_code_t err_code;

  err_code = pm_init ();
  APP_ERROR_CHECK (err_code);

  memset (&sec_param, 0, sizeof (ble_gap_sec_params_t));

  // Security parameters to be used for all security procedures.
  sec_param.bond = SEC_PARAM_BOND;
  sec_param.mitm = SEC_PARAM_MITM;
  sec_param.lesc = SEC_PARAM_LESC;
  sec_param.keypress = SEC_PARAM_KEYPRESS;
  sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
  sec_param.oob = SEC_PARAM_OOB;
  sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
  sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
  sec_param.kdist_own.enc = 1;
  sec_param.kdist_own.id = 1;
  sec_param.kdist_peer.enc = 1;
  sec_param.kdist_peer.id = 1;


  err_code = pm_sec_params_set (&sec_param);
  APP_ERROR_CHECK (err_code);


  err_code = pm_register (_pmEvtHandler);
  APP_ERROR_CHECK (err_code);


  err_code = lescGenerateKeyPair ();
  APP_ERROR_CHECK (err_code);


}


void
legacyPairing (void)
{
  ble_gap_sec_params_t sec_param;
  ret_code_t err_code;

  err_code = pm_init ();
  APP_ERROR_CHECK (err_code);

  //Code for Static Passkey
  /*
     uint8_t passkey[] = STATIC_PASSKEY ;
     m_static_pin_option.gap_opt.passkey.p_passkey = passkey;
     err_code =  sd_ble_opt_set(BLE_GAP_OPT_PASSKEY, &m_static_pin_option);
     APP_ERROR_CHECK(err_code);
   */

  //Code for Random Passkey
   /**/ m_static_pin_option.gap_opt.passkey.p_passkey = NULL;
  err_code = sd_ble_opt_set (BLE_GAP_OPT_PASSKEY, &m_static_pin_option);
  APP_ERROR_CHECK (err_code);


  memset (&sec_param, 0, sizeof (ble_gap_sec_params_t));

  // Security parameters to be used for all security procedures.
  sec_param.bond = SEC_PARAM_BOND;
  sec_param.mitm = SEC_PARAM_MITM;
  sec_param.lesc = SEC_PARAM_LESC;
  sec_param.keypress = SEC_PARAM_KEYPRESS;
  sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
  sec_param.oob = SEC_PARAM_OOB;
  sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
  sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
  sec_param.kdist_own.enc = 1;
  sec_param.kdist_own.id = 1;
  sec_param.kdist_peer.enc = 1;
  sec_param.kdist_peer.id = 1;

  err_code = pm_sec_params_set (&sec_param);
  APP_ERROR_CHECK (err_code);

  err_code = pm_register (_pmEvtHandler);
  APP_ERROR_CHECK (err_code);
}
