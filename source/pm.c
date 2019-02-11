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
#include "adv.h"
#include "app.h"

/********************************************************************************/
/*   FUNCTION PROTOTYPES                                                        */
/********************************************************************************/

static void
_deleteBonds(void);

static void
_pmEvtHandler(pm_evt_t const *p_evt);

/********************************************************************************/
/*   --- END of FUNCTION PROTOTYPES ---                                         */
/********************************************************************************/

/********************************************************************************/
/*   PAIRING MODES FUCTIONS                                                     */
/********************************************************************************/

void leSecureConn(void)
{
  ble_gap_sec_params_t secParam;
  ret_code_t err_code;

  err_code = pm_init();
  APP_ERROR_CHECK(err_code);

  memset(&secParam, 0, sizeof(ble_gap_sec_params_t));

  // Security parameters to be used for all security procedures.
  secParam.bond = SEC_PARAM_BOND;
  secParam.mitm = SEC_PARAM_MITM;
  secParam.lesc = SEC_PARAM_LESC;
  secParam.keypress = SEC_PARAM_KEYPRESS;
  secParam.io_caps = SEC_PARAM_IO_CAPABILITIES;
  secParam.oob = SEC_PARAM_OOB;
  secParam.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
  secParam.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
  secParam.kdist_own.enc = 1;
  secParam.kdist_own.id = 1;
  secParam.kdist_peer.enc = 1;
  secParam.kdist_peer.id = 1;

  err_code = pm_sec_params_set(&secParam);
  APP_ERROR_CHECK(err_code);

  err_code = pm_register(_pmEvtHandler);
  APP_ERROR_CHECK(err_code);

  err_code = lescGenerateKeyPair();
  APP_ERROR_CHECK(err_code);
}

void legacyPairing(void)
{
  ble_gap_sec_params_t secParam;
  ret_code_t err_code;

  err_code = pm_init();
  APP_ERROR_CHECK(err_code);

  //Code for Static Passkey
  /*
     uint8_t passkey[] = STATIC_PASSKEY ;
     m_static_pin_option.gap_opt.passkey.p_passkey = passkey;
     err_code =  sd_ble_opt_set(BLE_GAP_OPT_PASSKEY, &m_static_pin_option);
     APP_ERROR_CHECK(err_code);
   */

  //Code for Random Passkey
  /**/ m_static_pin_option.gap_opt.passkey.p_passkey = NULL;
  err_code = sd_ble_opt_set(BLE_GAP_OPT_PASSKEY, &m_static_pin_option);
  APP_ERROR_CHECK(err_code);

  memset(&secParam, 0, sizeof(ble_gap_sec_params_t));

  // Security parameters to be used for all security procedures.
  secParam.bond = SEC_PARAM_BOND;
  secParam.mitm = SEC_PARAM_MITM;
  secParam.lesc = SEC_PARAM_LESC;
  secParam.keypress = SEC_PARAM_KEYPRESS;
  secParam.io_caps = SEC_PARAM_IO_CAPABILITIES;
  secParam.oob = SEC_PARAM_OOB;
  secParam.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
  secParam.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
  secParam.kdist_own.enc = 1;
  secParam.kdist_own.id = 1;
  secParam.kdist_peer.enc = 1;
  secParam.kdist_peer.id = 1;

  err_code = pm_sec_params_set(&secParam);
  APP_ERROR_CHECK(err_code);

  err_code = pm_register(_pmEvtHandler);
  APP_ERROR_CHECK(err_code);
}

/********************************************************************************/
/*   --- ENd of PAIRING MODES FUCTIONS ---                                      */
/********************************************************************************/

/********************************************************************************/
/*   HANDLERS                                                                   */
/********************************************************************************/

/*
**  Handler to manage Peer Manager events
*/
static void
_pmEvtHandler(pm_evt_t const *p_evt)
{
  ret_code_t err_code;
  pm_conn_sec_status_t statusEncryption;

  switch (p_evt->evt_id)
  {
  case PM_EVT_BONDED_PEER_CONNECTED:
  {
    NRF_LOG_INFO("Connected to a previously bonded device.");
    //_deleteBonds();
  }
  break;

  case PM_EVT_CONN_SEC_SUCCEEDED:
  {
    NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.",
                 ble_conn_state_role(p_evt->conn_handle), p_evt->conn_handle,
                 p_evt->params.conn_sec_succeeded.procedure);

    err_code = pm_conn_sec_status_get(m_conn_handle, &statusEncryption);
    NRF_LOG_INFO("Status %d", statusEncryption.encrypted);
    /* On checke la valeur de "p_evt->params.conn_sec_succeeded.procedure" pour connaitre la procedure (pairing, bonding, encryption)
	 * PM_LINK_SECURED_PROCEDURE_BONDING == 1
	 * PM_LINK_SECURED_PROCEDURE_PAIRING == 2
	 * PM_LINK_SECURED_PROCEDURE_ENCRYPTION == 0
	 */
  }
  break;

  case PM_EVT_CONN_SEC_FAILED:
  {
    NRF_LOG_INFO("PM_EVT_CONN_SEC_FAILED");
    _deleteBonds();
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
    pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
    pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
  }
  break;

  case PM_EVT_STORAGE_FULL:
  {
    NRF_LOG_INFO("PM_EVT_STORAGE_FULL");
    // Run garbage collection on the flash.
    err_code = fds_gc();
    if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
    {
      // Retry.
    }
    else
    {
      APP_ERROR_CHECK(err_code);
    }
  }
  break;

  case PM_EVT_PEERS_DELETE_SUCCEEDED:
  {
    NRF_LOG_INFO("PM_EVT_PEERS_DELETE_SUCCEEDED");
    /* 
    ** A call to pm_peers_delete has completed successfully. Flash storage now contains 
    ** no peer data.
    */
  }
  break;

  case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
  {
    NRF_LOG_INFO("PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED");
    // The local database has likely changed, send service changed indications.
    pm_local_database_has_changed();
  }
  break;

  case PM_EVT_PEER_DATA_UPDATE_FAILED:
  {
    // Assert.
    APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
  }
  break;

  case PM_EVT_PEER_DELETE_FAILED:
  {
    NRF_LOG_INFO("PM_EVT_PEER_DELETE_FAILED");
    // Assert.
    APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
  }
  break;

  case PM_EVT_PEERS_DELETE_FAILED:
  {
    NRF_LOG_INFO("PM_EVT_PEERS_DELETE_FAILED");
    // Assert.
    APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
  }
  break;

  case PM_EVT_ERROR_UNEXPECTED:
  {
    NRF_LOG_INFO("PM_EVT_ERROR_UNEXPECTED");
    // Assert.
    APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
  }
  break;

  case PM_EVT_CONN_SEC_START:
  {
    NRF_LOG_INFO("PM_EVT_CONN_SEC_START");
  }
  break;
  case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
  {
    if (p_evt->params.peer_data_update_succeeded.flash_changed && (p_evt->params.peer_data_update_succeeded.data_id == PM_PEER_DATA_ID_BONDING))
    {
      NRF_LOG_INFO("New Bond, add the peer to the whitelist if possible");
      NRF_LOG_INFO("\tm_whitelist_peer_cnt %d, MAX_PEERS_WLIST %d",
                   pmWhitelistPeerCnt + 1,
                   BLE_GAP_WHITELIST_ADDR_MAX_COUNT);
      // Note: You should check on what kind of white list policy your application should use.

      if (pmWhitelistPeerCnt < BLE_GAP_WHITELIST_ADDR_MAX_COUNT)
      {
        // Bonded to a new peer, add it to the whitelist.
        pmWhitelistPeers[pmWhitelistPeerCnt++] = pmPeerID;

        // The whitelist has been modified, update it in the Peer Manager.
        err_code = pm_whitelist_set(pmWhitelistPeers, pmWhitelistPeerCnt);
        APP_ERROR_CHECK(err_code);

        err_code = pm_device_identities_list_set(pmWhitelistPeers, pmWhitelistPeerCnt);
        if (err_code != NRF_ERROR_NOT_SUPPORTED)
        {
          APP_ERROR_CHECK(err_code);
        }
      }
    }
  }
  case PM_EVT_PEER_DELETE_SUCCEEDED:
  case PM_EVT_LOCAL_DB_CACHE_APPLIED:
  case PM_EVT_SERVICE_CHANGED_IND_SENT:
  case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
  default:
    break;
  }
}

/*
**  Function to delete security information saved by the peer manager  
*/
static void
_deleteBonds(void)
{
  ret_code_t err_code;

  NRF_LOG_INFO("Erase bonds!...");

  err_code = pm_peers_delete();
  APP_ERROR_CHECK(err_code);
}

/********************************************************************************/
/*   --- END of HANDLERS ---                                                    */
/********************************************************************************/