#ifndef PIPELINE_H
#define PIPELINE_H

using namespace std;
using namespace cv;

void runPipeline();	//callback function
void doAbstraction();
void applyBilateralFilter(Mat, int);
void doEdgeDetection();
void doQuantization(int);
void cartoonThisFrame(Mat);
void doCannyEdge();
void CannyTreshold(int, void*);		//callback function
void Dilation();

#endif