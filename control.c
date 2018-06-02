
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
#include "landingController.h"

static int32_t outputs[CONTROL_NUM_CHANNELS] = {0};  // values to send to motor
static bool enabled[CONTROL_NUM_CHANNELS] = {0};

typedef void (*control_channel_update_func_t)(state_t*, uint32_t);

// defined later on
void updateHeightChannel(state_t* state, uint32_t deltaTime);
void updateYawChannel(state_t* state, uint32_t deltaTime);
void updateCalibrationChannelTail(state_t* state, uint32_t deltaTime);
void updateDescendingChannel(state_t* state, uint32_t deltaTime);
void updatePowerDownChannel(state_t* state, uint32_t deltaTime);

// functions which get called to update each channel
static control_channel_update_func_t chanelUpdateFuncs[CONTROL_NUM_CHANNELS] = {
    updateHeightChannel,  // updated first so yaw can use mainDuty
    updateYawChannel,
    updateDescendingChannel,
    updatePowerDownChannel
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
enum gains_e {KP=0, KD, KI};
static int32_t mainGains[] = {1500, 600, 400};
static int32_t tailGains[] = {1200, 800, 500};
static int32_t mainOffset = 33;  // temporary until calibration added


void controlInit(void)
{
    pwmInit();
}


// limit the value n between two lower and upper values (inclusive)
// return the limited value
int32_t clamp(int32_t n, int32_t lower, int32_t upper)
{
    if (n < lower)
        return lower;
    else if (n > upper)
        return upper;
    return n;  // the number is in bounds
}


void controlEnable(control_channel_t channel)
{
    // enable only one channel
    enabled[channel] = true;

    // handle inital conditions
    switch(channel) {
    case CONTROL_POWER_DOWN:
        // start ramping down from this point
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
    // add final conditions here
    default:
        outputs[channel] = 0;
    }
}


// Returns ture if the control channel 'channel' is enabled in
// the controller.
bool controlIsEnabled(control_channel_t channel)
{
    return enabled[channel];
}


// Returns the current actual value of motor duty cycle for a given channel.
// Returns -1 if the channel is not valid.
int32_t controlGetPWMDuty(control_duty_t channel)
{
    switch (channel) {
    case CONTROL_DUTY_MAIN:
        return mainDuty / PRECISION;
    case CONTROL_DUTY_TAIL:
        return tailDuty / PRECISION;
    default:
        break;
    }

    return -1;
}


void controlUpdate(state_t* state, uint32_t deltaTime)
{
    static bool wereAllDisabled = true;

    // always calculate velocities here so that we don't get discontinuities
    // get height and velocity
    height = heightAsPercentage(PRECISION);
    verticalVelocity = (height - previousHeight) * PRECISION / deltaTime;
    previousHeight = height;

    // get yaw and angular velocity
    yaw = yawGetDegrees(PRECISION);
    angularVelocity = (yaw - previousYaw) * PRECISION / deltaTime;
    previousYaw = yaw;

    // call all channel update functions
    int i = 0;
    bool areAllDisabled = true;
    for (; i < CONTROL_NUM_CHANNELS; i++) {
        if (enabled[i]) {
            chanelUpdateFuncs[i](state, deltaTime);
            areAllDisabled = false;
        }
    }

    // test if we have switched from controlling to not controlling the motors
    // or visa versa
    bool shouldToggleMotors = areAllDisabled != wereAllDisabled;
    wereAllDisabled = areAllDisabled;  // save for next call

    if (shouldToggleMotors) {
        // turn on/off motors only when enabling/disabling all channels
        pwmSetOutputState(!areAllDisabled, MAIN_ROTOR);
        pwmSetOutputState(!areAllDisabled, TAIL_ROTOR);
    }

    if (areAllDisabled) {
        mainDuty = tailDuty = 0;
    } else {
        // run the motors

        // main rotor equation
        mainDuty = outputs[CONTROL_HEIGHT] + outputs[CONTROL_POWER_DOWN];  // ang vel must be radians;
        mainDuty = clamp(mainDuty, MIN_DUTY * PRECISION, MAX_DUTY * PRECISION);

        // tail rotor equation
        // filter main to take into account time delay and smoothing so that we get less oscillation
        tailDuty = outputs[CONTROL_YAW];
        tailDuty = clamp(tailDuty, MIN_DUTY * PRECISION, MAX_DUTY * PRECISION);

        // Set motor speed
        // Since tail and main duty are clamped, it is safe to cast to uint32_t types
        pwmSetDuty((uint32_t)mainDuty, PRECISION, MAIN_ROTOR);
        pwmSetDuty((uint32_t)tailDuty, PRECISION, TAIL_ROTOR);
    }
}


///
/// Channel update functions
///


#define CONTROL_INTE_LIMIT (PRECISION * 200)  // set to 200% compensation
#define SIGN(n) ((n) < 0 ? -1 : 1)

static int32_t inte_h = 0;
void updateHeightChannel(state_t* state, uint32_t deltaTime)
{
    // calculate inital offset + a factor which varies with height.
    // this correction model was obtained experimentally.
    outputs[CONTROL_HEIGHT] = mainOffset * PRECISION + height * gravOffset / PRECISION;

    // difference between the target and actual height value
    int32_t error = state->targetHeight * PRECISION - height;

    // proportonal component = Kp * error
    int32_t prop = mainGains[KP] * error / PRECISION;
    outputs[CONTROL_HEIGHT] += prop;

    // derivitive component = Kd * d/dt(error) = Kd * (d/dt(target) - d/dt(verticalVelocity))
    // since d/dt(target) can be assumed 0 where the target is stationary, we get the
    // below simplification. Although the target is not always stationary, this is a good
    // appoximation.
    int32_t deri = mainGains[KD] * (0 - verticalVelocity) / PRECISION;
    outputs[CONTROL_HEIGHT] += deri;

    // cumulative component = Ki * sum(error) from t0 to t. Hence, we sum. However, a bound
    // is put on the cumulative component to stop overflow.
    inte_h += mainGains[KI] * (int32_t)deltaTime * error / MS_TO_SEC / PRECISION;
    if (abs(inte_h) > CONTROL_INTE_LIMIT) {
        // limit to the max integral value (as a +ve or -ve value) with the correct sign
        inte_h = SIGN(inte_h) * CONTROL_INTE_LIMIT;
    }
    outputs[CONTROL_HEIGHT] += inte_h;
}


static int32_t inte_y = 0;
void updateYawChannel(state_t* state, uint32_t deltaTime)
{
    // cuple to main rotor speed since changes in speed effect the tail rotor
    outputs[CONTROL_YAW] = mainTorqueConst * mainDuty / PRECISION;

    // difference between the target and actual yaw value
    int32_t error = state->targetYaw * PRECISION - yaw;

    // proportonal component = Kp * error
    int32_t prop = tailGains[KP] * error / PRECISION;
    outputs[CONTROL_YAW] += prop;

    // derivitive component = Kd * d/dt(error) = Kd * (d/dt(target) - d/dt(angularVelocity))
    // since d/dt(target) can be assumed 0 where the target is stationary, we get the
    // below simplification. Although the target is not always stationary, this is a good
    // appoximation
    int32_t deri = tailGains[KD] * (0 - angularVelocity) / PRECISION;
    outputs[CONTROL_YAW] += deri;

    // cumulative component = Ki * sum(error) from t0 to t. Hence, we sum. However, a bound
    // is put on the cumulative component to stop overflow.
    inte_y += tailGains[KI] * (int32_t)deltaTime * error / MS_TO_SEC / PRECISION;
    if (abs(inte_y) > CONTROL_INTE_LIMIT) {
        // limit to the max integral value and preserve
        inte_y = SIGN(inte_y) * CONTROL_INTE_LIMIT;
    }
    outputs[CONTROL_YAW] += inte_y;
}


// reset integral gains between runs
void controlReset(void)
{
    inte_h = 0;
    inte_y = 0;
}


void updateDescendingChannel(state_t* state, uint32_t deltaTime)
{
    if (checkLandingStability(state, deltaTime, yaw, height)) {
        controlDisable(CONTROL_DESCENDING);
    } else {
        land(state, deltaTime, yaw);
    }
}

void updatePowerDownChannel(state_t* state, uint32_t deltaTime) {
    if (outputs[CONTROL_POWER_DOWN] <= MIN_DUTY) {
        controlDisable(CONTROL_DESCENDING);
    } else if (outputs[CONTROL_POWER_DOWN] >= DUTY_DECREMENT_PER_CYCLE * deltaTime) { // prevent overflow
        outputs[CONTROL_POWER_DOWN] -= DUTY_DECREMENT_PER_CYCLE * deltaTime;
    } else {
        outputs[CONTROL_POWER_DOWN] = 0; // would have overflowed, so set to zero (this will be clamped)
    }
}
