#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "pipeline.h"
#include <time.h>

using namespace std;
using namespace cv;

//GLOBAL VARIABLES
Mat src, src_gray;

//--- 1 --- abstraction
Mat luminance, luminance_abs;
int cntr = 1, d = 1, dev_c = 40, dev_s = 90, abstraction_on = 0;

//---2---quantization
int n = 8, quantization_on = 0;

//---3---edge detection
Mat detected_edges;
int edgeThresh = 1, lowThreshold = 50, ratio = 3, kernel_size = 3, edgeDetection_on = 1;
int const max_lowThreshold = 100;

//---4---dilation
int dilation_size = 1;
Mat dilated_edges;

void cartoonThisFrame(Mat input)
{
	const clock_t begin_time = clock();
	
	src = input.clone();

	//USER GUI for adjusting parameters of operations
	namedWindow("Parameters", WINDOW_NORMAL);

	//taskbars for abstraction
	createTrackbar( "Abstraction on", "Parameters", &abstraction_on, 1 );
	createTrackbar( "Recursion cntr", "Parameters", &cntr, 40 );
	createTrackbar( "Diameter", "Parameters", &d, 9 );
	createTrackbar( "Deviation color", "Parameters", &dev_c, 100 );
	createTrackbar( "Deviation space", "Parameters", &dev_s, 100 );
	//taskbar for quantization
	createTrackbar( "Quantization on", "Parameters", &quantization_on, 1 );
	createTrackbar( "Quantization levels", "Parameters", &n, 50 );
	//taskbar for edge detection
	createTrackbar( "Edge detection on", "Parameters", &edgeDetection_on, 1 );
	createTrackbar( "Min Threshold", "Parameters", &lowThreshold, max_lowThreshold );
	//taskbar for dilation
	createTrackbar( "Dilation size", "Parameters", &dilation_size, 1 );
	
	runPipeline();

	cout << float(clock() - begin_time)/CLOCKS_PER_SEC << endl;

	namedWindow("Result", WINDOW_NORMAL);
	imshow("Result", src);
}
//callback function that executes the pipeline
void runPipeline()
{
	if(d % 2 != 0)
	{
		if(abstraction_on == 1)
		{
			cvtColor(src, src, CV_RGB2Lab);
			doAbstraction();
			cvtColor(src, src, CV_Lab2RGB);
		}
		if(quantization_on == 1)
		{
			cvtColor(src, src, CV_RGB2Lab);
			doQuantization(n);
			cvtColor(src, src, CV_Lab2RGB);
		}
		if(edgeDetection_on == 1)
		{
			doEdgeDetection();
			Dilation();

			//COMBINE RESULTS
			int rows = src.rows;
			int columns = src.cols;
			int channels = src.channels();
			int i, j;

			for (i = 0; i < rows; i++)
			{
				uchar* ptr_img = src.ptr<uchar>(i);
				uchar* ptr_img_edg = dilated_edges.ptr<uchar>(i);
				for (j = 0; j < columns; j++)
				{
					int val_edg = (int)ptr_img_edg[j];

					if(val_edg < 10)
					{
						ptr_img[j*channels+0] = val_edg;
						ptr_img[j*channels+1] = val_edg;
						ptr_img[j*channels+2] = val_edg;
					}
				}
			}
		}
	}
}
//recursive method, arguments are image and counter
void doAbstraction()
{
	int rows = src.rows;
	int columns = src.cols;
	int channels = src.channels();
	int i, j;

	luminance = Mat::zeros(rows, columns/3, src.type());		//matrix to save luminance channel of img
	//GET LUMINANCE CHANNEL
	for(i = 0; i<rows; i++)
	{
		uchar* ptr_img = src.ptr<uchar>(i);
		uchar* ptr_lum = luminance.ptr<uchar>(i);
		for(j = 0; j<columns; j++)
		{
			int val = (int)ptr_img[j*channels];		//get L from La*b*
			ptr_lum[j] = val;						//lum[i,j] = val
		}
	}

	luminance_abs = Mat::zeros(luminance.rows, luminance.cols, luminance.type());
	
	//APPLY BILATERAL FILTER  RECUSIVELY
	applyBilateralFilter(luminance, cntr);

	//UPDATE LUMINANCE CHANNEL
	for(i = 0; i<rows; i++)
	{
		uchar* ptr_img = src.ptr<uchar>(i);
		uchar* ptr_lum = luminance_abs.ptr<uchar>(i);
		for(j = 0; j<columns; j++)
		{
			ptr_img[j*channels] = ptr_lum[j];		//set the pixel to new abstracted luminance value
		}
	}
}
//recursive method, bilateral filter will be applied "c" times on "input"
void applyBilateralFilter(Mat input, int c)
{
	if(c == 0)
	{
		return;
	}
	else
	{
		Mat temp = input.clone();
		bilateralFilter(temp, luminance_abs, d, dev_c, dev_s);		//using bilateral filter to abstract the image
		temp.release();
		applyBilateralFilter(luminance_abs, c - 1);
		return;
	}
}
//edge calculation, based on canny edge
void doEdgeDetection()
{
	Mat temp;

	temp.create(src.size(), src.type());
	cvtColor( src, src_gray, CV_BGR2GRAY );

	/// Reduce noise with a kernel 3x3
	blur( src_gray, detected_edges, Size(3,3) );

	/// Canny detector
	Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

	/// Using Canny's output as a mask, we display our result
	temp = Scalar::all(0);

	src_gray.copyTo( temp, detected_edges);
}
//image quantization based on n, smaller the "n" - smaller the number of quantization levels
void doQuantization(int levels)
{
	int rows = src.rows;
	int columns = src.cols;
	int channels = src.channels();
	int i, j;

	for (i = 0; i < rows; i++)
	{
		uchar* ptr_img = src.ptr<uchar>(i);
		for (j = 0; j < columns; j++)
		{
			int b = (int)ptr_img[j*channels+0];
			int g = (int)ptr_img[j*channels+1];
			int r = (int)ptr_img[j*channels+2];
			
			b = floor(b/(256/levels))*(256/levels);		//function which calculates new quantized pixel value
			g = floor(g/(256/levels))*(256/levels);
			r = floor(r/(256/levels))*(256/levels);

			ptr_img[j*channels+0] = b;
			ptr_img[j*channels+1] = g;
			ptr_img[j*channels+2] = r;
		}
	}
}
void Dilation()
{
	Mat element = getStructuringElement( MORPH_ELLIPSE,
                                       Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                       Point( dilation_size, dilation_size ) );
	/// Apply the dilation operation
	dilate( detected_edges, dilated_edges, element );
	dilated_edges = ~dilated_edges;
}