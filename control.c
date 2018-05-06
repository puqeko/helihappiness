
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
#include "pwmModule.h"
#include "height.h"
#include "yaw.h"
#include "display.h"

static int32_t outputs[CONTROL_NUM_CHANNELS] = {};  // values to send to motor
static int32_t targets[CONTROL_NUM_CHANNELS] = {};  // target values to compare aganst
static bool enabled[CONTROL_NUM_CHANNELS] = {};

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
static int32_t height;  //, previousHeight = 0, verticalVelocity;
static int32_t yaw, previousYaw = 0, angularVelocity;

// configurable constants (scaled by PRECISION)
static int32_t gavitationalOffsetHeightCorrectionFactor = 50;
static int32_t mainRotorTorqueConstant = 0;

static int32_t mainGains[] = {1500, 0, 5};
static int32_t tailGains[] = {1000, 0, 1};

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


void controlMotorSet(bool state)
{
    pwmSetOutputState(state, MAIN_ROTOR);
    pwmSetOutputState(state, TAIL_ROTOR);
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
    // div by 1000 so that time is in seconds
//    verticalVelocity = (height - previousHeight) * deltaTime / 1000;
//    previousHeight = height;

    yaw = yawGetDegrees(PRECISION);  // TODO
    angularVelocity = (yaw - previousYaw) * deltaTime / 1000;
    previousYaw = yaw;

    // call all channel update functions
    int i = 0;
    for (; i < CONTROL_NUM_CHANNELS; i++) {
        if (enabled[i]) {
            chanelUpdateFuncs[i](deltaTime);
        }
    }

    // main rotor equation
    mainDuty = outputs[CONTROL_CALIBRATE_MAIN] + (height * gavitationalOffsetHeightCorrectionFactor) / PRECISION +
            /*angularVelocity*/ + outputs[CONTROL_HEIGHT];
    mainDuty = clamp(mainDuty, MIN_DUTY * PRECISION, MAX_DUTY * PRECISION);

    // tail rotor equation
    tailDuty = outputs[CONTROL_CALIBRATE_TAIL] /* + mainRotorTorqueConstant * mainDuty*/ + outputs[CONTROL_YAW];
    tailDuty = clamp(tailDuty, MIN_DUTY * PRECISION, MAX_DUTY * PRECISION);

    // Set motor speed
    pwmSetDuty((uint32_t)mainDuty, PRECISION, MAIN_ROTOR);
    pwmSetDuty((uint32_t)tailDuty, PRECISION, TAIL_ROTOR);
}


///
/// Channel update functions
///

#define KP 0
#define KD 1
#define KI 2

static int32_t proportionalInputMain, derivativeInputMain, integralInputMain = 0;
static int32_t proportionalInputTail, derivativeInputTail, integralInputTail = 0;

void updateHeightChannel(uint32_t deltaTime)
{
    // Proportional Control
    proportionalInputMain = (mainGains[KP] * targets[CONTROL_HEIGHT] - mainGains[KP] * height) / PRECISION;
    //outputs[CONTROL_HEIGHT] = targets[CONTROL_HEIGHT];

    // Integral Control
    integralInputMain = (integralInputMain * PRECISION + (mainGains[KI] * targets[CONTROL_HEIGHT] - mainGains[KI] * height)) / PRECISION;
    displayTwoValuesWithFormat("CY = %4d, %4d", proportionalInputMain / PRECISION, integralInputMain / PRECISION, 3);  // line 3
    outputs[CONTROL_HEIGHT] = proportionalInputMain + integralInputMain;
    }


void updateYawChannel(uint32_t deltaTime)
{
    proportionalInputTail = (tailGains[KP] * targets[CONTROL_YAW] - tailGains[KP] * yaw) / PRECISION;
    integralInputTail = (integralInputTail * PRECISION + (tailGains[KI] * targets[CONTROL_YAW] - tailGains[KI] * yaw)) / PRECISION;

    outputs[CONTROL_YAW] = proportionalInputTail + integralInputTail;

}


void updateCalibrationChannelMain(uint32_t deltaTime)
{
    outputs[CONTROL_CALIBRATE_MAIN] = 40 * PRECISION;
    controlDisable(CONTROL_CALIBRATE_MAIN);
}

void updateCalibrationChannelTail(uint32_t deltaTime)
{
    outputs[CONTROL_CALIBRATE_TAIL] = 40 * PRECISION;
    controlDisable(CONTROL_CALIBRATE_TAIL);
}
