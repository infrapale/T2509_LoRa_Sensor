#ifndef __SENSOR_H__
#define __SENSOR_H__

typedef struct
{
    bool        active;
    bool        updated;
    uint32_t    next_meas;
    uint32_t    next_send;
} sensor_meta_st;

typedef struct 
{
    sensor_meta_st meta;
    uint32_t  pressure;
    float     temperature;
} sensor_bmp180_st;

typedef struct
{
    sensor_bmp180_st    bmp180;
} sensors_st;

void sensor_initialize(void);

#endif