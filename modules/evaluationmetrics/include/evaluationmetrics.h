/*********************************************************************/
/* Project: uw-img-proc                                              */
/* Module:  evaluationmetrics                                        */
/* File:    evaluationmetrics.h                                      */
/* Created: 09/12/2018                                               */
/* Description:
    C++ Module of image quality metrics for image processing evaluation
 /********************************************************************/

 /********************************************************************/
 /* Created by:                                                      */
 /* Geraldine Barreto (@geraldinebc)                                 */
 /********************************************************************/

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
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

// C++ namespaces
using namespace cv;
using namespace cuda;
using namespace std;
using namespace xfeatures2d;

/// CUDA specific libraries
#if USE_GPU
#include <opencv2/cudafilters.hpp>
#include "opencv2/cudafeatures2d.hpp"
#include "opencv2/xfeatures2d/cuda.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudaarithm.hpp"
#endif

/*
    @brief      Computes the entropy of a grayscale image according to the Shannon Index
    @function   float entropy(cv::Mat img)
*/
float entropy(cv::Mat img);

/*
    @brief      Computes the average entropy of a RGB image according to the Shannon Index
    @function   float averageEntropy(std::vector<Mat> channel)
*/
float averageEntropy(std::vector<Mat> channel);

/*
    @brief      Computes the average contrast using image gradients
    @function   float averageContrast(std::vector<Mat> chanRGB)
*/
float averageContrast(std::vector<Mat> chanRGB);

/*
    @brief      Computes the average luminance using the luminance channel in YCrCb
    @function   float averageLuminance(cv::Mat L)
*/
float averageLuminance(cv::Mat L);

/*
    @brief      Computes the normalized neighborhood function using the average luminance
    @function   float getNNF(float AL)
*/
float getNNF(float AL);

/*
    @brief      Computes the comprehensive assessment function
    @function   float getCAF(float AE, float AC, float NNF)
*/
float getCAF(float AE, float AC, float NNF);

/*
    @brief      Computes the mean square error
    @function   float getMSE(cv::Mat src, cv::Mat dst)
*/
float getMSE(cv::Mat src, cv::Mat dst);

/*
    @brief      Computes the peak signal to noise ratio
    @function   float getPSNR(double mse)
*/
float getPSNR(float mse);

/*
    @brief      Computes the Frequency Domain Image Sharpness
    @function   float sharpness(cv::Mat src)
*/
float sharpness(cv::Mat src);

/*
    @brief      Computes the intensity distribution histogram
    @function   void getHistogram(cv::Mat *channel, cv::Mat *hist)
*/
void getHistogram(cv::Mat *channel, cv::Mat *hist);

/*
    @brief      Creates an image that represents the Histogram of one image channel
    @function   printHist(Mat histogram, Scalar color);
*/
cv::Mat printHist(cv::Mat histogram, Scalar color);

#if USE_GPU
float entropy_GPU(cv::cuda::GpuMat img);

float averageEntropy_GPU(std::vector<cv::cuda::GpuMat> chanRGB);

float averageContrast_GPU(std::vector<cv::cuda::GpuMat> chanRGB);

float averageLuminance_GPU(cv::cuda::GpuMat L);

float getMSE_GPU(cv::cuda::GpuMat src, cv::cuda::GpuMat dst);

float sharpness_GPU(cv::cuda::GpuMat srcGPU);
#endif