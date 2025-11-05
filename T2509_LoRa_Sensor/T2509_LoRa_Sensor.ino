
#include <SPI.h>
#include <Wire.h>
#include <RH_RF95.h>
#include "rfm.h"
#include "main.h"
#include "io.h"
#include "alpha.h"
#include "sensor.h"



main_ctrl_st main_ctrl = {
  .test_activated = false,
  .io_initialized = false,
  .debug_mode = false,
  .watchdog = false 
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
  
  while (!Serial) ; // Wait for serial port to be available
  Serial.print(APP_NAME); Serial.print(" Compiled: ");
  Serial.print(__DATE__); Serial.print(" ");
  Serial.print(__TIME__); Serial.println();
  // while(true){delay(1);}

  Serial.printf("Address:    %d \n", main_ctrl.node_addr);
  Serial.printf("Debug Mode: %s\n", (main_ctrl.debug_mode) ? "Activated" : "Not activated");
  Serial.printf("Watchdog:   %s\n", (main_ctrl.watchdog) ? "Activated" : "Not activated");

  Serial1.println("Serial1");
  if(main_ctrl.test_activated) Serial.println("Test Mode is Activated");
  alpha_initialize();
  rfm_initialize(); 
  rfm_task_initilaize();
  atask_add_new(&debug_print_handle);
  sensor_initialize();
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

