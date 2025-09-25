#ifndef __SENSOR_H__
#define __SENSOR_H__

#define I2C_ADDR_BMP180         0x77
#define I2C_ADDR_BMP280         0x11
#define I2C_ADDR_BME680         0x11
#define I2C_ADDR_SHT21          0x44
#define I2C_ADDR_AHT20          0x38

#define SENSOR_LABEL_LEN        8
#define NBR_TEST_SENSOR         4

typedef enum
{
    SENSOR_TYPE_UNDEFINED = 0,
    SENSOR_TYPE_BMP180,
    SENSOR_TYPE_BMP280,
    SENSOR_TYPE_BME680,
    SENSOR_TYPE_AHT20,
    SENSOR_TYPE_SHT21,
    SENSOR_TYPE_DS18B20,
    SENSOR_TYPE_PIR,
    SENSOR_TYPE_NBR_OF
} sensor_type_et;

typedef struct
{
    char        label[SENSOR_LABEL_LEN];
    uint8_t     i2c_addr;
    bool        active;
    bool        updated;
    bool        show_temperature;
    uint16_t    counter;
    uint32_t    next_meas;
    uint32_t    next_send;
} sensor_meta_st;



typedef struct 
{
    sensor_meta_st meta;
    uint32_t  pressure;
    float     temperature;
    float     humidity;
    float     float_val;
    bool      on_off;
} sensor_st;

typedef struct 
{
    uint8_t     sender;
    uint8_t     sensor;
    uint32_t    interval;
    uint32_t    next_send;
    float       temp;
    float       min_temp;
    float       max_temp;
    float       delta_temp;
    bool        going_up;
} sensor_test_st;


void sensor_initialize(void);

#endif