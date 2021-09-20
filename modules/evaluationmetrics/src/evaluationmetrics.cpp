/*********************************************************************/
/* Project: uw-img-proc                                              */
/* Module:  evaluationmetrics                                        */
/* File:    evaluationmetrics.cpp                                    */
/* Created: 09/12/2018                                               */
/* Description:
    C++ Module of image quality metrics for image processing evaluation
 /********************************************************************/

 /********************************************************************/
 /* Created by:                                                      */
 /* Geraldine Barreto (@geraldinebc)                                 */
 /********************************************************************/

/// Include auxiliary utility libraries
#include "../include/evaluationmetrics.h"

float entropy(cv::Mat img) {
    cv::Mat hist, normhist, prob, logP;
    getHistogram(&img, &hist);
    normalize(hist, normhist, 0, 1, NORM_MINMAX);               // Normalized histogram
    prob = normhist / sum(normhist).val[0];                     // Probability
    prob += 0.00000001;                                         // Added 0.00000001 to avoid errors calculating the logarithm
    log(prob, logP);                                            // Natural logarithm of the probability
    float entropy = -1.0*sum(prob.mul(logP/log(2))).val[0];     // Computes the entropy according to the Shannon Index  
    return entropy;
}

float averageEntropy(std::vector<Mat> chanRGB) {            // Entropy considering the RGB components
    float AE = sqrt((pow(entropy(chanRGB[0]), 2) + pow(entropy(chanRGB[1]), 2) + pow(entropy(chanRGB[2]), 2))/3);
    return AE;
}

float averageContrast(std::vector<Mat> chanRGB) {
    cv::Mat Grad[3], Grad2[3];
    int kernel_size = 3, scale = 1, delta = 0, ddepth = CV_32F;
    Laplacian(chanRGB[0], Grad[0], ddepth, kernel_size, scale, delta, BORDER_DEFAULT);      // RGB Gradients
    Laplacian(chanRGB[1], Grad[1], ddepth, kernel_size, scale, delta, BORDER_DEFAULT);
    Laplacian(chanRGB[2], Grad[2], ddepth, kernel_size, scale, delta, BORDER_DEFAULT);  
    
    pow(Grad[0], 2, Grad2[0]), pow(Grad[1], 2, Grad2[1]), pow(Grad[2], 2, Grad2[2]);        // Grad^2
    cv::Mat Grads, sqrGrads;
    Grads = (Grad2[0] + Grad2[1] + Grad2[2]) / 3;
    sqrt(Grads, sqrGrads);
    cv::Mat C = sqrGrads(Rect(0, 0, sqrGrads.cols-2, sqrGrads.rows-2));
    float AC = sum(C)[0] / ((sqrGrads.rows - 1)*(sqrGrads.cols - 1));                       // Normalized sum of gradients
    return AC;
}

float averageLuminance(cv::Mat L) {
    float AL = sum(L)[0] / L.total();                                       // Normalized sum of the luminance component
    return AL;
}

float getNNF(float AL) {
    float OL = 127.5;
    float NNF = (OL - abs(AL-OL)) / OL;                                     // Normalized Neighborhood Function
    return NNF;
}

float getCAF(float AE, float AC, float NNF) {
    float CAF = AE + pow(AC,1/4) + pow(NNF,3);                              // Comprehensive Assessment Function
    return CAF;
}

float getMSE(cv::Mat src, cv::Mat dst) {
    cv::Mat diff(src.rows, src.cols, CV_32F);
    absdiff(src, dst, diff);
    pow(diff, 2, diff);
    float mse = sum(diff)[0] / src.total();                             // Mean Square Error
    return mse;
}

float getPSNR(float mse) {
    float psnr = 20 * log10(255 / sqrt(mse));                               // Peak Signal to Noise Ratio
    return psnr;
}

float sharpness(cv::Mat src){
    cv::Mat padded, src_dft, dft_mag;
    int m = getOptimalDFTSize(src.rows);                    // Optimal Size to calculate the Discrete Fourier Transform 
    int n = getOptimalDFTSize(src.cols);
    copyMakeBorder(src, padded, 0, m - src.rows, 0, n - src.cols, BORDER_CONSTANT, Scalar::all(0)); // Resize to optimal FFT size
    cv::Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
    merge(planes, 2, src_dft);                              // Plane with zeros is added so the the real and complex results will fit in the source matrix
    dft(src_dft, src_dft);                                  // Discrete Fourier Transform
    split(src_dft / src.total(), planes);                   // The normalized result is splitted -> planes[0] = Re(DFT(src)), planes[1] = Im(DFT(src))
    magnitude(planes[0], planes[1], dft_mag);               // dft_mag = sqrt(Re(DFT(src))^2+Im(DFT(src))^2)
    double max;
    minMaxIdx(dft_mag, NULL, &max);                         // Maximum value of the Fourier transform magnitude
    double thresh = max/1000;                               // Threshold to calculate the IQM
    cv::Mat dft_thresh;
    threshold(dft_mag, dft_thresh, thresh, 1, 0);           // If dft_mag > thresh set to 1, else set to 0
    float TH = countNonZero(dft_thresh);                    // Number of pixels in dft_mag whose pixel value > thres
    float IQM = TH / src.total();                           // Computes the Sharpness Measure
    return IQM;
}

void getHistogram(cv::Mat *channel, cv::Mat *hist) {        // Computes the intensity distribution histogram
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };
    calcHist(channel, 1, 0, Mat(), *hist, 1, &histSize, &histRange, true, false);
}

cv::Mat printHist(cv::Mat histogram, Scalar color) {
    // Finding the maximum value of the histogram. It will be used to scale the histogram to fit the image
    int max = 0;
    for (int i = 0; i < 256; i++) {
        if (histogram.at<float>(i, 0) > max) max = histogram.at<float>(i, 0);
    }
    // Histogram Image
    cv::Mat imgHist(1480, 1580, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::Point pt1, pt2;
    pt1.y = 1380;
    for (int i = 0; i < 256; i++) {
        pt1.x = 150 + 5 * i + 1;
        pt2.x = 150 + 5 * i + 3;
        pt2.y = 1380 - 1280 * histogram.at<float>(i, 0) / max;
        cv::rectangle(imgHist, pt1, pt2, color, cv::FILLED);
    }
    // y-axis labels
    cv::rectangle(imgHist, cv::Point(130, 1400), cv::Point(1450, 80), cv::Scalar(0, 0, 0), 1);
    cv::putText(imgHist, std::to_string(max), cv::Point(10, 100), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 0), 2.0);
    cv::putText(imgHist, std::to_string(max * 3 / 4), cv::Point(10, 420), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 0), 2.0);
    cv::putText(imgHist, std::to_string(max / 2), cv::Point(10, 740), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 0), 2.0);
    cv::putText(imgHist, std::to_string(max / 4), cv::Point(10, 1060), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 0), 2.0);
    cv::putText(imgHist, std::to_string(0), cv::Point(10, 1380), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 0), 2.0);
    // x-axis labels
    cv::putText(imgHist, std::to_string(0), cv::Point(152 - 7 * 1, 1430), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 0), 2.0);
    cv::putText(imgHist, std::to_string(63), cv::Point(467 - 7 * 2, 1430), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 0), 2.0);
    cv::putText(imgHist, std::to_string(127), cv::Point(787 - 7 * 3, 1430), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 0), 2.0);
    cv::putText(imgHist, std::to_string(191), cv::Point(1107 - 7 * 3, 1430), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 0), 2.0);
    cv::putText(imgHist, std::to_string(255), cv::Point(1427 - 7 * 3, 1430), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 0), 2.0);

    return imgHist;
}

#if USE_GPU
float entropy_GPU(cv::cuda::GpuMat img) {
    cv::cuda::GpuMat hist, normhist, prob, logP, mult;
    cv::cuda::calcHist(img, hist);
    cv::cuda::normalize(hist, normhist, 0, 1, NORM_MINMAX);             // Normalized histogram
    cv::cuda::divide(normhist, cv::cuda::sum(normhist).val[0], prob);   // Probability
    cv::cuda::add(prob, 0.000000001, prob);                             // Add 0.000000001 to avoid errors calculating the logarithm
    cv::cuda::log(prob, logP);                                          // Natural logarithm of the probability
    cv::cuda::multiply(prob, logP, mult);
    cv::cuda::divide(mult, log(2), mult);
    float entropy = -1.0*cv::cuda::sum(mult).val[0];                    // Computes the entropy according to the Shannon Index  
    return entropy;
}

float averageEntropy_GPU(std::vector<cv::cuda::GpuMat> chanRGB) {           // Entropy considering the RGB components
    float AE = sqrt((pow(entropy_GPU(chanRGB[0]), 2) + pow(entropy_GPU(chanRGB[1]), 2) + pow(entropy_GPU(chanRGB[2]), 2))/3);
    return AE;
}

float averageContrast_GPU(std::vector<cv::cuda::GpuMat> chanRGB) {
    cv::cuda::GpuMat Grad[3], Grad2[3];
    int kernel_size = 3, scale = 1, delta = 0, ddepth = CV_32F;
    cv::cuda::Laplacian(chanRGB[0], Grad[0], ddepth, kernel_size, scale, delta, BORDER_DEFAULT);                    // RGB Gradients
    cv::cuda::Laplacian(chanRGB[1], Grad[1], ddepth, kernel_size, scale, delta, BORDER_DEFAULT);
    cv::cuda::Laplacian(chanRGB[2], Grad[2], ddepth, kernel_size, scale, delta, BORDER_DEFAULT);

    cv::cuda::pow(Grad[0], 2, Grad2[0]), cv::cuda::pow(Grad[1], 2, Grad2[1]), cv::cuda::pow(Grad[2], 2, Grad2[2]);  // Grad^2
    cv::cuda::GpuMat Grads, sqrGrads;
    cv::cuda::add(Grad2[0], Grad2[1], Grads);
    cv::cuda::add(Grads, Grad2[2], Grads);
    cv::cuda::divide(Grads, 3, Grads);
    cv::cuda::sqrt(Grads, sqrGrads);
    cv::cuda::GpuMat C = sqrGrads(Rect(0, 0, sqrGrads.cols - 2, sqrGrads.rows - 2));
    float AC = cv::cuda::sum(C)[0] / ((sqrGrads.rows - 1)*(sqrGrads.cols - 1));                                     // Normalized sum of gradients
    return AC;
}

float averageLuminance_GPU(cv::cuda::GpuMat L) {
    float AL = cv::cuda::sum(L)[0] / L.total();                                 // Normalized sum of the luminance component
    return AL;
}

float getMSE_GPU(cv::cuda::GpuMat src, cv::cuda::GpuMat dst) {
    cv::cuda::GpuMat diff, diff2;
    cv::cuda::absdiff(src, dst, diff);
    cv::cuda::pow(diff, 2, diff2);
    float mse = cv::cuda::sum(diff2)[0] / src.total();                          // Mean Square Error
    return mse;
}

float sharpness_GPU(cv::cuda::GpuMat srcGPU) {
    cv::cuda::GpuMat padded, gpu_dft, dft_mag;
    int m = getOptimalDFTSize(srcGPU.rows);                 // Optimal Size to calculate the Discrete Fourier Transform 
    int n = getOptimalDFTSize(srcGPU.cols);
    cv::cuda::copyMakeBorder(srcGPU, padded, 0, m - srcGPU.rows, 0, n - srcGPU.cols, BORDER_CONSTANT, Scalar::all(0));  // Resize to optimal FFT size
    cv::cuda::GpuMat gpu_dft = cv::cuda::GpuMat(m, n, CV_32FC2);
    cv::cuda::dft(padded, gpu_dft, padded.size());          // Discrete Fourier Transform
    cv::cuda::magnitude(gpu_dft, dft_mag);                  // dft_mag = sqrt(Re(DFT(src))^2+Im(DFT(src))^2)
    double max;
    cv::cuda::minMax(dft_mag, NULL, &max);                  // Maximum value of the Fourier transform magnitude
    double thresh = max / 1000;                             // Threshold to calculate the IQM
    cv::cuda::GpuMat dft_thresh;
    cv::cuda::threshold(dft_mag, dft_thresh, thresh, 1, 0); // If dft_mag > thresh set to 1, else set to 0
    float TH = cv::cuda::countNonZero(dft_thresh);          // Number of pixels in dft_mag whose pixel value > thres
    float IQM = TH / srcGPU.total();                        // Computes the Image Sharpness
    return IQM;
}
#endif