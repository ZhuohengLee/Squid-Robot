/**********************************************************************
 * KalmanFilter.h
 *
 * 杩欎釜鏂囦欢澹版槑浜岄樁杩愬姩瀛﹀崱灏旀浖婊ゆ尝鍣ㄣ€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef ESP32_KALMAN_FILTER_H
#ifndef ESP32_KALMAN_FILTER_H
// 中文逐行说明：下面这一行保留原始代码 -> #define ESP32_KALMAN_FILTER_H
#define ESP32_KALMAN_FILTER_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>

// 中文逐行说明：下面这一行保留原始代码 -> class KalmanFilter {
class KalmanFilter {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> KalmanFilter();
    KalmanFilter();

    // 中文逐行说明：下面这一行保留原始代码 -> void reset(float position = 0.0f, float velocity = 0.0f);
    void reset(float position = 0.0f, float velocity = 0.0f);
    // 中文逐行说明：下面这一行保留原始代码 -> void update(float measurement, float dt);
    void update(float measurement, float dt);

    // 中文逐行说明：下面这一行保留原始代码 -> float getPosition() const;
    float getPosition() const;
    // 中文逐行说明：下面这一行保留原始代码 -> float getVelocity() const;
    float getVelocity() const;

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 中文逐行说明：下面这一行保留原始代码 -> float _x[2];
    float _x[2];
    // 中文逐行说明：下面这一行保留原始代码 -> float _p[2][2];
    float _p[2][2];
    // 中文逐行说明：下面这一行保留原始代码 -> float _q[2][2];
    float _q[2][2];
    // 中文逐行说明：下面这一行保留原始代码 -> float _r;
    float _r;
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // ESP32_KALMAN_FILTER_H
#endif // ESP32_KALMAN_FILTER_H
