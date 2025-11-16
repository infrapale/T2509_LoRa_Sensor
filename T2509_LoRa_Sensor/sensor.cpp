#include "Arduino.h"
#include "sensor.h"
#include "main.h"
#include "io.h"
#include "atask.h"
//#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP085.h"
#include <Adafruit_BMP280.h>
#include "Adafruit_BME680.h"
#include <Adafruit_AHTX0.h>
#include "Adafruit_SHT31.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#include "alpha.h"
#include "rfm.h"

#define SEALEVELPRESSURE_HPA (1013.25)
typedef struct
{
    uint8_t     sensor_indx;
    uint32_t    timeout;
    uint16_t    pir_state;
    uint16_t    pir_cntr;
    bool        pir_is_active;
} sensor_ctrl_st;


extern TwoWire *Wirep;

// function prototype
void sensor_print_bmp280_data(void);
void sensor_set_bmp280_configuration(void);
void sensor_task(void);
void sensor_test_task(void);

sensor_ctrl_st sensor_ctrl ={0};

// atask_st:            = {"Label          ", ival, next, state, prev, cntr, run, task_ptr };
#ifndef TEST_MODE
atask_st sensor_handle  = {"Sensor Task   ", 1000,    0,     0,  255,    0,   1, sensor_task};
#else
atask_st sensor_handle  = {"Sensor Test   ", 1000,    0,     0,  255,    0,   1, sensor_test_task};
#endif

Adafruit_BMP085 bmp180;
Adafruit_BMP280 bmp280;
Adafruit_BME680 bme680(Wirep); 
Adafruit_AHTX0 aht20;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
OneWire oneWire(PIN_ONE_WIRE);
DallasTemperature ds18b20(&oneWire);

// #define I2C_ADDR_BMP180         0x77
// #define I2C_ADDR_BMP280         0x11
// #define I2C_ADDR_BMP680         0x11
// #define I2C_ADDR_SHT21          0x44
// #define I2C_ADDR_AHT20          0x38



sensor_st sensor[SENSOR_TYPE_NBR_OF] =
{
    [SENSOR_TYPE_UNDEFINED] = {
            .meta = {.label= "Undef  ", .i2c_addr = 0x00, .active=false, .updated=false, .show_temperature=false, .counter= 0, .next_meas=0, .next_send= 0},
            .pressure = 0, .temperature = 0.0, .humidity = 0.0, .float_val = 0.0, .on_off = 0
    },
    [SENSOR_TYPE_BMP180] = {
            .meta = {.label= "BMP180 ", .i2c_addr = I2C_ADDR_BMP180, .active=false, .updated=false, .show_temperature=true, .counter= 0, .next_meas=0, .next_send= 0},
            .pressure = 0, .temperature = 0.0, .humidity = 0.0, .float_val = 0.0, .on_off = 0
    },
    [SENSOR_TYPE_BMP280] = {
            .meta = {.label= "BMP280 ", .i2c_addr = I2C_ADDR_BMP280, .active=false, .updated=false,.show_temperature=true, .counter= 0, .next_meas=0, .next_send= 0},
            .pressure = 0, .temperature = 0.0, .humidity = 0.0, .float_val = 0.0, .on_off = 0
    },
    [SENSOR_TYPE_BME680] = {
            .meta = {.label= "BME680 ", .i2c_addr = I2C_ADDR_BME680, .active=false, .updated=false, .show_temperature=true, .counter= 0, .next_meas=0, .next_send= 0},
            .pressure = 0, .temperature = 0.0, .humidity = 0.0, .float_val = 0.0, .on_off = 0
    },
    [SENSOR_TYPE_AHT20] = {
            .meta = {.label= "AHT20  ", .i2c_addr = I2C_ADDR_AHT20 , .active=false, .updated=false, .show_temperature=true, .counter= 0, .next_meas=0, .next_send= 0},
            .pressure = 0, .temperature = 0.0, .humidity = 0.0, .float_val = 0.0, .on_off = 0
    },
    [SENSOR_TYPE_SHT21] = {
            .meta = {.label= "SHT21  ", .i2c_addr = I2C_ADDR_SHT21, .active=false, .updated=false, .show_temperature=true, .counter= 0, .next_meas=0, .next_send= 0},
            .pressure = 0, .temperature = 0.0, .humidity = 0.0, .float_val = 0.0, .on_off = 0
    },
    [SENSOR_TYPE_DS18B20] = {
            .meta = {.label= "DS18B20", .i2c_addr = 0x00, .active=true, .updated=false, .show_temperature=true, .counter= 0, .next_meas=0, .next_send= 0},
            .pressure = 0, .temperature = 0.0, .humidity = 0.0, .float_val = 0.0, .on_off = 0
    },
    [SENSOR_TYPE_PIR] = {
            .meta = {.label= "PIR    ", .i2c_addr = 0x00, .active=false, .updated=false, .counter= 0, .next_meas=0, .next_send= 0},
            .pressure = 0, .temperature = 0.0, .humidity = 0.0, .float_val = 0.0, .on_off = 0
    },
};

sensor_test_st sensor_test[NBR_TEST_SENSOR] =
{
    {.sender=12, .sensor=SENSOR_TYPE_BMP180,.interval=22000, .temp=14.0, .min_temp=4.0, .max_temp=38.0, .delta_temp=2.0, .going_up=false},
    {.sender=18, .sensor=SENSOR_TYPE_BMP280,.interval=37000, .temp=4.0,  .min_temp=4.0, .max_temp=38.0, .delta_temp=2.0, .going_up=true},
    {.sender=24, .sensor=SENSOR_TYPE_AHT20,.interval=61000,  .temp=10.0, .min_temp=4.0, .max_temp=38.0, .delta_temp=1.0, .going_up=false},
    {.sender=10, .sensor=SENSOR_TYPE_SHT21,.interval=120000, .temp=-1.0, .min_temp=-4.0, .max_temp=18.0, .delta_temp=2.0, .going_up=false},
};


void sensor_initialize(void)
{
    uint8_t i2c_addr;
    uint8_t i2c_error;
    uint8_t status;
    #ifndef TEST_MODE
    for (uint8_t sindx = 0; sindx < SENSOR_TYPE_NBR_OF; sindx++ ){
        i2c_addr = sensor[sindx].meta.i2c_addr;
        if ((i2c_addr != 0x00) &&  (i2c_addr < 0x80)) {
            Wirep->beginTransmission(i2c_addr);
            i2c_error = Wirep->endTransmission();
            if (i2c_error == 0) {
                sensor[sindx].meta.active = true; 
                Serial.printf("Sensor %s active\n", sensor[sindx].meta.label);
            }
        }
    }
    ds18b20.begin();
    float tempC;
    for (uint8_t sindx = 0; sindx < SENSOR_TYPE_NBR_OF; sindx++ ){
        if (sensor[sindx].meta.active){
            switch (sindx){
                case SENSOR_TYPE_BMP180:
                    status = bmp180.begin(I2C_ADDR_BMP180, Wirep);
                    break;
                case SENSOR_TYPE_BMP280:
                    status = bmp280.begin();
                    break;
                case SENSOR_TYPE_BME680:
                    status = bme680.begin();
                    if (status == 0) {
                        // Set up oversampling and filter initialization
                        bme680.setTemperatureOversampling(BME680_OS_8X);
                        bme680.setHumidityOversampling(BME680_OS_2X);
                        bme680.setPressureOversampling(BME680_OS_4X);
                        bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
                        bme680.setGasHeater(320, 150); // 320*C for 150 ms
                        status = 1;
                    } else status = 0;
                    break;
                case SENSOR_TYPE_AHT20:
                    status = aht20.begin(Wirep);
                    break;
                case SENSOR_TYPE_SHT21:
                    status = sht31.begin(sensor[SENSOR_TYPE_SHT21].meta.i2c_addr);
                    Serial.print("SHT21 Heater is ");
                    (sht31.isHeaterEnabled()) ? Serial.println("Enabled") : Serial.println("Disabled");
                    break;
                case SENSOR_TYPE_DS18B20:
                    ds18b20.requestTemperatures();
                    delay(1500);
                    tempC = ds18b20.getTempCByIndex(0);
                    if (tempC != DEVICE_DISCONNECTED_C) status = true;
                    else status = false;
                    ds18b20.requestTemperatures();
                    break;
                case SENSOR_TYPE_PIR:
                    break;
            }
        }
        if (!status) {
            Serial.printf("Sensor failure %s\n", sensor[sindx].meta.label);
            sensor[sindx].meta.active = false;

        }
    }

    for (uint8_t sindx = 0; sindx < SENSOR_TYPE_NBR_OF; sindx++ ){
        if (sensor[sindx].meta.active){
            sensor[sindx].meta.next_send = millis() + INTERVAL_SEND_TEMP;
        }
    }
 
    #else  //Test code
    for (uint8_t tindx = 0; tindx <  NBR_TEST_SENSOR; tindx++ ){
        uint8_t sindx = sensor_test[tindx].sensor;
        sensor[sindx].meta.active = true;
        sensor[sindx].meta.next_send = millis() + sensor_test[tindx].interval;
    }
    #endif
    atask_add_new(&sensor_handle);  
}


//   aht20.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
//   Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
//   Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");


void sensor_read_values(uint8_t sindx)
{
    bool read_ok = true;
    float fval;

    switch(sindx) {
        case SENSOR_TYPE_UNDEFINED:
            break;
        case SENSOR_TYPE_BMP180:
            sensor[sindx].temperature = bmp180.readTemperature();
            sensor[sindx].pressure = bmp180.readPressure();
            break;
        case SENSOR_TYPE_BMP280:
            sensor[sindx].temperature = bmp280.readTemperature();
            sensor[sindx].pressure = bmp280.readPressure();
            break;
        case SENSOR_TYPE_BME680:
            if (! bme680.performReading()) { 
                read_ok = false;
            }
            else {
                sensor[sindx].temperature = bme680.temperature;
                sensor[sindx].pressure = bme680.pressure / 100.0;
                sensor[sindx].humidity = bme680.humidity;
            }
            break;
        case SENSOR_TYPE_AHT20:
            sensors_event_t humidity, temp;
            aht20.getEvent(&humidity, &temp);
            sensor[sindx].humidity = humidity.relative_humidity;
            sensor[sindx].temperature = temp.temperature;
            break;
        case SENSOR_TYPE_SHT21:
            fval = sht31.readTemperature();
            if (isnan(fval)) read_ok = false; else sensor[sindx].temperature = fval;
            fval = sht31.readHumidity();
            if (isnan(fval)) read_ok = false; else sensor[sindx].humidity = fval;
            break;
        case SENSOR_TYPE_DS18B20:
            fval = ds18b20.getTempCByIndex(0);
            if (fval != DEVICE_DISCONNECTED_C)
            {
                Serial.printf("DS18B20 Temp: %.1f\n",fval);
                sensor[sindx].temperature = fval;
            }
            else
            {
                Serial.println("Error: Could not read DS18B20");
            }
            ds18b20.requestTemperatures();  // for next reading
            break;
        case SENSOR_TYPE_PIR:
            break;

    }
    sensor[sindx].meta.next_meas = millis() + INTERVAL_READ_SENSOR;
    if (read_ok) {
        sensor[sindx].meta.updated = true;
        // if(++sensor[sindx].meta.counter > 9999) sensor[sindx].meta.counter = 0;   
        if(sensor[sindx].meta.show_temperature){
            alpha_add_float(ALPHA_CH_TEMPERATURE, sensor[sindx].temperature);
        } 
    }
    else {
        Serial.printf("!!! Sensor read failure: %s\n",sensor[sindx].meta.label);
    }

}

void sensor_pir_state_machine(void)
{
    switch (sensor_ctrl.pir_state)
    {
        case 0:
            sensor_ctrl.pir_cntr = 0;
            sensor_ctrl.pir_state = 10;
            break;
        case 10:
            if(io_pir_detected())
            sensor_ctrl.pir_state = 20;
            break;
        case 20:
            if(!io_pir_detected())
            {
                if(++sensor_ctrl.pir_cntr > 999) sensor_ctrl.pir_cntr = 0 ;
                sensor_ctrl.pir_state = 10;
                sensor_ctrl.pir_is_active = true;
            }
            break;
    }
}


void sensor_task(void)
{
    //Serial.printf("sensor_indx = %d\n", sensor_ctrl.sensor_indx );
    switch(sensor_handle.state)
    {
        case 0:
            sensor_handle.state = 10;
            sensor_ctrl.sensor_indx = 0;
            break;
        case 10: 
            if(sensor[sensor_ctrl.sensor_indx].meta.active && (millis() > sensor[sensor_ctrl.sensor_indx].meta.next_meas)) {
                sensor_read_values(sensor_ctrl.sensor_indx);
                sensor_handle.state = 20;
                sensor_ctrl.timeout = millis() + 5000;
            }
            else {
                sensor_handle.state = 100;
            }                     
            break;
        case 20:
            if (millis() > sensor_ctrl.timeout){
                sensor_handle.state = 100;
            } 
            break;
        case 100:
            if(sensor_ctrl.sensor_indx < SENSOR_TYPE_NBR_OF-1) sensor_ctrl.sensor_indx++;
            else sensor_ctrl.sensor_indx = 0;
            sensor_handle.state = 10;
            break;
    }
}

void sensor_test_task(void)
{
    uint8_t sensor_type;
    //Serial.printf("sensor_indx = %d\n", sensor_ctrl.sensor_indx );
    switch(sensor_handle.state)
    {
        case 0:
            sensor_handle.state = 10;
            sensor_ctrl.sensor_indx = 0;
            break;
        case 10: 
            sensor_type = sensor_test[sensor_ctrl.sensor_indx].sensor;
            if((millis() > sensor[sensor_type].meta.next_send)) {
                sensor[sensor_type].temperature = sensor_test[sensor_ctrl.sensor_indx].temp;
                sensor[sensor_type].meta.updated = true;
                rfm_set_sender(sensor_test[sensor_ctrl.sensor_indx].sender);
                if(sensor[sensor_type].meta.show_temperature){
                    alpha_add_float(ALPHA_CH_TEMPERATURE,sensor[sensor_type].temperature);
                } 
                if(++sensor[sensor_type].meta.counter > 9999) sensor[sensor_type].meta.counter = 0;
                if (sensor_test[sensor_ctrl.sensor_indx].going_up) sensor_test[sensor_ctrl.sensor_indx].temp += sensor_test[sensor_ctrl.sensor_indx].delta_temp;
                else sensor_test[sensor_ctrl.sensor_indx].temp -= sensor_test[sensor_ctrl.sensor_indx].delta_temp;
                if(sensor_test[sensor_ctrl.sensor_indx].temp >= sensor_test[sensor_ctrl.sensor_indx].max_temp) sensor_test[sensor_ctrl.sensor_indx].going_up =false;
                if(sensor_test[sensor_ctrl.sensor_indx].temp <= sensor_test[sensor_ctrl.sensor_indx].min_temp) sensor_test[sensor_ctrl.sensor_indx].going_up =true;
                sensor[sensor_type].meta.next_send = millis() + sensor_test[sensor_ctrl.sensor_indx].interval;
                sensor_handle.state = 20;
                sensor_ctrl.timeout = millis() + 4000;
            }
            else {
                sensor_handle.state = 100;
            }                     
            break;
        case 20:
            if (millis() > sensor_ctrl.timeout) sensor_handle.state = 100;
            break;
        case 100:
            if(sensor_ctrl.sensor_indx < NBR_TEST_SENSOR-1) sensor_ctrl.sensor_indx++;
            else sensor_ctrl.sensor_indx = 0;
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

