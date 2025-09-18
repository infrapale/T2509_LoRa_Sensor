#include "Arduino.h"
#include "sensor.h"
#include "main.h"
#include "atask.h"
//#include <Adafruit_BMP280.h>
#include "Adafruit_BMP085.h"
#include "alpha.h"

#define BMP280_ADDR   0x76
#define BMP180_ADDR   0x76

extern TwoWire *Wirep;

// function prototype
void sensor_print_bmp280_data(void);
void sensor_set_bmp280_configuration(void);
void sensor_task(void);


// atask_st:            = {"Label          ", ival, next, state, prev, cntr, run, task_ptr };
atask_st sensor_handle  = {"Sensor Task    ", 1000,    0,     0,  255,    0,   1, sensor_task};

//Adafruit_BMP280 bmp280;
Adafruit_BMP085 bmp180;


sensors_st sensor =
{
    .bmp180 = {
        .meta = {.active=false, .updated=false, .next_meas=0, .next_send= 0},
        .pressure = 0, 
        .temperature = 0.0
    }
};

void sensor_initialize(void)
{
  uint8_t status = bmp180.begin(BMP180_ADDR, Wirep);

  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor!!"));
    // while (1) delay(10);
  }
  else {    
    // sensor_print_bmp280_data();
    sensor.bmp180.meta.active = true;
    sensor.bmp180.meta.next_send =  millis() + INTERVAL_SEND_TEMP;
  }
 
    atask_add_new(&sensor_handle);  
}

void sensor_task(void)
{
    switch(sensor_handle.state)
    {
        case 0:
            sensor_handle.state = 10;
            break;
        case 10:
            if(sensor.bmp180.meta.active && (millis() > sensor.bmp180.meta.next_meas))
            {
                sensor.bmp180.pressure = bmp180.readPressure();
                sensor.bmp180.temperature = bmp180.readTemperature();
                //Print the results
                Serial.printf("Temperature: %.2f °C \t", sensor.bmp180.temperature);
                alpha_show_float_event(sensor.bmp180.temperature, 2000, true);
                Serial.printf("Pressure: %d Pa\n", sensor.bmp180.pressure);
                sensor.bmp180.meta.next_meas = millis() + INTERVAL_READ_SENSOR;
                sensor.bmp180.meta.updated = true;   //cleared by MQTT code
            }                     
            sensor_handle.state = 20;
            break;
        case 20:
            // mqtt_update_sensor_values(&sensor);
            sensor_handle.state = 10;
            break;
        case 30:
            sensor_handle.state = 10;
            break;
    }
}

void sensor_print_bmp280_data(void)
{
    // Serial.print(F("SensorID was: 0x")); Serial.println(bmp280.sensorID(),16);
    // Serial.print(F("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n"));
    // Serial.print(F("   ID of 0x56-0x58 represents a BMP 280,\n"));
    // Serial.print(F("        ID of 0x60 represents a BME 280.\n"));
    // Serial.print(F("        ID of 0x61 represents a BME 680.\n"));
}

void sensor_set_bmp280_configuration(void)
{
  /* Default settings from the datasheet. */
//   bmp280.setSampling( Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
//                       Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
//                       Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
//                       Adafruit_BMP280::FILTER_X16,      /* Filtering. */
//                       Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

