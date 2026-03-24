/**********************************************************************
 * MotionControl.cpp
 *
 * 这个文件实现 Minima 侧的运动执行状态机。
 * 它负责把来自 ESP32 的高层命令转换成气泵和电磁阀动作。
 *********************************************************************/

// 引入运动控制类声明。
#include "MotionControl.h"

// 实现构造函数。
MotionController::MotionController() {
    // 初始化前进子系统“正在运行”标记。
    forward_active = false;
    // 初始化前进阀门“当前打开”标记。
    forward_valves_open = false;
    // 初始化前进平衡阶段标记。
    forward_balancing = false;
    // 初始化前进主相位起始时间。
    forward_phase_start = 0;
    // 初始化前进平衡起始时间。
    forward_balance_start = 0;

    // 初始化转向子系统“正在运行”标记。
    turn_active = false;
    // 初始化当前转向方向为“非左转”。
    turn_is_left = false;
    // 初始化转向平衡阶段标记。
    turn_balancing = false;
    // 初始化转向方向切换阶段标记。
    turn_switching = false;
    // 初始化转向阶段号。
    turn_stage = 0;
    // 初始化待执行转向方向为“无”。
    pending_turn_direction = TURN_NONE;
    // 初始化转向动作起始时间。
    turn_start_time = 0;
    // 初始化转向平衡起始时间。
    turn_balance_start = 0;

    // 初始化上浮动作标记。
    ascend_active = false;
    // 初始化下沉动作标记。
    descend_active = false;
    // 初始化上浮动作起始时间。
    ascend_start_time = 0;
    // 初始化下沉动作起始时间。
    descend_start_time = 0;
}

// 实现初始化函数。
void MotionController::begin() {
    // 把前进泵引脚配置为输出。
    pinMode(PUMP_A_PIN, OUTPUT);
    // 把前进阀门 B 引脚配置为输出。
    pinMode(VALVE_B_PIN, OUTPUT);
    // 把前进阀门 C 引脚配置为输出。
    pinMode(VALVE_C_PIN, OUTPUT);
    // 把转向泵引脚配置为输出。
    pinMode(PUMP_D_PIN, OUTPUT);
    // 把转向阀门 E 引脚配置为输出。
    pinMode(VALVE_E_PIN, OUTPUT);
    // 把转向阀门 F 引脚配置为输出。
    pinMode(VALVE_F_PIN, OUTPUT);
    // 把浮沉泵引脚配置为输出。
    pinMode(PUMP_G_PIN, OUTPUT);
    // 把浮沉阀门 H 引脚配置为输出。
    pinMode(VALVE_H_PIN, OUTPUT);
    // 把浮沉阀门 I 引脚配置为输出。
    pinMode(VALVE_I_PIN, OUTPUT);

    // 上电后立即执行一次急停，确保所有执行器都是关闭状态。
    emergencyStopAll();
}

// 实现状态机更新函数。
void MotionController::update() {
    // 更新前进主节拍逻辑。
    updateForward();
    // 更新前进停止后的平衡逻辑。
    updateForwardBalance();
    // 更新转向主阶段逻辑。
    updateTurn();
    // 更新转向平衡和方向切换逻辑。
    updateTurnBalance();
    // 更新浮沉定时逻辑。
    updateBuoyancy();
}

// 实现开始前进命令入口。
void MotionController::startForwardCommand() {
    // 如果前进已经在运行，或者正在做平衡，就忽略重复命令。
    if (forward_active || forward_balancing) {
        return;
    }
    // 调用真正的前进启动逻辑。
    startForward();
}

// 实现停止前进命令入口。
void MotionController::stopForwardCommand() {
    // 如果前进当前并未激活，就无需处理。
    if (!forward_active) {
        return;
    }
    // 调用真正的前进停止和平衡逻辑。
    stopForwardWithBalance();
}

// 实现兼容旧协议的前进切换入口。
void MotionController::toggleForward() {
    // 如果当前既不前进也不处于平衡阶段。
    if (!forward_active && !forward_balancing) {
        // 就启动前进。
        startForward();
    }
    // 如果当前已经处于前进状态。
    else if (forward_active) {
        // 就停止前进并进入平衡。
        stopForwardWithBalance();
    }
}

// 实现真正的前进启动逻辑。
void MotionController::startForward() {
    // 标记前进子系统为运行中。
    forward_active = true;
    // 标记当前前进阀门处于关闭状态。
    forward_valves_open = false;
    // 记录前进主相位起始时间。
    forward_phase_start = millis();

    // 打开前进主泵。
    digitalWrite(PUMP_A_PIN, HIGH);
    // 关闭前进阀门 B。
    digitalWrite(VALVE_B_PIN, LOW);
    // 关闭前进阀门 C。
    digitalWrite(VALVE_C_PIN, LOW);

    // 打印前进启动调试信息。
    Serial.println(F(">>> Forward STARTED"));
    // 打印当前执行器状态。
    Serial.println(F("    Pump A: ON, Valves B/C: OFF"));
}

// 实现真正的前进停止和平衡逻辑。
void MotionController::stopForwardWithBalance() {
    // 标记前进主阶段结束。
    forward_active = false;
    // 标记进入前进平衡阶段。
    forward_balancing = true;
    // 记录平衡起始时间。
    forward_balance_start = millis();

    // 关闭前进主泵。
    digitalWrite(PUMP_A_PIN, LOW);
    // 关闭前进阀门 B。
    digitalWrite(VALVE_B_PIN, LOW);
    // 关闭前进阀门 C。
    digitalWrite(VALVE_C_PIN, LOW);

    // 打印前进停止调试信息。
    Serial.println(F(">>> Forward STOPPING"));
    // 打印即将进入平衡阶段的说明。
    Serial.println(F("    Preparing pressure balance..."));
}

// 实现强制停止前进函数。
void MotionController::forceStopForward() {
    // 清除前进运行标记。
    forward_active = false;
    // 清除前进平衡标记。
    forward_balancing = false;
    // 关闭前进主泵。
    digitalWrite(PUMP_A_PIN, LOW);
    // 关闭前进阀门 B。
    digitalWrite(VALVE_B_PIN, LOW);
    // 关闭前进阀门 C。
    digitalWrite(VALVE_C_PIN, LOW);
}

// 实现前进主节拍更新函数。
void MotionController::updateForward() {
    // 如果前进当前没有激活，就直接返回。
    if (!forward_active) {
        return;
    }

    // 如果距离上次阀门切换已经超过设定周期。
    if (millis() - forward_phase_start >= FORWARD_VALVE_INTERVAL) {
        // 翻转阀门开关状态。
        forward_valves_open = !forward_valves_open;
        // 更新这一轮相位起始时间。
        forward_phase_start = millis();

        // 按新的状态设置阀门 B。
        digitalWrite(VALVE_B_PIN, forward_valves_open ? HIGH : LOW);
        // 按新的状态设置阀门 C。
        digitalWrite(VALVE_C_PIN, forward_valves_open ? HIGH : LOW);

        // 打印前进阀门状态提示前缀。
        Serial.print(F("  Forward: Valves B/C "));
        // 打印阀门当前是打开还是关闭。
        Serial.println(forward_valves_open ? F("OPEN") : F("CLOSED"));
    }
}

// 实现前进平衡阶段更新函数。
void MotionController::updateForwardBalance() {
    // 如果当前不在前进平衡阶段，就直接返回。
    if (!forward_balancing) {
        return;
    }

    // 计算平衡阶段已持续的时间。
    const unsigned long elapsed = millis() - forward_balance_start;

    // 如果已经达到平衡延时，但还处在短触发窗口内。
    if (elapsed >= FORWARD_BALANCE_DELAY && elapsed < FORWARD_BALANCE_DELAY + 5) {
        // 打开平衡用的阀门 B。
        digitalWrite(VALVE_B_PIN, HIGH);
        // 打印平衡阶段提示。
        Serial.println(F("    Balance: Valve B OPEN (500ms)"));
    }
    // 如果已经达到平衡结束时间。
    else if (elapsed >= FORWARD_BALANCE_DELAY + FORWARD_BALANCE_TIME) {
        // 关闭阀门 B。
        digitalWrite(VALVE_B_PIN, LOW);
        // 清除前进平衡标记。
        forward_balancing = false;
        // 打印平衡完成提示。
        Serial.println(F(">>> Forward STOPPED (balanced)"));
    }
}

// 实现读取前进激活状态函数。
bool MotionController::isForwardActive() {
    // 返回前进激活标记。
    return forward_active;
}

// 实现读取前进平衡状态函数。
bool MotionController::isForwardBalancing() {
    // 返回前进平衡标记。
    return forward_balancing;
}

// 实现读取前进阀门开关状态函数。
bool MotionController::isForwardValvesOpen() {
    // 返回当前阀门开关状态。
    return forward_valves_open;
}

// 实现开始右转命令入口。
void MotionController::startTurnRight() {
    // 如果当前正在左转。
    if (turn_active && turn_is_left) {
        // 打印方向切换提示。
        Serial.println(F(">>> Turn: Left->Right requires balance"));
        // 先执行方向切换平衡。
        startTurnSwitchBalance(TURN_RIGHT);
        // 结束本次调用。
        return;
    }

    // 如果当前处于转向平衡或切换阶段。
    if (turn_balancing || turn_switching) {
        // 记录待执行方向为右转。
        pending_turn_direction = TURN_RIGHT;
        // 结束本次调用。
        return;
    }

    // 直接执行右转启动逻辑。
    executeTurnRight();
}

// 实现开始左转命令入口。
void MotionController::startTurnLeft() {
    // 如果当前正在右转。
    if (turn_active && !turn_is_left) {
        // 打印方向切换提示。
        Serial.println(F(">>> Turn: Right->Left requires balance"));
        // 先执行方向切换平衡。
        startTurnSwitchBalance(TURN_LEFT);
        // 结束本次调用。
        return;
    }

    // 如果当前处于转向平衡或切换阶段。
    if (turn_balancing || turn_switching) {
        // 记录待执行方向为左转。
        pending_turn_direction = TURN_LEFT;
        // 结束本次调用。
        return;
    }

    // 直接执行左转启动逻辑。
    executeTurnLeft();
}

// 实现停止转向命令入口。
void MotionController::stopTurnCommand() {
    // 如果当前确实在转向。
    if (turn_active) {
        // 执行转向停止和平衡逻辑。
        stopTurnWithBalance();
    }
}

// 实现真正的右转启动逻辑。
void MotionController::executeTurnRight() {
    // 标记转向子系统处于激活状态。
    turn_active = true;
    // 标记当前方向不是左转，即右转。
    turn_is_left = false;
    // 把转向阶段重置为 0。
    turn_stage = 0;
    // 记录转向起始时间。
    turn_start_time = millis();

    // 打开转向主泵。
    digitalWrite(PUMP_D_PIN, HIGH);
    // 关闭阀门 E。
    digitalWrite(VALVE_E_PIN, LOW);
    // 关闭阀门 F。
    digitalWrite(VALVE_F_PIN, LOW);

    // 打印右转启动提示。
    Serial.println(F(">>> Turn RIGHT STARTED"));
    // 打印阶段说明。
    Serial.println(F("    Stage 0: Pump D ON, Valves E/F OFF (1000ms)"));
}

// 实现真正的左转启动逻辑。
void MotionController::executeTurnLeft() {
    // 标记转向子系统处于激活状态。
    turn_active = true;
    // 标记当前方向为左转。
    turn_is_left = true;
    // 把转向阶段重置为 0。
    turn_stage = 0;
    // 记录转向起始时间。
    turn_start_time = millis();

    // 打开转向主泵。
    digitalWrite(PUMP_D_PIN, HIGH);
    // 打开阀门 E。
    digitalWrite(VALVE_E_PIN, HIGH);
    // 打开阀门 F。
    digitalWrite(VALVE_F_PIN, HIGH);

    // 打印左转启动提示。
    Serial.println(F(">>> Turn LEFT STARTED"));
    // 打印阶段说明。
    Serial.println(F("    Stage 0: Pump D ON, Valves E/F ON (1000ms)"));
}

// 实现方向切换平衡逻辑。
void MotionController::startTurnSwitchBalance(uint8_t new_direction) {
    // 关闭转向主泵。
    digitalWrite(PUMP_D_PIN, LOW);
    // 关闭阀门 E。
    digitalWrite(VALVE_E_PIN, LOW);
    // 关闭阀门 F。
    digitalWrite(VALVE_F_PIN, LOW);

    // 清除当前转向激活状态。
    turn_active = false;
    // 标记进入方向切换阶段。
    turn_switching = true;
    // 保存切换完成后要执行的新方向。
    pending_turn_direction = new_direction;
    // 记录切换平衡开始时间。
    turn_balance_start = millis();

    // 打印方向切换平衡启动提示。
    Serial.println(F("    Stage: Switch balance starting..."));
}

// 实现转向停止和平衡逻辑。
void MotionController::stopTurnWithBalance() {
    // 清除当前转向激活状态。
    turn_active = false;
    // 标记进入转向平衡阶段。
    turn_balancing = true;
    // 把阶段号设置为 1，表示等待平衡开始。
    turn_stage = 1;
    // 记录平衡起始时间。
    turn_balance_start = millis();

    // 关闭转向主泵。
    digitalWrite(PUMP_D_PIN, LOW);
    // 关闭阀门 E。
    digitalWrite(VALVE_E_PIN, LOW);
    // 关闭阀门 F。
    digitalWrite(VALVE_F_PIN, LOW);

    // 打印转向停止提示。
    Serial.println(F("    Stage 1: All OFF, preparing balance..."));
}

// 实现强制停止转向函数。
void MotionController::forceStopTurn() {
    // 清除转向激活标记。
    turn_active = false;
    // 清除转向平衡标记。
    turn_balancing = false;
    // 清除方向切换标记。
    turn_switching = false;
    // 关闭转向主泵。
    digitalWrite(PUMP_D_PIN, LOW);
    // 关闭阀门 E。
    digitalWrite(VALVE_E_PIN, LOW);
    // 关闭阀门 F。
    digitalWrite(VALVE_F_PIN, LOW);
}

// 实现转向主阶段更新函数。
void MotionController::updateTurn() {
    // 如果当前没有在转向，就直接返回。
    if (!turn_active) {
        return;
    }

    // 计算转向已持续时间。
    const unsigned long elapsed = millis() - turn_start_time;
    // 如果仍处于阶段 0，且转向时间已经达到设定值。
    if (turn_stage == 0 && elapsed >= TURN_DURATION) {
        // 结束主转向阶段并进入平衡。
        stopTurnWithBalance();
    }
}

// 实现转向平衡和方向切换更新函数。
void MotionController::updateTurnBalance() {
    // 如果当前处于方向切换阶段。
    if (turn_switching) {
        // 计算方向切换阶段已持续时间。
        const unsigned long elapsed = millis() - turn_balance_start;

        // 如果已经达到平衡延时，但还在短触发窗口内。
        if (elapsed >= TURN_BALANCE_DELAY && elapsed < TURN_BALANCE_DELAY + 5) {
            // 打开阀门 E 做切换平衡。
            digitalWrite(VALVE_E_PIN, HIGH);
            // 打印切换平衡提示。
            Serial.println(F("    Switch balance: Valve E ON (200ms)"));
        }
        // 如果已经达到切换平衡结束时间。
        else if (elapsed >= TURN_BALANCE_DELAY + TURN_SWITCH_BALANCE_TIME) {
            // 关闭阀门 E。
            digitalWrite(VALVE_E_PIN, LOW);
            // 清除方向切换标记。
            turn_switching = false;

            // 打印切换平衡完成提示。
            Serial.println(F("    Switch balance complete"));

            // 如果待执行方向是右转。
            if (pending_turn_direction == TURN_RIGHT) {
                // 立即启动右转。
                executeTurnRight();
            }
            // 如果待执行方向是左转。
            else if (pending_turn_direction == TURN_LEFT) {
                // 立即启动左转。
                executeTurnLeft();
            }
            // 清空待执行方向。
            pending_turn_direction = TURN_NONE;
        }
        // 方向切换路径处理完后直接返回。
        return;
    }

    // 如果当前处于普通转向平衡阶段。
    if (turn_balancing) {
        // 计算平衡阶段已持续时间。
        const unsigned long elapsed = millis() - turn_balance_start;

        // 如果当前还处于阶段 1，且到达平衡延时。
        if (turn_stage == 1 && elapsed >= TURN_BALANCE_DELAY) {
            // 打开阀门 E 进行平衡。
            digitalWrite(VALVE_E_PIN, HIGH);
            // 把阶段号推进到 2。
            turn_stage = 2;
            // 打印平衡阶段提示。
            Serial.println(F("    Stage 2: Valve E ON (200ms balance)"));
        }
        // 如果当前处于阶段 2，且平衡持续时间已经达到。
        else if (turn_stage == 2 && elapsed >= TURN_BALANCE_DELAY + TURN_BALANCE_TIME) {
            // 关闭阀门 E。
            digitalWrite(VALVE_E_PIN, LOW);
            // 清除转向平衡标记。
            turn_balancing = false;
            // 把阶段号推进到 3。
            turn_stage = 3;

            // 打印完成提示前缀。
            Serial.print(F(">>> Turn "));
            // 根据方向打印 LEFT 或 RIGHT。
            Serial.print(turn_is_left ? F("LEFT") : F("RIGHT"));
            // 打印完成后缀。
            Serial.println(F(" COMPLETE (balanced)"));
        }
    }
}

// 实现读取转向激活状态函数。
bool MotionController::isTurnActive() {
    // 返回转向激活标记。
    return turn_active;
}

// 实现读取转向是否处于平衡相关阶段的函数。
bool MotionController::isTurnBalancing() {
    // 只要转向平衡或方向切换任一激活，就返回 true。
    return turn_balancing || turn_switching;
}

// 实现读取当前是否左转的函数。
bool MotionController::isTurnLeft() {
    // 返回当前方向标记。
    return turn_is_left;
}

// 实现读取转向阶段号的函数。
uint8_t MotionController::getTurnStage() {
    // 返回当前阶段号。
    return turn_stage;
}

// 实现开始上浮命令入口。
void MotionController::startAscend() {
    // 先停止当前任何浮沉动作。
    stopBuoyancy();

    // 标记上浮动作激活。
    ascend_active = true;
    // 记录上浮起始时间。
    ascend_start_time = millis();

    // 打开浮沉泵。
    digitalWrite(PUMP_G_PIN, HIGH);
    // 关闭阀门 H。
    digitalWrite(VALVE_H_PIN, LOW);
    // 关闭阀门 I。
    digitalWrite(VALVE_I_PIN, LOW);

    // 打印上浮启动提示。
    Serial.println(F(">>> ASCEND started"));
    // 打印当前执行器状态。
    Serial.println(F("    Pump G: ON, Valves H/I: OFF (1500ms)"));
}

// 实现开始下沉命令入口。
void MotionController::startDescend() {
    // 先停止当前任何浮沉动作。
    stopBuoyancy();

    // 标记下沉动作激活。
    descend_active = true;
    // 记录下沉起始时间。
    descend_start_time = millis();

    // 打开浮沉泵。
    digitalWrite(PUMP_G_PIN, HIGH);
    // 打开阀门 H。
    digitalWrite(VALVE_H_PIN, HIGH);
    // 打开阀门 I。
    digitalWrite(VALVE_I_PIN, HIGH);

    // 打印下沉启动提示。
    Serial.println(F(">>> DESCEND started"));
    // 打印当前执行器状态。
    Serial.println(F("    Pump G: ON, Valves H/I: ON (1500ms)"));
}

// 实现停止浮沉函数。
void MotionController::stopBuoyancy() {
    // 清除上浮标记。
    ascend_active = false;
    // 清除下沉标记。
    descend_active = false;
    // 关闭浮沉泵。
    digitalWrite(PUMP_G_PIN, LOW);
    // 关闭阀门 H。
    digitalWrite(VALVE_H_PIN, LOW);
    // 关闭阀门 I。
    digitalWrite(VALVE_I_PIN, LOW);
}

// 实现浮沉阶段更新函数。
void MotionController::updateBuoyancy() {
    // 如果当前上浮动作处于激活状态。
    if (ascend_active) {
        // 如果上浮持续时间已经达到设定值。
        if (millis() - ascend_start_time >= ASCEND_DURATION) {
            // 关闭浮沉泵。
            digitalWrite(PUMP_G_PIN, LOW);
            // 关闭阀门 H。
            digitalWrite(VALVE_H_PIN, LOW);
            // 关闭阀门 I。
            digitalWrite(VALVE_I_PIN, LOW);
            // 清除上浮标记。
            ascend_active = false;
            // 打印上浮完成提示。
            Serial.println(F(">>> ASCEND complete"));
        }
    }

    // 如果当前下沉动作处于激活状态。
    if (descend_active) {
        // 如果下沉持续时间已经达到设定值。
        if (millis() - descend_start_time >= DESCEND_DURATION) {
            // 关闭浮沉泵。
            digitalWrite(PUMP_G_PIN, LOW);
            // 关闭阀门 H。
            digitalWrite(VALVE_H_PIN, LOW);
            // 关闭阀门 I。
            digitalWrite(VALVE_I_PIN, LOW);
            // 清除下沉标记。
            descend_active = false;
            // 打印下沉完成提示。
            Serial.println(F(">>> DESCEND complete"));
        }
    }
}

// 实现读取上浮状态函数。
bool MotionController::isAscendActive() {
    // 返回上浮标记。
    return ascend_active;
}

// 实现读取下沉状态函数。
bool MotionController::isDescendActive() {
    // 返回下沉标记。
    return descend_active;
}

// 实现全局急停函数。
void MotionController::emergencyStopAll() {
    // 强制停止前进子系统。
    forceStopForward();
    // 强制停止转向子系统。
    forceStopTurn();
    // 停止浮沉子系统。
    stopBuoyancy();

    // 打印急停提示。
    Serial.println(F("\n>>> EMERGENCY STOP - All systems OFF"));
}

// 实现读取任一子系统是否活跃的函数。
bool MotionController::isAnyActive() {
    // 只要任一子系统正在运行或处于平衡阶段，就返回 true。
    return forward_active || turn_active || ascend_active || descend_active ||
           forward_balancing || turn_balancing || turn_switching;
}

// 实现打印当前状态函数。
void MotionController::printStatus() {
    // 打印状态前缀。
    Serial.print(F("Status: "));

    // 如果前进子系统正在运行。
    if (forward_active) {
        // 打印前进前缀。
        Serial.print(F("FWD"));
        // 根据阀门状态打印不同后缀。
        Serial.print(forward_valves_open ? F("(V+) ") : F("(V-) "));
    }
    // 如果前进子系统处于平衡阶段。
    if (forward_balancing) {
        // 打印前进平衡标记。
        Serial.print(F("FWD-BAL "));
    }

    // 如果转向子系统正在运行。
    if (turn_active) {
        // 根据方向打印左转或右转标记。
        Serial.print(turn_is_left ? F("TURN-L ") : F("TURN-R "));
    }
    // 如果转向子系统处于平衡阶段。
    if (turn_balancing) {
        // 打印转向平衡标记。
        Serial.print(F("TURN-BAL "));
    }
    // 如果当前处于转向切换平衡阶段。
    if (turn_switching) {
        // 打印转向切换标记。
        Serial.print(F("TURN-SW "));
    }

    // 如果当前上浮动作激活。
    if (ascend_active) {
        // 打印上浮标记。
        Serial.print(F("ASCEND "));
    }
    // 如果当前下沉动作激活。
    if (descend_active) {
        // 打印下沉标记。
        Serial.print(F("DESCEND "));
    }

    // 如果当前没有任何子系统活跃。
    if (!isAnyActive()) {
        // 打印空闲状态。
        Serial.print(F("IDLE"));
    }

    // 打印行结束。
    Serial.println();
}
