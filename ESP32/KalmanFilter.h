/**********************************************************************
 * KalmanFilter.h
 *
 * 这个文件声明二阶运动学卡尔曼滤波器。
 *********************************************************************/

#ifndef ESP32_KALMAN_FILTER_H
#define ESP32_KALMAN_FILTER_H

#include <Arduino.h>

class KalmanFilter {
public:
    KalmanFilter();

    void reset(float position = 0.0f, float velocity = 0.0f);
    void update(float measurement, float dt);

    float getPosition() const;
    float getVelocity() const;

private:
    float _x[2];
    float _p[2][2];
    float _q[2][2];
    float _r;
};

#endif // ESP32_KALMAN_FILTER_H
