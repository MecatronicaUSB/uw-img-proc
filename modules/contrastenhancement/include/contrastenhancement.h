/*********************************************************************/
/* Project: uw-img-proc									             */
/* Module:  contrastenhancement				  		                 */
/* File: 	contrastenhancement.h					         	     */
/* Created:	01/03/2018				                                 */
/* Description:
	C++ Module for contrast enhancement using histogram stretching   
	and equalization												 */
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
	@brief		Enhances an image with the Simplest Color Balance method
	@function	cv::Mat simplestColorBalance(cv::Mat src, float percent)
*/
cv::Mat simplestColorBalance(cv::Mat src, float percent);

/*
	@brief		Stretches the histogram of one image channel in a specific direction (right 0, both sides 1 or left 2)
	@function	cv::Mat histStretch(cv::Mat src, float percent, int direction);
*/
cv::Mat histStretch(cv::Mat src, float percent, int direction);

/*
	@brief		Enhances the contrast of an image using the Integrated Color Model by Iqbal et al. based on histogram stretching
	@function	cv::Mat ICM(cv::Mat src, float percent)
*/
cv::Mat ICM(cv::Mat src, float percent);

/*
	@brief		Enhances the contrast of an image using histogram stretching and the Von Kries Hypotesis
	@function	cv::Mat UCM(cv::Mat src, float percent);
*/
cv::Mat UCM(cv::Mat src, float percent);

/*
	@brief		Equalizes the histogram of one image channel using Rayleigh's distribution
	@function	cv::Mat rayleighEqualization(cv::Mat src)
*/
cv::Mat rayleighEqualization(cv::Mat src);


/*
	@brief		Computes the histogram of a single channel
	@function	void getHistogram(cv::Mat *channel, cv::Mat *hist)
*/
void getHistogram(cv::Mat *channel, cv::Mat *hist);


#if USE_GPU

	cv::cuda::GpuMat simplestColorBalance_GPU(cv::cuda::GpuMat srcGPU, float percent);

	cv::cuda::GpuMat histStretch_GPU(cv::cuda::GpuMat srcGPU, float percent, int direction);

	cv::cuda::GpuMat ICM_GPU(cv::cuda::GpuMat srcGPU, float percent);

	cv::cuda::GpuMat UCM_GPU(cv::cuda::GpuMat srcGPU, float percent);

	cv::cuda::GpuMat rayleighEqualization_GPU(cv::cuda::GpuMat srcGPU);

//	void getHistogram_GPU(cv::cuda::GpuMat *channel, cv::Mat *hist);

#endif