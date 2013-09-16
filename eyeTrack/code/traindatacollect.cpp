#include <unistd.h>
#include <pthread.h>
#include "trainer.h"

extern int faceInBox;

bool trainerRunning = false;
bool trainingCompleted = false;
bool inTestingPhase = false;

struct TrainData* trainList;
struct TrainData* curBlock 	= NULL;
int blockCount 				= HSPLIT * VSPLIT;
int tickCount 				= -1;
int curBlockCount 			= 0;
int imageWidth 				= 0;
int imageHeight 			= 0;

void* updateBlock(void* arg);

void freeTrainList()
{
	if(trainList)
	{
		struct TrainData* temp = trainList;
		while(temp)
		{
			struct TrainData* temp1 = temp->next;
			free(temp);
			temp = temp1;
		}
		trainList = NULL;
	}
}
void executeTrainer(int width, int height)
{
	if(!faceInBox)
	{
		printf("\n ERROR: Face not aligned");
		return;
	}
	printf("\n executeTrainer++");
	freeTrainList();
	imageWidth = width;
	imageHeight = height;
	curBlockCount = 0;
	trainerRunning = true;
	trainingCompleted = false;
	pthread_t id;
	pthread_create(&id, NULL, &updateBlock, NULL);
	printf("\n executeTrainer--");
	return;
}

bool isUpdated(int index)
{
	printf("\n isUpdated++ [%d]", index);
	bool foundStatus = false;
	struct TrainData* temp = trainList;
	while(temp)
	{
		if(temp->index == index)
		{
			foundStatus = true;
			break;
		}
		temp = temp->next;
	}
	printf("\n isUpdated--");
	return foundStatus;
}

void updateDisplayPoint(int temp)
{
	printf("\n updateDisplayPoint++");
	int widthStep = imageWidth/VSPLIT;
	int heightStep = imageHeight/HSPLIT;

	curBlock->refPoint.y = heightStep * (temp/HSPLIT);
	curBlock->refPoint.x = widthStep * (temp%HSPLIT);
	curBlock->refPoint.x += widthStep/2;
	curBlock->refPoint.y += heightStep/2;


	curBlock->pupilPoint.y = 0;
	curBlock->pupilPoint.x = 0;
	curBlock->index = temp;
	curBlock->numberofPoints = 0;
	printf("\n updateDisplayPoint--");
}

void* updateBlock(void* arg)
{
	printf("\n updateBlock++");
	int temp = 0;
loop:
	temp = rand()%blockCount;
	if(isUpdated(temp))
		goto loop;
	if(!trainList)
	{
		trainList = (struct TrainData*)calloc(1, sizeof(struct TrainData));
		curBlock = trainList;
	}
	else
	{
		struct TrainData* temp = (struct TrainData*)calloc(1, sizeof(struct TrainData));
		struct TrainData* last = trainList;
		while(last->next)
			last = last->next;
		last->next = temp;
		curBlock = temp;
	}
	updateDisplayPoint(temp);
	tickCount = MAXSECS;
ticker:
	sleep(1);
	tickCount--;
	if(tickCount != 0)
		goto ticker;

	curBlockCount++;
	curBlock = NULL;
	if(curBlockCount == blockCount)
		trainerRunning = false;

	if(trainerRunning)
		updateBlock(NULL);
	else
	{
		printf("\n .........Training stopped.......");
		//Dump details to the file;
		FILE* fp = fopen(TRAINDATA, "w");
		if(fp)
		{
			struct TrainData* temp = trainList;
			char str[50];
			sprintf(str, "%d %d %d %d ", imageWidth, imageHeight, HSPLIT, VSPLIT);
			fputs(str, fp);
			while(temp)
			{
				temp->pupilPoint.x = temp->pupilPoint.x/temp->numberofPoints;
				temp->pupilPoint.y = temp->pupilPoint.y/temp->numberofPoints;
				sprintf(str, "%d %d %d %d %d ", temp->refPoint.x, temp->refPoint.y, temp->pupilPoint.x, temp->pupilPoint.y, temp->index);
				fputs(str, fp);
				temp = temp->next;
			}
			fclose(fp);
			trainingCompleted = true;
		}
		else
			printf("\n ERROR: Unable to open file to write");
	}
	printf("\n updateBlock--");
	return NULL;
}
void* initTest(void* arg)
{
	printf("\n initTest++");
	FILE* fp = fopen(TRAINDATA, "r");
	if(fp == NULL)
	{
		printf("\n ERROR: Unable to open training data file %s", TRAINDATA);
		return NULL;
	}
	int imgw = 0;
	int imgh = 0;
	int hsplit = 0;
	int vsplit = 0;
	fscanf(fp, "%d %d %d %d ", &imgw, &imgh, &hsplit, &vsplit);
	if(imgw != imageWidth || imgh != imageHeight || hsplit != HSPLIT || vsplit != VSPLIT)
	{
		printf("\n ERROR: training data mismatch");
		return NULL;
	}

	freeTrainList();

	struct TrainData* temp;
	Point ref, pupil;
	int ind;

	int err = fscanf(fp, "%d %d %d %d %d ", &(ref.x), &(ref.y), &(pupil.x), &(pupil.y), &ind);

	while(err)
	{

		if(trainList == NULL)
		{
			trainList = (struct TrainData*)calloc(1, sizeof(struct TrainData));
			temp = trainList;
		}
		else
		{
			temp->next = (struct TrainData*)calloc(1, sizeof(struct TrainData));
			temp = temp->next;
		}

		temp->index = ind;
		temp->refPoint.x = ref.x;
		temp->refPoint.y = ref.y;
		temp->pupilPoint.x = pupil.x;
		temp->pupilPoint.y = pupil.y;

		err = fscanf(fp, "%d %d %d %d %d ", &(ref.x), &(ref.y), &(pupil.x), &(pupil.y), &ind);
		if(feof(fp))
			break;
	}
	fclose(fp);
	inTestingPhase = true;
	printf("\n initTest--");
	return NULL;

}

void initiateTestingPhase(int width, int height)
{
	printf("\n initiateTestingPhase++");
	imageWidth = width;
	imageHeight = height;
	pthread_t id;
	pthread_create(&id, NULL, &initTest, NULL);
	printf("\n initiateTestingPhase--");
}

Point findNearestNeighbour(Point testPoint)
{
	struct TrainData* temp = trainList;
	Point matchPoint;
	matchPoint.x = matchPoint.y = -1;
	double min = -1;
	double val = 0;
	while(temp)
	{
		val = ((temp->pupilPoint.x - testPoint.x)*(temp->pupilPoint.x - testPoint.x))+((temp->pupilPoint.y - testPoint.y)*(temp->pupilPoint.y - testPoint.y));
		val = sqrt(val);
		if(min == -1)
		{
			min = val;
			matchPoint.x = temp->refPoint.x;
			matchPoint.y = temp->refPoint.y;
		}
		else
		{
			if(val < min)
			{
				min = val;
				matchPoint.x = temp->refPoint.x;
				matchPoint.y = temp->refPoint.y;
			}
		}
		temp = temp->next;
	}
	return matchPoint;
}
