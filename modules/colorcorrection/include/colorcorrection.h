/*********************************************************************/
/* Project: uw-img-proc									             */
/* Module:  colorcorrection								             */
/* File: 	colorcorrection.h								         */
/* Created:	20/11/2018				                                 */
/* Description:
	C++ Module of color cast eliminination using Gray World Assumption
*/
/*********************************************************************/

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
	@brief		Computes Ruderman's Lab color space components
	@function	std::vector<Mat_<float>> BGRtoLab(cv::Mat src)
*/
std::vector<Mat_<float>> BGRtoLab(cv::Mat src);

/*
	@brief		Transforms an image in Ruderman's laB color space to BGR
	@function	std::vector<Mat_<uchar>> LaBtoBGR(std::vector<Mat_<float>> Lab)
*/
std::vector<Mat_<uchar>> LabtoBGR(std::vector<Mat_<float>> Lab);

/*
	@brief		Finds an aproximation of the median value of the matrix
	@function	double medianMat(cv::Mat src)
*/
double medianMat(cv::Mat src);

/*
	@brief		Corrects the color using the Grey World Assumption applied in Ruderman's Lab color space
	@function	cv::Mat GWA_Lab(cv::Mat src)
*/
cv::Mat GWA_Lab(cv::Mat src);

/*
	@brief		Corrects the color using the Grey World Assumption applied in CIELAB color space
	@function	cv::Mat GWA_CIELAB(cv::Mat src)
*/
cv::Mat GWA_CIELAB(cv::Mat src);

/*
	@brief		Corrects the color using the Grey World Assumption applied in RGB color space
	@function	cv::Mat GWA_RGB(cv::Mat src)
*/
cv::Mat GWA_RGB(cv::Mat src);

#if USE_GPU
	std::vector<cv::cuda::GpuMat> BGRtoLab_GPU(cv::cuda::GpuMat srcGPU);

	cv::cuda::GpuMat LabtoBGR_GPU(std::vector<cv::cuda::GpuMat> Lab);

	cv::cuda::GpuMat LabtoBGR_GPU(std::vector<cv::cuda::GpuMat> Lab);

	cv::cuda::GpuMat GWA_Lab_GPU(cv::cuda::GpuMat srcGPU);

	cv::cuda::GpuMat GWA_CIELAB_GPU(cv::cuda::GpuMat srcGPU);
#endif