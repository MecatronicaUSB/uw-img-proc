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

#define ABOUT_STRING "Color Correction Module based on Ruderman's laB color space"

/// Include auxiliary utility libraries
#include "../include/colorcorrection.h"

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
		"{show    |       | Show matched features or not (ON: 1,OFF: 0)}"		// Show results (optional)
		"{cuda    |       | Use CUDA or not (CUDA ON: 1, CUDA OFF: 0)}"         // Use CUDA (optional)
		"{time    |       | Show time measurements or not (ON: 1, OFF: 0)}"		// Show time measurements (optional)
		"{help h usage ?  |       | Print help message}";						// Show help (optional)

	CommandLineParser cvParser(argc, argv, keys);
	cvParser.about(ABOUT_STRING);												// Adds "about" information to the parser method

	//**************************************************************************
	std::cout << ABOUT_STRING << endl;
	std::cout << "Built with OpenCV " << CV_VERSION << endl;

	// If the number of arguments is lower than 3, or contains "help" keyword, then we show the help
	if (argc < 5 || cvParser.has("help")) {
		std::cout << endl << "C++ implementation of color correction module using Gray World Assumption" << endl;
		std::cout << endl << "Arguments are:" << endl;
		std::cout << "\t*Input: Input image name with path and extension" << endl;
		std::cout << "\t*Output: Output image name with path and extension" << endl;
		std::cout << "\t*-show=0 or -show=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-cuda=0 or -cuda=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-time=0 or -time=1 (ON: 1, OFF: 0)" << endl;
		std::cout << endl << "Example:" << endl;
		std::cout << "\tinput.jpg output.jpg -cuda=0 -time=0 -show=0" << endl;
		std::cout << "\tThis will open 'input.jpg' correct the color and save it in 'output.jpg'" << endl << endl;
		return 0;
	}

	int CUDA = 0;										// Default option (running with CPU)
	int Time = 0;                                       // Default option (not showing time)
	int Show = 0;										// Default option (not showing results)

	std::string InputFile = cvParser.get<cv::String>(0); // String containing the input file path+name+extension from cvParser function
	std::string OutputFile = cvParser.get<cv::String>(1);// String containing the input file path+name+extension from cvParser function
	std::string implementation;							 // CPU or GPU implementation
	Show = cvParser.get<int>("show");					 // Gets argument -show=x, where 'x' defines if the results will show or not
	Time = cvParser.get<int>("time");	                 // Gets argument -time=x, where 'x' defines ifexecution time will show or not

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

	cv::Mat src, dst;
	src = imread(InputFile, CV_LOAD_IMAGE_COLOR);

	if (src.empty()){
		std::cout << "Error occured when loading the image" << endl << endl;
		return -1;
	}

	std::cout << endl << "Applying color correction" << endl;

	// Start time measurement
	if (Time) t = (double)getTickCount();

	// GPU Implementation
#if USE_GPU
	if (CUDA) {
		GpuMat srcGPU, dstGPU, channelsGPU[3];
		Mat R, G, B, L, a, b;

		srcGPU.upload(src);

		// Now, according to parameters provided at CLI calling time, we must split and process the image

		// Split RGB channels (BGR for OpenCV)
		cuda::split(srcGPU, channelsGPU);

		// Transformation from RGB to laB color space
		RGBtoLab(channelsGPU[2], channelsGPU[1], channelsGPU[0], &L, &a, &b);

		// Gray world assumption (White balancing)
		a = a - mean(a), b = b - mean(b);

		// Corrected image is converted back to RGB
		LaBtoRGB(L, a, b, &R, &G, &B);

		R.convertTo(channelsGPU[2], CV_8U);
		G.convertTo(channelsGPU[1], CV_8U);
		B.convertTo(channelsGPU[0], CV_8U);

		// Merges the RGB channels in one image (BGR for OpenCV)
		cuda::merge(channelsGPU, 3, dstGPU);

		dstGPU.download(dst);
	}
#endif

	// CPU Implementation
	if (! CUDA) {
		// Transformation from RGB to laB color space
		std::vector<Mat_<float>> Lab = BGRtoLab(src);

		// Gray world assumption
		Lab[1] = Lab[1] - mean(Lab[1]).val[0];
		Lab[2] = Lab[2] - mean(Lab[2]).val[0];
		//Lab[1] = Lab[1] - medianMat(Lab[1]);										// Using the median instead of the mean 
		//Lab[2] = Lab[2] - medianMat(Lab[2]);										// Takes more time but in some cases gives better results

		// The color balanced image is converted back to BGR
		std::vector<Mat_<uchar>> BGR = LabtoBGR(Lab);
		merge(BGR, dst);
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
		file << endl << OutputFile << ";" << src.rows << ";" << src.cols << ";" << t;
	}

	std::cout << endl << "Saving processed image" << endl;
	imwrite(OutputFile, dst);

	if (Show) {
		std::cout << endl << "Showing image comparison" << endl;
		cv::Mat comparison;
		hconcat(src, dst, comparison);
		namedWindow("Comparison", WINDOW_KEEPRATIO);
		imshow("Comparison", comparison);
	}

	waitKey(0);
	return 0;
}