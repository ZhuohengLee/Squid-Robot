/**********************************************************************
 * LeftTurnControl.cpp
 *
 * 这个文件实现左转控制模块。
 *********************************************************************/

#include "LeftTurnControl.h"

LeftTurnControl::LeftTurnControl()
    : _motionLink(nullptr) {}

void LeftTurnControl::begin(MotionLink* motionLink) {
    _motionLink = motionLink;
}

void LeftTurnControl::execute() {
    if (_motionLink) {
        _motionLink->turnLeft();
    }
}
