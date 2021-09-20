/********************************************************************/
/* Project: uw-img-proc									            */
/* Module: 	illumination								            */
/* File: 	main.cpp										        */
/* Created: 30/07/2019				                                */
/* Description:
	C++ module for correcting the illumination of underwater images
	using a homomorphic filter
 /*******************************************************************/

 /*******************************************************************/
 /* Created by:                                                     */
 /* Geraldine Barreto (@geraldinebc)                                */
 /*******************************************************************/

#define ABOUT_STRING "Illumination Correction Module"

/// Include auxiliary utility libraries
#include "../include/illumination.h"

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

	std::cout << endl << "Applying illumination correction" << endl;

	// CPU Implementation
	if (!CUDA) {
		cv::Mat LAB, lab[3];
		cvtColor(input, LAB, COLOR_BGR2Lab);													// Conversion to the Lab color model
		split(LAB, lab);
		lab[0] = illuminationCorrection(lab[0]);												// Correction of ununiform illumination
		merge(lab, 3, LAB);
		cvtColor(LAB, dst, COLOR_Lab2BGR);														// Conversion to the BGR color model
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