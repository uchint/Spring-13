#ifndef COMMON_H_
#define COMMON_H_
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace cv;
using namespace std;

//#define __DRAW_CONTOURS__		/* Macro to enable/disable contour display */
//#define __DISPLAY_FEATURES__	/* Macro to enable/disable display of features detected */
//#define __DISPLAY_FGMASK__	/* Macro to enable/disable display of Foreground Mask */
//#define __DISPLAY_DIFFIMG__	/* Macro to enable/disable display of Difference image */
//#define __DISPLAY_FGSEG__		/* Macro to enable/disable display of FG + FGMask image */
//#define __DISPLAY_DIFFTRKED__	/* Macro to enable/disable display of tracked diff image */

/* RADIUS used in drawing a circle inside the detected connected component */
#define RADIUS		20
/* If a connected component is identified in a rect(x,y, w, h) of Frame X, the same feature is */
/* ignored in the successive frames for (Frame X + FRAMEDIFF) frames */
#define FRAMEDIFF	8

/* Once key features are identified, few important details are maintained */
/* which will be used by MeanShift module to track those features */
class RectDetails
{
public:
	RectDetails(int& idx, Rect& rectan)
	:focusRect(rectan), frameIndex(idx)
	{
		isBeingTracked = false;
	}
	Rect focusRect; 			/* Rectangle where a feature is detected */
	int frameIndex;				/* Frame number in which this feature is detected */
	bool isBeingTracked;        /* MeanShift module sets this to 'true' if it tracking */
								/* else it sets to 'false' */
	Mat hist;					/* Histogram of the feature is stored here */
	Rect trackRect;
};

#endif /* COMMON_H_ */
