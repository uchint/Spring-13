#include "common.h"
#include "eyefinder.h"
#include "trainer.h"

string haarFaceDefault = "/home/uchint/opencv2.4.4/workspace/eyeTrack/data/haarcascade_frontalface_default.xml";

extern bool trainerRunning;
extern bool trainingCompleted;
extern bool inTestingPhase;
extern TrainData* curBlock;
extern int tickCount;

CascadeClassifier cascadeFace, cascadeLEye, cascadeREye;
bool faceInBox = false;

void drawLines(IplImage* frame);

Point leftPupil;
Point rightPupil;

bool initializeCascades()
{
	bool result = true;

	/* Load face specific haarcascade file */
	result = cascadeFace.load(haarFaceDefault);
	if(!result)
		return false;

	return true;
}



void detectAndDraw( IplImage* frame, ExecuteMode mode)
{
	cvFlip(frame, frame, 1);
    double t = 0;
    vector<Rect> face, lEye, rEye;
    const static Scalar colors[] =  { CV_RGB(0,0,255),
        CV_RGB(0,128,255),
        CV_RGB(0,255,255),
        CV_RGB(0,255,0),
        CV_RGB(255,128,0),
        CV_RGB(255,255,0),
        CV_RGB(255,0,0),
        CV_RGB(255,0,255)} ;
    CvSize scaledImgSize;
    scaledImgSize.width = cvGetSize(frame).width/SCALE;
    scaledImgSize.height = cvGetSize(frame).height/SCALE;

    IplImage* scaledImg = cvCreateImage(scaledImgSize,IPL_DEPTH_8U, 1);
    IplImage* grayImg = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U, 1);


    cvCvtColor( frame, grayImg, CV_BGR2GRAY );
    cvResize( grayImg, scaledImg);
    cvEqualizeHist( scaledImg, scaledImg );

    t = (double)cvGetTickCount();
    cascadeFace.detectMultiScale( scaledImg, face,
        1.1, 2, 0
        //|CV_HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_ROUGH_SEARCH
        |CV_HAAR_SCALE_IMAGE
        ,
        Size(30, 30) );

    t = (double)cvGetTickCount() - t;

    if(!face.size())
    {
    	printf("\n Face not found [skip]");
    	return;
    }
    if(face.size() > 1)
    {
    	printf("\n More than one face identified [skip]");
  //  	return;
    }
    Scalar color;
    int fceX = 0;
    int fceY = 0;
    fceX = cvGetSize(frame).width/2 - FACEWIDTH/2;
    fceY = cvGetSize(frame).height/2 - FACEHEIGHT/2;

    color = colors[0];
    /*check if the face is with in the box */
    if(face[0].x >= fceX && FACEWIDTH > face[0].width && FACEWIDTH - face[0].width < FACEBIAS )
    	if(face[0].y >= fceY && FACEHEIGHT > face[0].height && FACEHEIGHT - face[0].height < FACEBIAS)
    		{
    			color = colors[3];
    			faceInBox = true;
    			if(!trainingCompleted && mode == EStartTraining)
    			{
    				if(inTestingPhase)
    					inTestingPhase = false;
    				printf("\n .........Training started.......");
    				executeTrainer(cvGetSize(frame).width, cvGetSize(frame).height);
    			}
    			else if(mode == EStartTesting && inTestingPhase == false)
    			{
    				printf("\n .........Testing started.......");
    				initiateTestingPhase(cvGetSize(frame).width, cvGetSize(frame).height);
    			}
    		}
    /* Draw the face border */
    cvRectangle( frame, cvPoint(cvRound((face[0].x)*SCALE), cvRound((face[0].y)*SCALE)),
    			   cvPoint(cvRound((face[0].x + face[0].width-1)*SCALE), cvRound((face[0].y + face[0].height-1)*SCALE)),
    			   color, 3, 4, 0);

	cvResetImageROI(scaledImg);
	if(faceInBox)
	{
		findEyes(scaledImg, face[0], leftPupil, rightPupil);
		if(curBlock && trainerRunning)
		{
			curBlock->pupilPoint.x += (leftPupil.x + rightPupil.x)/2;
			curBlock->pupilPoint.y += (leftPupil.y + rightPupil.y)/2;
			curBlock->numberofPoints++;
		}
	}
    DRAWLINES?drawLines(frame):cvShowImage( "Face", frame );

}

void drawLines(IplImage* frame)
{

	int imgWidth = cvGetSize(frame).width;
	int imgHeight = cvGetSize(frame).height;
	int widthStep = imgWidth/VSPLIT;
	int heightStep = imgHeight/HSPLIT;
	/* Draw Horizontal lines */
	int y = 0;
	int x = 0;
	for(int i=0; i< HSPLIT-1; i++)
	{
		y=y+heightStep;
		cvLine(frame, cvPoint(x, y), cvPoint(imgWidth-1, y), CV_RGB(255,255,255), 1, 8, 0);
	}
	y = x= 0;
	for(int i=0; i<VSPLIT-1; i++)
	{
		x = x+widthStep;
		cvLine(frame, cvPoint(x, y), cvPoint(x, imgHeight-1), CV_RGB(255,255,255), 1, 8, 0);
	}

	int facex = imgWidth/2 - FACEWIDTH/2;
	int facey = imgHeight/2 - FACEHEIGHT/2;
	/*Area where the face should come */
	if(!faceInBox)
	{
		cvRectangle(frame, cvPoint(facex, facey), cvPoint(facex+FACEWIDTH, facey+FACEHEIGHT), CV_RGB(191, 191, 191), 2, 8, 0);
		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5);
		cvPutText(frame, DISPLAYTEXT, cvPoint(50, 50), &font, CV_RGB(255, 255, 255));
	}
	else
	{
		Mat frameM(frame);
		if(trainerRunning && curBlock)
		{
			CvFont font;
			char text[3];
			sprintf(text, "%d", tickCount);
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 2, 2, 0, 3);
			cvPutText(frame, text, cvPoint(curBlock->refPoint.x, curBlock->refPoint.y), &font, CV_RGB(255, 0, 0));
		}
		else if(inTestingPhase && faceInBox)
		{
			Point matchPoint;
			matchPoint.x = (leftPupil.x+rightPupil.x)/2;
			matchPoint.y = (leftPupil.y+rightPupil.y)/2;

			Point lPoint = findNearestNeighbour(matchPoint);
			circle(frameM, lPoint, 15, 1234, 4);
		}
		// draw eye centers

		circle(frameM, rightPupil, 3, 1234);
		circle(frameM, leftPupil, 3, 1234);
		faceInBox = false;
	}

	cvShowImage( "Face/Pupil/Lines", frame );
}
