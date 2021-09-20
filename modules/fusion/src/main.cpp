/********************************************************************/
/* Project: uw-img-proc									            */
/* Module: 	fusion										            */
/* File: 	main.cpp										        */
/* Created:	18/02/2019				                                */
/* Description:
	C++ module for underwater image enhancement using a fusion based
	strategy
 /*******************************************************************/

 /*******************************************************************/
 /* Created by:                                                     */
 /* Geraldine Barreto (@geraldinebc)                                */
 /*******************************************************************/

#define ABOUT_STRING "Fusion Enhancement Module"

/// Include auxiliary utility libraries
#include "../include/fusion.h"

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
		"{@input  |<none> | Input image file}"									// Input image is the first argument (positional)
		"{@output |<none> | Output image file}"									// Output prefix is the second argument (positional)
		"{show    |       | Show image comparison or not (ON: 1,OFF: 0)}"		// Show image comparison (optional)
		"{cuda    |       | Use CUDA or not (ON: 1, OFF: 0)}"			        // Use CUDA (if available) (optional)
		"{time    |       | Show time measurements or not (ON: 1, OFF: 0)}"		// Show time measurements (optional)
		"{help h usage ?  |       | Print help message}";						// Show help (optional)

	CommandLineParser cvParser(argc, argv, keys);
	cvParser.about(ABOUT_STRING);	// Adds "about" information to the parser method

	//**************************************************************************
	std::cout << ABOUT_STRING << endl;
	std::cout << endl << "Built with OpenCV " << CV_VERSION << endl;

	// If the number of arguments is lower than 4, or contains "help" keyword, then we show the help
	if (argc < 5 || cvParser.has("help")) {
		std::cout << endl << "C++ Module for fusion enhancement" << endl;
		std::cout << endl << "Arguments are:" << endl;
		std::cout << "\t*Input: Input image name with path and extension" << endl;
		std::cout << "\t*Output: Output image name with path and extension" << endl;
		std::cout << "\t*-cuda=0 or -cuda=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-time=0 or -time=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-show=0 or -show=1 (ON: 1, OFF: 0)" << endl;
		std::cout << endl << "Example:" << endl;
		std::cout << "\timg1.jpg img2.jpg -cuda=0 -time=0 -show=0 -d=S -m=F" << endl;
		std::cout << "\tThis will open 'input.jpg' enhance the image and save it in 'output.jpg'" << endl << endl;
		return 0;
	}

	int CUDA = 0;                                   // Default option (running with CPU)
	int Time = 0;                                   // Default option (not showing time)
	int Show = 0;                                   // Default option (not showing comparison)

	std::string InputFile = cvParser.get<cv::String>(0);	// String containing the input file path+name+extension from cvParser function
	std::string OutputFile = cvParser.get<cv::String>(1);	// String containing the input file path+name+extension from cvParser function
	std::string implementation;								// CPU or GPU implementation
	Show = cvParser.get<int>("show");						// Gets argument -show=x, where 'x' defines if the matches will show or not
	Time = cvParser.get<int>("time");						// Gets argument -time=x, where 'x' defines if execution time will show or not

	// Check if any error occurred during parsing process
	if (!cvParser.check()) {
		cvParser.printErrors();
		return -1;
	}

	//************************************************************************************************
	int nCuda = -1;    //Defines number of detected CUDA devices. By default, -1 acting as error value
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

	std::cout << endl << "************************************************************************" << endl;
	std::cout << endl << "Input: " << InputFile << endl;
	std::cout << "Output: " << OutputFile << endl;

	cv::Mat input, dst;
	input = imread(InputFile, cv::IMREAD_COLOR);

	if (input.empty()) {
		std::cout << "Error occured when loading the image" << endl << endl;
		return -1;
	}

	// Start time measurement
	t = (double)getTickCount();

	// GPU Implementation
#if USE_GPU
	if (CUDA) {
		GpuMat srcGPU;
		srcGPU.upload(src);
	}
#endif

	std::cout << endl << "Applying fusion enhancement" << endl;

	// CPU Implementation
	if (!CUDA) {
		cv::Mat HSV, LAB, chanHSV[3], chanLAB[3];
		cvtColor(input, HSV, cv::COLOR_BGR2HSV);
		split(HSV, chanHSV);
		cvtColor(input, LAB, cv::COLOR_BGR2Luv);
		split(LAB, chanLAB);
		Scalar meanS, stddevS, meanV, stddevV;
		meanStdDev(chanHSV[1], meanS, stddevS);
		meanStdDev(chanHSV[2], meanV, stddevV);

		Mat src[2];
		vector<Mat_<uchar>> channels;
		split(input, channels);
		bool flag = 0;

		// Histogram Stretching or Hue and Illumination Correction
		if ((meanV[0] <= 115 & (mean(chanLAB[1])[0] <= 110 | mean(chanLAB[2])[0] <= 110)) | (meanS[0] >= 240 & stddevS[0] <= 15)) {
			src[0] = hueIllumination(input);
			flag = 1;
		}
		else src[0] = ICM(channels, 0.5);

		// Dehazing or Hue and Illumination Correction
		if (stddevV[0] >= 65 | (meanV[0] >= 160 & (mean(chanLAB[1])[0] <= 100 | mean(chanLAB[2])[0] <= 100))) {
			if (flag == 1) dst = src[0];
			else src[1] = hueIllumination(input);
		}
		else src[1] = dehazing(input);

		if (dst.empty()) {
			cv::Mat Lab[2], L[2];
			cvtColor(src[0], Lab[0], COLOR_BGR2Lab);
			extractChannel(Lab[0], L[0], 0);
			cvtColor(src[1], Lab[1], COLOR_BGR2Lab);
			extractChannel(Lab[1], L[1], 0);

			cv::Mat kernel = filter_mask();

			// Normalized weights
			vector<Mat> w1_norm, w2_norm, w3_norm, w4_norm;
			w1_norm = weight_norm(laplacian_contrast(L[0]), laplacian_contrast(L[1]));
			w2_norm = weight_norm(local_contrast(L[0], kernel), local_contrast(L[1], kernel));
			w3_norm = weight_norm(saliency(src[0], kernel), saliency(src[1], kernel));
			w4_norm = weight_norm(exposedness(L[0]), exposedness(L[1]));

			// Weight sum of each input
			cv::Mat w_norm[2];
			w_norm[0] = (w1_norm[0] + w2_norm[0] + w3_norm[0] + w4_norm[0]) / 4;
			w_norm[1] = (w1_norm[1] + w2_norm[1] + w3_norm[1] + w4_norm[1]) / 4;

			// Gaussian pyramids of the weights
			int levels = 5;
			vector<Mat> pyramid_g0, pyramid_g1;
			buildPyramid(w_norm[0], pyramid_g0, levels - 1);
			buildPyramid(w_norm[1], pyramid_g1, levels - 1);

			cv::Mat channels_0[3], channels_1[3];
			split(src[0], channels_0);
			split(src[1], channels_1);

			// Laplacian pyramids of the inputs channels (BGR)
			vector<Mat_<float>> pyramid_l0_b = laplacian_pyramid(channels_0[0], levels);
			vector<Mat_<float>> pyramid_l0_g = laplacian_pyramid(channels_0[1], levels);
			vector<Mat_<float>> pyramid_l0_r = laplacian_pyramid(channels_0[2], levels);

			vector<Mat_<float>> pyramid_l1_b = laplacian_pyramid(channels_1[0], levels);
			vector<Mat_<float>> pyramid_l1_g = laplacian_pyramid(channels_1[1], levels);
			vector<Mat_<float>> pyramid_l1_r = laplacian_pyramid(channels_1[2], levels);

			// Fusion of the inputs with their respective weights
			Mat chan_b[5], chan_g[5], chan_r[5];
			for (int i = 0; i < 5; i++) {
				pyramid_g0[i].convertTo(pyramid_g0[i], CV_32F);
				pyramid_g1[i].convertTo(pyramid_g1[i], CV_32F);
				add(pyramid_l0_b[i].mul(pyramid_g0[i]), pyramid_l1_b[i].mul(pyramid_g1[i]), chan_b[i]);
				add(pyramid_l0_g[i].mul(pyramid_g0[i]), pyramid_l1_g[i].mul(pyramid_g1[i]), chan_g[i]);
				add(pyramid_l0_r[i].mul(pyramid_g0[i]), pyramid_l1_r[i].mul(pyramid_g1[i]), chan_r[i]);
			}

			// Pyramid reconstruction
			cv::Mat channel[3];
			channel[0] = pyramid_fusion(chan_b, levels);
			channel[1] = pyramid_fusion(chan_g, levels);
			channel[2] = pyramid_fusion(chan_r, levels);
			merge(channel, 3, dst);
			
			//Mat test;
			//vconcat(src[0], src[1], test);
			//imwrite(OutputFile + "_t.jpg", test);

			//Mat weight1, weight2, weights;
			//hconcat(w1_norm[0], w2_norm[0], weight1);
			//hconcat(weight1, w3_norm[0], weight1);
			//hconcat(weight1, w4_norm[0], weight1);
			//hconcat(weight1, w_norm[0], weight1);
			//hconcat(w1_norm[1], w2_norm[1], weight2);
			//hconcat(weight2, w3_norm[1], weight2);
			//hconcat(weight2, w4_norm[1], weight2);
			//hconcat(weight2, w_norm[1], weight2);
			//vconcat(weight1, weight2, weights);
			//imwrite(OutputFile + "_w.jpg", 255.0*weights);
		}
	}

	//  End time measurement (Showing time results is optional)
	if (Time) {
		t = 1000 * ((double)getTickCount() - t) / getTickFrequency();
		std::cout << endl << "Execution Time" << implementation << ": " << t << " ms " << endl;

		// Name for the output csv file where the time will be saved
		std::size_t pos;
		if (OutputFile.find(92) != std::string::npos) pos = OutputFile.find_last_of(92);	// If string contains '\' search for the last one
		else pos = OutputFile.find_last_of('/');											// If does not contain '\' search for the last '/'
		std::string Output = OutputFile.substr(0, pos + 1);
		std::string ext = "execution_time.csv";
		Output.insert(pos + 1, ext);
		ofstream file;
		file.open(Output, std::ios::app);
		file << endl << OutputFile << ";" << input.rows << ";" << input.cols << ";" << t;
	}

	std::cout << endl << "Saving processed image" << endl;
	imwrite(OutputFile, dst);

	if (Show) {
		std::cout << endl << "Showing image comparison" << endl;
		Mat comparison;
		hconcat(input, dst, comparison);
		namedWindow("Comparison", WINDOW_KEEPRATIO);
		imshow("Comparison", comparison);
	}

	waitKey(0);
	return 0;
}