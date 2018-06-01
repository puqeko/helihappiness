
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



static int32_t prop_h = 0, inte_h = 0, deri_h = 0;
void updateHeightChannel(state_t* state, uint32_t deltaTime)
{
    // difference between the target and actual height value
    int32_t error = state->targetHeight * PRECISION - height;

    // proportonal component = Kp * error
    prop_h = mainGains[KP] * error / PRECISION;

    // derivitive component = Kd * d/dt(error) = Kd * (d/dt(target) - d/dt(verticalVelocity))
    // since d/dt(target) can be assumed 0 where the target is stationary, we get the
    // below simplification. Although the target is not always stationary, this is a good
    // appoximation.
    deri_h = mainGains[KD] * (0 - verticalVelocity) / PRECISION;

    // cumulative component = Ki * sum(error) from t0 to t. Hence, we sum. However, a bound
    // is put on the cumulative component to stop overflow.
    if (abs(inte_h) < 1e8)
            inte_h += mainGains[KI] * (int32_t)deltaTime * error / MS_TO_SEC / PRECISION;

    outputs[CONTROL_HEIGHT] = prop_h + deri_h + inte_h;
}


static int32_t prop_y = 0, inte_y = 0, deri_y = 0;
void updateYawChannel(state_t* state, uint32_t deltaTime)
{
    // difference between the target and actual yaw value
    int32_t error = state->targetYaw * PRECISION - yaw;

    // proportonal component = Kp * error
    prop_y = tailGains[KP] * error / PRECISION;

    // derivitive component = Kd * d/dt(error) = Kd * (d/dt(target) - d/dt(angularVelocity))
    // since d/dt(target) can be assumed 0 where the target is stationary, we get the
    // below simplification. Although the target is not always stationary, this is a good
    // appoximation
    deri_y = tailGains[KD] * (0 - angularVelocity) / PRECISION;

    // cumulative component = Ki * sum(error) from t0 to t. Hence, we sum. However, a bound
    // is put on the cumulative component to stop overflow.
    if (abs(inte_y) < 1e8)
        inte_y += tailGains[KI] * (int32_t)deltaTime * error / MS_TO_SEC / PRECISION;

    // combine components for the overall result
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
    land(state, deltaTime, yaw);
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
