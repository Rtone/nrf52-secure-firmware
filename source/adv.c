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
#include "peer_manager.h"
#include "le_secure.h"
#include "ble_init.h"
#include "led_button.h"
#include "app.h"
#include "adv.h"

/********************************************************************************/
/*   FUNCTION PROTOTYPES                                                        */
/********************************************************************************/

static void
_peerListGet(pm_peer_id_t *pmPeers, uint32_t *pmSize);

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
_peerListGet(pm_peer_id_t *pmPeers, uint32_t *pmSize)
{
  pm_peer_id_t peerID;
  uint32_t peersToCopy;

  peersToCopy = (*pmPeers < BLE_GAP_WHITELIST_ADDR_MAX_COUNT) ? *pmSize : BLE_GAP_WHITELIST_ADDR_MAX_COUNT;

  peerID = pm_next_peer_id_get(PM_PEER_ID_INVALID);
  *pmSize = 0;

  while ((peerID != PM_PEER_ID_INVALID) && (peersToCopy--))
  {
    pmPeers[(*pmSize)++] = peerID;
    peerID = pm_next_peer_id_get(peerID);
  }
}

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
    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    break;

  case BLE_ADV_EVT_DIRECTED:
    NRF_LOG_INFO("Directed advertising...");
    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    break;

  case BLE_ADV_EVT_FAST_WHITELIST:
    NRF_LOG_INFO("Fast advertising with whitelist...");
    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_ADV_EVT_SLOW_WHITELIST:
    NRF_LOG_INFO("Slow advertising with whitelist...");
    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
    APP_ERROR_CHECK(err_code);
    err_code = ble_advertising_restart_without_whitelist(&m_advertising);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_ADV_EVT_WHITELIST_REQUEST:
  {
    ble_gap_addr_t whitelistAddrs[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
    ble_gap_irk_t whitelistIRKs[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
    uint32_t addrCnt = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
    uint32_t irkCnt = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;

    err_code = pm_whitelist_get(whitelistAddrs, &addrCnt,
                                whitelistIRKs, &irkCnt);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEBUG("pm_whitelist_get returns %d addr in whitelist and %d irk whitelist",
                  addrCnt,
                  irkCnt);

    // Apply the whitelist.
    err_code = ble_advertising_whitelist_reply(&m_advertising,
                                               whitelistAddrs,
                                               addrCnt,
                                               whitelistIRKs,
                                               irkCnt);
    APP_ERROR_CHECK(err_code);
  }
  break;

  case BLE_ADV_EVT_PEER_ADDR_REQUEST:
  {
    NRF_LOG_INFO("BLE_ADV_EVT_PEER_ADDR_REQUEST...");

    pm_peer_data_bonding_t peerBondingData;

    // Only Give peer address if we have a handle to the bonded peer.
    if (pmPeerID != PM_PEER_ID_INVALID)
    {

      err_code = pm_peer_data_bonding_load(pmPeerID, &peerBondingData);
      if (err_code != NRF_ERROR_NOT_FOUND)
      {
        APP_ERROR_CHECK(err_code);

        ble_gap_addr_t *peerAddr = &(peerBondingData.peer_ble_id.id_addr_info);
        err_code = ble_advertising_peer_addr_reply(&m_advertising, peerAddr);
        APP_ERROR_CHECK(err_code);
      }
    }
    break;
  }

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
    if (advInit->whitelist)
    {
      init.config.ble_adv_whitelist_enabled = true;
    }
  }
  else if (advInit->slow)
  {
    init.config.ble_adv_slow_enabled = true;
    init.config.ble_adv_slow_interval = advInit->intervalAdv;
    init.config.ble_adv_slow_timeout = advInit->timeoutAdv;
    if (advInit->whitelist)
    {
      init.config.ble_adv_whitelist_enabled = true;
    }
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

    memset(pmWhitelistPeers, PM_PEER_ID_INVALID, sizeof(pmWhitelistPeers));
    pmWhitelistPeerCnt = (sizeof(pmWhitelistPeers) / sizeof(pm_peer_id_t));

    _peerListGet(pmWhitelistPeers, &pmWhitelistPeerCnt);

    ret_code_t err_code =
        pm_whitelist_set(pmWhitelistPeers, pmWhitelistPeerCnt);
    APP_ERROR_CHECK(err_code);

    // Setup the device identies list.
    // Some SoftDevices do not support this feature.
    err_code =
        pm_device_identities_list_set(pmWhitelistPeers, pmWhitelistPeerCnt);
    NRF_LOG_INFO("pm_device_identities_list_set :");

    if (err_code != NRF_ERROR_NOT_SUPPORTED)
    {
      APP_ERROR_CHECK(err_code);
    }

    err_code =
        ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
  }

  if (m_advertising.adv_modes_config.ble_adv_slow_enabled)
  {

    memset(pmWhitelistPeers, PM_PEER_ID_INVALID, sizeof(pmWhitelistPeers));
    pmWhitelistPeerCnt = (sizeof(pmWhitelistPeers) / sizeof(pm_peer_id_t));

    _peerListGet(pmWhitelistPeers, &pmWhitelistPeerCnt);

    ret_code_t err_code =
        pm_whitelist_set(pmWhitelistPeers, pmWhitelistPeerCnt);
    APP_ERROR_CHECK(err_code);

    // Setup the device identies list.
    // Some SoftDevices do not support this feature.
    err_code =
        pm_device_identities_list_set(pmWhitelistPeers, pmWhitelistPeerCnt);
    if (err_code != NRF_ERROR_NOT_SUPPORTED)
    {
      APP_ERROR_CHECK(err_code);
    }

    err_code =
        ble_advertising_start(&m_advertising, BLE_ADV_MODE_SLOW);
    APP_ERROR_CHECK(err_code);
  }
}

/********************************************************************************/
/*   --- END of BLE ADVERTISING FUCTIONS ---                                    */
/********************************************************************************/
