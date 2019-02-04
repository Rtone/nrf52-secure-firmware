#ifndef __SERVICES_H
#define __SERVICES_H

#include <ble.h>
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

#define BLE_SERVICE_DEF(_name)                         \
  static ble_service_t _name;                          \
  NRF_SDH_BLE_OBSERVER(_name##_obs,                    \
                       BLE_SERVICES_BLE_OBSERVER_PRIO, \
                       bleServiceOnBleEvt, &_name)

/************************************/
/*First Service                     */
/************************************/
#define SERVICE_UUID_SERVICE_X 0x1111
#define SERVICE_X_ID 0x0001
#define SERVICE_X_UUID_CHAR_A 0x0111
#define SERVICE_X_CHAR_A_ID 0X0001
#define SERVICE_X_UUID_CHAR_B 0x0222
#define SERVICE_X_CHAR_B_ID 0X0002

/************************************/
/*Second Service                    */
/************************************/
#define SERVICE_UUID_SERVICE_Y 0x2222
#define SERVICE_Y_ID 0x0002
#define SERVICE_Y_UUID_CHAR_A 0x0333
#define SERVICE_Y_CHAR_A_ID 0X0003
#define SERVICE_Y_UUID_CHAR_B 0x0444
#define SERVICE_Y_CHAR_B_ID 0X0004

/************************************/
/*Third Service                    */
/************************************/
#define SERVICE_UUID_SERVICE_Z 0x3333
#define SERVICE_Z_ID 0x0003
#define SERVICE_Z_UUID_CHAR_A 0x0555
#define SERVICE_Z_CHAR_A_ID 0X0005

#define MAX_CHARACTERISTIC_X 2
#define MAX_CHARACTERISTIC_Y 2
#define MAX_CHARACTERISTIC_Z 1

#define SERVICE_UUID_BASE                              \
  {                                                    \
    0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,    \
        0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00 \
  }

#define CONN_SEC_MODE_SET_NO_ACCESS 0x01
#define CONN_SEC_MODE_SET_OPEN 0x02
#define CONN_SEC_MODE_SET_ENC_NO_MITM 0x03
#define CONN_SEC_MODE_SET_ENC_WITH_MITM 0x04
#define CONN_SEC_MODE_SET_LESC_ENC_WITH_MITM 0x05
#define CONN_SEC_MODE_SET_SIGNED_NO_MITM 0x06
#define CONN_SEC_MODE_SET_SIGNED_WITH_MITM 0x07

typedef struct service_uuid_s
{
  const ble_uuid128_t uuid128;
  ble_uuid_t uuid_handle;
  uint16_t service_id;
} service_uuid_t;

typedef struct service_char_s
{
  uint16_t uuid16;
  uint16_t char_id;
} service_char_t;

typedef struct attr_perm_s
{
  uint8_t perm_read;
  uint8_t perm_write;
} attr_perm_t;

typedef struct prop_attr_s
{
  uint8_t read;
  uint8_t write;
  uint8_t notify;
} prop_attr_t;

typedef struct attr_char_s
{
  uint8_t attr_char_read[10];
  uint8_t attr_char_write[10];
} attr_char_t;

typedef struct ble_service_s ble_service_t;
typedef void (*ble_service_char_handler_t)(uint16_t conn_handle,
                                           ble_service_t *p_service,
                                           uint8_t new_state);

typedef struct
{
  ble_service_char_handler_t led_write_handler;
} ble_service_init_t;

struct ble_service_s
{
  uint16_t service_handle;
  uint8_t uuid_type;
  ble_gatts_char_handles_t char_default_handles;
  ble_gatts_char_handles_t char_xa_handles;
  ble_gatts_char_handles_t char_ya_handles;
  ble_gatts_char_handles_t char_notify_handles;
  ble_service_char_handler_t write_handler;
};

void bleServiceOnBleEvt(ble_evt_t const *p_ble_evt, void *p_context);
void ledWriteHandler(uint16_t conn_handle, ble_service_t *p_lbs,
                     uint8_t led_state);
uint32_t bleServiceInit(ble_service_t *p_service, service_uuid_t service_t,
                        service_char_t *char_t, prop_attr_t *prop,
                        attr_perm_t *attr_perm, attr_char_t *attr_char);

#endif /* __SERVICES_H */
