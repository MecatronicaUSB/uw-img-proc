/*********************************************************************/
/* Project: uw-img-proc									             */
/* Module: 	evaluationmetrics							             */
/* File: 	main.cpp										         */
/* Created:	09/12/2018				                                 */
/* Description:
	C++ Module of image quality metrics for image processing evaluation
 /********************************************************************/

 /********************************************************************/
 /* Created by:                                                      */
 /* Geraldine Barreto (@geraldinebc)                                 */
 /* Based on: Vision parameters measured using the mathematical model*/ 
 /*			  given by Xie and Wang and the frequency domain image	 */
 /*			  sharpness measure given by De and Masilamani.			 */
 /********************************************************************/

#define ABOUT_STRING "Image Quality Assessment Module"

/// Include auxiliary utility libraries
#include "../include/evaluationmetrics.h"

// Time measurements
#define _VERBOSE_ON_
double t;	// Timing monitor

/*!
	@fn		int main(int argc, char* argv[])
	@brief	Main function
*/
int main(int argc, char *argv[]) {

	//*********************************************************************************
	/*	PARSER section */
	/*  Uses built-in OpenCV parsing method cv::CommandLineParser. It requires a string containing the arguments to be parsed from
		the command line. Further details can be obtained from opencv webpage
	*/
	String keys =
		"{@processed |<none> | Processed image file}"							// Processed image is the first argument (positional)
		"{@original  |<none> | Original image file}"							// Original image is the second argument (positional)
		"{m       |r      | Image Quality Metric to calculate}"					// Metric to calculate
		"{cuda    |       | Use CUDA or not (CUDA ON: 1, CUDA OFF: 0)}"         // Use CUDA (if available) or not
		"{time    |       | Show time measurements or not (ON: 1, OFF: 0)}"		// Show time measurements or not
		"{show    |       | Show result (ON: 1, OFF: 0)}"						// Show the resulting image (optional)
		"{help h usage ?  |       | Print help message}";						// Show help (optional)

	CommandLineParser cvParser(argc, argv, keys);
	cvParser.about(ABOUT_STRING);	// Adds "about" information to the parser method

	//**************************************************************************
	std::cout << ABOUT_STRING << endl;
	std::cout << "Built with OpenCV " << CV_VERSION << endl;

	// If the number of arguments is lower than 4, or contains "help" keyword, then we show the help
	if (argc < 5 || cvParser.has("help")) {
		std::cout << endl << "C++ Module for calculating Image Quality Metrics" << endl;
		std::cout << endl << "Arguments are:" << endl;
		std::cout << "\t*Processed: Processed image name with path and extension" << endl;
		std::cout << "\t*Original: Original image name with path and extension" << endl;
		std::cout << "\t*-show=0 or -show=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-cuda=0 or -cuda=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-time=0 or -time=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*Argument 'm=<metrics>' is a string containing a list of the desired metrics to be calculated" << endl;
		std::cout << endl << "Complete options of evaluation metrics are:" << endl;
		std::cout << "\t-m=E for Entropy" << endl;
		std::cout << "\t-m=A for Average Entropy" << endl;
		std::cout << "\t-m=L for Average Luminance" << endl;
		std::cout << "\t-m=C for Average Contrast" << endl;
		std::cout << "\t-m=N for Normalized Neighborhood Function" << endl;
		std::cout << "\t-m=F for Comprehensive Assessment Function" << endl;
		std::cout << "\t-m=M for Mean Square Error" << endl;
		std::cout << "\t-m=P for Peak Signal to Noise Ratio" << endl;
		std::cout << "\t-m=S for Frequency Domain Image Sharpness Measure" << endl;
		std::cout << "\t-m=U for Feature detection using SURF" << endl;
		std::cout << "\t-m=H for Histogram" << endl;
		std::cout << "\t-m=X for All Metrics" << endl;
		std::cout << endl << "Example:" << endl;
		std::cout << "\tproc.jpg orig.jpg -cuda=0 -time=0 -m=EM -show=0" << endl;
		std::cout << "\tThis will open 'proc.jpg' and 'orig.jpg' and calculate the Entropy and the MSE of proc using orig as reference" << endl << endl;
		return 0;
	}

	int CUDA = 0;											// Default option (running with CPU)
	int Time = 0;											// Default option (not showing time)
	int Show = 0;											// Default option (not showing results)

	std::string ProcessedFile = cvParser.get<cv::String>(0);// String containing the input file path+name+extension from cvParser function
	std::string OriginalFile = cvParser.get<cv::String>(1); // String containing the input file path+name+extension from cvParser function
	std::string metric = cvParser.get<cv::String>("m");		// Gets argument -m=x, where 'x' is the quality metric
	std::string implementation;								// CPU or GPU implementation
	Show = cvParser.get<int>("show");						// Gets argument -show=x, where 'x' defines if the results will show or not
	Time = cvParser.get<int>("time");						// Gets argument -time=x, where 'x' defines ifexecution time will show or not

	// Check if any error occurred during parsing process
	if (!cvParser.check()) {
		cvParser.printErrors();
		return -1;
	}

	//************************************************************************************************
	int nCuda = -1;    // Defines number of detected CUDA devices. By default, -1 acting as error value
#if USE_GPU
	CUDA = cvParser.get<int>("cuda");	        // gets argument -cuda=x, where 'x' define to use CUDA or not
	nCuda = cuda::getCudaEnabledDeviceCount();	// Try to detect any existing CUDA device
	// Deactivate CUDA from parse
	if (CUDA == 0) {
		implementation = "CPU";
		cout << "CUDA deactivated" << endl;
		cout << "Exiting... use non-GPU version instead" << endl;
	}
	// Find CUDA devices
	else if (nCuda > 0) {
		implementation = "GPU";
		cuda::DeviceInfo deviceInfo;
		cout << "CUDA enabled devices detected: " << deviceInfo.name() << endl;
		cuda::setDevice(0);
	}
	else {
		CUDA = 0;
		implementation = "CPU";
		cout << "No CUDA device detected" << endl;
		cout << "Exiting... use non-GPU version instead" << endl;
	}
#endif
	//************************************************************************************************

	std::cout << endl << "********************************************************************************" << endl;
	std::cout << "Original Image: " << OriginalFile << endl;
	std::cout << "Processed Image: " << ProcessedFile << endl;

	Mat src, dst;
	src = imread(ProcessedFile, CV_LOAD_IMAGE_COLOR);
	dst = imread(OriginalFile, CV_LOAD_IMAGE_COLOR);

	if (src.empty() | dst.empty()) {
		std::cout << "Error occured when loading an image" << endl << endl;
		return -1;
	}

	int num_metrics = metric.length();

	// If all metrics are chosen the result will be saved in a csv file, if not the results will be shown in the command line
	bool Save;
	if (metric.find('X') != std::string::npos) Save = 1;
	else {
		Save = 0;
		Show = 1;
	}

	// Name for the output csv file where the results will be saved (where the processed images are)
	std::size_t pos;
	if (ProcessedFile.find(92) != std::string::npos) pos = ProcessedFile.find_last_of(92);	// If string contains '\' search for the last one
	else pos = ProcessedFile.find_last_of('/');												// If does not contain '\' search for the last '/'
	std::string Output = ProcessedFile.substr(0, pos + 1);
	std::string ext = "evaluation_metrics.csv";
	Output.insert(pos + 1, ext);
	ofstream file;
	std::string Name = ProcessedFile.substr(pos + 1);										// Name for the processed image

	std::cout << endl << "Calculating Image Quality Metrics" << endl << endl;

	// Start time measurement
	if (Time) t = (double)getTickCount();

	// GPU Implementation
#if USE_GPU
	if (CUDA) {
		cv::cuda::GpuMat srcGPU, dstGPU, descriptorsGPU[2];
		srcGPU.upload(src);
		dstGPU.upload(dst);

		GpuMat graysrcGPU, graydstGPU, chanRGB_srcGPU[3], chanRGB_dstGPU[3], YCCsrcGPU, YCCdstGPU, chanYCC_srcGPU[3], chanYCC_dstGPU[3], descriptorsGPU[2], hist_src[3], hist_dst[3];
		float E_srcGPU, E_dstGPU, AE_srcGPU = 0, AE_dstGPU = 0, AC_srcGPU = 0, AC_dstGPU = 0, AL_srcGPU = 0, AL_dstGPU = 0, NNF_srcGPU = 0, NNF_dstGPU = 0, CAF_srcGPU, CAF_dstGPU, MSEGPU = 0, psnrGPU;
		std::vector<KeyPoint> keypoints[2];
		Ptr<cuda::AKAZE> detector;
		std::size_t pos = ProcessedFile.find(".");
		std::string Histograms = ProcessedFile.substr(0, pos_dst) + "_hist.jpg";

		// Now, according to parameters provided at CLI calling time
		for (int nm = 0; nm < num_metrics; nm++) {
			char M = metric[nm];

			switch (M) {

			case 'E':	// Entropy
				if (graysrcGPU.empty()) {
					cuda::cvtColor(srcGPU, graysrcGPU, COLOR_BGR2GRAY);
					cuda::cvtColor(dstGPU, graydstGPU, COLOR_BGR2GRAY);
				}

				E_srcGPU = entropy(graysrcGPU);
				E_dstGPU = entropy(graydstGPU);

				if (Show) {
					cout << "Original Image Grayscale Entropy: " << fixed << setprecision(5) << E_srcGPU << endl;
					cout << "Processed Image Grayscale Entropy: " << fixed << setprecision(5) << E_dstGPU << endl << endl;
				}
			break;

			case 'A':	// Average Entropy
				if (chanRGB_srcGPU->empty()) {
					cuda::split(srcGPU, chanRGB_srcGPU);
					cuda::split(dstGPU, chanRGB_dstGPU);
				}

				AE_srcGPU = averageEntropy(chanRGB_srcGPU[2], chanRGB_srcGPU[1], chanRGB_srcGPU[0]);
				AE_dstGPU = averageEntropy(chanRGB_dstGPU[2], chanRGB_dstGPU[1], chanRGB_dstGPU[0]);

				if (Show) {
					cout << "Original Image Average Entropy: " << fixed << setprecision(3) << AE_srcGPU << endl;
					cout << "Processed Image Average Entropy: " << fixed << setprecision(3) << AE_dstGPU << endl << endl;
				}
			break;

			case 'C':	// Average Contrast
				if (chanRGB_srcGPU->empty()) {
					cuda::split(srcGPU, chanRGB_srcGPU);
					cuda::split(dstGPU, chanRGB_dstGPU);
				}

				AC_srcGPU = averageContrast(chanRGB_srcGPU[2], chanRGB_srcGPU[1], chanRGB_srcGPU[0]);
				AC_dstGPU = averageContrast(chanRGB_dstGPU[2], chanRGB_dstGPU[1], chanRGB_dstGPU[0]);

				if (Show) {
					cout << "Original Image Average Contrast: " << fixed << setprecision(3) << AC_srcGPU << endl;
					cout << "Processed Image Average Contrast: " << fixed << setprecision(3) << AC_dstGPU << endl << endl;
				}
			break;

			case 'L':	// Average Luminance
				cuda::cvtColor(srcGPU, YCCsrcGPU, COLOR_BGR2YCrCb);
				cuda::cvtColor(dstGPU, YCCdstGPU, COLOR_BGR2YCrCb);

				cuda::split(srcGPU, chanYCC_srcGPU);
				cuda::split(dstGPU, chanYCC_dstGPU);

				AL_srcGPU = averageLuminance(chanYCC_srcGPU[0]);
				AL_dstGPU = averageLuminance(chanYCC_dstGPU[0]);

				if (Show) {
					cout << "Original Image Average Luminance: " << fixed << setprecision(3) << AL_srcGPU << endl;
					cout << "Processed Image Average Luminance: " << fixed << setprecision(3) << AL_dstGPU << endl << endl;
				}
			break;

			case 'N':	// Normalized Neighborhood Function
				if (!AL_srcGPU) {
					cuda::cvtColor(srcGPU, YCCsrcGPU, COLOR_BGR2YCrCb);
					cuda::cvtColor(dstGPU, YCCdstGPU, COLOR_BGR2YCrCb);

					cuda::split(srcGPU, chanYCC_srcGPU);
					cuda::split(dstGPU, chanYCC_dstGPU);

					NNF_srcGPU = getNNF(averageLuminance(chanYCC_srcGPU[0]));
					NNF_dstGPU = getNNF(averageLuminance(chanYCC_dstGPU[0]));
				}
				else {
					NNF_srcGPU = getNNF(AL_srcGPU);
					NNF_dstGPU = getNNF(AL_dstGPU);
				}

				if (Show) {
					cout << "Original Image Normalized Neighborhood Function: " << fixed << setprecision(5) << NNF_srcGPU << endl;
					cout << "Processed Image Normalized Neighborhood Function: " << fixed << setprecision(5) << NNF_dstGPU << endl << endl;
				}
			break;

			case 'F':	// Comprehensive Assesment Function
				if (!AE_srcGPU) {
					if (chanRGB_srcGPU->empty()) {
						cuda::split(srcGPU, chanRGB_srcGPU);
						cuda::split(dstGPU, chanRGB_dstGPU);
					}

					AE_srcGPU = averageEntropy(chanRGB_srcGPU[2], chanRGB_srcGPU[1], chanRGB_srcGPU[0]);
					AE_dstGPU = averageEntropy(chanRGB_dstGPU[2], chanRGB_dstGPU[1], chanRGB_dstGPU[0]);
				}
				if (!AC_srcGPU) {
					if (chanRGB_srcGPU->empty()) {
						cuda::split(srcGPU, chanRGB_srcGPU);
						cuda::split(dstGPU, chanRGB_dstGPU);
					}

					AC_srcGPU = averageContrast(chanRGB_srcGPU[2], chanRGB_srcGPU[1], chanRGB_srcGPU[0]);
					AC_dstGPU = averageContrast(chanRGB_dstGPU[2], chanRGB_dstGPU[1], chanRGB_dstGPU[0]);
				}
				if (!NNF_srcGPU) {
					if (!AL_srcGPU) {
						cuda::cvtColor(srcGPU, YCCsrcGPU, COLOR_BGR2YCrCb);
						cuda::cvtColor(dstGPU, YCCdstGPU, COLOR_BGR2YCrCb);

						cuda::split(srcGPU, chanYCC_srcGPU);
						cuda::split(dstGPU, chanYCC_dstGPU);

						NNF_srcGPU = getNNF(averageLuminance(chanYCC_srcGPU[0]));
						NNF_dstGPU = getNNF(averageLuminance(chanYCC_dstGPU[0]));
					}
					else {
						NNF_srcGPU = getNNF(AL_srcGPU);
						NNF_dstGPU = getNNF(AL_dstGPU);
					}
				}
			
				CAF_srcGPU = getCAF(AE_srcGPU, AC_srcGPU, NNF_srcGPU);
				CAF_dstGPU = getCAF(AE_dstGPU, AC_dstGPU, NNF_dstGPU);

				if (Show) {
					cout << "Original Image Comprehensive Assessment Function: " << fixed << setprecision(5) << CAF_srcGPU << endl;
					cout << "Processed Image Comprehensive Assessment Function: " << fixed << setprecision(5) << CAF_dstGPU << endl << endl;
				}
				break;

				//case 'S':	// Image Sharpness in Frequency Domain
				//	if (graysrc.empty()) {
				//		cvtColor(src, graysrc, COLOR_BGR2GRAY);
				//		cvtColor(dst, graydst, COLOR_BGR2GRAY);
				//	}
				//	IQM_src = IQMfun(graysrc);
				//	IQM_dst = IQMfun(graydst);

				//	if (Show) {
				//		std::cout << "Original Frequency Domain Image Sharpness Measure: " << fixed << setprecision(5) << IQM_src << endl;
				//		std::cout << "Processed Frequency Domain Image Sharpness Measure: " << fixed << setprecision(5) << IQM_dst << endl << endl;
				//	}
				//	if (Save) {
				//		file.open(Output, std::ios::app);
				//		file << fixed << setprecision(5) << IQM_src << ";" << IQM_dst << ";";
				//	}
				//break;

				case 'M':	// Mean Square Error
					if (graysrcGPU.empty()) {
						cuda::cvtColor(srcGPU, graysrcGPU, COLOR_BGR2GRAY);
						cuda::cvtColor(dstGPU, graydstGPU, COLOR_BGR2GRAY);
					}

					MSEGPU = getMSE(graysrcGPU, graydstGPU);

					if (Show) cout << "MSE: " << int(MSEGPU) << endl << endl;
				break;

				case 'P':	// Peak Signal to Noise Ratio
					if (!MSEGPU) {
						if (graysrcGPU.empty()) {
							cuda::cvtColor(srcGPU, graysrcGPU, COLOR_BGR2GRAY);
							cuda::cvtColor(dstGPU, graydstGPU, COLOR_BGR2GRAY);
						}
						MSEGPU = getMSE(graysrcGPU, graydstGPU);
					}

					psnrGPU = getPSNR(MSEGPU);

					if (Show) cout << "PSNR: " << fixed << setprecision(3) << psnrGPU << endl << endl;
				break;

				case 'U':	// Feature Detection
					if (graysrcGPU.empty()) {
						cuda::cvtColor(srcGPU, graysrcGPU, COLOR_BGR2GRAY);
						cuda::cvtColor(dstGPU, graydstGPU, COLOR_BGR2GRAY);
					}

					detector = cv::cuda::SURF::create();
					detector->detectAndCompute(graysrcGPU, Mat(), keypoints[0], descriptorsGPU[0]);
					detector->detectAndCompute(graydstGPU, Mat(), keypoints[1], descriptorsGPU[1]);

					if (!keypoints[0].size() || !keypoints[1].size()) {
						std::cout << "No keypoints found, please provide different images" << endl;
						return -1;
					}

					if (Show) {
						std::cout << "Original Image Features Detected: " << keypoints[0].size() << endl;
						std::cout << "Processed Image Features Detected: " << keypoints[1].size() << endl << endl;
					}
				break;

				case 'H':	// Histogram
					if (chanRGB_srcGPU->empty()) {
						cuda::split(srcGPU, chanRGB_srcGPU);
						cuda::split(dstGPU, chanRGB_dstGPU);
					}

					//Original histogram
					getHistogram(&chanRGB_src[0], &hist_src[0]);
					src_hist[0] = printHist(hist_src[0], { 255,0,0 });
					getHistogram(&chanRGB_src[1], &hist_src[1]);
					src_hist[1] = printHist(hist_src[1], { 0,255,0 });
					getHistogram(&chanRGB_src[2], &hist_src[2]);
					src_hist[2] = printHist(hist_src[2], { 0,0,255 });
					vconcat(src_hist[0], src_hist[1], org_hist);
					vconcat(org_hist, src_hist[2], org_hist);

					//Processed histogram
					getHistogram(&chanRGB_dst[0], &hist_dst[0]);
					dst_hist[0] = printHist(hist_dst[0], { 255,0,0 });
					getHistogram(&chanRGB_dst[1], &hist_dst[1]);
					dst_hist[1] = printHist(hist_dst[1], { 0,255,0 });
					getHistogram(&chanRGB_dst[2], &hist_dst[2]);
					dst_hist[2] = printHist(hist_dst[2], { 0,0,255 });
					vconcat(dst_hist[0], dst_hist[1], pro_hist);
					vconcat(pro_hist, dst_hist[2], pro_hist);

					//Histogram comparison
					hconcat(org_hist, pro_hist, pro_hist);
					cv::imwrite(Histograms, pro_hist);

					if (Show) {
						std::cout << endl << "Showing histograms" << endl << endl;
						namedWindow("Histogram comparison", WINDOW_KEEPRATIO);
						imshow("Histogram comparison", pro_hist);
					}

					std::cout << "Histograms saved" << endl;
					break;

				case 'X':	// All Metrics
					metric = "EACLNFSMPUH";
					num_metrics = metric.length();
					nm--;

					file.open(Output, std::ios::app);
					file << endl << OriginalFile << ";" << ProcessedFile << ";";
				break;

				default:	// Unrecognized option
					cout << "Option " << M << " not recognized, skipping..." << endl;
				break;
				}
		}
	}
#endif

	std::cout << "********************************************************************************" << endl;

	// CPU Implementation
	if (!CUDA) {

		cv::Mat graysrc, graydst, chanRGB[3], chanLAB[3], src_LAB, hist_RGB[3], hist_LAB[3], RGB_hist[3], LAB_hist[3], histRGB, histLAB, hist, descriptor;
		float entropy_src, AE_src = 0, AC_src = 0, AL_src = 0, NNF_src = 0, CAF_src, IQM_src, MSE = 0, psnr = 0;
		int minHessian = 400;
		cv::Ptr<Feature2D> detector;
		std::vector<KeyPoint> keypoint;
		std::size_t pos = ProcessedFile.find(".");
		std::string Histograms = ProcessedFile.substr(0, pos) + "_hist.jpg";
		Scalar colorA, colorB;

		// Now, according to parameters provided at CLI calling time
		for (int nm = 0; nm < num_metrics; nm++) {
			char M = metric[nm];

			switch (M) {

				case 'E':	// Entropy
					if (graysrc.empty()) {
						cvtColor(src, graysrc, COLOR_BGR2GRAY);
						cvtColor(dst, graydst, COLOR_BGR2GRAY);
					}
					entropy_src = entropy(graysrc);
					if (Show) std::cout << "Grayscale Entropy: " << fixed << setprecision(3) << entropy_src << endl;
					if (Save) {
						file.open(Output, std::ios::app);
						file << fixed << setprecision(3) << entropy_src << ";";
					}
				break;

				case 'A':	// Average Entropy
					if (chanRGB->empty()) split(src, chanRGB);
					AE_src = averageEntropy(chanRGB[2], chanRGB[1], chanRGB[0]);
					if (Show) std::cout << "Average Color Entropy: " << fixed << setprecision(3) << AE_src << endl;
					if (Save) {
						file.open(Output, std::ios::app);
						file << fixed << setprecision(3) << AE_src << ";";
					}
				break;

				case 'C':	//  Average Contrast
					if (chanRGB->empty()) split(src, chanRGB);
					AC_src = averageContrast(chanRGB[2], chanRGB[1], chanRGB[0]);
					if (Show) std::cout << "Average Contrast: " << fixed << setprecision(3) << AC_src << endl;
					if (Save) {
						file.open(Output, std::ios::app);
						file << fixed << setprecision(3) << AC_src << ";";
					}
				break;

				case 'L':	// Average Luminance
					cvtColor(src, src, COLOR_BGR2Lab);
					split(src, chanLAB);
					AL_src = averageLuminance(chanLAB[0]);
					if (Show) std::cout << "Average Luminance: " << fixed << setprecision(3) << AL_src << endl;
					if (Save) {
						file.open(Output, std::ios::app);
						file << fixed << setprecision(3) << AL_src << ";";
					}
				break;

				case 'N':	// Normalized Neighborhood Function
					if (!AL_src) {
						cvtColor(src, src_LAB, COLOR_BGR2Lab);
						split(src, chanLAB);
						NNF_src = getNNF(averageLuminance(chanLAB[0]));
					}
					else NNF_src = getNNF(AL_src);
					if (Show) std::cout << "Normalized Neighborhood Function: " << fixed << setprecision(5) << NNF_src << endl;
					if (Save) {
						file.open(Output, std::ios::app);
						file << fixed << setprecision(5) << NNF_src << ";";
					}
				break;

				case 'F':	// Comprehensive Assesment Function
					if (!AE_src) {
						split(src, chanRGB);
						AE_src = averageEntropy(chanRGB[2], chanRGB[1], chanRGB[0]);
					}
					if (!AC_src) {
						split(src, chanRGB);
						AC_src = averageContrast(chanRGB[2], chanRGB[1], chanRGB[0]);
					}
					if (!NNF_src) {
						if (!AL_src) {
							cvtColor(src, src_LAB, COLOR_BGR2Lab);
							split(src, chanLAB);
							NNF_src = getNNF(averageLuminance(chanLAB[0]));
						}
						else NNF_src = getNNF(AL_src);
					}
					CAF_src = getCAF(AE_src, AC_src, NNF_src);
					if (Show) std::cout << "Comprehensive Assessment Function: " << fixed << setprecision(3) << CAF_src << endl;
					if (Save) {
						file.open(Output, std::ios::app);
						file << fixed << setprecision(3) << CAF_src << ";";
					}
				break;

				case 'S':	// Image Sharpness in Frequency Domain
					if (graysrc.empty()) cvtColor(src, graysrc, COLOR_BGR2GRAY);
					IQM_src = IQMfun(graysrc);
					if (Show) std::cout << "Image Sharpness: " << fixed << setprecision(5) << IQM_src << endl;
					if (Save) {
						file.open(Output, std::ios::app);
						file << fixed << setprecision(5) << IQM_src << ";";
					}
				break;

				case 'U':	// Feature detection using SURF
					if (graysrc.empty()) cvtColor(src, graysrc, COLOR_BGR2GRAY);
					detector = SURF::create(minHessian);
					detector->detectAndCompute(graysrc, Mat(), keypoint, descriptor);
					if (Show) std::cout << "Features Detected: " << keypoint.size() << endl;
					if (Save) {
						file.open(Output, std::ios::app);
						file << keypoint.size() << ";";
					}
				break;

				case 'M':	// Mean Square Error
					if (graysrc.empty()) {
						cvtColor(src, graysrc, COLOR_BGR2GRAY);
						cvtColor(dst, graydst, COLOR_BGR2GRAY);
					}
					MSE = getMSE(graysrc, graydst);
					if (Show) std::cout << fixed << setprecision(3) << "MSE: " << int(MSE) << endl;
					if (Save) {
						file.open(Output, std::ios::app);
						file << int(MSE) << ";";
					}
					break;

				case 'P':	// Peak Signal to Noise Ratio
					if (!MSE) {
						if (graysrc.empty()) {
							cvtColor(src, graysrc, COLOR_BGR2GRAY);
							cvtColor(dst, graydst, COLOR_BGR2GRAY);
						}
						MSE = getMSE(graysrc, graydst);
					}
					psnr = getPSNR(MSE);
					if (Show) std::cout << "PSNR: " << fixed << setprecision(3) << psnr << endl;
					if (Save) {
						file.open(Output, std::ios::app);
						file << fixed << setprecision(3) << psnr << ";";
					}
					break;

				case 'H':	// Histogram
					if (chanRGB->empty()) split(src, chanRGB);
					if (chanLAB->empty()) {
						cvtColor(src, src_LAB, COLOR_BGR2Lab);
						split(src_LAB, chanLAB);
					}

					// RGB histogram
					getHistogram(&chanRGB[0], &hist_RGB[0]);
					RGB_hist[0] = printHist(hist_RGB[0], {255,0,0});
					getHistogram(&chanRGB[1], &hist_RGB[1]);
					RGB_hist[1] = printHist(hist_RGB[1], {0,255,0});
					getHistogram(&chanRGB[2], &hist_RGB[2]);
					RGB_hist[2] = printHist(hist_RGB[2], {0,0,255});
					cv::vconcat(RGB_hist[0], RGB_hist[1], histRGB);
					cv::vconcat(histRGB, RGB_hist[2], histRGB);

					// LAB histogram
					getHistogram(&chanLAB[0], &hist_LAB[0]);
					LAB_hist[0] = printHist(hist_LAB[0], {0,0,0});
					getHistogram(&chanLAB[1], &hist_LAB[1]);
					if (mean(chanLAB[1])[0] > 127.5) colorA = {150,15,235};
					else colorA = {75,155,10};
					LAB_hist[1] = printHist(hist_LAB[1], colorA);
					getHistogram(&chanLAB[2], &hist_LAB[2]);
					if (mean(chanLAB[2])[0] > 127.5) colorB = {7,217,254};
					else colorB = {240,210,40};
					LAB_hist[2] = printHist(hist_LAB[2], colorB);
					cv::vconcat(LAB_hist[0], LAB_hist[1], histLAB);
					cv::vconcat(histLAB, LAB_hist[2], histLAB);

					// Histogram comparison
					cv::hconcat(histRGB, histLAB, hist);

					if (Show) {
						std::cout << "Showing histograms" << endl;
						namedWindow("Histograms", WINDOW_KEEPRATIO);
						imshow("Histograms", hist);
					}
					if (Save) {
						cv::imwrite(Histograms, hist);
						std::cout << "Histograms saved" << endl << endl;
					}
				break;

				case 'X':	// All Metrics
					if (OriginalFile.compare(ProcessedFile) == 0) metric = "EACLNFSUH";
					else metric = "EACLNFSUMPH";
					num_metrics = metric.length();
					nm--;

					file.open(Output, std::ios::app);
					file << endl << Name << ";";
				break;

				default:	// Unrecognized option
					std::cout << "Option " << M << " not recognized, skipping..." << endl << endl;
				break;
			}

			file.close();
		}
	}

	//  End time measurement (Showing time results is optional)
	if (Time) {
		t = 1000 * ((double)getTickCount() - t) / getTickFrequency();
		std::cout << "Execution Time" << implementation << ": " << t << " ms " << endl;
	}

	if (Save) std::cout << endl << "Evaluation metrics saved in " << Output << endl;

	waitKey(0);
	return 0;
}