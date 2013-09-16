/*
 * findEyeCenter.h
 *
 *  Created on: Apr 5, 2013
 *      Author: uchint
 */

#ifndef FINDEYECENTER_H_
#define FINDEYECENTER_H_
#include "common.h"
#include "constants.h"

void findEyes(cv::Mat frame_gray, cv::Rect face, Point& leftPuil, Point& rightPupil);

#endif /* FINDEYECENTER_H_ */
