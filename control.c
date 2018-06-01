
// *******************************************************
// pidControl.h
//
// Generates serial data to print for debugging
// P.J. Bones UCECE, modified by Ryan hall
// Last modified:  18.04.2017
// Helicopter project
// Group: A03 Group 10
// *******************************************************

#include "control.h"
#include "height.h"
#include "yaw.h"
#include "landingController.h"

static int32_t outputs[CONTROL_NUM_CHANNELS] = {0};  // values to send to motor
static int32_t targets[CONTROL_NUM_CHANNELS] = {0};  // target values to compare aganst
static bool enabled[CONTROL_NUM_CHANNELS] = {0};

typedef void (*control_channel_update_func_t)(state_t*, uint32_t);

// defined later on
void updateHeightChannel(state_t* state, uint32_t deltaTime);
void updateYawChannel(state_t* state, uint32_t deltaTime);
void updateCalibrationChannelMain(state_t* state, uint32_t deltaTime);
void updateCalibrationChannelTail(state_t* state, uint32_t deltaTime);
void updateDescendingChannel(state_t* state, uint32_t deltaTime);
void updatePowerDownChannel(state_t* state, uint32_t deltaTime);

// functions which get called to update each channel
static control_channel_update_func_t chanelUpdateFuncs[CONTROL_NUM_CHANNELS] = {
updateHeightChannel, updateYawChannel, updateCalibrationChannelMain, updateCalibrationChannelTail, updateDescendingChannel, updatePowerDownChannel
};

// final output parameters (so that we may display these in main)
static int32_t mainDuty = 0, tailDuty = 0;

// measured parameters (scaled by PRECISION)
static int32_t height, previousHeight = 0, verticalVelocity;
static int32_t yaw, previousYaw = 0, angularVelocity = 0;

// configurable constants (scaled by PRECISION)
static int32_t gravOffset = 200;
// ratio of main rotor speed to tail rotor speed
static int32_t mainTorqueConst = 800;

// Eventually change this to work on generic heli
static int32_t mainGains[] = {1500, 600, 400};
static int32_t tailGains[] = {1200, 800, 500};
static int32_t mainOffset = 33;  // temporary until calibration added

enum gains_e {KP=0, KD, KI};
#define NUM_GAINS 3

int32_t clamp(int32_t pwmLevel, int32_t minLevel, int32_t maxLevel)
{
    if (pwmLevel < minLevel) {
        pwmLevel = minLevel;
    } else if (pwmLevel > maxLevel) {
        pwmLevel = maxLevel;
    }
    return pwmLevel;
}


void controlInit(void)
{
    pwmInit();
}


void controlMotorSet(bool state, pwm_channel_t channel)
{
    pwmSetOutputState(state, channel);
}


void controlEnable(control_channel_t channel)
{
    // enable only one channel
    enabled[channel] = true;

    // handle inital conditions
    switch(channel) {
    case CONTROL_POWER_DOWN:
        outputs[CONTROL_POWER_DOWN] = mainDuty;
        break;
    default:
        outputs[channel] = 0;
    }
}


void controlDisable(control_channel_t channel)
{
    // enable only one channel
    enabled[channel] = false;

    // handle ending conditions
    switch(channel) {
    case CONTROL_CALIBRATE_TAIL:
        break;
    default:
        outputs[channel] = 0;
    }
}


bool controlIsEnabled(control_channel_t channel)
{
    return enabled[channel];
}


void controlSetTarget(int32_t target, control_channel_t channel)
{
    targets[channel] = target * PRECISION;
}


int32_t controlGetPWMDuty(control_channel_t channel)
{
    switch (channel) {
    case CONTROL_HEIGHT:
        return enabled[channel] ? (mainDuty / PRECISION) : 0;
    case CONTROL_YAW:
        return enabled[channel] ? (tailDuty / PRECISION) : 0;
    case CONTROL_POWER_DOWN:
        return mainDuty / PRECISION;
    default:
        return -1;  // error
    }
}


void controlUpdate(state_t* state, uint32_t deltaTime)
{
    // get height and velocity
    height = heightAsPercentage(PRECISION);
    verticalVelocity = (height - previousHeight) * PRECISION / deltaTime;
    previousHeight = height;

    yaw = yawGetDegrees(PRECISION);
    angularVelocity = (yaw - previousYaw) * PRECISION / deltaTime;
    previousYaw = yaw;

    // call all channel update functions
    int i = 0;
    for (; i < CONTROL_NUM_CHANNELS; i++) {
        if (enabled[i]) {
            chanelUpdateFuncs[i](state, deltaTime);
        }
    }


    // main rotor equation
    mainDuty = outputs[CONTROL_CALIBRATE_MAIN] + outputs[CONTROL_HEIGHT] + outputs[CONTROL_POWER_DOWN];  // ang vel must be radians;
    mainDuty = clamp(mainDuty, MIN_DUTY * PRECISION, MAX_DUTY * PRECISION);

    // tail rotor equation
    // filter main to take into account time delay and smoothing so that we get less oscillation
    tailDuty = mainTorqueConst * mainDuty / PRECISION + outputs[CONTROL_YAW];
    tailDuty = clamp(tailDuty, MIN_DUTY * PRECISION, MAX_DUTY * PRECISION);

    // Set motor speed
    pwmSetDuty((uint32_t)mainDuty, PRECISION, MAIN_ROTOR);
    pwmSetDuty((uint32_t)tailDuty, PRECISION, TAIL_ROTOR);
}


///
/// Channel update functions
///



static int32_t inte_h = 0;
int32_t deri_h = 0, prop_h = 0;
void updateHeightChannel(state_t* state, uint32_t deltaTime)
{
    int32_t kp = mainGains[KP];
    int32_t kd = mainGains[KD];
    int32_t ki = mainGains[KI];
    int32_t error = targets[CONTROL_HEIGHT] - height;
    prop_h = kp * error / PRECISION;
    // (kp * targets[CONTROL_HEIGHT] - kp * height)

//    if (abs(error) > 25000) {
//        deri = 0;
//    } else {
        deri_h = kd * (0 - verticalVelocity) / PRECISION;
//    }

//    if (abs(error) < 1000) {
        inte_h = (inte_h * PRECISION + (ki * (int32_t)deltaTime / MS_TO_SEC * targets[CONTROL_HEIGHT] - ki * (int32_t)deltaTime / MS_TO_SEC * height)) / PRECISION;
//    }

    outputs[CONTROL_HEIGHT] = prop_h + deri_h + inte_h;
}

static int32_t inte_y = 0;
int32_t prop_y = 0, deri_y = 0;
void updateYawChannel(state_t* state, uint32_t dt)
{
    int32_t kp = tailGains[KP];
    int32_t kd = tailGains[KD];
    int32_t ki = tailGains[KI];
    int32_t error = state->targetYaw * PRECISION - yaw;
    prop_y = kp * error / PRECISION;

//    if (abs(error) < 3000 || abs(error) > 45000) {
//        deri = 0;
//    } else {
        deri_y = kd * (0 - angularVelocity) / PRECISION;
//    }

    if (abs(inte_y) < 1e8) {
        inte_y += ki * (int32_t)dt * error / MS_TO_SEC / PRECISION;
    }

    outputs[CONTROL_YAW] = prop_y + deri_y + inte_y;
}


void updateCalibrationChannelMain(state_t* state, uint32_t deltaTime)
{
    outputs[CONTROL_CALIBRATE_MAIN] = mainOffset * PRECISION + height * gravOffset / PRECISION;
}

void updateCalibrationChannelTail(state_t* state, uint32_t deltaTime)
{
    // calc mainTourqueConst here ...
    controlDisable(CONTROL_CALIBRATE_TAIL);
}

void updateDescendingChannel(state_t* state, uint32_t deltaTime)
{
//    int32_t yaw = yawGetDegrees(1);
//    int32_t height = heightAsPercentage(1);
    land(state, deltaTime, yaw);
    controlSetTarget(state->targetHeight, CONTROL_HEIGHT);
    controlSetTarget(state->targetYaw, CONTROL_YAW);
    if (checkLandingStability(state, deltaTime, yaw, height)) {
        controlDisable(CONTROL_DESCENDING);
    }
}

void updatePowerDownChannel(state_t* state, uint32_t deltaTime) {
    if (outputs[CONTROL_POWER_DOWN] >= DUTY_DECREMENT_PER_CYCLE * deltaTime) {
        outputs[CONTROL_POWER_DOWN] -= DUTY_DECREMENT_PER_CYCLE * deltaTime;
    }
}


void resetController(void) {
    prop_h = 0;
    deri_h = 0;
    inte_h = 0;

    prop_y = 0;
    deri_y = 0;
    inte_y = 0;
}
