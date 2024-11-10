#include <Arduino.h>
#include "ads1115.h"

seismometer::ADS1115Sensor sensor(100);

void setup() {
    Serial.begin(9600);
    sensor.begin();
}

void loop() {
    sensor.get_acceleration();
    float scale = sensor.get_seismic_scale();
    Serial.printf(">scale: %f;\n", scale);
    sensor.wait_for_next();
}
