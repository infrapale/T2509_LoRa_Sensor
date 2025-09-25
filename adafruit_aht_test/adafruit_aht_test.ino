#include <Adafruit_AHTX0.h>
#define PIN_I2C_SDA         (4u)
#define PIN_I2C_SCL         (5u)

TwoWire *Wirep;
Adafruit_AHTX0 aht;


void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit AHT10/AHT20 demo!");
  Wirep = &Wire;
  Wirep->setSDA(PIN_I2C_SDA);
  Wirep->setSCL(PIN_I2C_SCL);
  if (! aht.begin(Wirep)) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

  delay(500);
}