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

#define LANDING_SPEED 25 // in % per second
#define LANDING_TIME_OUT 1000 // 1000 ms
#define STABILITY_TIME_MAIN 500 // 500 ms
#define STABILITY_TIME_TAIL 2000 // 2000 ms

void land(state_t *state, uint32_t deltaTime, int32_t yawDegrees);

bool isLandingYawStable(int32_t yawDegrees);



#endif /* LANDINGCONTROLLER_H_ */
