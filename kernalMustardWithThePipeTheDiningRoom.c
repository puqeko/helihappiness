/*
 * kernalMustardWithThePipeTheDiningRoom.c
 *
 *  Created on: 28/05/2018
 *      Author: Thomas
 */

#include "kernalMustardWithThePipeInTheDiningRoom.h"
#include "timerer.h"


void runTasks(task_t* tasks, state_t* sharedState, int32_t baseFreq)
{
    // initalise the value to count up to for each task so that
    // tasks can run at different frequencies
    int32_t deltaTime = 1000 / baseFreq;  // in milliseconds, hence the 1000 factor

    // loop until empty terminator task
    int i = 0;
    while (tasks[i].handler) {
        uint32_t triggerCount = baseFreq / tasks[i].updateFreq;

        // ensure a count of zero gets triggered since the counter will skip 0 and
        // start at 1.
        if (triggerCount == 0) {
            triggerCount = 1;
        }

        // initalise all tasks
        tasks[i].count = 0;
        tasks[i].triggerAt = triggerCount;
        i++;
    }

    // begin the main loop
    while (true) {
        int32_t referenceTime = timererGetTicks();

        int i = 0;
        while (tasks[i].handler) {
            tasks[i].count++;

            // check if task should run in this update
            if (tasks[i].count == tasks[i].triggerAt) {
                tasks[i].count = 0;

                // run the task
                tasks[i].handler(sharedState, deltaTime);
            }
              i++;
        }

        // make sure loop runs as a consistent speed
        timererWaitFrom(deltaTime, referenceTime);
    }
}
