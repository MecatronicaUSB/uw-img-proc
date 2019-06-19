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

cv::Mat illuminationCorrection(cv::Mat src) {												// Same as homomorphicFilter but just in V channel
	Mat imgTemp1 = Mat::zeros(src.size(), CV_32FC1);
	normalize(src, imgTemp1, 0, 1, NORM_MINMAX, CV_32FC1);									// Normalize the channel
	imgTemp1 = imgTemp1 + 0.000001;
	log(imgTemp1, imgTemp1);																// Calculate the logarithm

	cv::Mat fftimg;
	fft(imgTemp1, fftimg);																	// Fourier transform

	cv::Mat filter = gaussianFilter(fftimg, 0.7, 1.0, 0.5);									// Gaussian Emphasis High-Pass Filter
	cv::Mat bimg;
	cv::Mat bchannels[] = { cv::Mat_<float>(filter), cv::Mat::zeros(filter.size(), CV_32F) };
	cv::merge(bchannels, 2, bimg);

	dftShift(bimg);																			// Shift the filter
	cv::mulSpectrums(fftimg, bimg, fftimg, 0);												// Apply the filter to the image in frequency domain

	cv::Mat ifftimg;
	idft(fftimg, ifftimg, CV_HAL_DFT_REAL_OUTPUT);											// Apply the inverse Fourier transform

	cv::Mat expimg = Mat::zeros(ifftimg.size(), CV_32FC1);									// Calculate the exponent
	cv::exp(ifftimg, expimg);

	cv::Mat dst;
	dst = cv::Mat(expimg, cv::Rect(0, 0, src.cols, src.rows));								// Eliminate the padding from the image
	normalize(dst, dst, 0, 255, NORM_MINMAX, CV_8U);										// Normalize the results
	return dst;
}

void fft(const cv::Mat &src, cv::Mat &dst) {
	cv::Mat padded;
	int m = cv::getOptimalDFTSize(src.rows);
	int n = cv::getOptimalDFTSize(src.cols);
	cv::copyMakeBorder(src, padded, 0, m - src.rows, 0, n - src.cols, cv::BORDER_REPLICATE);// Resize to optimal size

	cv::Mat plane[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };	// Add complex column to store the imaginary result
	cv::Mat imgComplex;
	cv::merge(plane, 2, imgComplex);
	cv::dft(imgComplex, dst);																// Aply the Discrete Fourier Transform
	dst = dst / dst.total();																// Divide by the size of the image to normalize
}

cv::Mat gaussianFilter(cv::Mat img, float sigma, float high, float low) {
	cv::Mat filter(img.rows, img.cols, CV_32F);
	int cx = img.rows / 2;
	int cy = img.cols / 2;

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			float radius = pow(i - cx, 2) + pow(j - cy, 2);
			filter.at<float>(i, j) = (high - low) * (1 - exp(-radius / (2 * pow(sigma, 2)))) + low;		// High-pass Emphasis Filter
		}
	}
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

	q0.copyTo(tmp);																			// Each quadrant its reversed with its diagonally opposite quadrant
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);
}

cv::Mat colorCorrection(cv::Mat src) {
	cv::Mat LAB, lab[3], gwa, bgr[3], dst;
	cvtColor(src, LAB, COLOR_BGR2Lab);														// Conversion to the Lab color model
	split(LAB, lab);
	vector<uchar> means;
	double max;
	minMaxLoc(lab[0], NULL, &max);
	means.push_back(int(max));
	means.push_back(mean(lab[1])[0]);
	means.push_back(mean(lab[2])[0]);
	merge(means, LAB);
	cvtColor(LAB, gwa, COLOR_Lab2BGR);														// Conversion to the BGR color model
	split(src, bgr);
	bgr[0] = 255 * bgr[0] / gwa.at<uchar>(0, 0);
	bgr[1] = 255 * bgr[1] / gwa.at<uchar>(0, 1);
	bgr[2] = 255 * bgr[2] / gwa.at<uchar>(0, 2);
	merge(bgr, 3, dst);
	return dst;
}

cv::Mat hueIllumination(cv::Mat src) {
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

cv::Mat dehazing(cv::Mat src) {
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

	cv::Mat dst;
	vector<Mat_<float>> chan_dehazed;
	chan_dehazed.push_back(new_chan[0]);
	chan_dehazed.push_back(new_chan[1]);
	chan_dehazed.push_back(new_chan[2]);
	dst = dehaze(chan_dehazed, A, filtered);											// Dehaze the image channels
	return dst;
}

cv::Mat brightChannel(vector<Mat_<uchar>> channels, int size) {								// Generates the Bright Channel Image
	cv::Mat maxRGB = max(max(channels[0], channels[1]), channels[2]);						// Maximum Color Image
	cv::Mat element, bright_chan;
	element = getStructuringElement(MORPH_RECT, Size(size, size), Point(-1, -1));			// Maximum filter
	dilate(maxRGB, bright_chan, element);													// Dilates the maxRGB image
	return bright_chan;
}

cv::Mat maxColDiff(vector<Mat_<uchar>> channels) {											// Generates the Maximum Color Difference Image
	float means[3] = { mean(channels[0])[0], mean(channels[1])[0], mean(channels[2])[0] };
	cv::Mat c = Mat(1, 3, CV_32F, means);
	cv::Mat sorted;
	sortIdx(c, sorted, SORT_EVERY_ROW + SORT_ASCENDING);									// Orders the mean of the channels from low to high

	cv::Mat cmin = channels[sorted.at<int>(0, 0)];
	cv::Mat cmid = channels[sorted.at<int>(0, 1)];
	cv::Mat cmax = channels[sorted.at<int>(0, 2)];

	cv::Mat a, b, mcd;
	a = max(cmax - cmin, 0);																// Calculates the maximum values for the MCD image
	b = max(cmid - cmin, 0);
	mcd = 255 - max(a, b);
	return mcd;
}

cv::Mat  rectify(cv::Mat S, cv::Mat bc, cv::Mat mcd) {											// Rectifies the Bright Channel Image
	double lambda;
	minMaxIdx(S, NULL, &lambda);															// Maximum value of the Saturation channel
	lambda = lambda / 255.0;																// Normalization for the next step
	cv::Mat correct;
	addWeighted(bc, lambda, mcd, 1.0 - lambda, 0.0, correct);
	return correct;
}

vector<uchar> lightEstimation(cv::Mat src_gray, int size, cv::Mat bc, vector<Mat_<uchar>> channels) {	// Estimates the atmospheric light
	cv::Mat variance, sorted;
	sqrBoxFilter(src_gray, variance, -1, Size(size, size), Point(-1, -1), true, BORDER_DEFAULT);

	bc = bc.reshape(0, 1);
	sortIdx(bc, sorted, SORT_EVERY_ROW + SORT_ASCENDING);				// Sorts the bright channel pixels from low (dark) to high (bright)

	int x, y, percentage = round(src_gray.total()*0.01);				// 0.01 pixel percentage of the original image
	vector<float> darkest;
	vector<int> X, Y;
	for (int i = 0; i < percentage; i++) {
		x = floor(sorted.at<int>(0, i) / src_gray.cols);				// Conversion from the reshaped array to the original matrix coordinates
		if (x == 0) y = sorted.at<int>(0, i);
		else y = sorted.at<int>(0, i) - x * src_gray.cols;
		darkest.push_back(variance.at<float>(x, y));						// Values of the variance in the location of the bright channel 0.01% darkest pixels
		X.push_back(x);													// Matrix coordinates of the values
		Y.push_back(y);
	}

	Point minLoc;
	minMaxLoc(darkest, NULL, NULL, &minLoc, NULL);						// Location of the minimum value of the selected pixels from the variance

	vector<uchar> A;
	for (int i = 0; i < 3; i++) A.push_back(channels[i].at<uchar>(X[minLoc.x], Y[minLoc.x]));

	// PARA VISUALIZAR
	//src_gray.at<char>(X[minLoc.x], Y[minLoc.x]) = 255;
	//namedWindow("A point", WINDOW_KEEPRATIO);
	//imshow("A point", src_gray);

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

//Mat lookUpTable(1, 256, CV_32F);
//for (int i = 0; i < 256; ++i) lookUpTable.at<float>(0,i) = 255.0 * sqrt(0.32 * log(255.0/(255.0-i)));
//LUT(x, lookUpTable, dst);
//dst.convertTo(dst, CV_8U);

		//PESOS DEL PAPER

		//Mat w1[4], w2[4];
		//w1[0] = imread("/Users/Geraldine/Pictures/Datasets/TEST/test_52.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		//w1[1] = imread("/Users/Geraldine/Pictures/Datasets/TEST/test_54.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		//w1[2] = imread("/Users/Geraldine/Pictures/Datasets/TEST/test_56.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		//w1[3] = imread("/Users/Geraldine/Pictures/Datasets/TEST/test_58.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		//w2[0] = imread("/Users/Geraldine/Pictures/Datasets/TEST/test_53.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		//w2[1] = imread("/Users/Geraldine/Pictures/Datasets/TEST/test_55.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		//w2[2] = imread("/Users/Geraldine/Pictures/Datasets/TEST/test_57.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		//w2[3] = imread("/Users/Geraldine/Pictures/Datasets/TEST/test_59.jpg", CV_LOAD_IMAGE_GRAYSCALE);

		//w1[0].convertTo(w1[0], CV_32F);
		//w1[1].convertTo(w1[1], CV_32F);
		//w1[2].convertTo(w1[2], CV_32F);
		//w1[3].convertTo(w1[3], CV_32F);

		//w2[0].convertTo(w2[0], CV_32F);
		//w2[1].convertTo(w2[1], CV_32F);
		//w2[2].convertTo(w2[2], CV_32F);
		//w2[3].convertTo(w2[3], CV_32F);

		//Mat w_1 = Mat(w1[0].rows, w1[0].cols, CV_32F);
		//w_1 = w1[0] + w1[1] + w1[2] + w1[3];
		//Mat w_2 = Mat(src[0].rows, src[0].cols, CV_32F); 
		//w_2 = w2[0] + w2[1] + w2[2] + w2[3];

		//Mat w_1_norm = Mat(src[0].rows, src[0].cols, CV_32F);
		//w_1_norm = 255.0 * w_1 / (w_1 + w_2);
		//Mat w_2_norm = Mat(src[0].rows, src[0].cols, CV_32F);
		//w_2_norm = 255.0 * w_2 / (w_1 + w_2);

		//w_1_norm.convertTo(w_1_norm, CV_8U);
		//w_2_norm.convertTo(w_2_norm, CV_8U);

		//Mat w_1_norm = imread("/Users/Geraldine/Pictures/Datasets/TEST/test_a_1.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		//Mat w_2_norm = imread("/Users/Geraldine/Pictures/Datasets/TEST/test_a_2.jpg", CV_LOAD_IMAGE_GRAYSCALE);