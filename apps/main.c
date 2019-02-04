#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "ble_advertising.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "nrf_ble_gatt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_crypto.h"
#include "nrf_crypto_keys.h"
#include "peer_manager.h"
#include "app.h"
#include "ble_init.h"
#include "services.h"
#include "adv.h"
#include "pm.h"
#include "counter.h"

#if LE_SEC_CONN
#include "le_secure.h"
#endif

/************************************/
/*First Service                     */
/************************************/
// instance of "ble_s_X" service
BLE_SERVICE_DEF(ble_s_X);

void servicesInitApp_X(void)
{
  ret_code_t err_code;

  service_uuid_t service_x;
  attr_char_t attr_char_x[MAX_CHARACTERISTIC_X];

  service_char_t char_x[MAX_CHARACTERISTIC_X];

  attr_perm_t perm[MAX_CHARACTERISTIC_X];
  prop_attr_t prop[MAX_CHARACTERISTIC_X];

  service_x.uuid_handle.uuid = SERVICE_UUID_SERVICE_X;
  service_x.service_id = 0x0001;
  char_x[0].char_id = SERVICE_X_CHAR_A_ID;
  char_x[0].uuid16 = SERVICE_X_UUID_CHAR_A;

  // To test the protection of the read and write attributes of the service
  perm[0].perm_read = CONN_SEC_MODE_SET_ENC_WITH_MITM; //Attribute require LESC encryption and MITM protection in read mode
  perm[0].perm_write = CONN_SEC_MODE_SET_NO_ACCESS;    //Attribute require no protection, no access link in write mode

  prop[0].read = 1;
  if (prop[0].read == 1)
  {
    uint8_t value_A[10] =
        {0x01, 0x02, 0x01, 0x01, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    memcpy(attr_char_x[0].attr_char_a, value_A, sizeof(value_A));
  }
  prop[0].write = 0;
  prop[0].notify = 0;

  /*################################################### */

  char_x[1].char_id = SERVICE_X_CHAR_B_ID;
  char_x[1].uuid16 = SERVICE_X_UUID_CHAR_B;

  perm[1].perm_read = CONN_SEC_MODE_SET_NO_ACCESS;      //Attribute require no protection, no access link in read mode
  perm[1].perm_write = CONN_SEC_MODE_SET_ENC_WITH_MITM; //Attribute require LESC encryption and MITM protection in write mode

  prop[1].read = 0;
  if (prop[0].read == 1)
  {
    uint8_t value_B[10] =
        {0x02, 0x01, 0x02, 0x02, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    memcpy(attr_char_x[1].attr_char_a, value_B, sizeof(value_B));
  }
  prop[1].write = 1;
  prop[1].notify = 0;

  err_code =
      bleServiceInit(&ble_s_X, service_x, char_x, prop, perm, attr_char_x);
  APP_ERROR_CHECK(err_code);
}

/************************************/
/*Second Service                    */
/************************************/
// instance of "ble_s_Y" service
BLE_SERVICE_DEF(ble_s_Y);

void servicesInitApp_Y(void)
{
  ret_code_t err_code;
  service_uuid_t service_y;
  attr_char_t attr_char_y[MAX_CHARACTERISTIC_Y];
  service_char_t char_y[MAX_CHARACTERISTIC_Y];
  attr_perm_t perm[MAX_CHARACTERISTIC_Y];
  prop_attr_t prop[MAX_CHARACTERISTIC_Y];

  service_y.uuid_handle.uuid = SERVICE_UUID_SERVICE_Y;
  service_y.service_id = 0x0002;

  char_y[0].char_id = SERVICE_Y_CHAR_A_ID;
  char_y[0].uuid16 = SERVICE_Y_UUID_CHAR_A;

  perm[0].perm_read = CONN_SEC_MODE_SET_ENC_WITH_MITM;
  perm[0].perm_write = CONN_SEC_MODE_SET_NO_ACCESS;

  prop[0].read = 1;
  if (prop[0].read == 1)
  {
    uint8_t value_C[10] =
        {0x00, 0x01, 0x01, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    memcpy(attr_char_y[0].attr_char_a, value_C, sizeof(value_C));
  }
  prop[0].write = 0;
  prop[0].notify = 0;

  /*################################################### */

  char_y[1].char_id = SERVICE_Y_CHAR_B_ID;
  char_y[1].uuid16 = SERVICE_Y_UUID_CHAR_B;

  perm[1].perm_read = CONN_SEC_MODE_SET_NO_ACCESS;
  perm[1].perm_write = CONN_SEC_MODE_SET_ENC_WITH_MITM;

  prop[1].read = 0;
  if (prop[1].read == 1)
  {
    uint8_t value_C[10] =
        {0x00, 0x01, 0x01, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    memcpy(attr_char_y[0].attr_char_a, value_C, sizeof(value_C));
  }
  prop[1].write = 1;
  prop[1].notify = 0;

  err_code =
      bleServiceInit(&ble_s_Y, service_y, char_y, prop, perm, attr_char_y);
  APP_ERROR_CHECK(err_code);
}

void advInitApp()
{

  ble_adv_init config_adv;

  config_adv.name_type = BLE_ADVDATA_FULL_NAME;
  config_adv.appearance = true;
  config_adv.fast = true;
  config_adv.interval_adv = 300;
  config_adv.timeout_adv = 50;
  config_adv.uuid_cnt = 0;
  config_adv.p_uuids = NULL;

  advertisingInit(&config_adv);
}

int main(void)
{

  // Initialize.
  bleInit();
  advInitApp();
  counter_init();

#if LE_SEC_CONN
  counter_start();
  cryptoInit();
  leSecureConn();
  counter_stop();
  uint32_t time_elapsed = counter_get();
  NRF_LOG_INFO("Time for LE Sec to take place : %u.%.2u seconds elapsed.", (time_elapsed / 1000), (time_elapsed % 1000));
#endif

#if LE_LEGACY
  legacyPairing();
#endif

  servicesInitApp_X();
  servicesInitApp_Y();

  advertisingStart();

  NRF_LOG_INFO("BLE Peripheral for nRF52 :");

  // Enter main loop.
  for (;;)
  {

#if LE_SEC_CONN
    serviceDhkeyRequests();
#endif

    if (NRF_LOG_PROCESS() == false)
    {
      powerManage();
    }
  }
}
