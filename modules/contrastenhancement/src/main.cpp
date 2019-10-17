/*********************************************************************/
/* Project: uw-img-proc									             */
/* Module:  contrastenhancement				  		                 */
/* File: 	main.cpp								         	     */
/* Created:	01/03/2018				                                 */
/* Description:
	C++ Module for contrast enhancement using histogram stretching   
	and equalization												 */
 /********************************************************************/

 /********************************************************************/
 /* Created by:                                                      */
 /* Geraldine Barreto (@geraldinebc)                                 */
/*********************************************************************/

#define ABOUT_STRING "Contrast Enhancement Module based on Histogram Stretching and Equalization"

/// Include auxiliary utility libraries
#include "../include/contrastenhancement.h"

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
		"{m       |r      | Method}"											// Enhancement method to use
		"{show    |       | Show result (ON: 1, OFF: 0)}"						// Show the resulting image (optional)
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
		std::cout << endl << "C++ implementation of contrast enhancement module using histogram stretching" << endl;
		std::cout << endl << "Arguments are:" << endl;
		std::cout << "\t*Input: Input image name with path and extension" << endl;
		std::cout << "\t*Output: Output image name with path and extension" << endl;
		std::cout << "\t*-show=0 or -show=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-cuda=0 or -cuda=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-time=0 or -time=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*Argument 'm=<method>' is a string containing a list of the desired method to use" << endl;
		std::cout << endl << "Complete options of evaluation metrics are:" << endl;
		std::cout << "\t-m=S for Simplest Color Balance" << endl;
		std::cout << "\t-m=I for Integrated Color Model " << endl;
		std::cout << "\t-m=U for Unsupervised Color Correction Method" << endl;
		std::cout << "\t-m=E for Histogram Equalization" << endl;
		std::cout << "\t-m=R for Rayleigh Equalization" << endl;
		std::cout << endl << "Example:" << endl;
		std::cout << "\tinput.jpg output.jpg -cuda=0 -time=0 -show=0 -m=I" << endl;
		std::cout << "\tThis will open 'input.jpg' enhance the contrast using the ICM Method and save it in 'output.jpg'" << endl << endl;
		return 0;
	}

	int CUDA = 0;										// Default option (running with CPU)
	int Time = 0;                                       // Default option (not showing time)
	int Show = 0;										// Default option (not showing results)

	std::string InputFile = cvParser.get<cv::String>(0); // String containing the input file path+name+extension from cvParser function
	std::string OutputFile = cvParser.get<cv::String>(1);// String containing the input file path+name+extension from cvParser function
	std::string method = cvParser.get<cv::String>("m");	 // Gets argument -m=x, where 'x' is the enhancement method
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
	src = imread(InputFile, IMREAD_COLOR);

	if (src.empty()){
		std::cout << "Error occured when loading the image" << endl << endl;
		return -1;
	}

	// Start time measurement
	if (Time) t = (double)getTickCount();

	// GPU Implementation
#if USE_GPU
	if (CUDA) {
		cv::cuda::GpuMat srcGPU, channelsGPU[3], dstGPU;
		srcGPU.upload(src);

		switch (method[0]) {

			case 'S':	// Simplest Color Balance
				std::cout << endl << "Applying contrast enhancement using Simplest Color Balance" << endl;
				dstGPU = simplestColorBalance_GPU(srcGPU, 0.5);
				break;

			case 'I':	// Integrated Color Model
				std::cout << endl << "Applying contrast enhancement using an Integrated Color Model" << endl;
				dstGPU = ICM_GPU(srcGPU, 0.5);
				break;

			case 'U':	// Unsupervised Color Correction Method
				std::cout << endl << "Applying contrast enhancement using an Unsupervised Color Correction Method" << endl;
				dstGPU = UCM_GPU(srcGPU, 0.2);
				break;

			case 'E':	// Normal Equalization
				std::cout << endl << "Applying contrast enhancement using Normal Equalization" << endl;
				cv::cuda::split(srcGPU, channelsGPU);
				for (int i = 0; i < 3; i++) cv::cuda::equalizeHist(channelsGPU[i], channelsGPU[i]);
				cv::cuda::merge(channelsGPU, 3, dstGPU);
				break;

			case 'R':	// Rayleigh Equalization
				std::cout << endl << "Applying contrast enhancement using Rayleigh Equalization" << endl;
				cv::cuda::split(srcGPU, channelsGPU);
				for (int i = 0; i < 3; i++) channelsGPU[i] = rayleighEqualization_GPU(channelsGPU[i]);
				cv::cuda::merge(channelsGPU, 3, dstGPU);
				break;

			default:	// Unrecognized Option
				std::cout << "Option " << method[0] << " not recognized" << endl;
			break;
		}
		dstGPU.download(dst);
	}
#endif

	// CPU Implementation
	if (! CUDA) {

		cv::Mat balanced;
		vector<Mat_<uchar>> channels;

		switch (method[0]) {

			case 'S':	// Simplest Color Balance
				std::cout << endl << "Applying contrast enhancement using Simplest Color Balance" << endl;
				dst = simplestColorBalance(src, 0.5);
			break;

			case 'I':	// Integrated Color Model
				std::cout << endl << "Applying contrast enhancement using an Integrated Color Model" << endl;
				dst = ICM(src, 0.5);
			break;

			case 'U':	// Unsupervised Color Correction Method
				std::cout << endl << "Applying contrast enhancement using an Unsupervised Color Correction Method" << endl;
				dst = UCM(src, 0.2);
			break;

			case 'E':	// Normal Equalization
				std::cout << endl << "Applying contrast enhancement using Normal Equalization" << endl;
				split(src, channels);
				for (int i = 0; i < 3; i++) cv::equalizeHist(channels[i], channels[i]);
				merge(channels, dst);
			break;

			case 'R':	// Rayleigh Equalization
				std::cout << endl << "Applying contrast enhancement using Rayleigh Equalization" << endl;
				split(src, channels);
				for (int i = 0; i < 3; i++) channels[i] = rayleighEqualization(channels[i]);
				merge(channels, dst);
			break;

			default:	// Unrecognized Option
				std::cout << "Option " << method[0] << " not recognized" << endl;
			break;
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
		file << endl << OutputFile << ";" << src.rows << ";" << src.cols << ";" << t;
	}

	std::cout << endl << "Saving processed image" << endl;
	imwrite(OutputFile, dst);

	if (Show) {
		std::cout << endl << "Showing image comparison" << endl;
		Mat comparison;
		hconcat(src, dst, comparison);
		namedWindow("Comparison", WINDOW_KEEPRATIO);
		imshow("Comparison", comparison);
	}

	waitKey(0);
	return 0;
}