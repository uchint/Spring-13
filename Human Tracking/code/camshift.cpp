#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "common.h"

int vmin = 181, vmax = 256, smin = 20;	/* parameters used by CamShift Algorithm */

extern vector<RectDetails> focusList;

/* trackFeature() tracks the features in the focusList array
 * one by one. If it is not able to track, it returns false
 * The function takes below as input parameters:
 * 1. Array index of the feature to track
 * 2. The cropped, scaled and differenced image
 * 3. the actual original image
 *
 * The function makes use of CamShift Algorithm for tracking
 */
bool trackFeature(int index, Mat diffImg, Mat original)
{

    Rect trackWindow;
    int hsize = 16;
    float hranges[] = {0,180};
    const float* phranges = hranges;
    Mat frame, hsv, hue, mask, hist, backproj;

    cvtColor(diffImg, hsv, CV_BGR2HSV);

	int _vmin = vmin, _vmax = vmax;

	inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
			Scalar(180, 256, MAX(_vmin, _vmax)), mask);
	int ch[] = {0, 0};
	hue.create(hsv.size(), hsv.depth());
	mixChannels(&hsv, 1, &hue, 1, ch, 1);

	RectDetails ltemp = focusList.at(index);
	if( !ltemp.isBeingTracked )
	{
		Mat roi(hue, ltemp.focusRect), maskroi(mask, ltemp.focusRect);
		calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
		normalize(hist, hist, 0, 255, CV_MINMAX);

		trackWindow = ltemp.focusRect;
		ltemp.isBeingTracked = true;

		focusList.at(index).hist = hist;
	}
	else
		trackWindow = focusList.at(index).trackRect;
	calcBackProject(&hue, 1, 0, focusList.at(index).hist, backproj, &phranges);
	backproj &= mask;

	RotatedRect trackBox = CamShift(backproj, trackWindow,
						TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));

	if( trackWindow.area() <= 1 )
	{
		return false;
	}
	focusList.at(index).trackRect = trackBox.boundingRect();
#ifdef __DISPLAY_DIFFTRKED__
	circle(diffImg, trackBox.center, RADIUS/3, Scalar(0, 0, 255), -1, 8);
	imshow( "Diff-Track", diffImg );
#endif

	trackBox.center.x = trackBox.center.x/2;
	trackBox.center.y = trackBox.center.y/2;
	trackBox.center.x += 290;
	trackBox.center.y += 20;
	circle(original, trackBox.center, RADIUS/3, Scalar(0, 0, 255), -1, 8);
	imshow( "Tracking", original );
    return true;
}


