#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "bsp_btn_ble.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_crypto.h"
#include "nrf_crypto_keys.h"
#include "le_secure.h"
#include "ble_init.h"
#include "led_button.h"
#include "app.h"
#include "adv.h"

/********************************************************************************/
/*   FUNCTION PROTOTYPES                                                        */
/********************************************************************************/

static void
_sleepModeEnter(void);

static void
_onAdvEvt(ble_adv_evt_t bleAdvEvt);

/********************************************************************************/
/*   --- END of FUNCTION PROTOTYPES ---                                         */
/********************************************************************************/

/********************************************************************************/
/*   HANDLERS                                                                   */
/********************************************************************************/

static void
_sleepModeEnter(void)
{
  ret_code_t err_code;

  err_code = bsp_indication_set(BSP_INDICATE_IDLE);
  APP_ERROR_CHECK(err_code);

  err_code = bsp_btn_ble_sleep_mode_prepare();
  APP_ERROR_CHECK(err_code);

  err_code = sd_power_system_off();
  APP_ERROR_CHECK(err_code);
}

static void
_onAdvEvt(ble_adv_evt_t bleAdvEvt)
{
  ret_code_t err_code;

  switch (bleAdvEvt)
  {
  case BLE_ADV_EVT_FAST:
    NRF_LOG_INFO("Fast advertising...");
    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_ADV_EVT_SLOW:
    NRF_LOG_INFO("Slow advertising...");
    break;

  case BLE_ADV_EVT_DIRECTED:
    NRF_LOG_INFO("Directed advertising...");
    break;

  case BLE_ADV_EVT_IDLE:
    _sleepModeEnter();
    break;

  default:
    break;
  }
}

/********************************************************************************/
/*   --- END of HANDLERS ---                                                    */
/********************************************************************************/

/********************************************************************************/
/*   BLE ADVERTISING FUCTIONS                                                   */
/********************************************************************************/

void advertisingInit(ble_adv_init *advInit)
{
  ret_code_t err_code;
  ble_advertising_init_t init;

  memset(&init, 0, sizeof(init));

  init.advdata.name_type = advInit->nameType;
  init.advdata.include_appearance = advInit->appearance;
  init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  init.advdata.uuids_complete.uuid_cnt = advInit->uuid_cnt;
  init.advdata.uuids_complete.p_uuids = advInit->p_uuids;

  if (advInit->fast)
  {
    init.config.ble_adv_fast_enabled = true;
    init.config.ble_adv_fast_interval = advInit->intervalAdv;
    init.config.ble_adv_fast_timeout = advInit->timeoutAdv;
  }

  if (advInit->slow)
  {
    init.config.ble_adv_slow_enabled = true;
    init.config.ble_adv_slow_interval = advInit->intervalAdv;
    init.config.ble_adv_slow_timeout = advInit->timeoutAdv;
  }

  init.evt_handler = _onAdvEvt;

  err_code = ble_advertising_init(&m_advertising, &init);
  APP_ERROR_CHECK(err_code);

  ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

void advertisingStart()
{
  if (m_advertising.adv_modes_config.ble_adv_fast_enabled)
  {
    ret_code_t err_code =
        ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
  }

  if (m_advertising.adv_modes_config.ble_adv_slow_enabled)
  {
    ret_code_t err_code =
        ble_advertising_start(&m_advertising, BLE_ADV_MODE_SLOW);
    APP_ERROR_CHECK(err_code);
  }
}

/********************************************************************************/
/*   --- END of BLE ADVERTISING FUCTIONS ---                                    */
/********************************************************************************/
