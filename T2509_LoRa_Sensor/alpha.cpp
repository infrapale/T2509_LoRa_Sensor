#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "io.h"
#include "alpha.h"
#include "atask.h"


extern TwoWire *Wirep;

void alpha_task(void);

// atask_st:            = {"Label          ", ival, next, state, prev, cntr, run, task_ptr };
atask_st alpha_handle   = {"Alpha Show Task", 100,     0,     0,  255,    0,   1, alpha_task};

alpha_ctrl_st  ctrl = {
  .event = ALPHA_SHOW_UNDEFINED,
  .default_event = 0,
  .fval = 0.0,
  .ival = 42,
  .str  = "ABCD",
  .timeout = 0,
  .new_timeout = 0
};

    uint16_t        event;
    uint16_t        default_event;
    float           fval;
    uint16_t        ival;
    char            str[SHOW_MAX_STR_LEN];
    uint32_t        timeout;
    uint32_t        new_timeout;


Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
char displaybuffer[4] = {' ', ' ', ' ', ' '};

void alpha_initialize(void) { 
  alpha4.begin(0x70, Wirep );  // pass in the address
  // alpha4.writeDigitRaw(3, 0x0);
  // alpha4.writeDigitRaw(0, 0xFFFF);
  // alpha4.writeDisplay();
  // delay(200);
  // alpha4.writeDigitRaw(0, 0x0);
  // alpha4.writeDigitRaw(1, 0xFFFF);
  // alpha4.writeDisplay();
  // delay(200);
  // alpha4.writeDigitRaw(1, 0x0);
  // alpha4.writeDigitRaw(2, 0xFFFF);
  // alpha4.writeDisplay();
  // delay(200);
  // alpha4.writeDigitRaw(2, 0x0);
  // alpha4.writeDigitRaw(3, 0xFFFF);
  // alpha4.writeDisplay();
  // delay(200);

  ctrl.timeout = millis() + 5000;
  atask_add_new(&alpha_handle);  

  alpha_show_str4_event("LoRa",5000,false);

//   alpha4.clear();
//   alpha4.writeDisplay();
//   alpha_show_str("LoRa");
//   delay(2000);
//   alpha_show_float(12.4);
//   alpha4.writeDisplay();
//   delay(2000);
//   alpha_show_float(-23.1);
//   alpha4.writeDisplay();
}

void alpha_show_str(char *str){
  for(uint8_t i=0; i < 4;i++){
    alpha4.writeDigitAscii(i, str[i]);
  }
    alpha4.writeDisplay();
}


void alpha_show_float(float value)
{
    char buff[8];
    // dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
    dtostrf(value, 4, 1, buff); 
    //Serial.print(value); Serial.print(" --> "); Serial.println(buff);

    uint8_t pos = 0;

      if (value >= 0.0 ){
        for (uint8_t i = 4; i > 0; i--){buff[i] = buff[i-1];}
        buff[0] = '+';
        buff[5] = 0x00;
      }
      // Serial.print(value); Serial.print(" ..> "); Serial.println(buff);
      buff[5] = 0x00;
      for(uint8_t i=0; i < 5;i++){
          if (buff[i+1] == '.') {
              alpha4.writeDigitAscii(pos++, buff[i], true);
              i++;
          }
          else {
              alpha4.writeDigitAscii(pos++, buff[i], false);
          }
      }
      alpha4.writeDisplay();
}

void alpha_show_int(uint16_t ival)
{
    char buff[8];
    // itoa(ival,buff,10);
    sprintf(buff,"%04d",ival);
    alpha_show_str(buff);
    Serial.print("alpha_show_int() ");Serial.println(buff);

}

void alpha_show_float_event(float fval, uint32_t timeout, bool set_default)
{
    ctrl.event |= ALPHA_SHOW_FLOAT;
    ctrl.fval   = fval;
    ctrl.new_timeout = timeout;
    if (set_default) ctrl.default_event = ALPHA_SHOW_FLOAT;
}

void alpha_show_integer_event(uint16_t ival, uint32_t timeout, bool set_default)
{
    ctrl.event |= ALPHA_SHOW_INTEGER;
    ctrl.ival   = ival;
    ctrl.new_timeout = timeout;
    if (set_default) ctrl.default_event = ALPHA_SHOW_INTEGER;
}

void alpha_show_str4_event(const char *str4, uint32_t timeout, bool set_default)
{
    ctrl.event |= ALPHA_SHOW_STR4;
    memset(ctrl.str,0x00, 5);
    strncpy(ctrl.str,str4, 4);
    ctrl.new_timeout = timeout;
    if (set_default) ctrl.default_event = ALPHA_SHOW_INTEGER;
}


void alpha_task(void)
{
    switch(alpha_handle.state)
    {
        case 0:
            alpha_handle.state = 10;
            break;
        case 10:
            alpha_handle.state = 20;
            break;
        case 20:
            if(ctrl.event != 0) alpha_handle.state = 100;
            break;
        case 100:
            if ((ctrl.event & ALPHA_SHOW_FLOAT) != 0) alpha_handle.state = 110;
            else if ((ctrl.event & ALPHA_SHOW_INTEGER) != 0) alpha_handle.state = 120;
            else if ((ctrl.event & ALPHA_SHOW_STR4) != 0) alpha_handle.state = 130;
            break;
        case 110:
            alpha_show_float( ctrl.fval);
            ctrl.timeout = millis() + ctrl.new_timeout;
            alpha_handle.state = 115;
            break;
        case 115:
            if (millis() > ctrl.timeout)
            {
                ctrl.event &=  ~ALPHA_SHOW_FLOAT;
                alpha_handle.state = 150;
            }
            break;
        case 120:
            alpha_show_int(ctrl.ival);
            ctrl.timeout = millis() + ctrl.new_timeout;
            alpha_handle.state = 125;
            break;
        case 125:
            if (millis() > ctrl.timeout)
            {
                ctrl.event &= ~ALPHA_SHOW_INTEGER;
                alpha_handle.state = 150;
            }
            break;
        case 130:
            alpha_show_str( ctrl.str);
            ctrl.timeout = millis() + ctrl.new_timeout;
            alpha_handle.state = 135;
            break;
        case 135:
            if (millis() > ctrl.timeout)
            {
                ctrl.event &= ~ALPHA_SHOW_STR4;
                alpha_handle.state = 150;
            }
            break;
        case 150:
            Serial.printf("Default =%X\n",ctrl.default_event);
            if (ctrl.event == 0) ctrl.event = ctrl.default_event;
            alpha_handle.state = 20;
            break;
    }

}

