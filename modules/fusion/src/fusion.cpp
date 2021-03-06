/********************************************************************/
/* Project: uw-img-proc									            */
/* Module: 	fusion										            */
/* File: 	fusion.cpp										        */
/* Created:	18/02/2019				                                */
/* Description:
	C++ module for underwater image enhancement using a fusion based
	strategy
 /*******************************************************************/

 /*******************************************************************/
 /* Created by:                                                     */
 /* Geraldine Barreto (@geraldinebc)                                */
 /*******************************************************************/

/// Include auxiliary utility libraries
#include "../include/fusion.h"

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

cv::Mat hueIllumination(cv::Mat src) {														// Corrects the color and illumination
	cv::Mat LAB, lab[3], dst;
	cvtColor(src, LAB, COLOR_BGR2Lab);														// Conversion to the Lab color model
	split(LAB, lab);
	lab[0] = illuminationCorrection(lab[0]);												// Correction of ununiform illumination
	lab[1] = 127.5 * lab[1] / mean(lab[1])[0];												// Grey World Assumption
	lab[2] = 127.5 * lab[2] / mean(lab[2])[0];
	merge(lab, 3, LAB);
	cvtColor(LAB, dst, COLOR_Lab2BGR);														// Conversion to the BGR color model
	return dst;
}

void getHistogram(cv::Mat *channel, cv::Mat *hist) {										// Computes the histogram of a single channel
	int histSize = 256;
	float range[] = { 0, 256 };																// The histograms ranges from 0 to 255
	const float* histRange = { range };
	calcHist(channel, 1, 0, Mat(), *hist, 1, &histSize, &histRange, true, false);
}

cv::Mat histStretch(cv::Mat src, float percent, int direction) {
	cv::Mat histogram;
	getHistogram(&src, &histogram);
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
	return dst;
}

cv::Mat ICM(vector<Mat_<uchar>> channel, float percent) {								// Integrated Color Model
	Mat chan[3], result;
	for (int i = 0; i < 3; i++) chan[i] = histStretch(channel[i], percent, 1);			// Histogram stretching of each color channel
	merge(chan, 3, result);
	Mat HSV, hsv[3], dst;
	cvtColor(result, HSV, COLOR_BGR2HSV);												// Conversion to the HSV color model
	split(HSV, hsv);
	for (int i = 1; i < 3; i++) hsv[i] = histStretch(hsv[i], percent, 1);				// Histogram stretching of the Saturation and Value Channels
	merge(hsv, 3, HSV);
	cvtColor(HSV, dst, COLOR_HSV2BGR);													// Conversion to the BGR color model
	return dst;
}

cv::Mat dehazing(cv::Mat src) {															// Dehazed an underwater image
	vector<Mat_<uchar>> src_chan, new_chan;
	split(src, src_chan);

	new_chan.push_back(255 - src_chan[0]);												// Compute the new channels for the dehazing process
	new_chan.push_back(255 - src_chan[1]);
	new_chan.push_back(src_chan[2]);

	int size = sqrt(src.total()) / 40;													// Making the size bigger creates halos around objects
	cv::Mat bright_chan = brightChannel(new_chan, size);								// Compute the bright channel image
	cv::Mat mcd = maxColDiff(src_chan);													// Compute the maximum color difference

	cv::Mat src_HSV, S;
	cv::cvtColor(src, src_HSV, COLOR_BGR2HSV);
	extractChannel(src_HSV, S, 1);
	cv::Mat rectified = rectify(S, bright_chan, mcd);									// Rectify the bright channel image

	cv::Mat src_gray;
	cv::cvtColor(src, src_gray, COLOR_BGR2GRAY);
	vector<uchar> A;
	A = lightEstimation(src_gray, size, bright_chan, new_chan);							// Estimate the atmospheric light
	cv::Mat trans = transmittance(rectified, A);										// Compute the transmittance image

	cv::Mat filtered;
	guidedFilter(src_gray, trans, filtered, 30, 0.001, -1);								// Refine the transmittance image

	vector<Mat_<float>> chan_dehazed;
	chan_dehazed.push_back(new_chan[0]);
	chan_dehazed.push_back(new_chan[1]);
	chan_dehazed.push_back(new_chan[2]);
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

cv::Mat filter_mask() {
	float h[5] = { 1.0 / 16.0, 4.0 / 16.0, 6.0 / 16.0, 4.0 / 16.0, 1.0 / 16.0 };
	cv::Mat kernel = Mat(5, 5, CV_32F);
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) kernel.at<float>(i, j) = h[i] * h[j];
	}
	return kernel;
}

cv::Mat laplacian_contrast(cv::Mat img) {
	img.convertTo(img, CV_32F);
	cv::Mat laplacian = Mat(img.rows, img.cols, CV_32F);
	Laplacian(img, laplacian, img.depth());
	convertScaleAbs(laplacian, laplacian);
	return laplacian;
}

cv::Mat local_contrast(cv::Mat img, cv::Mat kernel) {
	img.convertTo(img, CV_32F);
	cv::Mat blurred = Mat(img.rows, img.cols, CV_32F);
	filter2D(img, blurred, img.depth(), kernel);
	cv::Mat contrast = Mat(img.rows, img.cols, CV_32F);
	contrast = abs(img.mul(img) - blurred.mul(blurred));
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			contrast.at<float>(i, j) = sqrt(contrast.at<float>(i, j));
		}
	}
	return contrast;
}

cv::Mat saliency(cv::Mat img, cv::Mat kernel) {
	cv::Mat blurred, img_Lab;
	blurred = Mat(img.rows, img.cols, CV_32F);
	filter2D(img, blurred, img.depth(), kernel);
	cvtColor(blurred, img_Lab, COLOR_BGR2Lab);
	cv::Mat chan_lab[3], l, a, b;
	split(img_Lab, chan_lab);
	chan_lab[0].convertTo(l, CV_32F);
	chan_lab[1].convertTo(a, CV_32F);
	chan_lab[2].convertTo(b, CV_32F);
	l = mean(l).val[0] - l;
	a = mean(a).val[0] - a;
	b = mean(b).val[0] - b;
	cv::Mat saliency = Mat::zeros(img.rows, img.cols, CV_32F);
	accumulateSquare(l, saliency);
	accumulateSquare(a, saliency);
	accumulateSquare(b, saliency);
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			saliency.at<float>(i, j) = sqrt(saliency.at<float>(i, j));
		}
	}
	return saliency;
}

cv::Mat exposedness(cv::Mat img) {
	img.convertTo(img, CV_32F, 1.0 / 255.0);
	cv::Mat exposedness = Mat(img.rows, img.cols, CV_32F);
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			exposedness.at<float>(i, j) = exp(-1.0 * pow(img.at<float>(i, j) - 0.5, 2.0) / (2.0 * pow(0.25, 2.0)));
		}
	}
	return exposedness;
}

vector<Mat> weight_norm(cv::Mat w1, cv::Mat w2) {
	w1.convertTo(w1, CV_32F);
	w2.convertTo(w2, CV_32F);
	vector<Mat> norm;
	norm.push_back(w1 / (w1 + w2));
	norm.push_back(w2 / (w1 + w2));
	return norm;
}

vector<Mat_<float>> laplacian_pyramid(cv::Mat img, int levels) {
	vector<Mat_<float>> l_pyr;
	Mat_<float> downsampled, upsampled, lap_pyr, current_layer = img;
	for (int i = 0; i < levels - 1; i++) {
		pyrDown(current_layer, downsampled);
		pyrUp(downsampled, upsampled, current_layer.size());
		subtract(current_layer, upsampled, lap_pyr);
		l_pyr.push_back(lap_pyr);
		current_layer = downsampled;
	}
	l_pyr.push_back(current_layer);
	return l_pyr;
}

Mat pyramid_fusion(Mat *pyramid, int levels) {
	for (int i = levels-1; i > 0; i--) {
		Mat_<float> upsampled;
		pyrUp(pyramid[i], upsampled, pyramid[i - 1].size());
		add(pyramid[i - 1], upsampled, pyramid[i - 1]);
	}
	pyramid[0].convertTo(pyramid[0], CV_8U);
	return pyramid[0];
}

#if USE_GPU
cv::cuda::GpuMat illuminationCorrection_GPU(cv::cuda::GpuMat srcGPU) {						// Homomorphic Filter
	cv::cuda::GpuMat imgTemp1 = Mat::zeros(srcGPU.size(), CV_32FC1);
	cv::cuda::normalize(srcGPU, imgTemp1, 0, 1, NORM_MINMAX, CV_32FC1);						// Normalize the channel
	cv::cuda::add(imgTemp1, imgTemp1, 0.000001);
	cv::cuda::log(imgTemp1, imgTemp1);														// Calculate the logarithm

	cv::cuda::GpuMat fftimg;
	fft_GPU(imgTemp1, fftimg);																// Fourier transform

	cv::Mat filter = gaussianFilter_GPU(fftimg, 0.7, 1.0, 0.5);								// Gaussian Emphasis High-Pass Filter
	cv::Mat bimg;
	cv::Mat bchannels[] = { cv::Mat_<float>(filter), cv::Mat::zeros(filter.size(), CV_32F) };
	merge(bchannels, 2, bimg);

	dftShift(bimg);																			// Shift the filter
	cv::cuda::GpuMat bimgGPU;
	bimgGPU.upload(bimg);
	cv::cuda::mulSpectrums(fftimg, bimgGPU, fftimg, 0);										// Apply the filter to the image in frequency domain

	cv::cuda::GpuMat ifftimg;
	cv::cuda::dft(fftimg, ifftimg, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);					// Apply the inverse Fourier transform
	cv::cuda::GpuMat expimg = Mat::zeros(ifftimg.size(), CV_32FC1);							// Calculate the exponent
	cv::cuda::exp(ifftimg, expimg);

	cv::cuda::GpuMat dst;
	dst = cv::cuda::GpuMat(expimg, cv::Rect(0, 0, srcGPU.cols, srcGPU.rows));					// Eliminate the padding from the image
	cv::cuda::normalize(dst, dst, 0, 255, NORM_MINMAX, CV_8U);								// Normalize the results
	return dst;
}

void fft_GPU(const cv::cuda::GpuMat &srcGPU, cv::cuda::GpuMat &dst) {						// Fast Fourier Transform
	cv::cuda::GpuMat padded, gpu_dft, dft_mag;
	int m = getOptimalDFTSize(srcGPU.rows);													// Optimal Size to calculate the Discrete Fourier Transform 
	int n = getOptimalDFTSize(srcGPU.cols);
	cv::cuda::copyMakeBorder(srcGPU, padded, 0, m - srcGPU.rows, 0, n - srcGPU.cols, BORDER_REPLICATE);	// Resize to optimal FFT size
	dst = cv::cuda::GpuMat(m, n, CV_32FC2);
	cv::cuda::dft(padded, dst, padded.size());												// Aply the Discrete Fourier Transform
	cv::cuda::divide(dst, dst.total(), dst);												// Normalize
}

cv::Mat gaussianFilter_GPU(cv::Mat img, float sigma, float high, float low) {
	cv::Mat radius(img.rows, img.cols, CV_32F);
	int cx = img.rows / 2;
	int cy = img.cols / 2;
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			radius.at<float>(i, j) = - (pow(i - cx, 2) + pow(j - cy, 2)) / (2 * pow(sigma, 2));
		}
	}
	cv::cuda::GpuMat radius_GPU, expo, filterGPU, dst;
	radius_GPU.upload(radius);
	cv::cuda::exp(radius_GPU ), expo);
	cv::cuda::subtract(1, expo, dst);
	cv::cuda::addWeighted(dst, high - low, 0, 0, low, filterGPU);								// High-pass Emphasis Filter
	cv::Mat filter;
	filterGPU.download(filter);
	return filter;
}

cv::cuda::GpuMat histStretch_GPU(cv::cuda::GpuMat srcGPU, float percent, int direction) {
	cv::cuda::GpuMat hist;
	cv::Mat histogram;
	cv::cuda::calcHist(srcGPU, hist);
	hist.download(histogram);
	float percent_sum = 0.0, channel_min = -1.0, channel_max = -1.0;
	float percent_min = percent / 100.0, percent_max = 1.0 - percent_min;
	int i = 0;

	while (percent_sum < percent_max * srcGPU.total()) {
		if (percent_sum < percent_min * srcGPU.total()) channel_min++;
		percent_sum += histogram.at<float>(i, 0);
		channel_max++;
		i++;
	}

	cv::cuda::GpuMat dst;
	if (direction = 0) {
		cv::cuda::subtract(srcGPU, channel_min, dst);										// Stretches the channel towards the Upper side
		cv::cuda::multiply(dst, (255.0 - channel_min) / (channel_max - channel_min), dst);
		cv::cuda::add(dst, channel_min, dst);
	}
	else if (direction = 2) {
		cv::cuda::subtract(srcGPU, channel_min, dst);										// Stretches the channel towards the Lower side
		cv::cuda::multiply(dst, channel_max / (channel_max - channel_min), dst);
	}
	else {
		cv::cuda::subtract(srcGPU, channel_min, dst);										// Stretches the channel towards both sides
		cv::cuda::multiply(dst, 255.0 / (channel_max - channel_min), dst);
	}
	dst.convertTo(dst, CV_8U);
	return dst;
}

cv::cuda::GpuMat ICM_GPU(cv::cuda::GpuMat srcGPU, float percent) {						// Integrated Color Model
	cv::cuda::GpuMat channel[3];
	cv::cuda::split(srcGPU, channel);
	cv::cuda::GpuMat chan[3], result;
	for (int i = 0; i < 3; i++) chan[i] = histStretchGPU(channel[i], percent, 1);		// Histogram stretching of each color channel
	cv::cuda::merge(chan, 3, result);
	cv::cuda::GpuMat HSV, hsv[3], dst;
	cv::cuda::cvtColor(result, HSV, COLOR_BGR2HSV);										// Conversion to the HSV color model
	cv::cuda::split(HSV, hsv);
	for (int i = 1; i < 3; i++) hsv[i] = histStretchGPU(hsv[i], percent, 1);			// Histogram stretching of the Saturation and Value Channels
	cv::cuda::merge(hsv, 3, HSV);
	cv::cuda::cvtColor(HSV, dst, COLOR_HSV2BGR);										// Conversion to the BGR color model
	return dst;
}

cv::cuda::GpuMat dehazing_GPU(cv::cuda::GpuMat srcGPU) {
	cv::cuda::GpuMat new_chanGPU, dstGPU;

	// Split the RGB channels (BGR for OpenCV)
	std::vector<cv::cuda::GpuMat> srcGPU_chan, newGPU_chan;
	cv::cuda::split(srcGPU, srcGPU_chan);

	// Compute the new channels for the dehazing process
	cv::cuda::subtract(255, srcGPU_chan[0], new_chanGPU);
	newGPU_chan.push_back(new_chanGPU);
	cv::cuda::subtract(255, srcGPU_chan[1], new_chanGPU);
	newGPU_chan.push_back(new_chanGPU);
	newGPU_chan.push_back(srcGPU_chan[2]);

	// Compute the bright channel image
	int size = sqrt(src.total()) / 50;
	cv::cuda::GpuMat bright_chan = brightChannel_GPU(newGPU_chan, size);

	// Compute the maximum color difference
	cv::cuda::GpuMat mcd = maxColDiff_GPU(srcGPU_chan);

	// Rectify the bright channel image
	cv::cuda::GpuMat src_HSV, HSV_chan[3];
	cv::cuda::cvtColor(srcGPU, src_HSV, COLOR_BGR2HSV);
	cv::cuda::split(src_HSV, HSV_chan);
	cv::cuda::GpuMat rectified = rectify_GPU(HSV_chan[1], bright_chan, mcd);

	// Estimate the atmospheric light
	cv::Mat src_gray;
	cv::cvtColor(src, src_gray, COLOR_BGR2GRAY);
	vector<uchar> A;
	A = lightEstimation_GPU(src_gray, size, bright_chan, newGPU_chan);

	// Compute the transmittance image
	cv::cuda::GpuMat trans = transmittance(rectified, A);

	// Refine the transmittance image
	cv::Mat filtered;
	trans.download(transmit);
	cv::cuda::guidedFilter(src_gray, transmit, filtered, 30, 0.001, -1);

	// Dehaze the image channels
	std::vector<cv::cuda::GpuMat> chan_dehazed;
	chan_dehazed.push_back(newGPU_chan[0]);
	chan_dehazed.push_back(newGPU_chan[1]);
	chan_dehazed.push_back(newGPU_chan[2]);

	trans.upload(filtered);
	dstGPU = dehaze(chan_dehazed, A, trans);
}

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
		cv::cuda::multiply(sub, 255.0 / (255.0 - A[i]), t[i]);
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
	cv::cuda::subtract(255.0, B[1], C[1]);
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

//Mat lookUpTable(1, 256, CV_32F);
//for (int i = 0; i < 256; ++i) lookUpTable.at<float>(0,i) = 255.0 * sqrt(0.32 * log(255.0/(255.0-i)));
//LUT(x, lookUpTable, dst);
//dst.convertTo(dst, CV_8U);