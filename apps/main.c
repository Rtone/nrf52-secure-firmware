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

/********************************************************************************/
/*   FUNCTION PROTOTYPES                                                        */
/********************************************************************************/

void servicesInitApp_X(void);

void servicesInitApp_Y(void);

void servicesInitApp_Z(void);

void advInitApp();

/********************************************************************************/
/*   --- END of FUNCTION PROTOTYPES ---                                         */
/********************************************************************************/

/********************************************************************************/
/*   FIRST SERVICE                                                              */
/********************************************************************************/

// Instance of "ble_s_X" service
BLE_SERVICE_DEF(ble_s_X);

void servicesInitApp_X(void)
{
  ret_code_t err_code;

  service_uuid_t service_X;
  attr_char_t attrChar_X[MAX_CHARACTERISTIC_X];

  service_char_t char_X[MAX_CHARACTERISTIC_X];

  attr_perm_t perm[MAX_CHARACTERISTIC_X];
  prop_attr_t prop[MAX_CHARACTERISTIC_X];

  service_X.uuidHandle.uuid = SERVICE_UUID_SERVICE_X;
  service_X.serviceID = SERVICE_X_ID;
  char_X[0].charID = SERVICE_X_CHAR_A_ID;
  char_X[0].uuid16 = SERVICE_X_UUID_CHAR_A;

  // To test the protection of the read and write attributes of the service
  perm[0].permRead = CONN_SEC_MODE_SET_ENC_WITH_MITM; //Attribute require LESC encryption and MITM protection in read mode
  perm[0].permWrite = CONN_SEC_MODE_SET_NO_ACCESS;    //Attribute require no protection, no access link in write mode

  prop[0].read = 1;
  if (prop[0].read == 1)
  {
    uint8_t valueRead[10] =
        {0x01, 0x02, 0x01, 0x01, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    memcpy(attrChar_X[0].attrCharRead, valueRead, sizeof(valueRead));
  }
  prop[0].write = 0;
  if (prop[0].write == 1)
  {
    uint8_t valueWrite[4] =
        {0x00, 0x00, 0x00, 0x00};
    memcpy(attrChar_X[0].attrCharWrite, valueWrite, sizeof(valueWrite));
  }
  prop[0].notify = 0;

  /*###########################################################*/

  char_X[1].charID = SERVICE_X_CHAR_B_ID;
  char_X[1].uuid16 = SERVICE_X_UUID_CHAR_B;

  perm[1].permRead = CONN_SEC_MODE_SET_NO_ACCESS;      //Attribute require no protection, no access link in read mode
  perm[1].permWrite = CONN_SEC_MODE_SET_ENC_WITH_MITM; //Attribute require LESC encryption and MITM protection in write mode

  prop[1].read = 0;
  if (prop[0].read == 1)
  {
    uint8_t valueRead[10] =
        {0x02, 0x01, 0x02, 0x02, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    memcpy(attrChar_X[1].attrCharRead, valueRead, sizeof(valueRead));
  }
  prop[1].write = 1;
  if (prop[1].write == 1)
  {
    uint8_t valueWrite[4] =
        {0x00, 0x00, 0x00, 0x00};
    memcpy(attrChar_X[1].attrCharWrite, valueWrite, sizeof(valueWrite));
  }
  prop[1].notify = 0;

  err_code =
      bleServiceInit(&ble_s_X, service_X, char_X, prop, perm, attrChar_X);
  APP_ERROR_CHECK(err_code);
}

/********************************************************************************/
/*   SECOND SERVICE                                                             */
/********************************************************************************/

// Instance of "ble_s_Y" service
BLE_SERVICE_DEF(ble_s_Y);

void servicesInitApp_Y(void)
{
  ret_code_t err_code;
  service_uuid_t service_Y;
  attr_char_t attrChar_Y[MAX_CHARACTERISTIC_Y];
  service_char_t char_Y[MAX_CHARACTERISTIC_Y];
  attr_perm_t perm[MAX_CHARACTERISTIC_Y];
  prop_attr_t prop[MAX_CHARACTERISTIC_Y];

  service_Y.uuidHandle.uuid = SERVICE_UUID_SERVICE_Y;
  service_Y.serviceID = SERVICE_Y_ID;

  char_Y[0].charID = SERVICE_Y_CHAR_A_ID;
  char_Y[0].uuid16 = SERVICE_Y_UUID_CHAR_A;

  perm[0].permRead = CONN_SEC_MODE_SET_ENC_WITH_MITM;
  perm[0].permWrite = CONN_SEC_MODE_SET_NO_ACCESS;

  prop[0].read = 1;
  if (prop[0].read == 1)
  {
    uint8_t valueRead[10] =
        {0x00, 0x01, 0x01, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    memcpy(attrChar_Y[0].attrCharRead, valueRead, sizeof(valueRead));
  }
  prop[0].write = 0;
  if (prop[0].write == 1)
  {
    uint8_t valueWrite[4] =
        {0x00, 0x00, 0x00, 0x00};
    memcpy(attrChar_Y[0].attrCharWrite, valueWrite, sizeof(valueWrite));
  }
  prop[0].notify = 0;

  /*###########################################################*/

  char_Y[1].charID = SERVICE_Y_CHAR_B_ID;
  char_Y[1].uuid16 = SERVICE_Y_UUID_CHAR_B;

  perm[1].permRead = CONN_SEC_MODE_SET_NO_ACCESS;
  perm[1].permWrite = CONN_SEC_MODE_SET_ENC_WITH_MITM;

  prop[1].read = 0;
  if (prop[1].read == 1)
  {
    uint8_t valueRead[10] =
        {0x00, 0x01, 0x01, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    memcpy(attrChar_Y[0].attrCharRead, valueRead, sizeof(valueRead));
  }
  prop[1].write = 1;
  if (prop[1].write == 1)
  {
    uint8_t valueWrite[4] =
        {0x00, 0x00, 0x00, 0x00};
    memcpy(attrChar_Y[1].attrCharWrite, valueWrite, sizeof(valueWrite));
  }
  prop[1].notify = 0;

  err_code =
      bleServiceInit(&ble_s_Y, service_Y, char_Y, prop, perm, attrChar_Y);
  APP_ERROR_CHECK(err_code);
}

/********************************************************************************/
/*   THIRD SERVICE                                                              */
/********************************************************************************/

// instance of "ble_s_Z" service
BLE_SERVICE_DEF(ble_s_Z);

void servicesInitApp_Z(void)
{
  ret_code_t err_code;
  service_uuid_t service_Z;
  attr_char_t attrChar_Z[MAX_CHARACTERISTIC_Z];
  service_char_t char_Z[MAX_CHARACTERISTIC_Z];
  attr_perm_t perm[MAX_CHARACTERISTIC_Z];
  prop_attr_t prop[MAX_CHARACTERISTIC_Z];

  service_Z.uuidHandle.uuid = SERVICE_UUID_SERVICE_Z;
  service_Z.serviceID = SERVICE_Z_ID;

  char_Z[0].charID = SERVICE_Z_CHAR_A_ID;
  char_Z[0].uuid16 = SERVICE_Z_UUID_CHAR_A;

  perm[0].permRead = CONN_SEC_MODE_SET_ENC_WITH_MITM;
  perm[0].permWrite = CONN_SEC_MODE_SET_ENC_WITH_MITM;

  prop[0].read = 1;
  if (prop[0].read == 1)
  {
    uint8_t valueRead[10] =
        {0x00, 0x01, 0x01, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    memcpy(attrChar_Z[0].attrCharRead, valueRead, sizeof(valueRead));
  }
  prop[0].write = 1;
  if (prop[0].write == 1)
  {
    uint8_t valueWrite[4] =
        {0x00, 0x00, 0x00, 0x00};
    memcpy(attrChar_Z[0].attrCharWrite, valueWrite, sizeof(valueWrite));
  }
  prop[0].notify = 1;

  err_code =
      bleServiceInit(&ble_s_Y, service_Z, char_Z, prop, perm, attrChar_Z);
  APP_ERROR_CHECK(err_code);
}

void advInitApp()
{

  ble_adv_init configAdv;

  configAdv.nameType = BLE_ADVDATA_FULL_NAME;
  configAdv.appearance = true;
  configAdv.fast = true;
  configAdv.intervalAdv = 300;
  configAdv.timeoutAdv = 50;
  configAdv.uuid_cnt = 0;
  configAdv.p_uuids = NULL;

  advertisingInit(&configAdv);
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
  uint32_t timeElapsed = counter_get();
  NRF_LOG_INFO("Time for LE Sec to take place : %u.%.2u seconds elapsed.", (timeElapsed / 1000), (timeElapsed % 1000));
#endif

#if LE_LEGACY
  legacyPairing();
#endif

  servicesInitApp_X();
  servicesInitApp_Y();
  servicesInitApp_Z();

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
