#ifndef __MAIN_H__
#define __MAIN_H__

//#define LORA_CLIENT
//#define LORA_SERVER

#define LABEL_LEN           12
#define TIME_ZONE_OFFS      3
#define UNIT_LABEL_LEN      10
#define MEASURE_LABEL_LEN   16

#define APP_NAME            "T2509_LoRa_Sensor"
#define SW_BM_TEST          0b00001000
#define SW_BM_ROLE          0b00000100
#define SW_BM_ADDR_1        0b00000010
#define SW_BM_ADDR_0        0b00000001
#define SW_BM_ADDR          0b00000011
#define DEBUG_PRINT

#define UART_0              Serial
#define DEBUG_UART_BPS          9600
#define INTERCONNECT_UART_BPS   9600

#define ADDR_TARGET             0
#define ADDR_SENDER             42

#undef TEST_MODE
#define I2C_ADDR_ALPHANUM       0x70


#define INTERVAL_READ_SENSOR    10000
#define INTERVAL_SEND_TEMP      600000
#define INTERVAL_FAST_SEND_TEMP 20000

typedef struct
{
    uint8_t sensor;
    uint8_t radio;
    uint8_t display;
    uint8_t watchdog;
} error_st;



typedef struct 
{
    uint8_t         node_addr;
    bool            test_activated;
    bool            io_initialized;
    bool            debug_mode;
    bool            watchdog;
    bool            long_range_modulation;
    bool            fast_interval;
    error_st        error;
} main_ctrl_st;




typedef struct date_time
{
    uint16_t  year;
    uint8_t   month;
    uint8_t   day;
    uint8_t   hour;
    uint8_t   minute;
    uint8_t   second;
 } date_time_st;

#endif
