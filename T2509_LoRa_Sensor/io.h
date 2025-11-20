#ifndef __IO_H__ 
#define __IO_H__
#include <Arduino.h>
//#include "main.h"

#define BOARD_T2504_PICO_RFM95_80x70    1
#define BOARD_PROTO_PICO_RFM95          2
#define BOARD_LORA_SENSOR               3
#define BOARD BOARD_LORA_SENSOR

#if BOARD == BOARD_LORA_SENSOR
#define PIN_UART0_TX        (0u)
#define PIN_UART0_RX        (1u)
#define PIN_PIR             (2u)
#define PIN_ONE_WIRE        (3u)
#define PIN_I2C0_SDA        (4u)
#define PIN_I2C0_SCL        (5u)
//#define PIN_I2C1_SDA      (6u)
//#define PIN_I2C1_SCL      (7u)
#define PIN_WD_BEATBACK     (6u)
#define PIN_WD_HEARTBEAT    (7u)
#define PIN_SW1		          (8u)
#define PIN_SW2		          (9u)
#define PIN_SW3		          (10u)
#define PIN_SW4             (11u)
#define PIN_SW5             (12u)
#define PIN_SW6             (13u)
#define PIN_SW7             (14u)
#define PIN_SW8             (15u)
#define PIN_RFM_MISO        (16u)
#define PIN_RFM_CS          (17u)
#define PIN_RFM_SCK         (18u)
#define PIN_RFM_MOSI        (19u)
#define PIN_RFM_RESET       (20u)
#define PIN_RFM_IRQ         (21u)
#define PIN_EN_VSYSX        (22u)
#define PIN_LDR             (26u)
#define PIN_EN_3V3X         (27u)
#define PIN_RST_412         (28u)

#define PIN_ENABLE_WATCHDOG   PIN_SW7
#define PIN_ENABLE_TEST_MODE  PIN_SW8

#elif BOARD == BOARD_PROTO_PICO_RFM95
#define PIN_RFM_MOSI        (19u)
#define PIN_RFM_MISO        (16u)
#define PIN_RFM_SCK         (18u)
#define PIN_RFM_CS          (10u)
#define PIN_RFM_IRQ         (21u)
#define PIN_RFM_RESET       (11u)
#else
#error No board was defined
#endif



#define PIN_LED_RED         (2u)
#define PIN_LED_GREEN       (3u)
#define PIN_LED_BLUE        (22u)

#define PIN_I2C_SDA         (4u)
#define PIN_I2C_SCL         (5u)
#define PIN_I2C_PWR         (10u)



typedef enum
{
  COLOR_RED = 0,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_NBR_OF
} color_et;

typedef enum
{
  BLINK_OFF = 0,
  BLINK_ON,
  BLINK_SHORT_FLASH,
  BLINK_FLASH,
  BLINK_FAST_FLASH,
  BLINK_SLOW_BLINK,
  BLINK_BLINK,
  BLINK_FAST_BLINK,
  BLINK_CLIENT,
  BLINK_RELIABLE_CLIENT,
  BLINK_SERVER,
  BLINK_RELIABLE_SERVER,
  BLINK_NBR_OF
} blink_et;

void io_initialize(void);

void io_task_initialize(void);
void io_task(void);

uint8_t io_get_addr(void);

bool io_get_debug_mode(void);

bool io_get_watchdog(void);

void io_set_wd_heartbeat(uint8_t value);

uint8_t io_get_wd_beatback(void);

bool io_pir_detected(void);

void io_pwr_sensor(bool pwr_on);

void io_pwr_vsysx(bool pwr_on);

void io_reset_412(uint8_t value);

bool io_pir_detected(void);
uint16_t io_read_ldr(void);
void io_set_onboard_led(bool is_on);


#endif