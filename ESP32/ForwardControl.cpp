/**********************************************************************
 * ForwardControl.cpp
 *
 * 这个文件实现前进控制模块。
 *********************************************************************/

#include "ForwardControl.h"

ForwardControl::ForwardControl()
    : _motionLink(nullptr), _active(false) {}

void ForwardControl::begin(MotionLink* motionLink) {
    _motionLink = motionLink;
}

void ForwardControl::start() {
    if (_motionLink && !_active) {
        _motionLink->startForward();
        _active = true;
    }
}

void ForwardControl::stop() {
    if (_motionLink && _active) {
        _motionLink->stopForward();
        _active = false;
    }
}

bool ForwardControl::isActive() const {
    return _active;
}
