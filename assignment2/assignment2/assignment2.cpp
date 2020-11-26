#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>

using namespace cv;
using namespace std;
using namespace std::filesystem;

#define NUMBER_OF_POSTBOXES 6
int PostboxLocations[NUMBER_OF_POSTBOXES][8] = {
	{ 26, 113, 106, 113, 13, 133, 107, 134 },
	{ 119, 115, 199, 115, 119, 135, 210, 136 },
	{ 30, 218, 108, 218, 18, 255, 109, 254 },
	{ 119, 217, 194, 217, 118, 253, 207, 253 },
	{ 32, 317, 106, 315, 22, 365, 108, 363 },
	{ 119, 315, 191, 314, 118, 362, 202, 361 } };
#define POSTBOX_TOP_LEFT_COLUMN 0
#define POSTBOX_TOP_LEFT_ROW 1
#define POSTBOX_TOP_RIGHT_COLUMN 2
#define POSTBOX_TOP_RIGHT_ROW 3
#define POSTBOX_BOTTOM_LEFT_COLUMN 4
#define POSTBOX_BOTTOM_LEFT_ROW 5
#define POSTBOX_BOTTOM_RIGHT_COLUMN 6
#define POSTBOX_BOTTOM_RIGHT_ROW 7

int main()
{
	setBreakOnError(true);
	VideoCapture cap("files/PostboxesWithLines.avi");
	ofstream output;
	output.open("output.txt", std::ofstream::out | std::ofstream::trunc);
	//VideoCapture cap(0);
	cap.set(CAP_PROP_POS_AVI_RATIO, 0);
	Mat frame0;
	Mat orig_frame;
	Mat frame;

	int results[6][2];
	int column;

	cap >> frame0;
	orig_frame = frame0;
	cvtColor(frame0, frame0, COLOR_BGRA2GRAY, 0);
	int frame_count = 1;
	bool frame_has_post;
	while (1) {
		if (frame_count == 1) {
			frame = frame0.clone();
		}
		else {
			// Capture frame-by-frame
			cap >> orig_frame;
			if (frame_count == 4 || frame_count == 19 || frame_count == 41 || frame_count == 44)
				imwrite("images/original" + to_string(frame_count) + ".jpg", orig_frame);
			if (orig_frame.empty())
				break;
			cvtColor(orig_frame, frame, COLOR_BGRA2GRAY, 0);
			if (frame_count == 4 || frame_count == 19 || frame_count == 41 || frame_count == 44)
				imwrite("images/grayscale" + to_string(frame_count) + ".jpg", frame);
		}
		// Modify Frame and Calculate Diff
		absdiff(frame, frame0, frame);
		if (frame_count == 4 || frame_count == 19 || frame_count == 41 || frame_count == 44)
			imwrite("images/subtract_background" + to_string(frame_count) + ".jpg", frame);
		threshold(frame, frame, 150, 255, THRESH_BINARY);
		if (frame_count == 4 || frame_count == 19 || frame_count == 41 || frame_count == 44) {
			imwrite("images/binary_threshold"+to_string(frame_count)+".jpg", frame);
			waitKey(0);
		}

		output << to_string(frame_count) + ", ";
		cout << "Frame " + to_string(frame_count++) + ":\n";
		// Check Camera is not Obscured
		column = 0;
		for (int i = 0; i < frame.rows; i++) {
			if (frame.at<BYTE>(i, 10))
				column++;
		}
		if (column == 0 || frame.rows / column > 100) {
			frame_has_post = false;
			// Check contents of each mailbox
			for (int i = 0; i < NUMBER_OF_POSTBOXES; i++) {
				results[i][0] = 0;
				results[i][1] = 0;
				// Count Pixels Inside Postbox
				for (int y = PostboxLocations[i][1]; y < PostboxLocations[i][5]; y++) {
					for (int x = PostboxLocations[i][0]; x < PostboxLocations[i][2]; x++) {
						if (frame.at<BYTE>(y, x))
							results[i][1]++;
						else
							results[i][0]++;
					}
				}
				if (results[i][1] > results[i][0]/8) {
					if (!frame_has_post)
						output << "Post in ";
					output << to_string(i+1) + " ";
					frame_has_post = true;
				}
			}
			if (!frame_has_post) {
				output << "No post";
			}
			output << "\n";
		}
		else
			output << "Camera Obscured\n";
	}
	output.close();
	destroyAllWindows();
	cap.release();
	return 0;
}