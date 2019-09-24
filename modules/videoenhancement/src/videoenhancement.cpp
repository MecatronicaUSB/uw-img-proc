/*********************************************************************/
/* Project: uw-img-proc									             */
/* Module:  videoenhancement				  		                 */
/* File: 	videoenhancement.cpp						             */
/* Created:	20/09/2019				                                 */
/* Description:
	C++ Module for underwater video enhancement						 */
/********************************************************************/

 /********************************************************************/
 /* Created by:                                                      */
 /* Geraldine Barreto (@geraldinebc)                                 */
/*********************************************************************/

/// Include auxiliary utility libraries
#include "../include/videoenhancement.h"

void getHistogram(cv::Mat *channel, cv::Mat *hist) {								// Computes the histogram of a single channel
	int histSize = 256;
	float range[] = { 0, 256 };														// The histograms ranges from 0 to 255
	const float* histRange = { range };
	calcHist(channel, 1, 0, Mat(), *hist, 1, &histSize, &histRange, true, false);
}

cv::Mat histStretch(cv::Mat prev, cv::Mat src, float percent, int direction) {
	cv::Mat sum;
	addWeighted(prev, 0.7, src, 0.3, 0, sum);

	cv::Mat histogram;
	getHistogram(&sum, &histogram);
	float percent_sum = 0.0, channel_min = -1.0, channel_max = -1.0;
	float percent_min = percent / 100.0, percent_max = 1.0 - percent_min;
	int i = 0;
	
	while (percent_sum < percent_max * src.total()) {
		if (percent_sum < percent_min * src.total()) channel_min++;
		percent_sum += histogram.at<float>(i, 0);
		channel_max++;
		i++;
	}
	
	cv::Mat dst;
	if (direction == 0) dst = (src - channel_min) * (255.0 - channel_min) / (channel_max - channel_min) + channel_min;	// Stretches the channel towards the Upper side
	else if (direction == 2) dst = (src - channel_min) * channel_max / (channel_max - channel_min);						// Stretches the channel towards the Lower side
	else dst = (src - channel_min) * 255.0 / (channel_max - channel_min);												// Stretches the channel towards both sides
	dst.convertTo(dst, CV_8UC1);
	return dst;
}

cv::Mat ICM(cv::Mat prev, cv::Mat src, float percent) {												// Integrated Color Model
	vector<Mat_<uchar>> chann, channel;
	split(prev, chann);
	split(src, channel);
	Mat chan[3], result;
	for (int i = 0; i < 3; i++) chan[i] = histStretch(chann[i], channel[i], percent, 1);			// Histogram stretching of each color channel
	merge(chan, 3, result);
	Mat HSV, hsv[3], dst;
	cvtColor(result, HSV, COLOR_BGR2HSV);												// Conversion to the HSV color model
	split(HSV, hsv);
	for (int i = 1; i < 3; i++) hsv[i] = histStretch(hsv[i], hsv[i], percent, 1);		// Histogram stretching of the Saturation and Value Channels
	merge(hsv, 3, HSV);
	cvtColor(HSV, dst, COLOR_HSV2BGR);													// Conversion to the BGR color model
	return dst;
}

cv::Mat colorcorrection(cv::Mat src) {														// Corrects the color
	cv::Mat LAB, lab[3], dst;
	cvtColor(src, LAB, COLOR_BGR2Lab);														// Conversion to the CIELAB color space
	split(LAB, lab);
	lab[0] = histStretch(lab[0], lab[0], 1, 1);											// Histogram stretching
	lab[1] = 127.5 * lab[1] / mean(lab[1])[0];												// Grey World Assumption
	lab[2] = 127.5 * lab[2] / mean(lab[2])[0];
	merge(lab, 3, LAB);
	cvtColor(LAB, dst, COLOR_Lab2BGR);														// Conversion to the BGR color space
	return dst;
}
cv::Mat dehazing(cv::Mat prev, cv::Mat src) {											// Dehazed an underwater image
	cv::Mat sum;
	addWeighted(prev, 0.7, src, 0.3, 0, sum);

	vector<Mat_<uchar>> sum_chan, new_chan;
	split(sum, sum_chan);

	new_chan.push_back(255 - sum_chan[0]);												// Compute the new channels for the dehazing process
	new_chan.push_back(255 - sum_chan[1]);
	new_chan.push_back(sum_chan[2]);

	int size = sqrt(src.total()) / 40;													// Making the size bigger creates halos around objects
	cv::Mat bright_chan = brightChannel(new_chan, size);								// Compute the bright channel image
	cv::Mat mcd = maxColDiff(sum_chan);													// Compute the maximum color difference

	cv::Mat sum_HSV, S;
	cv::cvtColor(sum, sum_HSV, COLOR_BGR2HSV);
	extractChannel(sum_HSV, S, 1);
	cv::Mat rectified = rectify(S, bright_chan, mcd);									// Rectify the bright channel image

	cv::Mat sum_gray;
	cv::cvtColor(sum, sum_gray, COLOR_BGR2GRAY);
	vector<uchar> A;
	A = lightEstimation(sum_gray, size, bright_chan, new_chan);							// Estimate the atmospheric light
	cv::Mat trans = transmittance(rectified, A);										// Compute the transmittance image

	cv::Mat filtered;
	guidedFilter(sum_gray, trans, filtered, 30, 0.001, -1);								// Refine the transmittance image

	vector<Mat_<uchar>> src_chan;
	vector<Mat_<float>> chan_dehazed;
	split(src, src_chan);
	chan_dehazed.push_back(255 - src_chan[0]);											// Compute the new channels for the dehazing process
	chan_dehazed.push_back(255 - src_chan[1]);
	chan_dehazed.push_back(src_chan[2]);
	cv::Mat dst = dehaze(chan_dehazed, A, filtered);									// Dehaze the image channels
	return dst;
}

cv::Mat brightChannel(std::vector<cv::Mat_<uchar>> channels, int size) {				// Generates the Bright Channel Image
	cv::Mat maxRGB = max(max(channels[0], channels[1]), channels[2]);					// Maximum Color Image
	cv::Mat element, bright_chan;
	element = getStructuringElement(MORPH_RECT, Size(size, size), Point(-1, -1));		// Maximum filter
	dilate(maxRGB, bright_chan, element);												// Dilates the maxRGB image
	return bright_chan;
}

cv::Mat maxColDiff(std::vector<cv::Mat_<uchar>> channels) {								// Generates the Maximum Color Difference Image
	vector<float> means;
	means.push_back(mean(channels[0])[0]);
	means.push_back(mean(channels[1])[0]);
	means.push_back(mean(channels[2])[0]);
	cv::Mat sorted;
	sortIdx(means, sorted, SORT_EVERY_ROW + SORT_ASCENDING);							// Orders the mean of the channels from low to high

	cv::Mat cmin = channels[sorted.at<int>(0, 0)];
	cv::Mat cmid = channels[sorted.at<int>(0, 1)];
	cv::Mat cmax = channels[sorted.at<int>(0, 2)];

	cv::Mat a, b, mcd;
	a = max(cmax - cmin, 0);															// Calculates the maximum values for the MCD image
	b = max(cmid - cmin, 0);
	mcd = 255 - max(a, b);
	return mcd;
}

cv::Mat rectify(cv::Mat S, cv::Mat bc, cv::Mat mcd) {									// Rectifies the Bright Channel Image
	double lambda;
	minMaxLoc(S, NULL, &lambda);														// Maximum value of the Saturation channel
	lambda = lambda / 255.0;															// Normalization for the next step
	cv::Mat correct;
	addWeighted(bc, lambda, mcd, 1.0 - lambda, 0.0, correct);
	return correct;
}

std::vector<uchar> lightEstimation(cv::Mat src_gray, int size, cv::Mat bright_chan, std::vector<Mat_<uchar>> channels) {
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
	return A;
}

cv::Mat transmittance(cv::Mat correct, vector<uchar> A) {						// Computes the Transmittance Image
	correct.convertTo(correct, CV_32F);
	cv::Mat t[3], acc(correct.size(), CV_32F, Scalar(0));
	for (int i = 0; i < 3; i++) {
		t[i] = 255.0 * ((correct - A[i]) / (255.0 - A[i]));
		accumulate(t[i], acc);
	}
	cv::Mat trans = acc / 3;
	trans.convertTo(trans, CV_8U);
	return trans;
}

cv::Mat dehaze(vector<Mat_<float>> channels, vector<uchar> A, cv::Mat trans) {	// Restores the Underwater Image using the Bright Channel Prior
	trans.convertTo(trans, CV_32F, 1.0 / 255.0);
	channels[0] = 255.0 - ((channels[0] - (A[0] * (1.0 - trans))) / trans);
	channels[1] = 255.0 - ((channels[1] - (A[1] * (1.0 - trans))) / trans);
	channels[2] = (channels[2] - A[2]) / trans + A[2];
	cv::Mat dehazed, dst;
	merge(channels, dehazed);
	dehazed.convertTo(dst, CV_8U);
	return dst;
}
