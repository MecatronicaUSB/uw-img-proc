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

void getHistogram(cv::Mat *channel, cv::Mat *hist);

cv::Mat histStretch(cv::Mat src, float percent, int direction);

/*
	@brief		Corrects the hue shift and ununiform illumination of the underwater image
	@function	cv::Mat hueIllumination(cv::Mat src)
*/
cv::Mat hueIllumination(cv::Mat src);

/*
	@brief		Enhances the contrast of the image using histogram stretching
	@function	cv::Mat IUCM(vector<Mat_<uchar>> channel, float percent)
*/
cv::Mat ICM(vector<Mat_<uchar>> channel, float percent);

/*
	@brief		Dehazes an underwater image using the Bright channel Prior
	@function	cv::Mat dehazing(cv::Mat src)
*/
cv::Mat dehazing(cv::Mat src);

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

/*
	@brief		Creates a kernel for a 5x5 Gaussian Filter
	@function	cv::Mat filter_mask()
*/
cv::Mat filter_mask();

/*
	@brief		Computes a laplaican contrast weight map
	@function	cv::Mat laplacian_contrast(cv::Mat img)
*/
cv::Mat laplacian_contrast(cv::Mat img);

/*
	@brief		Computes a local contrast weight map
	@function	cv::Mat local_contrast(cv::Mat img, cv::Mat kernel)
*/
cv::Mat local_contrast(cv::Mat img, cv::Mat kernel);

/*
	@brief		Computes a saliency weight map
	@function	cv::Mat saliency(cv::Mat img, cv::Mat kernel)
*/
cv::Mat saliency(cv::Mat img, cv::Mat kernel);

/*
	@brief		Computes a exposedness weight map
	@function	cv::Mat exposedness(cv::Mat img)
*/
cv::Mat exposedness(cv::Mat img);

/*
	@brief		Normalizes two weigh maps
	@function	vector<Mat> weight_norm(cv::Mat w1, cv::Mat w2)
*/
vector<Mat> weight_norm(cv::Mat w1, cv::Mat w2);

/*
	@brief		Creates a laplacian pyramid
	@function	vector<Mat_<float>> laplacian_pyramid(cv::Mat img, int levels)
*/
vector<Mat_<float>> laplacian_pyramid(cv::Mat img, int levels);

/*
	@brief		Fuses two pyramids
	@function	Mat_<float> pyramid_fusion(Mat_<float> *pyramid, int levels)
*/
Mat pyramid_fusion(Mat *pyramid, int levels);

#if USE_GPU
	cv::cuda::GpuMat illuminationCorrection_GPU(cv::cuda::GpuMat srcGPU);

	void fft_GPU(const cv::cuda::GpuMat &srcGPU, cv::cuda::GpuMat &dst);

	cv::Mat gaussianFilter_GPU(cv::Mat img, float sigma, float high, float low);

	cv::cuda::GpuMat histStretch_GPU(cv::cuda::GpuMat srcGPU, float percent, int direction);

	cv::cuda::GpuMat ICM_GPU(cv::cuda::GpuMat srcGPU, float percent);

	cv::cuda::GpuMat dehazing_GPU(cv::cuda::GpuMat srcGPU);

	cv::cuda::GpuMat brightChannel_GPU(std::vector<cv::cuda::GpuMat> channels, int size);

	cv::cuda::GpuMat maxColDiff_GPU(std::vector<cv::cuda::GpuMat> channels);

	cv::cuda::GpuMat rectify_GPU(cv::cuda::GpuMat S, cv::cuda::GpuMat bc, cv::cuda::GpuMat mcd);

	std::vector<uchar> lightEstimation_GPU(cv::Mat src_gray, int size, cv::cuda::GpuMat bright_chan, std::vector<cv::Mat> channels);

	cv::cuda::GpuMat transmittance_GPU(cv::cuda::GpuMat correct, std::vector<uchar> A);

	cv::cuda::GpuMat dehaze_GPU(std::vector<cv::cuda::GpuMat> channels, std::vector<uchar> A, cv::cuda::GpuMat trans);
#endif