/*
 * landingController.h
 *
 *  Created on: 28/05/2018
 *      Author: bebop
 */

#ifndef LANDINGCONTROLLER_H_
#define LANDINGCONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

#include "kernalMustardWithThePipeInTheDiningRoom.h"

#define LANDING_RATE 25 // % per second
#define LANDING_TIME_OUT 7500 // 7.5 s
#define STABILITY_TIME_MAIN 500 // 500 ms
#define MS_TO_SEC 1000

void land(state_t *state, uint32_t deltaTime, int32_t yawDegrees);

bool isLandingYawStable(int32_t yawDegrees);

bool checkLandingStability (state_t *state, uint32_t deltaTime, int32_t yawDegrees, int32_t heightPercentage);


#endif /* LANDINGCONTROLLER_H_ */
