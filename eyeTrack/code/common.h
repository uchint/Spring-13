#ifndef COMMON_H_
#define COMMON_H_

#include "opencv2/video/tracking.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv/cv.h"
#include "opencv/cxcore.h"
#include "opencv/highgui.h"

#include <cctype>
#include <iostream>
#include <iterator>
#include <stdio.h>

#include <stdio.h>

using namespace cv;
using namespace std;

#define HSPLIT		3
#define VSPLIT		3
#define DRAWLINES	1
#define FACEWIDTH	240
#define FACEHEIGHT	240
#define FACEBIAS	30
#define SCALE 		1
#define MAXSECS		5
#define DISPLAYTEXT "Make sure the identified face is in the box"
#define TRAINDATA	"train.dat"
//#define EYE_DETECT

enum ExecuteMode
{
	EStartTraining,
	EStartTesting,
	ENone
};

struct TrainData
{
	CvPoint refPoint;
	CvPoint pupilPoint;
	int 	index;
	int 	numberofPoints;
	struct TrainData* next;

};

#endif /* COMMON_H_ */
