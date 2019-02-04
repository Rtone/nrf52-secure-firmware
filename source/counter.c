#include "counter.h"
#include "nrf_drv_rtc.h"

// RTC driver instance using RTC2.
// RTC0 is used by the SoftDevice, and RTC1 by the app_timer library.
static const nrf_drv_rtc_t m_rtc = NRF_DRV_RTC_INSTANCE (2);

static void
rtc_handler (nrf_drv_rtc_int_type_t int_type) 
{
 
    // Likely a counter overflow.
    APP_ERROR_CHECK (0xFFFFFFFF);
} 

void
counter_init (void) 
{
  ret_code_t err_code;
  
  // Initialize the RTC instance.
  nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
  
  // 1 ms interval.
  config.prescaler = 32;
  err_code = nrf_drv_rtc_init (&m_rtc, &config, rtc_handler);
  APP_ERROR_CHECK (err_code);
  nrf_drv_rtc_tick_disable (&m_rtc);
}

void
counter_start (void)
{
  nrf_drv_rtc_counter_clear (&m_rtc);
  
  // Power on!
  nrf_drv_rtc_enable (&m_rtc);
} 

void
counter_stop (void)
{
  nrf_drv_rtc_disable (&m_rtc);
} 

uint32_t counter_get (void)
{
  return (nrf_drv_rtc_counter_get (&m_rtc));
}


/*
*  @endcond
*/
