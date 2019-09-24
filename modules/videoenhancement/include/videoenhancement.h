/*********************************************************************/
/* Project: uw-img-proc									             */
/* Module:  videoenhancement				  		                 */
/* File: 	videoenhancement.h					         	     */
/* Created:	20/09/2019				                                 */
/* Description:
	C++ Module for underwater video enhancement						 */
/********************************************************************/

 /********************************************************************/
 /* Created by:                                                      */
 /* Geraldine Barreto (@geraldinebc)                                 */
/*********************************************************************/

///Basic C and C++ libraries
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

/// OpenCV libraries. May need review for the final release
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ximgproc.hpp>

// C++ namespaces
using namespace cv;
using namespace cuda;
using namespace std;
using namespace ximgproc;

/// CUDA specific libraries
#if USE_GPU
#include <opencv2/cudafilters.hpp>
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudaarithm.hpp"
#endif

/*
	@brief		Stretches the histogram of one image channel in a specific direction (right 0, both sides 1 or left 2)
	@function	cv::Mat histStretch(cv::Mat prev, cv::Mat src, float percent, int direction);
*/
cv::Mat histStretch(cv::Mat prev, cv::Mat src, float percent, int direction);

/*
	@brief		Enhances the contrast of an image using the Integrated Color Model by Iqbal et al. based on histogram stretching
	@function	cv::Mat ICM(cv::Mat prev, cv::Mat src, float percent)
*/
cv::Mat ICM(cv::Mat prev, cv::Mat src, float percent);

/*
	@brief		Corrects the color of an image using Grey World Assumption and histogram strething
	@function	cv::Mat UCM(cv::Mat src, float percent);
*/
cv::Mat colorcorrection(cv::Mat src);

/*
	@brief		Dehazes an underwater image using the Bright channel Prior
	@function	cv::Mat dehazing(cv::Mat prev, cv::Mat src)
*/
cv::Mat dehazing(cv::Mat prev, cv::Mat src);

/*
	@brief		Generates the Bright Channel Image of an underwater image
	@function	cv::Mat brightChannel(vector<Mat_<uchar>> channels, int size)
*/
cv::Mat brightChannel(vector<Mat_<uchar>> channels, int size);

/*
	@brief		Generates the Maximum Color Difference Image of an underwater image
	@function	cv::Mat mcd = maxColDiff(src_chan)
*/
cv::Mat maxColDiff(vector<Mat_<uchar>> channels);

/*
	@brief		Rectifies the Bright Channel Image
	@function	cv::Mat  rectify(cv::Mat S, cv::Mat bc, cv::Mat mcd)
*/
cv::Mat rectify(cv::Mat S, cv::Mat bc, cv::Mat mcd);

/*
	@brief		Estimates the atmospheric light of an underwater image
	@function	vector<uchar> lightEstimation(cv::Mat src_gray, int size, cv::Mat bc, vector<Mat_<uchar>> channels)
*/
vector<uchar> lightEstimation(cv::Mat src_gray, int size, cv::Mat bc, vector<Mat_<uchar>> channels);

/*
	@brief		Computes the transmittance image
	@function	cv::Mat transmittance(cv::Mat correct, vector<uchar> A)
*/
cv::Mat transmittance(cv::Mat correct, vector<uchar> A);

/*
	@brief		Dehazes the underwater image
	@function	cv::Mat dehaze(vector<Mat_<float>> channels, vector<uchar> A, cv::Mat trans)
*/
cv::Mat dehaze(vector<Mat_<float>> channels, vector<uchar> A, cv::Mat trans);