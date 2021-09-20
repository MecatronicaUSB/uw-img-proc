/********************************************************************/
/* Project: uw_img_proc									            */
/* Module:  dehazing								                */
/* File: 	main.cpp									            */
/* Created:	05/02/2019				                                */
/* Description:
	C++ Module for image dehazing using Gao's Bright Channel Prior	*/
 /*******************************************************************/

 /*******************************************************************/
 /* Created by:                                                     */
 /* Geraldine Barreto (@geraldinebc)                                */
 /*******************************************************************/

#define ABOUT_STRING "Dehazing module using the Bright Channel Prior"

/// Include auxiliary utility libraries
#include "../include/dehazing.h"

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
		"{show    |       | Show resulting image (ON: 1, OFF: 0)}"				// Show resulting image (optional)
		"{cuda    |       | Use CUDA or not (CUDA ON: 1, CUDA OFF: 0)}"         // Use CUDA (optional)
		"{time    |       | Show time measurements or not (ON: 1, OFF: 0)}"		// Show time measurements (optional)
		"{help h usage ?  |       | Print help message}";						// Show help (optional)

	CommandLineParser cvParser(argc, argv, keys);
	cvParser.about(ABOUT_STRING);												// Adds "about" information to the parser method

	//*********************************************************************************
	std::cout << ABOUT_STRING << endl;
	std::cout << "Built with OpenCV " << CV_VERSION << endl;

	// If the number of arguments is lower than 5, or contains "help" keyword, then we show the help
	if (argc < 5 || cvParser.has("help")) {
		std::cout << endl << "C++ implementation of Gao's dehazing algorithm" << endl;
		std::cout << endl << "Arguments are:" << endl;
		std::cout << "\t*Input: Input image name with path and extension" << endl;
		std::cout << "\t*Output: Output image name with path and extension" << endl;
		std::cout << "\t*-show=0 or -show=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-cuda=0 or -cuda=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-time=0 or -time=1 (ON: 1, OFF: 0)" << endl;
		std::cout << endl << "\tExample:" << endl;
		std::cout << "\t input.jpg output.jpg -show=0 -cuda=0 -time=0" << endl;
		std::cout << "\tThis will open 'input.jpg' dehaze the image and save the result in 'output.jpg'" << endl << endl;
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
	CUDA = cvParser.get<int>("cuda");	        // Gets argument -cuda=x, where 'x' define to use CUDA or not
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
	src = imread(InputFile, cv::IMREAD_COLOR);

	if (src.empty()){
		std::cout << endl << "Error occured when loading the image" << endl << endl;
		return -1;
	}

	std::cout << endl << "Aplying dehazing algorithm" << endl;

	// Start time measurement
	if (Time) t = (double)getTickCount();

	// GPU Implementation
#if USE_GPU
	if (CUDA) {
		cv::cuda::GpuMat srcGPU, new_chanGPU, dstGPU;
		srcGPU.upload(src);

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

		dstGPU.download(dst);
	}
#endif

// CPU Implementation
if (!CUDA) {
	// Split the RGB channels (BGR for OpenCV)
	std::vector<cv::Mat_<uchar>> src_chan, new_chan;
	split(src, src_chan);

	// Compute the new channels for the dehazing process
	new_chan.push_back(255 - src_chan[0]);
	new_chan.push_back(255 - src_chan[1]);
	new_chan.push_back(src_chan[2]);

	// Compute the bright channel image
	int size = sqrt(src.total()) / 50;						// Making the size bigger creates halos around objects
	cv::Mat bright_chan = brightChannel(new_chan, size);

	// Compute the maximum color difference
	cv::Mat mcd = maxColDiff(src_chan);

	// Rectify the bright channel image
	cv::Mat src_HSV, S;
	cv::cvtColor(src, src_HSV, COLOR_BGR2HSV);
	extractChannel(src_HSV, S, 1);
	cv::Mat rectified = rectify(S, bright_chan, mcd);

	// Estimate the atmospheric light
	cv::Mat src_gray;
	cv::cvtColor(src, src_gray, COLOR_BGR2GRAY);
	std::vector<uchar> A;
	A = lightEstimation(src_gray, size, bright_chan, new_chan);

	// Compute the transmittance image
	cv::Mat trans = transmittance(rectified, A);

	// Refine the transmittance image
	cv::Mat filtered;
	guidedFilter(src_gray, trans, filtered, 30, 0.001, -1);

	// Dehaze the image channels
	std::vector<cv::Mat_<float>> chan_dehazed;
	chan_dehazed.push_back(new_chan[0]);
	chan_dehazed.push_back(new_chan[1]);
	chan_dehazed.push_back(new_chan[2]);
	dst = dehaze(chan_dehazed, A, filtered);
}

//  End time measurement (Showing time results is optional)
if (Time) {
	t = 1000 * ((double)getTickCount() - t) / getTickFrequency();
	std::cout << endl << "Execution Time" << implementation << ": " << t << " ms " << endl;
	
	// Name for the output csv file where the execution time will be saved
	std::size_t pos;
	if (OutputFile.find(92) != std::string::npos) pos = OutputFile.find_last_of(92);	// If string contains '\' search for the last one
	else pos = OutputFile.find_last_of('/');											// If does not containn '\' search for the last '/'
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