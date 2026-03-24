/**********************************************************************
 * RightTurnControl.cpp
 *
 * 这个文件实现右转控制模块。
 *********************************************************************/

#include "RightTurnControl.h"

RightTurnControl::RightTurnControl()
    : _motionLink(nullptr) {}

void RightTurnControl::begin(MotionLink* motionLink) {
    _motionLink = motionLink;
}

void RightTurnControl::execute() {
    if (_motionLink) {
        _motionLink->turnRight();
    }
}
