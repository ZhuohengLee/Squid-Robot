/**********************************************************************
 * KalmanFilter.cpp
 *
 * 这个文件实现二阶运动学卡尔曼滤波器。
 *********************************************************************/

#include "KalmanFilter.h"

KalmanFilter::KalmanFilter() {
    _q[0][0] = 0.01f;
    _q[0][1] = 0.0f;
    _q[1][0] = 0.0f;
    _q[1][1] = 0.05f;
    _r = 0.5f;
    reset();
}

void KalmanFilter::reset(float position, float velocity) {
    _x[0] = position;
    _x[1] = velocity;
    _p[0][0] = 1.0f;
    _p[0][1] = 0.0f;
    _p[1][0] = 0.0f;
    _p[1][1] = 1.0f;
}

void KalmanFilter::update(float measurement, float dt) {
    float xPred[2];
    xPred[0] = _x[0] + _x[1] * dt;
    xPred[1] = _x[1];

    float pPred[2][2];
    pPred[0][0] = _p[0][0] + dt * (_p[1][0] + _p[0][1]) + dt * dt * _p[1][1] + _q[0][0];
    pPred[0][1] = _p[0][1] + dt * _p[1][1] + _q[0][1];
    pPred[1][0] = _p[1][0] + dt * _p[1][1] + _q[1][0];
    pPred[1][1] = _p[1][1] + _q[1][1];

    float innovation = measurement - xPred[0];
    float s = pPred[0][0] + _r;
    float k[2];
    k[0] = pPred[0][0] / s;
    k[1] = pPred[1][0] / s;

    _x[0] = xPred[0] + k[0] * innovation;
    _x[1] = xPred[1] + k[1] * innovation;

    _p[0][0] = (1.0f - k[0]) * pPred[0][0];
    _p[0][1] = (1.0f - k[0]) * pPred[0][1];
    _p[1][0] = pPred[1][0] - k[1] * pPred[0][0];
    _p[1][1] = pPred[1][1] - k[1] * pPred[0][1];
}

float KalmanFilter::getPosition() const {
    return _x[0];
}

float KalmanFilter::getVelocity() const {
    return _x[1];
}
