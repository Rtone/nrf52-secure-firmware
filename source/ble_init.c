#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "bsp_btn_ble.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_crypto.h"
#include "le_secure.h"
#include "ble_init.h"
#include "led_button.h"
#include "app.h"
#include "conn.h"
#include "adv.h"

char *rolesStr[] = {
    "INVALID_ROLE",
    "CENTRAL",
    "PERIPHERAL",
};

/********************************************************************************/
/*   FUNCTION PROTOTYPES                                                        */
/********************************************************************************/

static void
_secReqTimeoutHandler(void *p_context);

static uint8_t
_randomVectorGenerate(uint8_t *pBuff, uint8_t size);

static void
_bleEvtHandler(ble_evt_t const *p_ble_evt, void *p_context);

/********************************************************************************/
/*   --- END of FUNCTION PROTOTYPES ---                                         */
/********************************************************************************/

/********************************************************************************/
/*   HANDLERS                                                                   */
/********************************************************************************/

static void
_secReqTimeoutHandler(void *p_context)
{
  uint32_t err_code;
  pm_conn_sec_status_t statusEncryption;

  if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
  {
    err_code = pm_conn_sec_status_get(m_conn_handle, &statusEncryption);
    APP_ERROR_CHECK(err_code);

    // If the link is still not secured by the peer, initiate security procedure.
    if (!statusEncryption.encrypted)
    {
      NRF_LOG_INFO("Start encryption.");
      err_code = pm_conn_secure(m_conn_handle, false);
      APP_ERROR_CHECK(err_code);
    }
  }
}

static void
_bleEvtHandler(ble_evt_t const *p_ble_evt, void *p_context)
{
  uint16_t conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
  ret_code_t err_code = NRF_SUCCESS;
  pm_conn_sec_status_t statusEncryption;
  char passkey[BLE_GAP_PASSKEY_LEN + 1];
  uint16_t role = ble_conn_state_role(conn_handle);

  switch (p_ble_evt->header.evt_id)
  {
  case BLE_GAP_EVT_DISCONNECTED:
    NRF_LOG_INFO("Disconnected...");
    bsp_board_led_off(CONNECTED_LED);
    m_conn_handle = BLE_CONN_HANDLE_INVALID;
    err_code = app_button_disable();
    APP_ERROR_CHECK(err_code);
    advertisingStart();
    //_buttonsLedsInit();
    //_macChange();
    //_deleteBonds();
    //_advertisingStart();

    break;

  case BLE_GAP_EVT_CONNECTED:
    NRF_LOG_INFO("Connected...");
    err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
    APP_ERROR_CHECK(err_code);

    bsp_board_led_on(CONNECTED_LED);
    bsp_board_led_off(ADVERTISING_LED);

    //err_code = ble_db_discovery_start(&m_ble_db_discovery,p_ble_evt->evt.gap_evt.conn_handle);
    err_code = app_button_enable();

    APP_ERROR_CHECK(err_code);

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
      err_code = pm_conn_sec_status_get(m_conn_handle, &statusEncryption);
      APP_ERROR_CHECK(err_code);
      NRF_LOG_INFO("Status %d", statusEncryption.encrypted);
      // If the link is still not secured by the peer, initiate security procedure.
      if (!statusEncryption.encrypted)
      {
        NRF_LOG_INFO("Start encryption...");
        err_code = pm_conn_secure(m_conn_handle, false);
        APP_ERROR_CHECK(err_code);
      }
    }

    m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    err_code = app_button_enable();

    break;
  case BLE_GAP_EVT_PASSKEY_DISPLAY:
    memcpy(passkey, p_ble_evt->evt.gap_evt.params.passkey_display.passkey,
           BLE_GAP_PASSKEY_LEN);
    passkey[BLE_GAP_PASSKEY_LEN] = 0x00;
    NRF_LOG_INFO("%s: BLE_GAP_EVT_PASSKEY_DISPLAY: passkey=%s match_req=%d",
                 nrf_log_push(rolesStr[role]), nrf_log_push(passkey),
                 p_ble_evt->evt.gap_evt.params.passkey_display.match_request);

    if (p_ble_evt->evt.gap_evt.params.passkey_display.match_request)
    {
      onMatchRequest(conn_handle, role);
    }
    break;

  case BLE_GAP_EVT_AUTH_KEY_REQUEST:
    NRF_LOG_INFO("%s: BLE_GAP_EVT_AUTH_KEY_REQUEST",
                 nrf_log_push(rolesStr[role]));
    break;

  case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
    NRF_LOG_INFO("%s: BLE_GAP_EVT_LESC_DHKEY_REQUEST",
                 nrf_log_push(rolesStr[role]));
    onDhkeyRequest(conn_handle,
                   &p_ble_evt->evt.gap_evt.params.lesc_dhkey_request,
                   role);
    break;

  case BLE_GAP_EVT_AUTH_STATUS:
    NRF_LOG_INFO("%s: BLE_GAP_EVT_AUTH_STATUS: status=0x%x bond=0x%x lv4: %d kdist_own:0x%x kdist_peer:0x%x",
                 nrf_log_push(rolesStr[role]),
                 p_ble_evt->evt.gap_evt.params.auth_status.auth_status,
                 p_ble_evt->evt.gap_evt.params.auth_status.bonded,
                 p_ble_evt->evt.gap_evt.params.auth_status.sm1_levels.lv4,
                 *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_own),
                 *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_peer));
    break;

#ifndef S140
  case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
  {
    NRF_LOG_DEBUG("PHY update request.");
    ble_gap_phys_t const phys = {
        .rx_phys = BLE_GAP_PHY_AUTO,
        .tx_phys = BLE_GAP_PHY_AUTO,
    };
    err_code =
        sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
    APP_ERROR_CHECK(err_code);
  }
  break;
#endif

  case BLE_GATTC_EVT_TIMEOUT:
    // Disconnect on GATT Client timeout event.
    NRF_LOG_DEBUG("GATT Client Timeout.");
    err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                     BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_GATTS_EVT_TIMEOUT:
    // Disconnect on GATT Server timeout event.
    NRF_LOG_DEBUG("GATT Server Timeout.");
    err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                     BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_EVT_USER_MEM_REQUEST:
    err_code =
        sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
  {
    ble_gatts_evt_rw_authorize_request_t req;
    ble_gatts_rw_authorize_reply_params_t auth_reply;

    req = p_ble_evt->evt.gatts_evt.params.authorize_request;

    if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
    {
      if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ) ||
          (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
          (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
      {
        if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
        {
          auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
        }
        else
        {
          auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
        }
        auth_reply.params.write.gatt_status =
            APP_FEATURE_NOT_SUPPORTED;
        err_code =
            sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
        APP_ERROR_CHECK(err_code);
      }
    }
  }
  break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

  default:
    // No implementation needed.
    break;
  }
}

/********************************************************************************/
/*   --- END of HANDLERS ---                                                    */
/********************************************************************************/

/********************************************************************************/
/*   FUNCTIONS to INITIALIZE BLE STACK                                          */
/********************************************************************************/

void logInit(void)
{
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
}

void timersInit(void)
{
  // Initialize timer module.
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);

  // Create security request timer.
  err_code = app_timer_create(&m_sec_req_timer_id,
                              APP_TIMER_MODE_SINGLE_SHOT,
                              _secReqTimeoutHandler);
  APP_ERROR_CHECK(err_code);
}

static uint8_t
_randomVectorGenerate(uint8_t *pBuff, uint8_t size)
{

  uint8_t bytesAvailable;
  do
  {
    sd_rand_application_bytes_available_get(&bytesAvailable);
  } while (bytesAvailable < size);

  ret_code_t err_code = sd_rand_application_vector_get(pBuff, size);
  APP_ERROR_CHECK(err_code);
  return size;
}

void macChange(void)
{

  ble_gap_addr_t gapAddr;

  /*
  ** Mac Address type 
  ** Choose between :
  ** 1] BLE_GAP_ADDR_TYPE_PUBLIC
  ** 2] BLE_GAP_ADDR_TYPE_RANDOM_STATIC
  ** 3] BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE
  ** 4] BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE
  ** adapted to your use case
  */
  gapAddr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
  ble_gap_addr_t macAddress;
  uint8_t randomBuff[2];
  uint16_t combined = 0;

  _randomVectorGenerate(randomBuff, 2);
  combined = (randomBuff[0] << 8) | (randomBuff[1] = 0xFF);
  srand(combined);
  for (uint8_t i = 0; i < 5; i++)
  {
    /*
    ** For a random MAC Address
    */
    //macAddress.addr[i] = rand()%100 ^rand()%100;

    /*
    ** For a specific MAC Address
    */
    macAddress.addr[i] = 0x11;
  }
  memcpy(&gapAddr.addr, macAddress.addr, sizeof(gapAddr.addr));
  gapAddr.addr[5] |= 0x11;
  sd_ble_gap_addr_set(&gapAddr);
}

void powerManage(void)
{
  ret_code_t err_code = sd_app_evt_wait();
  APP_ERROR_CHECK(err_code);
}

void gapParamsInit(void)
{
  ret_code_t err_code;
  ble_gap_conn_params_t gapConnParams;
  ble_gap_conn_sec_mode_t secMode;

  ble_gap_adv_params_t adv_params;
  memset(&adv_params, 0, sizeof(adv_params));
  adv_params.type = BLE_GAP_ADV_TYPE_ADV_IND;
  adv_params.p_peer_addr = NULL;
  adv_params.interval = 64;
  adv_params.timeout = 180;

  /*
  ** Security Mode 1
  ** Level 1: No security is needed
  ** BLE_GAP_CONN_SEC_MODE_SET_OPEN(&secMode);
  **  
  ** Level 2: Encrypted link required, MITM protection not necessary
  ** BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&secMode);
  ** 
  ** Level 3: Encrypted link required with MITM protection 
  ** BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(&secMode); 
  **
  ** Level 4: LESC encryption link required with MITM protection 
  ** BLE_GAP_CONN_SEC_MODE_SET_LESC_ENC_WITH_MITM(&secMode); 
  **
  ** Security Mode 2
  ** Level 1: Signing or encryption required, MITM protection not necessary
  ** BLE_GAP_CONN_SEC_MODE_SET_SIGNED_NO_MITM(&secMode);
  **
  ** Level 2: Signing or encryption required with MITM protection
  ** BLE_GAP_CONN_SEC_MODE_SET_SIGNED_WITH_MITM(&secMode);  
  */

  BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(&secMode);

  err_code = sd_ble_gap_device_name_set(&secMode,
                                        (const uint8_t *)DEVICE_NAME,
                                        strlen(DEVICE_NAME));
  APP_ERROR_CHECK(err_code);

  /* YOUR_JOB: Use an appearance value matching the application's use case.
     err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
     APP_ERROR_CHECK(err_code); */

  memset(&gapConnParams, 0, sizeof(gapConnParams));

  gapConnParams.min_conn_interval = MIN_CONN_INTERVAL;
  gapConnParams.max_conn_interval = MAX_CONN_INTERVAL;
  gapConnParams.slave_latency = SLAVE_LATENCY;
  gapConnParams.conn_sup_timeout = CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_ppcp_set(&gapConnParams);
  APP_ERROR_CHECK(err_code);
}

void bleStackInit(void)
{
  ret_code_t err_code;

  err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);

  uint32_t ramStart = 0;
  err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ramStart);
  APP_ERROR_CHECK(err_code);

  // Enable BLE stack.
  err_code = nrf_sdh_ble_enable(&ramStart);
  APP_ERROR_CHECK(err_code);

  // Register a handler for BLE events.
  NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, _bleEvtHandler,
                       NULL);
}

void gattInit(void)
{
  ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
  APP_ERROR_CHECK(err_code);
}

void bleInit()
{
  logInit();
  timersInit();
  bleStackInit();
  macChange();
  gapParamsInit();
  gattInit();
  connParamsInit();
  buttonsLedsInit();
}

/********************************************************************************/
/*   --- END of FUNCTIONS to INITIALIZE BLE STACK ---                           */
/********************************************************************************/
