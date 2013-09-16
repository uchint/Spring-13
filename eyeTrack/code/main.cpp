#include "common.h"

extern void detectAndDraw( IplImage* img, ExecuteMode mode);
extern bool initializeCascades();

int main()
{
	CvCapture *capture =  cvCaptureFromCAM(0);

	// if camara failed, return
	if( !capture )
	{
		printf("Could not initialize capturing device...\n");
		return -1;
	}

	IplImage* frame=0;
	frame = cvQueryFrame(capture);
	if(!frame) return -1;
	if(!initializeCascades())
	{
		printf("\n ERROR:haarcascades not loaded");
		cvReleaseCapture(&capture);
		return -1;
	}

	while(true)
	{
		frame = cvQueryFrame(capture);
		if(!frame) break;
		//cvShowImage("Camera", frame);

		int c = cvWaitKey(10);
		//If 'ESC' is pressed, break the loop
		if((char)c==27 ) break;

		ExecuteMode mode = ENone;
		if(c == 't') //training mode
		{
			printf("\n TRAINING MODE SELECTED");
			mode = EStartTraining;
		}
		else if(c == 's') //testing mode
		{
			printf("\n TESTING MODE SELECTED");
			mode = EStartTesting;
		}
		detectAndDraw(frame, mode);

	}
	cvReleaseCapture(&capture);
	return 0;
}
