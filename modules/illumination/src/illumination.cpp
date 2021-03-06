/********************************************************************/
/* Project: uw-img-proc									            */
/* Module: 	illumination								            */
/* File: 	illumination.cpp								        */
/* Created:	30/07/2019				                                */
/* Description:
	C++ module for correcting the illumination of underwater images
	using a homomorphic filter
 /*******************************************************************/

 /*******************************************************************/
 /* Created by:                                                     */
 /* Geraldine Barreto (@geraldinebc)                                */
 /*******************************************************************/

/// Include auxiliary utility libraries
#include "../include/illumination.h"

cv::Mat illuminationCorrection(cv::Mat src) {												// Homomorphic Filter
	Mat imgTemp1 = Mat::zeros(src.size(), CV_32FC1);
	normalize(src, imgTemp1, 0, 1, NORM_MINMAX, CV_32FC1);									// Normalize the channel
	imgTemp1 = imgTemp1 + 0.000001;
	log(imgTemp1, imgTemp1);																// Calculate the logarithm

	cv::Mat fftimg;
	fft(imgTemp1, fftimg);																	// Fourier transform

	cv::Mat_<float> filter = gaussianFilter(fftimg, 0.7, 1.0, 0.1);							// Gaussian Emphasis High-Pass Filter
	cv::Mat bimg;
	cv::Mat bchannels[] = { cv::Mat_<float>(filter), cv::Mat::zeros(filter.size(), CV_32F) };
	cv::merge(bchannels, 2, bimg);

	dftShift(bimg);																			// Shift the filter
	cv::mulSpectrums(fftimg, bimg, fftimg, 0);												// Apply the filter to the image in frequency domain

	cv::Mat ifftimg;					
	cv::dft(fftimg, ifftimg, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);						// Apply the inverse Fourier transform

	cv::Mat expimg = Mat::zeros(ifftimg.size(), CV_32FC1);									// Calculate the exponent
	cv::exp(ifftimg, expimg);

	cv::Mat dst;
	dst = cv::Mat(expimg, cv::Rect(0, 0, src.cols, src.rows));								// Eliminate the padding from the image
	normalize(dst, dst, 0, 255, NORM_MINMAX, CV_8U);										// Normalize the results
	return dst;
}

void fft(const cv::Mat &src, cv::Mat &dst) {												// Fast Fourier Transform
	cv::Mat padded;
	int m = cv::getOptimalDFTSize(src.rows);
	int n = cv::getOptimalDFTSize(src.cols);
	cv::copyMakeBorder(src, padded, 0, m - src.rows, 0, n - src.cols, cv::BORDER_REPLICATE);// Resize to optimal size

	cv::Mat plane[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };	// Add complex column to store the imaginary result
	cv::Mat imgComplex;
	cv::merge(plane, 2, imgComplex);
	cv::dft(imgComplex, dst);																// Aply the Discrete Fourier Transform
	dst = dst / dst.total();																// Normalize
}

cv::Mat gaussianFilter(cv::Mat img, float sigma, float high, float low) {
	cv::Mat radius(img.rows, img.cols, CV_32F);
	cv::Mat expo(img.rows, img.cols, CV_32F);
	cv::Mat filter(img.rows, img.cols, CV_32F);
	int cx = img.rows / 2;
	int cy = img.cols / 2;

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			radius.at<float>(i, j) = pow(i - cx, 2) + pow(j - cy, 2);
		}
	}
	exp(-radius / (2 * pow(sigma, 2)), expo);
	filter = (high - low) * (1 - expo) + low;												// High-pass Emphasis Filter
	return filter;
}

void dftShift(Mat &fImage) {																// Rearranges the quadrants
	Mat tmp, q0, q1, q2, q3;
	int cx = fImage.cols / 2;
	int cy = fImage.rows / 2;

	q0 = fImage(Rect(0, 0, cx, cy));
	q1 = fImage(Rect(cx, 0, cx, cy));
	q2 = fImage(Rect(0, cy, cx, cy));
	q3 = fImage(Rect(cx, cy, cx, cy));

	q0.copyTo(tmp);																			// Each quadrant its replaced for its diagonally opposite quadrant
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);
}