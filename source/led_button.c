#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "ble_advertising.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "bsp_btn_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_log.h"
#include "nrf_crypto.h"
#include "led_button.h"
#include "le_secure.h"
#include "pm.h"
#include "app.h"
#include "adv.h"


void
buttonsLedsInit ()
{
  ret_code_t err_code;
  bsp_event_t startup_event;

  err_code = bsp_init (BSP_INIT_LED | BSP_INIT_BUTTONS, bspEventHandler);
  APP_ERROR_CHECK (err_code);

  err_code = bsp_btn_ble_init (NULL, &startup_event);
  APP_ERROR_CHECK (err_code);

}



void
bspEventHandler (bsp_event_t event)
{
  //ret_code_t err_code;
  switch (event)
    {
    case BSP_EVENT_KEY_0:
      /*
       * Event to do when Button 1 is pressed
       */
      //sd_ble_gap_adv_stop();
      //sd_ble_gap_disconnect(m_conn_handle,BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      //_peerManagerInit2();
      //_advertisingStart();

      break;

    case BSP_EVENT_KEY_1:
      /*
       * Event to do when Button 2 is pressed
       */
      //sd_ble_gap_adv_stop();
      //sd_ble_gap_disconnect(m_conn_handle,BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      //_cryptoInit();
      //_peerManagerInit1();
      //_deleteBonds();
      //_advertisingStart();
      break;

    case BSP_EVENT_KEY_2:
      /*
       * Event to do when Button 3 is pressed
       */
      onNumCompButtonPress (true);
      break;

    case BSP_EVENT_KEY_3:
      /*
       * Event to do when Button 2 is pressed
       */
      onNumCompButtonPress (false);
      break;
    default:
      break;
    }
}
