
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
#include "display.h"

static int32_t outputs[CONTROL_NUM_CHANNELS] = {};  // values to send to motor
static int32_t targets[CONTROL_NUM_CHANNELS] = {};  // target values to compare aganst
static bool enabled[CONTROL_NUM_CHANNELS] = {};

static bool landingFlag = 0;

typedef void (*control_channel_update_func_t)(uint32_t);

// defined later on
void updateHeightChannel(uint32_t deltaTime);
void updateYawChannel(uint32_t deltaTime);
void updateCalibrationChannelMain(uint32_t deltaTime);
void updateCalibrationChannelTail(uint32_t deltaTime);

// functions which get called to update each channel
static control_channel_update_func_t chanelUpdateFuncs[CONTROL_NUM_CHANNELS] = {
    updateHeightChannel, updateYawChannel, updateCalibrationChannelMain, updateCalibrationChannelTail
};

// final output parameters (so that we may display these in main)
static int32_t mainDuty = 0, tailDuty = 0;

// measured parameters (scaled by PRECISION)
static int32_t height, previousHeight = 0, verticalVelocity;
static int32_t yaw, previousYaw = 0, angularVelocity = 0;

// configurable constants (scaled by PRECISION)
static int32_t gravOffsets[] = {250, 190};
// ratio of main rotor speed to tail rotor speed
static int32_t mainTorqueConsts[] = {1000, 800};

enum gains_e {KP=0, KD, KI};
enum heli_e {HELI_1=0, HELI_2};
#define NUM_GAINS 3
#define CURRENT_HELI HELI_2

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
    // add here ...
    default:
        outputs[channel] = 0;
        targets[channel] = 0;
    }
}


void controlDisable(control_channel_t channel)
{
    // enable only one channel
    enabled[channel] = false;

    // handle ending conditions
    switch(channel) {
    case CONTROL_CALIBRATE_MAIN:
        break;
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
    if (channel == CONTROL_HEIGHT) {
        return enabled[channel] ? (mainDuty / PRECISION) : 0;
    } else if (channel == CONTROL_YAW) {
        return enabled[channel] ? (tailDuty / PRECISION) : 0;
    }
    return -1;  // error
}


void controlUpdate(uint32_t deltaTime)
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
            chanelUpdateFuncs[i](deltaTime);
        }
    }

    if (!landingFlag) {
    // main rotor equation
        mainDuty = outputs[CONTROL_CALIBRATE_MAIN] + height * gravOffsets[CURRENT_HELI] / PRECISION +
            /*angularVelocity +*/ outputs[CONTROL_HEIGHT];  // ang vel must be radians;
    } else {
        if (mainDuty > LANDING_DUTY) {
            mainDuty = mainDuty - LANDING_DECREMENT;
        }
    }
    mainDuty = clamp(mainDuty, MIN_DUTY * PRECISION, MAX_DUTY * PRECISION);

    // tail rotor equation
    tailDuty = mainTorqueConsts[CURRENT_HELI] * mainDuty / PRECISION + outputs[CONTROL_YAW];
    tailDuty = clamp(tailDuty, MIN_DUTY * PRECISION, MAX_DUTY * PRECISION);

    // Set motor speed
    pwmSetDuty((uint32_t)mainDuty, PRECISION, MAIN_ROTOR);
    pwmSetDuty((uint32_t)tailDuty, PRECISION, TAIL_ROTOR);
}


///
/// Channel update functions
///

// Eventually change this to work on generic heli
static int32_t mainGains[][NUM_GAINS] = {
    {2500, 500, 800}, //2500
    {1500, 400, 500}
//    {1500, 200, 500},
//    {1500, 800, 500}
};
static int32_t tailGains[][NUM_GAINS] = {
    {2000, 500, 200},
    {1500, 400, 500}
//    {2000, 0, 500},
//    {500, 0, 500}
};
static int32_t mainOffsets[] = {37, 40};  // temporary until calibration added

static int32_t inte_h = 0;
int32_t deri_h = 0, prop_h = 0;
void updateHeightChannel(uint32_t deltaTime)
{
    int32_t kp = mainGains[CURRENT_HELI][KP];
    int32_t kd = mainGains[CURRENT_HELI][KD];
    int32_t ki = mainGains[CURRENT_HELI][KI];
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
void updateYawChannel(uint32_t dt)
{
    int32_t kp = tailGains[CURRENT_HELI][KP];
    int32_t kd = tailGains[CURRENT_HELI][KD];
    int32_t ki = tailGains[CURRENT_HELI][KI];
    int32_t error = targets[CONTROL_YAW] - yaw;
    prop_y = kp * error / PRECISION;

//    if (abs(error) < 3000 || abs(error) > 45000) {
//        deri = 0;
//    } else {
        deri_y = kd * (0 - angularVelocity) / PRECISION;
//    }

//    if (abs(error) < 1000) {
        inte_y = (inte_y * PRECISION + (ki * (int32_t)dt / MS_TO_SEC * targets[CONTROL_YAW] - ki * (int32_t)dt / MS_TO_SEC * yaw)) / PRECISION;
//    }

    outputs[CONTROL_YAW] = prop_y + deri_y + inte_y;
}


void updateCalibrationChannelMain(uint32_t deltaTime)
{
    outputs[CONTROL_CALIBRATE_MAIN] = mainOffsets[CURRENT_HELI] * PRECISION;
    controlDisable(CONTROL_CALIBRATE_MAIN);
}

void updateCalibrationChannelTail(uint32_t deltaTime)
{
    // calc mainTourqueConst here ...
    controlDisable(CONTROL_CALIBRATE_TAIL);
}

void controlSetLandingSequence(bool state) {
    landingFlag = state;
}

bool controlLandingStability(void) {
    mainDuty = mainDuty - LANDING_DECREMENT;
    mainDuty = clamp(mainDuty, MIN_DUTY * PRECISION, MAX_DUTY * PRECISION);
    return mainDuty <= MIN_DUTY * PRECISION;
}

void resetController(void) {
    prop_h = 0;
    deri_h = 0;
    inte_h = 0;

    prop_y = 0;
    deri_y = 0;
    inte_y = 0;
}
