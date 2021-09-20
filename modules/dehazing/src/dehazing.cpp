/********************************************************************/
/* Project: uw_img_proc									            */
/* Module:  dehazing								                */
/* File: 	dehazing.cpp								            */
/* Created:	05/02/2019				                                */
/* Description:
	C++ Module for image dehazing using Gao's Bright Channel Prior	*/
 /*******************************************************************/

 /*******************************************************************/
 /* Created by:                                                     */
 /* Geraldine Barreto (@geraldinebc)                                */
 /*******************************************************************/

/// Include auxiliary utility libraries
#include "../include/dehazing.h"

cv::Mat brightChannel(std::vector<cv::Mat_<uchar>> channels, int size) {					// Generates the Bright Channel Image
	cv::Mat maxRGB = max(max(channels[0], channels[1]), channels[2]);						// Maximum Color Image
	cv::Mat element, bright_chan;
	element = getStructuringElement(MORPH_RECT, Size(size, size), Point(-1, -1));			// Maximum filter
	dilate(maxRGB, bright_chan, element);													// Dilates the maxRGB image
	return bright_chan;
}

cv::Mat maxColDiff(std::vector<cv::Mat_<uchar>> channels) {									// Generates the Maximum Color Difference Image
	vector<float> means;
	means.push_back(mean(channels[0])[0]);
	means.push_back(mean(channels[1])[0]);
	means.push_back(mean(channels[2])[0]);
	cv::Mat sorted;
	sortIdx(means, sorted, SORT_EVERY_ROW + SORT_ASCENDING);								// Orders the mean of the channels from low to high

	cv::Mat cmin = channels[sorted.at<int>(0, 0)];
	cv::Mat cmid = channels[sorted.at<int>(0, 1)];
	cv::Mat cmax = channels[sorted.at<int>(0, 2)];
	
	cv::Mat a, b, mcd;
	a = max(cmax - cmin, 0);																// Calculates the maximum values for the MCD image
	b = max(cmid - cmin, 0);
	mcd = 255 - max(a,b);
	return mcd;
}

cv::Mat rectify(cv::Mat S, cv::Mat bc, cv::Mat mcd) {										// Rectifies the Bright Channel Image
	double lambda;
	minMaxLoc(S, NULL, &lambda);															// Maximum value of the Saturation channel
	lambda = lambda / 255.0;																// Normalization for the next step
	cv::Mat correct;
	addWeighted(bc, lambda, mcd, 1.0 - lambda, 0.0, correct);
	return correct;
}

void getHistogram(cv::Mat *channel, cv::Mat *hist) {								// Computes the histogram of a single channel
	int histSize = 256;
	float range[] = { 0, 256 };														// The histograms ranges from 0 to 255
	const float* histRange = { range };
	calcHist(channel, 1, 0, Mat(), *hist, 1, &histSize, &histRange, true, false);
}

std::vector<uchar> lightEstimation(cv::Mat src_gray, int size, cv::Mat bright_chan, std::vector<Mat_<uchar>> channels) {	// Estimates the atmospheric light
	cv::Mat variance, thresholded;
	sqrBoxFilter(src_gray, variance, -1, Size(size, size), Point(-1, -1), true, BORDER_DEFAULT);	// Variance Filter
	cv::Mat histogram;
	getHistogram(&bright_chan, &histogram);
	float percent = 1.0, sum = 0.0, thresh = -1.0;
	int i = 0;
	while (sum <= bright_chan.total()*percent / 100) {
		sum += histogram.at<float>(i, 0);
		thresh++;																					// Finds the threshold value for the 1% darkest pixels in the BC
		i++;
	}
	threshold(bright_chan, thresholded, thresh, 255, 1);											// If the pixels are higher than thresh Mask -> 0 else -> 1
	Point minLoc;
	minMaxLoc(variance, NULL, NULL, &minLoc, NULL, thresholded);									// Finds the variance darkest pixel using the calculated mask
	std::vector<uchar> A;
	for (int i = 0; i < 3; i++) A.push_back(channels[i].at<uchar>(minLoc.y, minLoc.x));
	////PARA VISUALIZAR
	//src_gray.at<char>(minLoc.y, minLoc.x) = 255;
	//namedWindow("A point", WINDOW_KEEPRATIO);
	//imshow("A point", thresholded);
	return A;
}

cv::Mat transmittance(cv::Mat correct, std::vector<uchar> A) {						// Computes the Transmittance Image
	correct.convertTo(correct, CV_32F);
	cv::Mat t[3], acc(correct.size(), CV_32F, Scalar(0));
	for (int i = 0; i < 3; i++) {
		t[i] = 255.0 * ( (correct - A[i]) / (255.0 - A[i]) );
		accumulate(t[i], acc);
	}
	cv::Mat trans = acc/3;
	trans.convertTo(trans, CV_8U);
	return trans;
}

cv::Mat dehaze(vector<Mat_<float>> channels, std::vector<uchar> A, cv::Mat trans) {	// Restores the Underwater Image using the Bright Channel Prior
	trans.convertTo(trans, CV_32F, 1.0/255.0);
	channels[0] = 255.0 - ((channels[0] - (A[0] * (1.0 - trans))) / trans);
	channels[1] = 255.0 - ((channels[1] - (A[1] * (1.0 - trans))) / trans);
	channels[2] = (channels[2] - A[2]) / trans + A[2];
	cv::Mat dehazed, dst;
	merge(channels, dehazed);
	dehazed.convertTo(dst, CV_8U);
	return dst;
}

#if USE_GPU
cv::cuda::GpuMat brightChannel_GPU(std::vector<cv::cuda::GpuMat> channels, int size) {				// Generates the Bright Channel Image
	cv::cuda::GpuMat maxRGB = cv::cuda::max(cv::cuda::max(channels[0], channels[1]), channels[2]);	// Maximum Color Image
	cv::Mat element = getStructuringElement(MORPH_RECT, Size(size, size), Point(-1, -1));			// Maximum filter
	cv::cuda::GpuMat bright_chan;
	cv::cuda::dilate(maxRGB, bright_chan, element);													// Dilates the maxRGB image
	return bright_chan;
}

cv::cuda::GpuMat maxColDiff_GPU(std::vector<cv::cuda::GpuMat> channels) {						// Generates the Maximum Color Difference Image
	vector<float> means;
	means.push_back(mean(channels[0])[0]);
	means.push_back(mean(channels[1])[0]);
	means.push_back(mean(channels[2])[0]);
	cv::Mat sorted;
	sortIdx(means, sorted, SORT_EVERY_ROW + SORT_ASCENDING);									// Orders the mean of the channels from low to high

	cv::cuda::GpuMat cmin = channels[sorted.at<int>(0, 0)];
	cv::cuda::GpuMat cmid = channels[sorted.at<int>(0, 1)];
	cv::cuda::GpuMat cmax = channels[sorted.at<int>(0, 2)];

	cv::cuda::GpuMat diff1, diff2, a, b, maxi, mcd;
	cv::cuda::subtract(cmax, cmin, diff1);
	cv::cuda::subtract(cmid, cmin, diff2);
	cv::cuda::max(diff1, 0, a);
	cv::cuda::max(diff2, 0, b);
	cv::cuda::max(a, b, maxi);
	cv::cuda::subtract(255, maxi, mcd);															// Calculates the maximum values for the MCD image
	return mcd;
}

cv::cuda::GpuMat rectify_GPU(cv::cuda::GpuMat S, cv::cuda::GpuMat bc, cv::cuda::GpuMat mcd) {	// Rectifies the Bright Channel Image
	double lambda;
	cv::cuda::minMax(S, NULL, &lambda);															// Maximum value of the Saturation channel
	lambda = lambda / 255.0;																	// Normalization for the next step
	cv::cuda::GpuMat correct;
	cv::cuda::addWeighted(bc, lambda, mcd, 1.0 - lambda, 0.0, correct);
	return correct;
}

std::vector<uchar> lightEstimation_GPU(cv::Mat src_gray, int size, cv::cuda::GpuMat bright_chan, std::vector<cv::Mat> channels) {	// Estimates the atmospheric light
	cv::Mat variance, histogram;
	sqrBoxFilter(src_gray, variance, -1, Size(size, size), Point(-1, -1), true, BORDER_DEFAULT);		// Variance Filter
	cv::cuda::GpuMat histogramGPU;
	cv::cuda::calHist(bright_chan, histogramGPU);
	histogramGPU.download(histogram);
	float percent = 1.0, sum = 0.0, thresh = -1.0;
	int i = 0;
	while (sum <= bright_chan.total()*percent / 100) {
		sum += histogram.at<float>(i, 0);
		thresh++;
		i++;
	}
	cv::cuda::GpuMat thresholded, varianceGPU;
	cv::cuda::threshold(bright_chan, thresholded, thresh, 255, 1);
	Point minLoc;
	varianceGPU.upload(variance);
	cv::cuda::minMaxLoc(varianceGPU, NULL, NULL, &minLoc, NULL, thresholded);
	std::vector<uchar> A;
	for (int i = 0; i < 3; i++) A.push_back(channels[i].at<uchar>(minLoc.y, minLoc.x));
	return A;
}

cv::cuda::GpuMat transmittance_GPU(cv::cuda::GpuMat correct, std::vector<uchar> A) {			// Computes the Transmittance Image
	correct.convertTo(correct, CV_32F);
	cv::cuda::GpuMat t[3], sub, acc(correct.size(), CV_32F, Scalar(0));
	for (int i = 0; i < 3; i++) {
		cv::cuda::subtract(correct, A[i], sub);
		cv::cuda::multiply(sub, 255.0/(255.0 - A[i]), t[i]);
		cv::cuda::add(acc, t[i], acc);
	}
	cv::cuda::GpuMat trans;
	cv::cuda::divide(acc, 3, trans, CV_8U);
	return trans;
}

cv::cuda::GpuMat dehaze_GPU(std::vector<cv::cuda::GpuMat> channels, std::vector<uchar> A, cv::cuda::GpuMat trans) {	// Restores the Underwater Image using the BCP
	trans.convertTo(trans, CV_32F, 1.0 / 255.0);
	cv::cuda::GpuMat B[3], C[3];
	cv::cuda::addWeighted(channels[0], 1, -A[0], 1.0 - trans, 0.0, B[0]);
	cv::cuda::subtract(255.0, B[0], C[0]);
	cv::cuda::divide(C[0], trans, channels[0]);
	cv::cuda::addWeighted(channels[1], 1, -A[1], 1.0 - trans, 0.0, B[1]);
	cv::cuda::substract(255.0, B[1], C[1]);
	cv::cuda::divide(C[1], trans, channels[1]);
	cv::cuda::subtract(channels[2], A[2], B[2]);
	cv::cuda::divide(B[2], trans, C[2]);
	cv::cuda::add(C[2], A[2], channels[2]);
	cv::cuda::GpuMat dehazed, dst;
	cv::cuda::merge(channels, dehazed);
	dehazed.convertTo(dst, CV_8U);
	return dst;
}
#endif