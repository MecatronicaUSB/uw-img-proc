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
	log(l, lnL);
	log(m, lnM);
	log(s, lnS);
	
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

cv::Mat GWA_CLIELAB(cv::Mat src) {
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