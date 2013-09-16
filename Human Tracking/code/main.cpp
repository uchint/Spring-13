
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "common.h"

#define VIDEOFRAME 	"/home/uchint/opencv2.4.4/workspace/videoframes/img"

int totalPeople = 0;			/* counter to count the number of people */
int frameCount = 0;				/* counter to indicate which frame is being processed */
Mat frame, frameOrig;			/* variables to hold frame data */
vector<RectDetails> focusList;	/* list of features being detected and tracked */

extern bool trackFeature(int index, Mat image, Mat Original);

/* isOverlapping() identifies whether the same feature from the same human is identified in */
/* successive frames. If so, such feature will be discarded */
bool isOverlapping(int index, Rect lRect)
{
	bool result = false;
	int xdiff, ydiff;
	int framediff;
	for(unsigned int i=0; i< focusList.size(); i++)
	{
		RectDetails temp = focusList[i];
		xdiff = lRect.x - temp.focusRect.x; if(xdiff < 0) xdiff = -xdiff;
		ydiff = lRect.y - temp.focusRect.y; if(ydiff < 0) ydiff = -ydiff;
		framediff = index - temp.frameIndex; if(framediff < 0) framediff = -framediff;

		if(framediff < FRAMEDIFF && xdiff < 2* RADIUS && ydiff < 2* RADIUS)
		{
			result = true;
			break;
		}
	}
	return result;
}

/* detectAndTrack() function does the following steps:
 * 1. Identify all the connected components
 * 2. Refine components which meet the 'area' requirement
 * 3. update the features list array
 * 4. execute trackFeature function for each feature
 */
void detectAndTrack(Mat diffImgForDetecting)
{
	Mat diffImgForTracking;				/* Difference Image from Caller */
	Mat grayImg;						/* GrayedImage for difference Image */
	Mat blackAndWhiteImg;				/* Black and White Image obtained from GrayedImage */

	vector<vector<Point> > contours;	/* vector to store vector of contours */
	vector<Vec4i> hierarchy;			/* vector to store hierarchy of contours */

	/* Creating instance of the same image */
	diffImgForDetecting.copyTo(diffImgForTracking);

	/* 1. Converting difference image to gray image
	 * 2. converting gray image to black and white applying a threshold
	 */
	cvtColor(diffImgForDetecting, grayImg, CV_BGR2GRAY);
	blackAndWhiteImg =  (grayImg < 130);


	/*CV_RETR_CCOMP retrieves all of the contours and organizes them into a
	 * two-level hierarchy. At the top level, there are external boundaries of
	 * the components. At the second level, there are boundaries of the holes.
	 * If there is another contour inside a hole of a connected component, it is
	 * still put at the top level.
	 */

	/* CV_CHAIN_APPROX_SIMPLE compresses horizontal, vertical, and diagonal segments
	 * and leaves only their end points. For example, an up-right rectangular contour
	 * is encoded with 4 points
	 */
    findContours( blackAndWhiteImg, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

#ifdef __DRAW_CONTOURS__
    Mat dst = Mat::zeros(diffImgForDetecting.size(), CV_8UC3);
    if( !contours.empty() && !hierarchy.empty() )
    {
        int idx = 0;
        for( ; idx >= 0; idx = hierarchy[idx][0] )
        {
        	Scalar color( (rand()&255), (rand()&255), (rand()&255) );
            drawContours( dst, contours, idx, color, 1/*CV_FILLED*/, 8, hierarchy );
        }
    }
    imshow("contours", dst);
#endif

    frameCount++;
    for(unsigned int i=0; i< contours.size(); i++)
    {
    	RotatedRect rect = minAreaRect(contours[i]);
    	double rectArea = rect.size.width* rect.size.height;

    	/* Based on contour area, only those contours whose area meets the below condition will be considered
    	 * Those areas correspond to head, body and legs individually
    	 * These values are specific only to the video provided. Will change for some other video
    	 */
    	if( (rectArea >= 5000.0f && rectArea <= 10000.00f && rect.center.y < 200) \
    			|| (rectArea >= 15000.0f && rectArea <= 25000.00f && rect.center.y >= 200))
    	{
    		/* If the contour meets the requirement, then a quick
    		 * set of basic details like the center of contour,
    		 * frame count are stored. Each and every feature is
    		 * validated for overlapping. Only those which are not
    		 * overlapping with the previous are only considered
    		 */
    		Rect lRect;
    		lRect.x = rect.center.x - RADIUS;
    		lRect.y = rect.center.y - RADIUS;
    		lRect.width = lRect.height =  2* RADIUS;
    		RectDetails lDetails(frameCount, lRect);
    		if(!isOverlapping(frameCount, lRect))
    		{
    			focusList.push_back(lDetails);
#ifdef __DISPLAY_FEATURES__
				circle(diffImgForDetecting, rect.center, RADIUS, Scalar(255, 0, 0), 2, 8);
#endif
    		}
    	}
    }
#ifdef __DISPLAY_FEATURES__
    imshow( "Detection", diffImgForDetecting );
#endif

    /* The validated/stored features are sent to tracker for further processing */
    for(int j = focusList.size() -1; j>=0; j--)
    {
    	bool result = trackFeature(j, diffImgForTracking, frameOrig);
    	if(!result)
    	{
    		/* If tracker is not able to track a particular feature,
    		 * an assumption is made either the feature moved out of the window
    		 * or CamShift Algorithm is not behaving properly. As we can not
    		 * distinguish between those two, as a safe bet, the people count is
    		 * incremented
    		 */
    		totalPeople++;
    		/* Once a feature is out of track, it is removed from the list */
    		focusList.erase(focusList.begin()+j);
    	}
    }
}


int main(int argc, char** argv)
{
	Ptr<BackgroundSubtractorGMG> fgbg = Algorithm::create<BackgroundSubtractorGMG>("BackgroundSubtractor.GMG");
    if (fgbg.empty())
    {
        std::cerr << "Failed to create BackgroundSubtractor.GMG Algorithm." << std::endl;
        return -1;
    }

    fgbg->set("initializationFrames", 1);
    fgbg->set("decisionThreshold", 0.7);

    char filename[100];
	int count = 0;


    Mat fgmask, segm;

    while(1)
    {
    	sprintf(filename, "%s%d.jpg", VIDEOFRAME, count++);
    	IplImage* framel = cvLoadImage(filename);
    	printf("\n %d", count);
    	if (framel == NULL)
		{
			printf("\n ************ Total People Identified = %d ***************\n", totalPeople);
			break;
		}

         frameOrig = Mat(framel);

         /* An approximation is made that people are moving in/out
          * of the rectangle (290, 20, 250, 380) and below operations
          * are done:
          * 1. Crop the image with in the required rectangle
          * 2. Scale the image to 2x for better identification of people
          * 3. All the processing will be done on this cropped and scaled image
          * 4. Final results are transformed appropriately and displayed on original image
          */
         IplImage* frame_crop = cvCreateImage(cvSize(250, 380), framel->depth, framel->nChannels);
         IplImage* frame_scale = cvCreateImage(cvSize(250*2, 380*2), framel->depth, framel->nChannels);
         cvSetImageROI(framel, Rect(290, 20, 250, 380));
         cvCopy(framel, frame_crop, NULL);
         cvResize(frame_crop, frame_scale, 2);
         frame = Mat(frame_scale);

        (*fgbg)(frame, fgmask);

        /* On the obtained foreground mask, erosion is applied to refine the outlines
         * MORPH_ELLIPSE option is used to get a smooth layout
         * */
		int erosion_size = 2;
		cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
							  cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
							  cv::Point(erosion_size, erosion_size) );
		erode(fgmask, fgmask, element);

#ifdef __DISPLAY_FGMASK__
		imshow("Foreground Mask", fgmask);
#endif

        frame.copyTo(segm);
        add(frame, Scalar(255, 255, 0), segm, fgmask);

        Mat diffImg;
        frame.copyTo(diffImg, fgmask);
#ifdef __DISPLAY_DIFFIMG__
        imshow("Difference Image", diffImg);
#endif
#ifdef __DISPLAY_FGSEG__
        imshow("FG Segmentation", segm);
#endif

        /* detectAndTrack() process the difference image
         * to detect and track the features
         */
        detectAndTrack(diffImg);

        /* Memory Clean up to avoid 'Out of Memory' */
        cvReleaseImage(&framel);
        cvReleaseImage(&frame_scale);
        cvReleaseImage(&frame_crop);

        int c = waitKey(30);
        if (c == 'q' || c == 'Q' || (c & 255) == 27)
            break;
    }
    return 0;
}
