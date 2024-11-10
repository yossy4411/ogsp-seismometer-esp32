#include "ads1115.h"

namespace seismometer {
    void ADS1115Sensor::begin()
    {
        Wire.begin();
        ADS.begin();
        ADS.setDataRate(7);
        ADS.setGain(1);  // 4.096V
    }

    void ADS1115Sensor::get_acceleration()
    {
        last_time = millis();
        
        int16_t val_x = ADS.readADC(0);
        int16_t val_y = ADS.readADC(1);
        int16_t val_z = ADS.readADC(2);

        float factor = ADS.toVoltage();
        float voltage_x = factor * val_x;
        float voltage_y = factor * val_y;
        float voltage_z = factor * val_z;
        
        // オフセットは1.65V, 660mV/gの場合
        float sensorOffset = 1.65, sensorSensitivity = 0.66;
        float g = 980.665;  // 1G = 1000mg = 980.665cm/s^2 = 980.665gal (重力加速度)
        Acceleration acc = {
            (voltage_x - sensorOffset) / sensorSensitivity * g,
            (voltage_y - sensorOffset) / sensorSensitivity * g,
            (voltage_z - sensorOffset) / sensorSensitivity * g
        };  // gal

        index = (index + 1) % offset_length;
        float lengthF = offset_length;

        sum += (acc - offset[index]) / lengthF;

        offset[index] = acc;
        acc -= sum;  // オフセット補正 これは低周波ノイズを除去するため
        scaleCache = (scaleCache * 0.9) + (acc * 0.1);  // 高周波ノイズを除去するために移動平均を取る
        acceleration.push_back(scaleCache);

        if (acceleration.size() > sampling_rate) {
            acceleration.erase(acceleration.begin());
        }

        Serial.print(">x:"); Serial.println(scaleCache.x);
        Serial.print(">y:"); Serial.println(scaleCache.y);
        Serial.print(">z:"); Serial.println(scaleCache.z);
    }

    void ADS1115Sensor::wait_for_next()
    {
        unsigned long current_time = millis();
        unsigned int a = 1000 / sampling_rate;
        if (last_time + a > current_time) {
            delay(last_time + a - current_time);
        }
    }

    float ADS1115Sensor::get_seismic_scale()
    {
        // 加速度のパワーを計算
        std::vector<float> power;
        for (auto &acc : acceleration) {
            power.push_back(acc.magnitude());
        }

        // ソート (昇順)
        std::sort(power.begin(), power.end());

        // 降順0.3sを取得
        int length = power.size();
        int index = length * 0.3;
        float power_03s = power[index];

        // 0.3sの加速度を震度に変換
        // 震度[Sjma] = 2log(A)+0.94
        return 2 * log10(power_03s) + 0.94;

    }
} // namespace seismometer