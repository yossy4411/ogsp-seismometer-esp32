#pragma once

#include "seismometer.h"
#include <Wire.h>
#include "ADS1X15.h"

namespace seismometer {
    class ADS1115Sensor : public Seismometer
    {
    private:
        ADS1115 ADS;
        int offset_length;
        int index = 0;
        Acceleration scaleCache;
        Acceleration* offset;
        Acceleration sum = {0, 0, 0};
        unsigned long last_time = 0;
    public:
        ADS1115Sensor(uint8_t sampling_rate, uint8_t address = ADS1115_ADDRESS) 
            : Seismometer(sampling_rate), ADS(address), offset_length(sampling_rate * 3), offset(new Acceleration[offset_length])
        {
            for (int i = 0; i < offset_length; i++) {
                offset[i] = {0, 0, 0};
            }
        }
        ~ADS1115Sensor() {
            delete[] offset;
        }
        void begin() override;
        void get_acceleration() override;
        void wait_for_next() override;

        // 震度を取得する
        float get_seismic_scale() override;
    };
} // namespace seismometer