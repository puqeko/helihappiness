// ************************************************************
// stateInfo.h
// Helicopter project
// Group: A03 Group 10
// Last edited: 02-06-18
//
// Purpose: Define the states (modes) of the heli program and a state object
// to be passed around the tasks.
// ************************************************************

#ifndef STATE_INFO_H_
#define STATE_INFO_H_


// The mode of the helicoptor.
typedef enum {
    STATE_LANDED = 0,
    STATE_CALIBRATE_YAW,
    STATE_FLYING,
    STATE_DESCENDING,
    STATE_POWER_DOWN,

    // the value of this enum is the number of heli states defined above
    NUM_HELI_STATES
} heli_state_t;


// Entries which more than one task needs to know about.
typedef struct {
    heli_state_t heliMode;
    int32_t targetHeight;
    int32_t targetYaw;
    int32_t outputMainDuty;
    int32_t outputTailDuty;
} state_t;


#endif /* STATE_INFO_H_ */
