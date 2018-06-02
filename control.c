// ************************************************************
// control.c
// Helicopter project
// Group: A03 Group 10
// Last edited: 2-06-2018
//
// Purpose: Control the helicopter using the tail and main rotor.
// Explaination: Control the output to the two helicoptor motors. This module
// preforms PID control by enabling and disabling channels. The motor duty cycle
// is limited to between 5 % and 95 %, otherwise the motors are off and PWM output
// is disabled. Different channels will be active depending on the helicopter's
// current mode. Channels use pwmModule.h with yaw.h and height.h in a feedback loop.
// ************************************************************

#include "control.h"
#include "pwmModule.h"
#include "height.h"
#include "yaw.h"
#include "landingController.h"  // advance landing behaviour

#define MS_TO_SEC 1000  // number of ms in one s
#define CONTROL_INTE_LIMIT (PRECISION * 200)  // set to 200% max compensation
#define DUTY_DECREMENT_PER_CYCLE (DUTY_DECREMENT_PER_SECOND * PRECISION / MS_TO_SEC)
#define SIGN(n) ((n) < 0 ? -1 : 1)

typedef void (*control_channel_update_func_t)(state_t*, uint32_t);  // pointer to handler function

static int32_t outputs[CONTROL_NUM_CHANNELS] = {0};  // values to send to motors
static bool enabled[CONTROL_NUM_CHANNELS] = {0};  // specify which channels have control

// functions which get called to update each channel
void updateHeightChannel(state_t* state, uint32_t deltaTime);
void updateYawChannel(state_t* state, uint32_t deltaTime);
void updateDescendingChannel(state_t* state, uint32_t deltaTime);
void updatePowerDownChannel(state_t* state, uint32_t deltaTime);

static const control_channel_update_func_t chanelUpdateFuncs[CONTROL_NUM_CHANNELS] = {
    updateHeightChannel,
    updateYawChannel,
    updateDescendingChannel,
    updatePowerDownChannel
    // add channel handlers here
};

// configurable constants (scaled by PRECISION)
enum gains_e {KP=0, KD, KI};
static const int32_t mainGains[] = {1500, 600, 400};  // in PID order
static const int32_t tailGains[] = {1200, 800, 500};
static const int32_t mainTorqueConst = 800;  // ratio of main rotor speed to tail rotor speed
static const int32_t mainOffset = 33;  // temporary until calibration added
static const int32_t gravOffset = 200;  // ratio of height to down force

// measured parameters (scaled by PRECISION)
static int32_t height, previousHeight = 0, verticalVelocity;
static int32_t yaw, previousYaw = 0, angularVelocity = 0;

static int32_t inte_y = 0, inte_h = 0;  // for integral calculations


// A helper function. Limit the value n between two lower and upper
// values (inclusive). Return the limited value.
int32_t clamp(int32_t n, int32_t lower, int32_t upper)
{
    if (n < lower)
        return lower;
    else if (n > upper)
        return upper;
    return n;  // the number is in bounds
}


// Initalise PWM outputs and motors
void controlInit(void)
{
    pwmInit();
}


// Resets the integral components of the controllers between runs
void controlReset(void)
{
    inte_h = 0;
    inte_y = 0;
}


// Sets the specified control channel to be enabled. If all channels were previously
// disabled, then the pwm signal output will be enabled in the next call to controlUpdate(..).
void controlEnable(state_t* state, control_channel_t channel)
{
    // enable only one channel
    enabled[channel] = true;

    // handle inital conditions
    switch(channel) {
    case CONTROL_POWER_DOWN:
        // start ramping down from this point
        outputs[CONTROL_POWER_DOWN] = state->outputMainDuty * PRECISION;
        break;
    default:
        outputs[channel] = 0;
    }
}


// Sets the specified control channel to be disabled. If all channels become disabled
// then the pwm output will be disabled in the next call to controlUpdate(..)
void controlDisable(state_t* state, control_channel_t channel)
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


// Return true if the specified control channel is currently enabled.
// All channels are disabled by default.
bool controlIsEnabled(control_channel_t channel)
{
    return enabled[channel];
}


// Checks which channels are active and applies the associated controls to the
// main and tail pwm rotor output for the helicopter. If all channels are disabled,
// then the pwm output is also disabled. Takes a state object contiaining the target
// yaw and height and the time between updates in milliseconds as arguments.
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

    // calculate the duty cycle for each motor
    int32_t mainDuty = 0, tailDuty = 0;

    if (!areAllDisabled) {
        // only set the duty cycle if we are running the motors

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

    // update state so that other tasks know what is going on
    state->outputMainDuty = mainDuty / PRECISION;
    state->outputTailDuty = tailDuty / PRECISION;
}


///
/// Channel update functions.
/// Not defined in the module interface.
///


// Update PID control on helicopter's main rotor (height).
// Accounts for gravity and other factors before the PID stage.
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


// Update PID control on helicopter's tail rotor (yaw).
// Accounts for torque due to the main rotor by coupling with the main duty cycle
void updateYawChannel(state_t* state, uint32_t deltaTime)
{
    // couple to main rotor speed since changes in speed effect the tail rotor.
    // the value of mainDuty will be one update cycle behind.
    // although state->outputMainDuty is not multiplied by PRECISION, it doesn't
    // matter since mainTorqueConst has a factor of PRECSION in it.
    outputs[CONTROL_YAW] = mainTorqueConst * state->outputMainDuty;

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


// Update descention control. Assume that the height channel is also active and running
// PID on the main rotor. Hence, this channel only updates the controller targets.
// Use the landing module to check for stability. This channel is automatically disabled
// once either: stability is reached, or the stability checker times out.
void updateDescendingChannel(state_t* state, uint32_t deltaTime)
{
    if (checkLandingStability(state, deltaTime, yaw, height)) {
        controlDisable(state, CONTROL_DESCENDING);
    } else {
        land(state, deltaTime, yaw);
    }
}


// Update motor power down controller. Assume that PID is disabled on the main rotor and enabled
// on the tail rotor. When enabled, this channel is initalised with the previous mainDuty output
// and decrements the main duty until the MIN_DUTY value is reached. The main duty cycle decreases
// as a rate according to DUTY_DECREMENT_PER_SECOND. This channel automatically disables once the
// main duty reaches its minimum value.
void updatePowerDownChannel(state_t* state, uint32_t deltaTime) {
    if (outputs[CONTROL_POWER_DOWN] <= MIN_DUTY * PRECISION) {
        controlDisable(state, CONTROL_POWER_DOWN);
    } else if (outputs[CONTROL_POWER_DOWN] >= DUTY_DECREMENT_PER_CYCLE * deltaTime) { // prevent overflow
        outputs[CONTROL_POWER_DOWN] -= DUTY_DECREMENT_PER_CYCLE * deltaTime;
    } else {
        outputs[CONTROL_POWER_DOWN] = 0; // would have overflowed, so set to zero (this will be clamped)
    }
}
