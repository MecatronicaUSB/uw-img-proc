/********************************************************************/
/* Project: uw-img-proc									            */
/* Module: 	fusion										            */
/* File: 	fusion.h										        */
/* Created:	18/02/2019				                                */
/* Description:
	C++ module for underwater image enhancement using a fusion based
	strategy
 /*******************************************************************/

 /*******************************************************************/
 /* Created by:                                                     */
 /* Geraldine Barreto (@geraldinebc)                                */
 /*******************************************************************/

#define ABOUT_STRING "Fusion Enhancement Module"

///Basic C and C++ libraries
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

/// OpenCV libraries. May need review for the final release
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// C++ namespaces
using namespace cv;
using namespace cuda;
using namespace std;

/// CUDA specific libraries
#if USE_GPU
#include <opencv2/cudafilters.hpp>
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudaarithm.hpp"
#endif

/*
	@brief		Corrects ununinform illumination using a homomorphic filter
	@function	illuminationCorrection(cv::Mat src)
*/
cv::Mat illuminationCorrection(cv::Mat src);

/*
	@brief		Computes the Normalized Discrete Fourier Transform
	@function	void fft(const cv::Mat &src, cv::Mat &dst)
*/
void fft(const cv::Mat &src, cv::Mat &dst);

/*
	@brief		Creates an Emphasis Highpass Gaussian Filter
	@function	cv::Mat gaussianFilter(cv::Mat img, float sigma, float high, float low)
*/
cv::Mat gaussianFilter(cv::Mat img, float sigma, float high, float low);

/*
	@brief		Rearranges the quadrants of a zero centered filter
	@function	void dftShift(Mat &fImage)
*/
void dftShift(Mat &fImage);