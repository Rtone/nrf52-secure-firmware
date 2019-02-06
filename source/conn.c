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

/********************************************************************************/
/*   FUNCTION PROTOTYPES                                                        */
/********************************************************************************/

static void
_connParamsErrorHandler(uint32_t nrf_error);

static void
_onConnParamsEvt(ble_conn_params_evt_t *p_evt);

/********************************************************************************/
/*   --- END of FUNCTION PROTOTYPES ---                                         */
/********************************************************************************/

/********************************************************************************/
/*   HANDLERS                                                                   */
/********************************************************************************/

static void
_connParamsErrorHandler(uint32_t nrf_error)
{
  APP_ERROR_HANDLER(nrf_error);
}

static void
_onConnParamsEvt(ble_conn_params_evt_t *p_evt)
{
  ret_code_t err_code;

  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
  {
    err_code =
        sd_ble_gap_disconnect(m_conn_handle,
                              BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    APP_ERROR_CHECK(err_code);
  }
  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
  {
    NRF_LOG_INFO("Conn succeeded ...");
  }
}

/********************************************************************************/
/*   --- END of HANDLERS ---                                                    */
/********************************************************************************/

/********************************************************************************/
/*   CONNECTION PARAMS. FUCTION                                                 */
/********************************************************************************/

void connParamsInit(void)
{
  ret_code_t err_code;
  ble_conn_params_init_t cpInit;
  ble_gap_conn_params_t preferredCnxnParam;

  preferredCnxnParam.min_conn_interval = MIN_CONN_INTERVAL;
  preferredCnxnParam.max_conn_interval = MAX_CONN_INTERVAL;
  preferredCnxnParam.slave_latency = SLAVE_LATENCY;
  preferredCnxnParam.conn_sup_timeout = CONN_SUP_TIMEOUT;

  memset(&cpInit, 0, sizeof(cpInit));

  cpInit.p_conn_params = &preferredCnxnParam;
  cpInit.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cpInit.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cpInit.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
  cpInit.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
  cpInit.disconnect_on_fail = true;
  cpInit.evt_handler = _onConnParamsEvt;
  cpInit.error_handler = _connParamsErrorHandler;

  err_code = ble_conn_params_init(&cpInit);

  APP_ERROR_CHECK(err_code);
}

/********************************************************************************/
/*   --- END of CONNECTION PARAMS. FUCTION ---                                  */
/********************************************************************************/
