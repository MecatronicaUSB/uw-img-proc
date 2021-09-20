/*********************************************************************/
/* Project: uw-img-proc									             */
/* Module:  videoenhancement				  		                 */
/* File: 	main.cpp								         	     */
/* Created:	20/09/2019				                                 */
/* Description:
	C++ Module for underwater video enhancement						 */
 /********************************************************************/

 /********************************************************************/
 /* Created by:                                                      */
 /* Geraldine Barreto (@geraldinebc)                                 */
/*********************************************************************/

#define ABOUT_STRING "Video Enhancement Module"

/// Include auxiliary utility libraries
#include "../include/videoenhancement.h"

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
		"{@input  |<none> | Input image file}"									// Input video is the first argument (positional)
		"{m       |r      | Method}"											// Enhancement method to use
		"{comp    |       | Save video comparison (ON: 1, OFF: 0)}"				// Show video comparison (optional)
		"{cuda    |       | Use CUDA or not (CUDA ON: 1, CUDA OFF: 0)}"         // Use CUDA (optional)
		"{time    |       | Show time measurements or not (ON: 1, OFF: 0)}"		// Show time measurements (optional)
		"{help h usage ?  |       | Print help message}";						// Show help (optional)

	CommandLineParser cvParser(argc, argv, keys);
	cvParser.about(ABOUT_STRING);												// Adds "about" information to the parser method

	//**************************************************************************
	std::cout << ABOUT_STRING << endl;
	std::cout << "\nBuilt with OpenCV" << CV_VERSION << endl;

	// If the number of arguments is lower than 3, or contains "help" keyword, then we show the help
	if (argc < 5 || cvParser.has("help")) {
		std::cout << endl << "C++ implementation of video enhancement module" << endl;
		std::cout << endl << "Arguments are:" << endl;
		std::cout << "\t*Input: Input video name with path and extension" << endl;
		std::cout << "\t*-comp=0 or -comp=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-cuda=0 or -cuda=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*-time=0 or -time=1 (ON: 1, OFF: 0)" << endl;
		std::cout << "\t*Argument 'm=<method>' is a string containing a list of the desired method to use" << endl;
		std::cout << endl << "Complete options are:" << endl;
		std::cout << "\t-m=C for Color Correction" << endl;
		std::cout << "\t-m=E for Histogram Equalization" << endl;
		std::cout << "\t-m=S for Histogram Stretching" << endl;
		std::cout << "\t-m=D for Dehazing" << endl;
		std::cout << endl << "Example:" << endl;
		std::cout << "\tv1.mp4 -cuda=0 -time=0 -comp=0 -m=E" << endl;
		std::cout << "\tThis will open 'v1.mp4' enhance the video using histogram equalization and save it in 'v1_E.avi'" << endl << endl;
		return 0;
	}

	int CUDA = 0;										// Default option (running with CPU)
	int Time = 0;                                       // Default option (not showing time)
	int Comp = 0;										// Default option (not showing results)

	std::string InputFile = cvParser.get<cv::String>(0); // String containing the input file path+name+extension from cvParser function
	std::string method = cvParser.get<cv::String>("m");	 // Gets argument -m=x, where 'x' is the enhancement method
	std::string implementation;							 // CPU or GPU implementation
	Comp = cvParser.get<int>("comp");					 // Gets argument -comp=x, where 'x' defines if the comparison video will be saved or not
	Time = cvParser.get<int>("time");	                 // Gets argument -time=x, where 'x' defines if execution time will show or not

	// Check if any error occurred during parsing process
	if (!cvParser.check()) {
		cvParser.printErrors();
		return -1;
	}

	int nCuda = -1;    //Defines number of detected CUDA devices. By default, -1 acting as error value

	std::size_t filename;
	if (InputFile.find('.') != std::string::npos) filename = InputFile.find_last_of('.');	// Find the last '.'
	std::string ext_out = '_' + method + ".avi";
	std::string OutputFile = InputFile.substr(0, filename);
	OutputFile.insert(filename, ext_out);

	std::cout << endl << "************************************************************************" << endl;
	std::cout << endl << "Input: " << InputFile << endl;
	std::cout << "Output: " << OutputFile << endl;

	// Opend the video file
	cv::VideoCapture cap(InputFile);
	if (!cap.isOpened()) {
		std::cout << "\nUnable to open the video \n";
		return -1;
	}

	// Get the width/height frame count and the FPS of the video
	int width = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
	int height = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));
	int n_frames = int(cap.get(CAP_PROP_FRAME_COUNT));
	double FPS = cap.get(CAP_PROP_FPS);

	// Open a video file for writing the output
	cv::VideoWriter out(OutputFile,cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), FPS, cv::Size(width, height));
	if (!out.isOpened()) {
		std::cout << "\nError! Unable to open video file for the output video \n\n" << std::endl;
		return -1;
	}

	std::string ext_comp = '_' + method + "_comp.avi";
	std::string Comparison = InputFile.substr(0, filename);
	Comparison.insert(filename, ext_comp);

	// Open a video file for writing the comparison
	cv::VideoWriter comp(Comparison, cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), FPS, cv::Size(2 * width, height));
	if (!comp.isOpened()) {
		std::cout << "\nError! Unable to open video file for the comparison video \n\n" << std::endl;
		return -1;
	}

	// Start time measurement
	if (Time) t = (double)getTickCount();

	// CPU Implementation
	if (! CUDA) {

		cv::Mat image, image_out, comparison, top;
		vector<cv::Mat> frames;
		cv::Mat sum(cv::Size(width, height), CV_32FC3, Scalar());
		cv::Mat avgImg(cv::Size(width, height), CV_32FC3, Scalar());
		vector<Mat_<uchar>> channels;

		int i = 0, j = 0;
		float n;
		if (n_frames / FPS < 7) n = n_frames / FPS * 0.5;
		else n = FPS * 7;

		switch (method[0]) {

			case 'C':	// Color Correction
				std::cout << endl << "Applying video enhancement using the Gray World Assumption" << endl;
				for (i = 0; i < n_frames - 1; i++) {
					cap >> image;
					image_out = colorcorrection(image);
					out << image_out;
					if (Comp) {
						hconcat(image, image_out, comparison);
						comp << comparison;
					}
				}
				std::cout << "\nProcessed video saved\n";
				break;

			case 'E':	// Histogram Equalization
				std::cout << endl << "Applying video enhancement using Histogram Equalization" << endl;
				for (i = 0; i < n_frames - 1; i++) {
					cap >> image;
					split(image, channels);
					for (j = 0; j < 3; j++) equalizeHist(channels[j], channels[j]);
					merge(channels, image_out);
					out << image_out;
					if (Comp) {
						hconcat(image, image_out, comparison);
						comp << comparison;
					}
				}
				std::cout << "\nProcessed video saved\n";
			break;

			case 'H':	// Histogram Stretching
				std::cout << endl << "Applying video enhancement using Histogram Stretching" << endl;
				while (true) {
					if (frames.size() < n) {
						cap >> image;
						if (image.empty()) {
							for (int i = 0; i < (n - 1) / 2; i++) {
								image_out = ICM(avgImg, frames[(n - 1) / 2 + i], 0.3);
								out << image_out;
								if (Comp) {
									hconcat(frames[(n - 1) / 2 + i], image_out, comparison);
									comp << comparison;
								}
							}
							std::cout << "\nProcessed video saved\n";
							break;
						}
						frames.push_back(image);
						image.convertTo(image, CV_32FC3);
						accumulate(image, sum);
					}
					else {
						sum.convertTo(avgImg, CV_8UC3, 1.0 / n);
						for (j; j < (n - 1) / 2; j++) {
							image_out = ICM(avgImg, frames[j], 0.5);
							out << image_out;
							if (Comp) {
								hconcat(frames[j], image_out, comparison);
								comp << comparison;
							}
						}
						image_out = ICM(avgImg, frames[(n - 1) / 2], 0.5);
						out << image_out;
						if (Comp) {
							hconcat(frames[(n - 1) / 2], image_out, comparison);
							comp << comparison;
						}
						frames.erase(frames.begin());
						frames[0].convertTo(top, CV_32FC3);
						subtract(sum, top, sum);
					}
				}
			break;

			case 'D':	// Dehazing
				std::cout << endl << "Applying video enhancement using the Bright Channel Prior" << endl;
				while (true) {
					if (frames.size() < n) {
						cap >> image;
						if (image.empty()) {
							for (int i = 0; i < (n - 1) / 2; i++) {
								image_out = dehazing(avgImg, frames[(n - 1) / 2 + i]);
								out << image_out;
								if (Comp) {
									hconcat(frames[(n - 1) / 2 + i], image_out, comparison);
									comp << comparison;
								}
							}
							std::cout << "\nProcessed video saved\n";
							break;
						}
						frames.push_back(image);
						image.convertTo(image, CV_32FC3);
						accumulate(image, sum);
					}
					else {
						sum.convertTo(avgImg, CV_8UC3, 1.0 / n);
						for (j; j < (n - 1) / 2; j++) {
							image_out = dehazing(avgImg, frames[j]);
							out << image_out;
							if (Comp) {
								hconcat(frames[j], image_out, comparison);
								comp << comparison;
							}
						}
						image_out = dehazing(avgImg, frames[(n - 1) / 2]);
						out << image_out;
						if (Comp) {
							hconcat(frames[(n - 1) / 2], image_out, comparison);
							comp << comparison;
						}
						frames.erase(frames.begin());
						frames[0].convertTo(top, CV_32FC3);
						subtract(sum, top, sum);
					}
				}
			break;

			default:	// Unrecognized Option
				std::cout << "Option " << method[0] << " not recognized" << endl;
			break;
		}

		// When everything done, release the video capture object
		cap.release();
	}

	// End time measurement (Showing time results is optional)
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
		file << endl << OutputFile << ";" << width << ";" << height << ";" << t;
	}

	waitKey(0);
	return 0;
}