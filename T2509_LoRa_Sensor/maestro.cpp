#include "Arduino.h"
#include "hardware/watchdog.h"
#include "maestro.h"
#include "sensor.h"
#include "main.h"
#include "io.h"
#include "atask.h"
#include "alpha.h"
#include "rfm.h"

#define MAESTRO_ERR_MSG_TIMEOUT   60000;

typedef struct
{
    uint32_t    timeout;
    uint32_t    err_msg_timeout;
    uint16_t    ldr_value;
    uint32_t    wd_timeout;
    uint32_t    wd_bb_timeout;
    uint8_t     wd_bb_faults;
    bool        first_run = true;
    bool        hw_watchdog_activated;
} maestro_st;

extern main_ctrl_st main_ctrl;

void maestro_task(void);
void wd_task(void);


maestro_st maestro = {
    .timeout=0, 
    .err_msg_timeout= 0,
    .ldr_value = 0,
    .wd_timeout= 0,
    .wd_bb_timeout = 0,
    .wd_bb_faults = 0,
    .first_run = true,
    .hw_watchdog_activated = false,
};

// atask_st:            = {"Label          ", ival, next, state, prev, cntr, run, task_ptr };
atask_st maestro_handle  = {"Maestro Task   ", 1000,    0,     0,  255,    0,   1, maestro_task};
atask_st wd_handle       = {"Watchdog Task  ", 100,    0,     0,  255,    0,   1, wd_task};

void maestro_initialize(void)
{
    atask_add_new(&maestro_handle);  
    atask_add_new(&wd_handle);  
}

void maestro_task(void)
{
    bool first_run = true;
    switch(maestro_handle.state)
    {
        case 0:
            maestro.timeout = millis() + MAESTRO_INITIAL_DELAY + 3 * main_ctrl.node_addr;     //Initial delay
            maestro.err_msg_timeout = millis() + MAESTRO_ERR_MSG_TIMEOUT;
            maestro_handle.state = 10;
            break;
        case 10:
            if (millis() >maestro.timeout) maestro_handle.state = 20;
            maestro.ldr_value = constrain(analogRead(PIN_LDR),200,2000);
            // Serial.printf("LDR = %d -> %d\n", maestro.ldr_value, map(maestro.ldr_value, 200,2000,15,1));
            alpha_set_brightness(map(maestro.ldr_value, 200,2000,15,1));
            break;
        case 20:
            if (main_ctrl.error.radio > 0){
                Serial.printf("Radio Module Error !!!\n");
                alpha_add_short_str( ALPHA_CH_ERR, (char*)"*Rad");
            }
            maestro_handle.state = 30;
            break;
        case 30:
            if (main_ctrl.error.sensor > 0){
                Serial.printf("Sensor Error !!!\n");
                alpha_add_short_str( ALPHA_CH_ERR, (char*)"*Tmp");
            }
            maestro_handle.state = 40;
            break;
        case 40:
            if (main_ctrl.error.display > 0){
                Serial.printf("Display Error !!!\n");
            }
            if(maestro.first_run){
                rfm_send_restart_msg();
                maestro.first_run = false;
            } 
            maestro_handle.state = 100;
            break;
        case 100:
            if (millis() >  maestro.err_msg_timeout){
                if ((main_ctrl.error.display > 0) || 
                    (main_ctrl.error.display > 0) ||
                    (main_ctrl.error.display > 0) ||
                    (main_ctrl.error.display > 0)) {
                    rfm_send_error_msg();
                }
                maestro.err_msg_timeout = millis() + MAESTRO_ERR_MSG_TIMEOUT;
            }
            maestro_handle.state = 0;
            break;

    }
}


#define HEARTBEAT_ACTIVATE      500
#define HEARTBEAT_INTERVAL      1000
#define BEATBACK_TIMEOUT        200
#define BEATBACK_FAULT_LIMIT    4
#define WATCHDOG_RESET_PULSE    1000
#define WATCHDOG_RESET_TIME     2000

void wd_task(void)
{
    switch(wd_handle.state)
    {
        case 0:
            if(io_get_watchdog()){
                //io_set_wd_heartbeat(LOW);
                wd_handle.state = 100;
                maestro.wd_bb_faults = 0;
                maestro.hw_watchdog_activated = true;
                watchdog_enable(4000, true);
            } 
            break;
        case 100:
            io_set_wd_heartbeat(HIGH);
            maestro.wd_timeout = millis() + HEARTBEAT_ACTIVATE;
            wd_handle.state = 110;
            break;
        case 110:
            if(millis() > maestro.wd_timeout){
                io_set_wd_heartbeat(LOW);
                maestro.wd_timeout = millis() + HEARTBEAT_ACTIVATE;
                wd_handle.state = 200;
            } 
            break;
        case 200:  // External Watchdog is now activated
            wd_handle.state = 210;
            break;
        case 210: 
            io_set_wd_heartbeat(HIGH);
            maestro.wd_timeout = millis() + HEARTBEAT_INTERVAL;
            maestro.wd_bb_timeout = millis() + BEATBACK_TIMEOUT;
            wd_handle.state = 220;
            break;
        case 220:
            if(io_get_wd_beatback() == LOW) wd_handle.state = 230;
            else if(millis() > maestro.wd_bb_timeout){
                maestro.wd_bb_faults++;
                wd_handle.state = 230;
            }
            break;
        case 230:
            if(millis() > maestro.wd_timeout){
                wd_handle.state = 240;
            }
            break;
        case 240: 
            io_set_wd_heartbeat(LOW);
            maestro.wd_timeout = millis() + HEARTBEAT_INTERVAL;
            maestro.wd_bb_timeout = millis() + BEATBACK_TIMEOUT;
            wd_handle.state = 250;
            break;
        case 250:
            if(io_get_wd_beatback() == HIGH) wd_handle.state = 260;
            else if(millis() > maestro.wd_bb_timeout){
                maestro.wd_bb_faults++;
                wd_handle.state = 260;
            }
            break;
        case 260:
            if(millis() > maestro.wd_timeout){
                wd_handle.state = 300;
            }
            break;


        case 300:
            Serial.printf("BB Faults= %d\n", maestro.wd_bb_faults);
            if (maestro.wd_bb_faults > BEATBACK_FAULT_LIMIT) {
                Serial.println("Restarting watchdog!!");
                maestro.wd_bb_faults = 0;
                if (main_ctrl.error.watchdog < 255) main_ctrl.error.watchdog;
                wd_handle.state = 200; // WD Reset is leading into a reset loop   
            }
            else {
                wd_handle.state = 200;
            }
            break;
        case 400: 
            io_reset_412(HIGH);
            maestro.wd_timeout = millis() + WATCHDOG_RESET_PULSE;
            wd_handle.state = 410;
            break;
        case 410:
            if(millis() > maestro.wd_timeout){
                io_reset_412(LOW);
                maestro.wd_timeout = millis() + WATCHDOG_RESET_TIME;
                wd_handle.state = 420;
            }
            break;
        case 420:
            if(millis() > maestro.wd_timeout){
                wd_handle.state = 0;
            }
            break;
    
    }
    
    if (maestro.hw_watchdog_activated) {
        if((main_ctrl.error.radio == 0) && (main_ctrl.error.sensor == 0)){
            watchdog_update();
        }
        // else wait for the watchdog to bite
    }
}

    