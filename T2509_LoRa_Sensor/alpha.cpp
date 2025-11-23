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
  .long_str  = "ABCD",
  .timeout = 0,
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

void alpha_set_brightness(uint8_t br)
{
    // br = 0..15
    br = constrain(br,0,15);
    alpha4.setBrightness(br);
}

void alpha_initialize(void) { 
    if (alpha4.begin(I2C_ADDR_ALPHANUM, Wirep )) main_ctrl.error.display = 0;
    else main_ctrl.error.display = 1;
    alpha_set_brightness(2);

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
}


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
                        // Serial.println("One Time -> 120");
                        break;
                    case ALPHA_FUNCTION_PERMANENT:
                        alpha_display_ch_short((alpha_channel_et)ctrl.channel);
                        alpha_handle.state = 120;
                        // Serial.println("Permanent -> 120");
                        break;
                    case ALPHA_FUNCTION_SCROLLING:
                        ctrl.len = strlen(ctrl.long_str);
                        alpha_handle.state = 200;
                        ctrl.ready = false;
                        //Serial.println("Scroll -> 200");
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

