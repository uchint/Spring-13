/*
 * trainer.h
 *
 *  Created on: Apr 11, 2013
 *      Author: uchint
 */

#ifndef TRAINER_H_
#define TRAINER_H_
#include "common.h"


void executeTrainer(int width, int height);

void initiateTestingPhase(int width, int height);

Point findNearestNeighbour(Point testPoint);

#endif /* TRAINER_H_ */
