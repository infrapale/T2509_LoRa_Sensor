
#include <SPI.h>
#include <Wire.h>
#include <RH_RF95.h>
#include "rfm.h"
#include "main.h"
#include "io.h"
#include "alpha.h"
#include "sensor.h"
#include "maestro.h"



main_ctrl_st main_ctrl = {
  .test_activated = false,
  .io_initialized = false,
  .debug_mode = false,
  .watchdog = false,
  .long_range_modulation = false,
  .fast_interval = false,
  .error = {
    .sensor = 0,
    .radio = 0,
    .display = 0,
  }
};
 
void debug_print_task(void) {
  if (main_ctrl.debug_mode) atask_print_status(true);
}

//                                  123456789012345      ival  next  state  prev  cntr flag  call backup
atask_st debug_print_handle      = {"Debug Print    ",   5000, 0,    0,     255,  0,   1, debug_print_task};

TwoWire *Wirep;

void setup() 
{
  Serial.begin(DEBUG_UART_BPS);
  Serial1.setTX(PIN_UART0_TX);   // UART0
  Serial1.setRX(PIN_UART0_RX);
  Serial.begin(9600);
  Serial1.begin(9600);

  Wirep = &Wire;
 
  io_initialize();
  main_ctrl.node_addr = io_get_addr();
  main_ctrl.debug_mode = io_get_debug_mode();
  main_ctrl.watchdog = io_get_watchdog();
  main_ctrl.long_range_modulation = io_get_long_range_modulation();
  main_ctrl.fast_interval = !main_ctrl.long_range_modulation;


  if(io_get_debug_mode()){
    while (!Serial) ; // Wait for serial port to be available
  } else delay(2000);
  Serial.print(APP_NAME); Serial.print(" Compiled: ");
  Serial.print(__DATE__); Serial.print(" ");
  Serial.print(__TIME__); Serial.println();
  
  Serial.printf("Address:    %d \n", main_ctrl.node_addr);
  Serial.printf("Debug Mode: %s\n", (main_ctrl.debug_mode) ? "Activated" : "Not activated");
  Serial.printf("Watchdog:   %s\n", (main_ctrl.watchdog) ? "Activated" : "Not activated");
  Serial.printf("Long Range: %s\n", (main_ctrl.long_range_modulation) ? "Activated" : "Not activated");
 
  if(main_ctrl.test_activated) Serial.println("Test Mode is Activated");
  io_pwr_vsysx(true);
  alpha_initialize();
  rfm_initialize(); 
  rfm_task_initilaize();
  atask_add_new(&debug_print_handle);
  sensor_initialize();
  maestro_initialize();
}

void setup1(void)
{
}

void loop()
{ 
    atask_run();
}

uint32_t io_run_time = millis();
void loop1()
{
    if(millis() > io_run_time)
    {
        io_run_time = millis() + 100;
        // io_task();  // currently empty
    }
}

