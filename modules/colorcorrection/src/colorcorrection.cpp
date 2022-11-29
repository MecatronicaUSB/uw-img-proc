/*********************************************************************/
/* Project: uw-img-proc									             */
/* Module:  colorcorrection								             */
/* File: 	colorcorrection.cpp								         */
/* Created:	20/11/2018				                                 */
/* Description:
	C++ Module of color cast eliminination using Gray World Assumption
*/
/*********************************************************************/

 /********************************************************************/
 /* Created by:                                                      */
 /* Geraldine Barreto (@geraldinebc)                                 */
/*********************************************************************/

/// Include auxiliary utility libraries
#include "../include/colorcorrection.h"

std::vector<Mat_<float>> BGRtoLab(cv::Mat src) {
	src.convertTo(src, CV_32F);
	std::vector<Mat_<float>> channels;
	split(src, channels);

	// RGB to LMS (0.000001 added to avoid errors calculating log)
	cv::Mat_<float> l, m, s;
	l = 0.3811*channels[2] + 0.5783*channels[1] + 0.0402*channels[0] + 0.000001;
	m = 0.1976*channels[2] + 0.7244*channels[1] + 0.0782*channels[0] + 0.000001;
	s = 0.0241*channels[2] + 0.1288*channels[1] + 0.8444*channels[0] + 0.000001;

	// ln(LMS)
	cv::Mat_<float> lnL, lnM, lnS;
	cv::log(l, lnL);
	cv::log(m, lnM);
	cv::log(s, lnS);

	std::vector<Mat_<float>> Lab;
	// log(LMS) to Lab
	Lab.push_back(((1 / sqrt(3))*lnL + (1 / sqrt(3))*lnM + (1 / sqrt(3))*lnS) / log(10));
	Lab.push_back(((1 / sqrt(6))*lnL + (1 / sqrt(6))*lnM + (-2 / sqrt(6))*lnS) / log(10));
	Lab.push_back(((1 / sqrt(2))*lnL + (-1 / sqrt(2))*lnM) / log(10));
	return Lab;
}

std::vector<Mat_<uchar>> LabtoBGR(std::vector<Mat_<float>> Lab) {
	// Lab to log(LMS)
	cv::Mat_<float> logL, logM, logS;
	logL = (sqrt(3) / 3)*Lab[0] + (sqrt(6) / 6)*Lab[1] + (sqrt(2) / 2)*Lab[2];
	logM = (sqrt(3) / 3)*Lab[0] + (sqrt(6) / 6)*Lab[1] + (-sqrt(2) / 2)*Lab[2];
	logS = (sqrt(3) / 3)*Lab[0] + (-sqrt(6) / 3)*Lab[1];

	// log(LMS) to LMS -> c = 10^LMS is equivalent to c = e^(LMS*ln(10))
	cv::Mat L, M, S;
	cv::exp(logL*log(10), L);
	cv::exp(logM*log(10), M);
	cv::exp(logS*log(10), S);

	// LMS to BGR
	std::vector<Mat_<uchar>> BGR;
	BGR.push_back(0.0497*L + (-0.2439)*M + 1.2045*S);
	BGR.push_back((-1.2186)*L + 2.3809*M + (-0.1624)*S);
	BGR.push_back(4.4679*L + (-3.5873)*M + 0.1193*S);
	return BGR;
}

double medianMat(cv::Mat src) {
	src = src.reshape(0, 1);
	vector<double> vectsrc;
	src.copyTo(vectsrc);
	nth_element(vectsrc.begin(), vectsrc.begin() + vectsrc.size() / 2, vectsrc.end());
	return vectsrc[vectsrc.size() / 2];
}

cv::Mat GWA_Lab(cv::Mat src) {
	std::vector<Mat_<float>> Lab = BGRtoLab(src);							// Transformation from RGB to laB color space
	Lab[1] = Lab[1] - mean(Lab[1]).val[0];									// Gray world assumption
	Lab[2] = Lab[2] - mean(Lab[2]).val[0];
	//Lab[1] = Lab[1] - medianMat(Lab[1]);									// Using the median instead of the mean 
	//Lab[2] = Lab[2] - medianMat(Lab[2]);									// Takes more time but in some cases gives better results
	std::vector<Mat_<uchar>> BGR = LabtoBGR(Lab);							// The color balanced image is converted back to BGR
	cv::Mat dst;
	merge(BGR, dst);
	return dst;
}

cv::Mat GWA_CIELAB(cv::Mat src) {
	cv::Mat LAB, lab[3], MEANS, gwa, bgr[3], dst;
	cv::cvtColor(src, LAB, COLOR_BGR2Lab);										// Conversion to the Lab color space
	split(LAB, lab);
	vector<uchar> means;
	double max;
	minMaxLoc(lab[0], NULL, &max);
	means.push_back(int(max));												// L -> Max(L)
	means.push_back(mean(lab[1])[0]);										// a -> mean(a)
	means.push_back(mean(lab[2])[0]);										// b -> mean(b)
	merge(means, MEANS);
	cv::cvtColor(MEANS, gwa, COLOR_Lab2BGR);									// Conversion to the BGR color space
	split(src, bgr);
	bgr[0] = 255 * bgr[0] / gwa.at<uchar>(0, 0);							// Gray World Assumption using the values calculated in CIELAB
	bgr[1] = 255 * bgr[1] / gwa.at<uchar>(0, 1);
	bgr[2] = 255 * bgr[2] / gwa.at<uchar>(0, 2);
	merge(bgr, 3, dst);
	return dst;
}

cv::Mat GWA_RGB(cv::Mat src) {
	cv::Mat channel[3];
	split(src, channel);
	float scale = (mean(channel[0])[0] + mean(channel[1])[0] + mean(channel[2])[0]) / 3;
	channel[0] = scale * channel[0] / mean(channel[0])[0];
	channel[1] = scale * channel[1] / mean(channel[1])[0];
	channel[2] = scale * channel[2] / mean(channel[2])[0];
	cv::Mat dst;
	merge(channel, 3, dst);
	return dst;
}

#if USE_GPU
	std::vector<cv::cuda::GpuMat> BGRtoLab_GPU(cv::cuda::GpuMat srcGPU) {
		cv::cuda::GpuMat BGR[3];
		cv::cuda::split(srcGPU, BGR);

		// RGB to LMS (0.000001 added to avoid errors calculating log)
		cv::cuda::GpuMat l, m, s;
		cv::cuda::addWeighted(BGR[2], 0.3811, BGR[1], 0.5783, 0.0, l);
		cv::cuda::addWeighted(l, 1.0, BGR[0], 0.0402, 0.000001, l);

		cv::cuda::addWeighted(BGR[2], 0.1976, BGR[1], 0.7244, 0.0, m);
		cv::cuda::addWeighted(m, 1.0, BGR[0], 0.0782, 0.000001, m);

		cv::cuda::addWeighted(BGR[2], 0.0241, BGR[1], 0.1288, 0.0, s);
		cv::cuda::addWeighted(s, 1.0, BGR[0], 0.8444, 0.000001, s);

		// ln(LMS)
		cv::cuda::GpuMat lnL, lnM, lnS;
		cv::cuda::log(l, lnL);
		cv::cuda::log(m, lnM);
		cv::cuda::log(s, lnS);

		// log(LMS) to Lab
		cv::cuda::GpuMat Lab[3];
		cv::cuda::addWeighted(lnL, sqrt(1 / 3), lnM, sqrt(1 / 3), 0.0, Lab[0]);
		cv::cuda::addWeighted(Lab[0], 1.0, lnS, sqrt(1 / 3), 0.0, Lab[0]);
		cv::cuda::divide(Lab[0], log(10), Lab[0]);

		cv::cuda::addWeighted(lnL, sqrt(1 / 6), lnM, sqrt(1 / 6), 0.0, Lab[1]);
		cv::cuda::addWeighted(Lab[1], 1.0, lnS, -2 / sqrt(6), 0.0, Lab[1]);
		cv::cuda::divide(Lab[1], log(10), Lab[1]);

		cv::cuda::addWeighted(lnL, sqrt(1 / 2), lnM, -sqrt(1 / 2), 0.0, Lab[2]);
		cv::cuda::divide(Lab[2], log(10), Lab[2]);

		std::vector<cv::cuda::GpuMat> LAB;
		LAB.push_back(Lab[0]);
		LAB.push_back(Lab[1]);
		LAB.push_back(Lab[2]);
		return LAB;
	}

	cv::cuda::GpuMat LabtoBGR_GPU(std::vector<cv::cuda::GpuMat> Lab) {
		// Lab to log(LMS)
		cv::cuda::GpuMat logL, logM, logS;
		cv::cuda::addWeighted(Lab[0], sqrt(3)/3, Lab[1], sqrt(6)/6, 0.0, logL);
		cv::cuda::addWeighted(logL, 1.0, Lab[2], sqrt(2)/2, 0.0, logL);

		cv::cuda::addWeighted(Lab[0], sqrt(3)/3, Lab[1], sqrt(6)/6, 0.0, logM);
		cv::cuda::addWeighted(logM, 1.0, Lab[2], -sqrt(2)/2, 0.0, logM);

		cv::cuda::addWeighted(Lab[0], sqrt(3)/3, Lab[1], -sqrt(6)/3, 0.0, logS);

		// log(LMS) to LMS -> c = 10^LMS is equivalent to c = e^(LMS*ln(10))
		cv::cuda::GpuMat L, M, S;
/*		cv::cuda::exp(logL*log(10), L);
		cv::cuda::exp(logM*log(10), M);
		cv::cuda::exp(logS*log(10), S);*/
		cv::cuda::exp(logL, L);
		cv::cuda::exp(logM, M);
		cv::cuda::exp(logS, S);

		// LMS to BGR
		cv::cuda::GpuMat bgr[3], BGR;
		cv::cuda::addWeighted(L, 0.0497, M, -0.2439, 0.0, bgr[0]);
		cv::cuda::addWeighted(bgr[0], 1.0, S, 1.2045, 0.0, bgr[0]);

		cv::cuda::addWeighted(L, -1.2186, M, 2.3809, 0.0, bgr[1]);
		cv::cuda::addWeighted(bgr[1], 1.0, S, -0.1624, 0.0, bgr[1]);

		cv::cuda::addWeighted(L, 4.4679, M, -3.5873, 0.0, bgr[2]);
		cv::cuda::addWeighted(bgr[2], 1.0, S, 0.1193, 0.0, bgr[2]);

		cv::cuda::merge(bgr, 3, BGR);
		return BGR;
	}

	cv::cuda::GpuMat GWA_Lab_GPU(cv::cuda::GpuMat srcGPU) {
		std::vector<GpuMat> Lab = BGRtoLab_GPU(srcGPU);						// Transformation from BGR to laB color space
		cv::cuda::subtract(Lab[1], mean(Lab[1])[0], Lab[1]);				// Gray world assumption (White balancing)
		cv::cuda::subtract(Lab[2], mean(Lab[2])[0], Lab[2]);
		cv::cuda::GpuMat dstGPU = LabtoBGR_GPU(Lab);						// Corrected image is converted back to RGB
		return dstGPU;
	}

	cv::cuda::GpuMat GWA_CIELAB_GPU(cv::cuda::GpuMat srcGPU) {
		cv::cuda::GpuMat LAB, lab[3], bgr[3], dstGPU;
		cv::Mat MEANS, gwa;
		cv::cuda::cvtColor(srcGPU, LAB, COLOR_BGR2Lab);							// Conversion to the Lab color space
		cv::cuda::split(LAB, lab);
		vector<uchar> means;
		double max;
		cv::cuda::minMax(lab[0], NULL, &max);
		means.push_back(int(max));												// L -> Max(L)
		means.push_back(mean(lab[1])[0]);										// a -> mean(a)
		means.push_back(mean(lab[2])[0]);										// b -> mean(b)
		merge(means, MEANS);
		cv::cuda::cvtColor(MEANS, gwa, cv::COLOR_Lab2BGR, 3);									// Conversion to the BGR color space
		cv::cuda::split(srcGPU, bgr);
		cv::cuda::multiply(bgr[0], 255 / gwa.at<uchar>(0, 0), bgr[0]);
		cv::cuda::multiply(bgr[1], 255 / gwa.at<uchar>(0, 1), bgr[1]);
		cv::cuda::multiply(bgr[2], 255 / gwa.at<uchar>(0, 2), bgr[2]);
		cv::cuda::merge(bgr, 3, dstGPU);
		return dstGPU;
	}

	cv::cuda::GpuMat GWA_RGB_GPU(cv::cuda::GpuMat srcGPU) {
		cv::cuda::GpuMat channel[3];
		cv::cuda::split(srcGPU, channel);
		float scale = (mean(channel[0])[0] + mean(channel[1])[0] + mean(channel[2])[0]) / 3;
		cv::cuda::multiply(channel[0], scale / mean(channel[0])[0], channel[0]);
		cv::cuda::multiply(channel[1], scale / mean(channel[1])[0], channel[1]);
		cv::cuda::multiply(channel[2], scale / mean(channel[2])[0], channel[2]);
		cv::cuda::GpuMat dstGPU;
		cv::cuda::merge(channel, 3, dstGPU);
		return dstGPU;
}
#endif