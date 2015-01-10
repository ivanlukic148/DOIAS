#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "video.h"
#include "pipeline.h"

using namespace std;
using namespace cv;

void cartoonCamera()
{
	VideoCapture cap(0);
	if(!cap.isOpened())		//if file is not found, return
		return;
	else
	{
		for(;;)
		{
			try
			{
				Mat frame;
				cap >> frame;		//read current frame
				if(frame.empty())
					break;
				cartoonThisFrame(frame);		//modify this frame
				frame.release();
				if(waitKey(10) == 27)		//waits 10 sec for esc
					break;
			}
			catch(Exception ex)
			{
				cout << ex.msg << endl;
			}
		}
		cout << "Reading from camera finished" <<  endl;
		//when reading is finished, release the camera and destroy the windows
		cap.release();
		destroyAllWindows();
	}
		
}