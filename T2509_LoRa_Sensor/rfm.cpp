/*
https://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
https://learn.adafruit.com/feather-rp2040-rfm95/arduino-ide-setup
https://www.airspayce.com/mikem/arduino/RadioHead/

*/

#include <SPI.h>
//#include <RH_RF95.h>
#include "main.h"
#include "io.h"
#include "rfm.h"
#include "sensor.h"
#include "alpha.h"


// Singleton instance of the radio driver
RH_RF95 rf95(PIN_RFM_CS, PIN_RFM_IRQ );

rfm_ctrl_st rfm_ctrl = {0};
extern main_ctrl_st main_ctrl;
extern sensor_st sensor[SENSOR_TYPE_NBR_OF];

rfm_sensor_msg_st sensor_msg = {
    .target = ADDR_TARGET,
    .sender = ADDR_SENDER,
    .next_send = 0,
    .interval = INTERVAL_SEND_TEMP,
};

//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st rfm_task_handle      =  {"RFM Task       ", 10,    0,     0,  255,    0,  1,  rfm_task };

RH_RF95::ModemConfigChoice modem_config[NBR_OF_MODEM_CONF] = 
{
	rf95.Bw125Cr45Sf128,	   ///< Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range
	rf95.Bw500Cr45Sf128,       ///< Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Fast+short range
	rf95.Bw31_25Cr48Sf512,	   ///< Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on. Slow+long range
	rf95.Bw125Cr48Sf4096,      ///< Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, low data rate, CRC on. Slow+long range
	rf95.Bw125Cr45Sf2048       ///< Bw = 125 kHz, Cr = 4/5, Sf = 2048chips/symbol, CRC on. Slow+long range
};

void rfm_initialize(void)
{
    rfm_ctrl.rx_msg.avail = false;
    if (main_ctrl.fast_interval) sensor_msg.interval = INTERVAL_FAST_SEND_TEMP;
    else sensor_msg.interval = INTERVAL_SEND_TEMP;
    if (rf95.init())
    {
        rfm_set_frequency(867.5);
        rfm_set_power(14);
        if (main_ctrl.long_range_modulation) rfm_set_modem_conf(3);
        else rfm_set_modem_conf(0);
    }
    else
    {
        Serial.println("!!! RFM95 init() failed");
        main_ctrl.error.radio = 1;
    }  
    rfm_ctrl.rec_msg_len    = sizeof(rfm_ctrl.rec_msg);
    rfm_ctrl.send_msg_len   = sizeof(rfm_ctrl.send_msg);
    sensor_msg.next_send = millis() + sensor_msg.interval;
    sensor_msg.sender = main_ctrl.node_addr;
    Serial.printf("Max mesage length= %d\n",RH_RF95_MAX_MESSAGE_LEN);
}

void rfm_task_initilaize(void)
{
    rfm_ctrl.tindx =  atask_add_new(&rfm_task_handle);
}

void rfm_set_sender(uint8_t sender)
{
    sensor_msg.sender = sender;
}

void rfm_reset(void)
{
    digitalWrite(PIN_RFM_RESET, LOW);
    delay(100);
    digitalWrite(PIN_RFM_RESET, HIGH);
    delay(100);
    rfm_initialize();
} 

uint32_t rfm_timeout;
uint8_t sdata[] = "And hello back to you";


void rfm_send_str(char *msg)
{
    uint8_t msg_len = strlen(msg);
    if(main_ctrl.error.radio == 0){
        Serial.printf("Radiating: %s, len=%d",rfm_ctrl.buff, msg_len);
        memset(rfm_ctrl.send_msg,0x00,RH_RF95_MAX_MESSAGE_LEN);
        memcpy(rfm_ctrl.send_msg, msg, msg_len);
        rfm_ctrl.send_msg_len = msg_len; 
        rf95.send(rfm_ctrl.send_msg, rfm_ctrl.send_msg_len);
        rf95.waitPacketSent();
        Serial.printf(" .. packet done\n");
    }
    else {
        Serial.printf("Radio kaputt? Not able to send: %s, len=%d\n",rfm_ctrl.buff, msg_len);
    }
    // Serial1.printf("%s len=%d",msg,msg_len);
}

void rfm_set_power(int8_t pwr)
{
    rfm_ctrl.power =  pwr;
    rf95.setTxPower( pwr);
}

void rfm_set_modem_conf(int8_t modem_conf_indx)
{
    rfm_ctrl.modem_conf =  modem_config[modem_conf_indx];
    rf95.setModemConfig(rfm_ctrl.modem_conf);
}

void rfm_set_frequency(float freq)
{
    rfm_ctrl.frequency = freq;
    rf95.setFrequency(freq);
}

void rfm_set_sf(uint8_t sf)
{
    rfm_ctrl.sf  = sf;
    rf95.setSpreadingFactor(sf);
}

uint8_t data[] = "Hello World!";

void fix_serial1(void)
{
    //Serial1.end();
    //Serial1.begin(INTERCONNECT_UART_BPS);
    //delay(1000);
    Serial1.clearWriteError();
    Serial1.setPollingMode(false);
}


void rfm_build_sensor_msg(uint8_t sindx)
{ 
    switch(sindx){
        case SENSOR_TYPE_BMP180:
            sprintf( rfm_ctrl.buff,"<%d;%d;S%d;T%.1f;C%d;#180>",
                sensor_msg.target,
                sensor_msg.sender,
                sindx,
                sensor[sindx].temperature,
                sensor[sindx].meta.counter
            );
            break;
        case SENSOR_TYPE_BMP280:
            sprintf( rfm_ctrl.buff,"<%d;%d;S%d;T%.1f;C%d;#180>",
                sensor_msg.target,
                sensor_msg.sender,
                sindx,
                sensor[sindx].temperature,
                sensor[sindx].meta.counter
            );
            break;
        case SENSOR_TYPE_BME680:
            sprintf( rfm_ctrl.buff,"<%d;%d;S%d;T%.1f;H%.0f;C%d;#680>",
                sensor_msg.target,
                sensor_msg.sender,
                sindx,
                sensor[sindx].temperature,
                sensor[sindx].humidity,
                sensor[sindx].meta.counter
            );
            break;
        case SENSOR_TYPE_AHT20:
            sprintf( rfm_ctrl.buff,"<%d;%d;S%d;T%.1f;H%.0f;C%d;#20>",
                sensor_msg.target,
                sensor_msg.sender,
                sindx,
                sensor[sindx].temperature,
                sensor[sindx].humidity,
                sensor[sindx].meta.counter
            );
            break;
        case SENSOR_TYPE_SHT21:
            sprintf( rfm_ctrl.buff,"<%d;%d;S%d;T%.1f;H%.0f;C%d;#21>",
                sensor_msg.target,
                sensor_msg.sender,
                sindx,
                sensor[sindx].temperature,
                sensor[sindx].humidity,
                sensor[sindx].meta.counter
            );
            break;
        case SENSOR_TYPE_DS18B20:
            sprintf( rfm_ctrl.buff,"<%d;%d;S%d;T%.1f;C%d;#1820>",
                sensor_msg.target,
                sensor_msg.sender,
                sindx,
                sensor[sindx].temperature,
                sensor[sindx].meta.counter
            );
            break;
        case SENSOR_TYPE_PIR:
            break;
    }
}

void rfm_send_restart_msg(void)
{ 
    sprintf( rfm_ctrl.buff,"<%d;%d;a%s;d%s;t%s>",
        sensor_msg.target,
        sensor_msg.sender,
        APP_NAME,
        __DATE__,
        __TIME__
    );
    rfm_send_str(rfm_ctrl.buff);
}

void rfm_send_error_msg(void)
{ 
    sprintf( rfm_ctrl.buff,"<%d;%d;x%d;y%d;z%d;w%d>",
        sensor_msg.target,
        sensor_msg.sender,
        main_ctrl.error.sensor,
        main_ctrl.error.display,
        main_ctrl.error.radio,
        main_ctrl.error.watchdog
    );
    rfm_send_str(rfm_ctrl.buff);
}




void rfm_task(void)
{
    switch(rfm_task_handle.state)
    {
        case 0:
            rfm_task_handle.state = 5;
            rfm_ctrl.sensor_indx = 0;
            rfm_ctrl.timeout = millis() + main_ctrl.node_addr * 3;
            break;
        case 5: 
            if (millis() > rfm_ctrl.timeout) rfm_task_handle.state = 10;
            break;
        case 10:    
            if ((sensor[rfm_ctrl.sensor_indx].meta.active) &&
                (sensor[rfm_ctrl.sensor_indx].meta.updated) &&
                (millis() > sensor[rfm_ctrl.sensor_indx].meta.next_send))
            {
                rfm_build_sensor_msg(rfm_ctrl.sensor_indx);
                rfm_send_str(rfm_ctrl.buff);
                sensor[rfm_ctrl.sensor_indx].meta.updated = false;
                sensor[rfm_ctrl.sensor_indx].meta.next_send = millis() + sensor_msg.interval;
                if(++sensor[rfm_ctrl.sensor_indx].meta.counter >= 9999) 
                    sensor[rfm_ctrl.sensor_indx].meta.counter = 0;
                rfm_task_handle.state = 20;
                io_set_onboard_led(true);
            }
            else rfm_task_handle.state = 100;
            break;
        case 20:
            alpha_add_short_str( ALPHA_CH_SENDING, (char*)"Send");
            rfm_ctrl.timeout = millis() + 4000;
            rfm_task_handle.state = 30;
            break;
        case 30:
            if (millis() > rfm_ctrl.timeout) {
                io_set_onboard_led(false);                
                alpha_add_integer(ALPHA_CH_MSG_CNTR, sensor[rfm_ctrl.sensor_indx].meta.counter);
                rfm_ctrl.timeout = millis() + 1000 + (main_ctrl.node_addr*500);
                rfm_task_handle.state = 40;
            } 
            break;
        case 40:
            if(millis() > rfm_ctrl.timeout) rfm_task_handle.state = 100;
            break;
        case 100:
            if(++rfm_ctrl.sensor_indx >= SENSOR_TYPE_NBR_OF) rfm_ctrl.sensor_indx = 0;
            rfm_task_handle.state = 10;
            break;
    }
}



