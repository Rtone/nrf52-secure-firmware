#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "bsp_btn_ble.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_log.h"
#include "nrf_crypto.h"
#include "app.h"
#include "conn.h"



static void
_connParamsErrorHandler (uint32_t nrf_error)
{
  APP_ERROR_HANDLER (nrf_error);
}


static void
_onConnParamsEvt (ble_conn_params_evt_t * p_evt)
{
  ret_code_t err_code;

  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
      err_code =
	sd_ble_gap_disconnect (m_conn_handle,
			       BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
      APP_ERROR_CHECK (err_code);
    }
  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
    {
      NRF_LOG_INFO ("conn succeeded");
    }
}



void
connParamsInit (void)
{
  ret_code_t err_code;
  ble_conn_params_init_t cp_init;
  ble_gap_conn_params_t preferred_cnxn_param;

  preferred_cnxn_param.min_conn_interval = MIN_CONN_INTERVAL;
  preferred_cnxn_param.max_conn_interval = MAX_CONN_INTERVAL;
  preferred_cnxn_param.slave_latency = SLAVE_LATENCY;
  preferred_cnxn_param.conn_sup_timeout = CONN_SUP_TIMEOUT;

  memset (&cp_init, 0, sizeof (cp_init));

  cp_init.p_conn_params = &preferred_cnxn_param;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
  cp_init.disconnect_on_fail = true;
  cp_init.evt_handler = _onConnParamsEvt;
  cp_init.error_handler = _connParamsErrorHandler;

  err_code = ble_conn_params_init (&cp_init);

  APP_ERROR_CHECK (err_code);
}
