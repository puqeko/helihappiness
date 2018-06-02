/*
 * kernalMustardWithThePipeInTheDiningRoom.h
 *
 *  Created on: 28/05/2018
 *      Author: Thomas
 */

#include <stdint.h>
#include <stdbool.h>


#ifndef KERNAL_H_
#define KERNAL_H_


// Inform tasks about the global state of the helicopter.
typedef enum {
    LANDED = 0,
    DESCENDING,
    POWER_DOWN,
    FLYING,
    CALIBRATE_YAW,

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


// Object to configure a handler for use in the task scheduler.
typedef struct {
    void (*handler) (state_t* state, uint32_t deltaTime);  // pointer to task handler function
    uint32_t updateFreq;  // number of ms between runs
    uint32_t count;  // used by the kernal only
    uint32_t triggerAt;  // used by the kernal only
} task_t;


// A simple round robin scheduler.
// Uses an infinite loop to run the tasks at specified frequencies relative to baseFreq. Make
// sure baseFreq is greater than or equal to all task frequencies otherwise the tasks will
// not be run at the correct rate. A pointer to a state object stores entries applicable to
// many tasks.
void runTasks(task_t* tasks, state_t* sharedState, int32_t baseFreq);


#endif /* KERNAL_H_ */
