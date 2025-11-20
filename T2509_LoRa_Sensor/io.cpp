#include <Arduino.h>
#include <Wire.h>
#include "atask.h"
#include "main.h"
#include "io.h"

typedef struct
{
  uint8_t pattern_bit;
  uint8_t switches;
  uint8_t tindx;
} io_ctrl_st;

io_ctrl_st io_ctrl;


const uint8_t sw_pin[8] = 
  {PIN_SW1, PIN_SW2, PIN_SW3, PIN_SW4, PIN_SW5, PIN_SW6, PIN_ENABLE_WATCHDOG, PIN_ENABLE_TEST_MODE};

void io_initialize(void)
{
  analogReadResolution(12);
  pinMode(PIN_PIR, INPUT_PULLUP);
  for (uint8_t i = 0; i < 8; i++) pinMode(sw_pin[i], INPUT_PULLUP);

  //RFM95 Reset
  pinMode(PIN_RFM_RESET, OUTPUT);
  digitalWrite(PIN_RFM_RESET, HIGH);
  pinMode(PIN_RFM_CS, OUTPUT);
  digitalWrite(PIN_RFM_CS, HIGH);
  
  pinMode(PIN_EN_VSYSX, OUTPUT);
  pinMode(PIN_LDR, INPUT);

  pinMode(PIN_WD_BEATBACK,INPUT);
  pinMode(PIN_WD_HEARTBEAT,OUTPUT);

  pinMode(PIN_EN_3V3X, OUTPUT);
  pinMode(PIN_RST_412, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT); // Set onboard LED pin as output

  //digitalWrite(PIN_EN_VSYSX, LOW);  // Low == enable
  digitalWrite(PIN_EN_3V3X, LOW);   // Low == enable
  digitalWrite(PIN_RST_412, HIGH);  // High == reset ATTiny412


  io_pwr_sensor(true);
  Wire.setSDA(PIN_I2C0_SDA);
  Wire.setSCL(PIN_I2C0_SCL);
  //Wire1.setSDA(PIN_I2C1_SDA);
  //Wire1.setSCL(PIN_I2C1_SCL);

//   analogWriteFreq(1000);
//   analogWriteRange(1023);
//   analogWrite(PIN_EN_VSYSX, 500);

}

uint8_t io_get_addr(void)
{
    io_ctrl.switches = 0b00000000;
    #if BOARD == BOARD_LORA_SENSOR
    delay(20);
    for (uint8_t i=0; i < 6; i++)
    {
        if (digitalRead(sw_pin[i]) == LOW) io_ctrl.switches |= (1 << i);
    }     
    #endif
    return io_ctrl.switches;
}

bool io_get_debug_mode(void)
{
    return (digitalRead(PIN_ENABLE_TEST_MODE) == LOW);
}

bool io_get_watchdog(void)
{
    return (digitalRead(PIN_ENABLE_WATCHDOG) == LOW);
}


void io_task(void)
{
}

void io_set_onboard_led(bool is_on)
{
    if(is_on) digitalWrite(LED_BUILTIN, HIGH);
    else digitalWrite(LED_BUILTIN, LOW);
}
void io_pwr_sensor(bool pwr_on)
{
    // Can also be controlled by the ATTiny412 module
    if(pwr_on) digitalWrite(PIN_EN_3V3X, LOW);
    else digitalWrite(PIN_EN_3V3X, HIGH);
}
void io_pwr_vsysx(bool pwr_on)
{
    if(pwr_on) digitalWrite(PIN_EN_VSYSX, LOW);
    else digitalWrite(PIN_EN_VSYSX, HIGH);
}

void io_reset_412(uint8_t value)
{
    digitalWrite(PIN_RST_412, value);
}

void io_set_wd_heartbeat(uint8_t value)
{
    digitalWrite(PIN_WD_HEARTBEAT, value);
}

uint8_t io_get_wd_beatback(void)
{
    return digitalRead(PIN_WD_BEATBACK);
}

bool io_pir_detected(void)
{
    return (digitalRead(PIN_PIR) == 1);
}

uint16_t io_read_ldr(void)
{
    return analogRead(PIN_LDR);
}

