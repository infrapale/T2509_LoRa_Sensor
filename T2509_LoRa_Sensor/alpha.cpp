#include <Wire.h>
#include <Adafruit_GFX.h>
#include "main.h"
#include "Adafruit_LEDBackpack.h"
#include "io.h"
#include "alpha.h"
#include "atask.h"


extern TwoWire *Wirep;
extern main_ctrl_st main_ctrl;

void alpha_task(void);

// atask_st:            = {"Label          ", ival, next, state, prev, cntr, run, task_ptr };
atask_st alpha_handle   = {"Alpha Show Task", 100,     0,     0,  255,    0,   1, alpha_task};

alpha_ctrl_st  ctrl = {
  .channel = ALPHA_CH_TEMPERATURE,  
  .event = ALPHA_SHOW_UNDEFINED,
  .default_event = 0,
  .fval = 0.0,
  .ival = 42,
  .long_str  = "ABCD",
  .timeout = 0,
  .new_timeout = 0,
  .ready = true
};

 
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
char displaybuffer[4] = {' ', ' ', ' ', ' '};
alpha_channel_st ach[ALPHA_CH_NBR_OF] =
{
    [ALPHA_CH_TEMPERATURE]  = {.active= false, .duration=4000, .timeout=0, .str = "XXXX", .function = ALPHA_FUNCTION_PERMANENT},
    [ALPHA_CH_HUMIDITY]     = {.active= false, .duration=2000, .timeout=0, .str = "XXXX", .function = ALPHA_FUNCTION_PERMANENT},
    [ALPHA_CH_MSG_CNTR]     = {.active= false, .duration=2000, .timeout=0, .str = "XXXX", .function = ALPHA_FUNCTION_ONE_TIME},
    [ALPHA_CH_ERR]          = {.active= false, .duration=4000, .timeout=0, .str = "XXXX", .function = ALPHA_FUNCTION_PERMANENT},
    [ALPHA_CH_SENDING]      = {.active= false, .duration=2000, .timeout=0, .str = "XXXX", .function = ALPHA_FUNCTION_ONE_TIME},
    [ALPHA_CH_ADDR]         = {.active= false, .duration=4000, .timeout=0, .str = "XXXX", .function = ALPHA_FUNCTION_ONE_TIME},
    [ALPHA_CH_NUMBER]       = {.active= false, .duration=4000, .timeout=0, .str = "XXXX", .function = ALPHA_FUNCTION_ONE_TIME},
    [ALPHA_CH_LONG_STR]     = {.active= false, .duration=500,  .timeout=0, .str = "XXXX", .function = ALPHA_FUNCTION_SCROLLING},
};


void alpha_show_int(uint16_t ival);
bool alpha_is_ready(void);


void alpha_initialize(void) { 
    alpha4.begin(I2C_ADDR_ALPHANUM, Wirep );  // pass in the address
    // for (uint16_t i = 0; i< 1000; i++)  alpha_show_int(i);

    ctrl.timeout = millis() + 5000;
    atask_add_new(&alpha_handle);  
    delay(1000);
    alpha_add_long_str(ALPHA_CH_LONG_STR, (char*)"Telabech LoRa Sensor");
    delay(2000);
    while (!alpha_is_ready()) {;}
    char buff[6] = {0};
    sprintf(buff,"@%3d", main_ctrl.node_addr);
    alpha_add_short_str(ALPHA_CH_ADDR, buff);
    delay(4000);
}

// void alpha_show_str(char *str){
//   for(uint8_t i=0; i < 4;i++){
//     alpha4.writeDigitAscii(i, str[i]);
//   }
//     alpha4.writeDisplay();
// }


// void alpha_show_float(float value)
// {
//     char buff[8];
//     // dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
//     dtostrf(value, 4, 1, buff); 
//     //Serial.print(value); Serial.print(" --> "); Serial.println(buff);

//     uint8_t pos = 0;

//       if (value >= 0.0 ){
//         for (uint8_t i = 4; i > 0; i--){buff[i] = buff[i-1];}
//         buff[0] = '+';
//         buff[5] = 0x00;
//       }
//       // Serial.print(value); Serial.print(" ..> "); Serial.println(buff);
//       buff[5] = 0x00;
//       for(uint8_t i=0; i < 5;i++){
//           if (buff[i+1] == '.') {
//               alpha4.writeDigitAscii(pos++, buff[i], true);
//               i++;
//           }
//           else {
//               alpha4.writeDigitAscii(pos++, buff[i], false);
//           }
//       }
//       alpha4.writeDisplay();
// }

// void alpha_show_int(uint16_t ival)
// {
//     char buff[8];
//     // itoa(ival,buff,10);
//     sprintf(buff,"%04d",ival);
//     alpha_show_str(buff);
//     //Serial.print("alpha_show_int() ");Serial.println(buff);
// }

bool alpha_is_ready(void)
{
   return (ctrl.ready);
}


void alpha_add_float(alpha_channel_et ch, float fval)
{
    char *str = ach[ch].str;
    memset(str,0x00, SHORT_STR_LEN);
    if (fval > 0.0) {
        str[0] = '+';
        dtostrf(fval, 4, 1, &str[1]); 
    }
    else dtostrf(fval, 4, 1, &str[0]);
    ach[ch].active = true;
}

void alpha_add_integer(alpha_channel_et ch, uint16_t ival)
{
    char *str = ach[ch].str;
    memset(str,0x00, SHORT_STR_LEN);
    sprintf(str,"%04d",ival);
    ach[ch].active = true;
}


void alpha_add_short_str(alpha_channel_et ch, char *buff)
{
    char *str = ach[ch].str;
    memset(str,0x00, SHORT_STR_LEN);
    strcpy(str,buff);
    ach[ch].active = true;
}

void alpha_add_long_str(alpha_channel_et ch, char *buff)
{
    memset(ctrl.long_str,0x00, LONG_STR_LEN);
    strcpy(ctrl.long_str, buff);
    ach[ch].active = true;
    ctrl.ready = false;
}


void alpha_display_short(char *str)
{
    uint8_t i = 0;
    uint8_t pos = 0;
    bool    not_done = true;

    while (not_done){
        if (str[i+1] == '.') {
                alpha4.writeDigitAscii(pos++, str[i++], true);
                i++;
        }
        else {alpha4.writeDigitAscii(pos++, str[i++], false);}
        if(pos >= 4) not_done = false;
    }
    alpha4.writeDisplay();
}


void alpha_display_ch_short(alpha_channel_et ch)
{
    alpha_display_short(ach[ch].str);   
    // uint8_t i = 0;
    // uint8_t pos = 0;
    // bool    not_done = true;
    // char    *str = ach[ch].str;

    // while (not_done){
    //     if (str[i+1] == '.') {
    //             alpha4.writeDigitAscii(pos++, str[i++], true);
    //             i++;
    //     }
    //     else {alpha4.writeDigitAscii(pos++, str[i++], false);}
    //     if(pos >= 4) not_done = false;
    // }
    // alpha4.writeDisplay();
}

// void alpha_show_float_event(float fval, uint32_t timeout, bool set_default)
// {
//     ctrl.event |= ALPHA_SHOW_FLOAT;
//     ctrl.fval   = fval;
//     ctrl.new_timeout = timeout;
//     ctrl.ready = false;
//     if (set_default) ctrl.default_event = ALPHA_SHOW_FLOAT;
// }



// void alpha_show_integer_event(uint16_t ival, uint32_t timeout, bool set_default)
// {
//     ctrl.event |= ALPHA_SHOW_INTEGER;
//     ctrl.ival   = ival;
//     ctrl.new_timeout = timeout;
//     ctrl.ready = false;
//     if (set_default) ctrl.default_event = ALPHA_SHOW_INTEGER;
// }

// void alpha_show_str4_event(const char *str4, uint32_t timeout, bool set_default)
// {
//     ctrl.event |= ALPHA_SHOW_STR4;
//     memset(ctrl.str,0x00, 5);
//     strncpy(ctrl.str,str4, 4);
//     ctrl.ready = false;
//     ctrl.new_timeout = timeout;
//     if (set_default) ctrl.default_event = ALPHA_SHOW_INTEGER;
// }

// void alpha_show_long_str_event(char *str, uint32_t timeout)
// {
//     ctrl.len = strlen(str);
//     if (ctrl.len < SHOW_MAX_STR_LEN) {
//         ctrl.event |= ALPHA_SHOW_LONG_STR;
//         strcpy(ctrl.str, str);
//         ctrl.new_timeout = timeout;
//         ctrl.cursor = 0;
//         ctrl.ready = false;
//     }
// }

void alpha_task(void)
{
    //Serial.print("Q");
    switch(alpha_handle.state)
    {
        case 0:
            alpha_handle.state = 10;
            break;
        case 10:
            alpha_handle.state = 100;
            break;
        case 100:
            //Serial.printf("Channel: %d  active: %d\n",ctrl.channel, ach[ctrl.channel].active );
            if(ach[ctrl.channel].active)
            {
                ctrl.timeout = millis() + ach[ctrl.channel].duration;
                //Serial.printf("Function: %d\n", ach[ctrl.channel].function );
                switch(ach[ctrl.channel].function)
                {
                    case ALPHA_FUNCTION_ONE_TIME:
                        ach[ctrl.channel].active = false;
                        alpha_display_ch_short((alpha_channel_et)ctrl.channel);
                        alpha_handle.state = 120;
                         Serial.println("One Time -> 120");
                        break;
                    case ALPHA_FUNCTION_PERMANENT:
                         Serial.println("Permanent -> 120");
                        alpha_display_ch_short((alpha_channel_et)ctrl.channel);
                        alpha_handle.state = 120;
                        break;
                    case ALPHA_FUNCTION_SCROLLING:
                        ctrl.len = strlen(ctrl.long_str);
                        alpha_handle.state = 200;
                        ctrl.ready = false;
                        Serial.println("Scroll -> 200");
                        break;
                }
            }
            else
            {
                alpha_handle.state = 110;
            }
            break;
        case 110:
            ctrl.channel++;
            ctrl.ready = true;
            if (ctrl.channel >= ALPHA_CH_NBR_OF) ctrl.channel = 0;
            //Serial.printf("Alpha Channel: %d active: %d function: %d \n", ctrl.channel, ach[ctrl.channel].active, ach[ctrl.channel].function);
            alpha_handle.state = 100;
            break;

        case 120:
            if (millis() > ctrl.timeout) alpha_handle.state = 110;
            break;

        case 200:
            alpha_display_short( (ctrl.long_str+ctrl.cursor));
            ctrl.timeout = millis() + 100;   //ach[ctrl.channel].duration;
            alpha_handle.state = 210;
            break;
        case 210:
            if (millis() > ctrl.timeout)
            {
                ctrl.cursor++;
                if (ctrl.cursor <= (ctrl.len -4)) { alpha_handle.state = 200; } 
                else { 
                    alpha_handle.state = 110; 
                    ach[ctrl.channel].active = false; 
                }
            }
            break;
    }

}

