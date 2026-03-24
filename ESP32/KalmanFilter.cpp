/**********************************************************************
 * KalmanFilter.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇浜岄樁杩愬姩瀛﹀崱灏旀浖婊ゆ尝鍣ㄣ€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include "KalmanFilter.h"
#include "KalmanFilter.h"

// 中文逐行说明：下面这一行保留原始代码 -> KalmanFilter::KalmanFilter() {
KalmanFilter::KalmanFilter() {
    // 中文逐行说明：下面这一行保留原始代码 -> _q[0][0] = 0.01f;
    _q[0][0] = 0.01f;
    // 中文逐行说明：下面这一行保留原始代码 -> _q[0][1] = 0.0f;
    _q[0][1] = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _q[1][0] = 0.0f;
    _q[1][0] = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _q[1][1] = 0.05f;
    _q[1][1] = 0.05f;
    // 中文逐行说明：下面这一行保留原始代码 -> _r = 0.5f;
    _r = 0.5f;
    // 中文逐行说明：下面这一行保留原始代码 -> reset();
    reset();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void KalmanFilter::reset(float position, float velocity) {
void KalmanFilter::reset(float position, float velocity) {
    // 中文逐行说明：下面这一行保留原始代码 -> _x[0] = position;
    _x[0] = position;
    // 中文逐行说明：下面这一行保留原始代码 -> _x[1] = velocity;
    _x[1] = velocity;
    // 中文逐行说明：下面这一行保留原始代码 -> _p[0][0] = 1.0f;
    _p[0][0] = 1.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _p[0][1] = 0.0f;
    _p[0][1] = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _p[1][0] = 0.0f;
    _p[1][0] = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _p[1][1] = 1.0f;
    _p[1][1] = 1.0f;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void KalmanFilter::update(float measurement, float dt) {
void KalmanFilter::update(float measurement, float dt) {
    // 中文逐行说明：下面这一行保留原始代码 -> float xPred[2];
    float xPred[2];
    // 中文逐行说明：下面这一行保留原始代码 -> xPred[0] = _x[0] + _x[1] * dt;
    xPred[0] = _x[0] + _x[1] * dt;
    // 中文逐行说明：下面这一行保留原始代码 -> xPred[1] = _x[1];
    xPred[1] = _x[1];

    // 中文逐行说明：下面这一行保留原始代码 -> float pPred[2][2];
    float pPred[2][2];
    // 中文逐行说明：下面这一行保留原始代码 -> pPred[0][0] = _p[0][0] + dt * (_p[1][0] + _p[0][1]) + dt * dt * _p[1][1] + _q[0][0];
    pPred[0][0] = _p[0][0] + dt * (_p[1][0] + _p[0][1]) + dt * dt * _p[1][1] + _q[0][0];
    // 中文逐行说明：下面这一行保留原始代码 -> pPred[0][1] = _p[0][1] + dt * _p[1][1] + _q[0][1];
    pPred[0][1] = _p[0][1] + dt * _p[1][1] + _q[0][1];
    // 中文逐行说明：下面这一行保留原始代码 -> pPred[1][0] = _p[1][0] + dt * _p[1][1] + _q[1][0];
    pPred[1][0] = _p[1][0] + dt * _p[1][1] + _q[1][0];
    // 中文逐行说明：下面这一行保留原始代码 -> pPred[1][1] = _p[1][1] + _q[1][1];
    pPred[1][1] = _p[1][1] + _q[1][1];

    // 中文逐行说明：下面这一行保留原始代码 -> float innovation = measurement - xPred[0];
    float innovation = measurement - xPred[0];
    // 中文逐行说明：下面这一行保留原始代码 -> float s = pPred[0][0] + _r;
    float s = pPred[0][0] + _r;
    // 中文逐行说明：下面这一行保留原始代码 -> float k[2];
    float k[2];
    // 中文逐行说明：下面这一行保留原始代码 -> k[0] = pPred[0][0] / s;
    k[0] = pPred[0][0] / s;
    // 中文逐行说明：下面这一行保留原始代码 -> k[1] = pPred[1][0] / s;
    k[1] = pPred[1][0] / s;

    // 中文逐行说明：下面这一行保留原始代码 -> _x[0] = xPred[0] + k[0] * innovation;
    _x[0] = xPred[0] + k[0] * innovation;
    // 中文逐行说明：下面这一行保留原始代码 -> _x[1] = xPred[1] + k[1] * innovation;
    _x[1] = xPred[1] + k[1] * innovation;

    // 中文逐行说明：下面这一行保留原始代码 -> _p[0][0] = (1.0f - k[0]) * pPred[0][0];
    _p[0][0] = (1.0f - k[0]) * pPred[0][0];
    // 中文逐行说明：下面这一行保留原始代码 -> _p[0][1] = (1.0f - k[0]) * pPred[0][1];
    _p[0][1] = (1.0f - k[0]) * pPred[0][1];
    // 中文逐行说明：下面这一行保留原始代码 -> _p[1][0] = pPred[1][0] - k[1] * pPred[0][0];
    _p[1][0] = pPred[1][0] - k[1] * pPred[0][0];
    // 中文逐行说明：下面这一行保留原始代码 -> _p[1][1] = pPred[1][1] - k[1] * pPred[0][1];
    _p[1][1] = pPred[1][1] - k[1] * pPred[0][1];
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> float KalmanFilter::getPosition() const {
float KalmanFilter::getPosition() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _x[0];
    return _x[0];
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> float KalmanFilter::getVelocity() const {
float KalmanFilter::getVelocity() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _x[1];
    return _x[1];
// 中文逐行说明：下面这一行保留原始代码 -> }
}
