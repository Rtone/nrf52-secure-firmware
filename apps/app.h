#ifndef __GLOBAL_H
#define __GLOBAL_H

#define ADVERTISING_LED                 BSP_BOARD_LED_0
#define CONNECTED_LED                   BSP_BOARD_LED_1
#define LEDBUTTON_BUTTON                BSP_BUTTON_0

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2

#define DEVICE_NAME                     "nrf52-BLE-Secure-Firmware"
#define MANUFACTURER_NAME               "Rtone-Security"
#define APP_ADV_INTERVAL                300
#define APP_ADV_TIMEOUT_IN_SECONDS      30

#define APP_BLE_OBSERVER_PRIO           3
#define APP_BLE_CONN_CFG_TAG            1

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)
#define SLAVE_LATENCY                   0
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(800, UNIT_10_MS)

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)
#define MAX_CONN_PARAMS_UPDATE_COUNT    3

#define SEC_PARAM_TIME_OUT			   30
#define SEC_PARAM_BOND                  1
#define SEC_PARAM_MITM                  1
#define SEC_PARAM_LESC                  1
#define SEC_PARAM_KEYPRESS              0
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_DISPLAY_ONLY
#define SEC_PARAM_OOB                   0
#define SEC_PARAM_MIN_KEY_SIZE          7
#define SEC_PARAM_MAX_KEY_SIZE          16

#ifndef LE_SEC_CONN
#define LE_SEC_CONN 1
#endif

#ifndef LE_LEGACY
#define LE_LEGACY 0
#endif


#define SECURITY_REQUEST_DELAY          APP_TIMER_TICKS(1500)

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)

#define STATIC_PASSKEY                  "123456"				    /**< Static pin. */

#define DEAD_BEEF                       0xDEADBEEF

NRF_BLE_GATT_DEF (m_gatt);
APP_TIMER_DEF (m_sec_req_timer_id);
uint16_t m_conn_handle;
ble_advertising_t m_advertising;


#endif /* __GLOBAL_H */
