#ifdef NRF52_SERIES

#include "WatchdogNRF.h"
#include "Arduino.h"
#include "nrf_wdt.h"

WatchdogNRF::WatchdogNRF() { _wdto = -1; }

int WatchdogNRF::enable(int maxPeriodMS) {
  if (maxPeriodMS < 0)
    return 0;

  // cannot change wdt config register once it is started
  // return previous configured timeout
#if defined(NRFX_CONFIG_API_VER_MAJOR) && NRFX_CONFIG_API_VER_MAJOR >= 3
  if (nrf_wdt_started_check(NRF_WDT))
#else
  if (nrf_wdt_started(NRF_WDT))
#endif
    return _wdto;

  // WDT run when CPU is sleep
#if defined(NRFX_CONFIG_API_VER_MAJOR) && NRFX_CONFIG_API_VER_MAJOR >= 3
  nrf_wdt_behaviour_set(NRF_WDT, NRF_WDT_BEHAVIOUR_RUN_SLEEP_MASK);
#else
  nrf_wdt_behaviour_set(NRF_WDT, NRF_WDT_BEHAVIOUR_RUN_SLEEP);
#endif
  nrf_wdt_reload_value_set(NRF_WDT, (maxPeriodMS * 32768) / 1000);

  // use channel 0
  nrf_wdt_reload_request_enable(NRF_WDT, NRF_WDT_RR0);

  // Start WDT
  // After started CRV, RREN and CONFIG is blocked
  // There is no way to stop/disable watchdog using source code
  // It can only be reset by WDT timeout, Pin reset, Power reset
  nrf_wdt_task_trigger(NRF_WDT, NRF_WDT_TASK_START);

  _wdto = maxPeriodMS;

  return maxPeriodMS;
}

void WatchdogNRF::reset() { nrf_wdt_reload_request_set(NRF_WDT, NRF_WDT_RR0); }

// There is no way to stop/disable watchdog using source code
void WatchdogNRF::disable() {}

int WatchdogNRF::sleep(int maxPeriodMS) {
  if (maxPeriodMS < 0)
    return 0;

#ifdef ARDUINO_NRF52_ADAFRUIT
  // Mimic AVR to use 8 seconds.
  if (maxPeriodMS == 0)
    maxPeriodMS = 8000;

  // Bluefruit freeRTOS tickless implementation will
  // automatically put CPU into low power mode with delay()
  delay(maxPeriodMS);

  return maxPeriodMS;
#else
  return 0;
#endif
}

#endif
