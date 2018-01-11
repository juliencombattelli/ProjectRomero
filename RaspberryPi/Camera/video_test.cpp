#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/videoio/videoio.hpp"
//#include "opencv2/opencv.hpp"

#include <iostream>
#include <unistd.h>

using namespace cv;
using namespace std;

int countLeft(Mat img){
	int h = img.rows;
	int w = img.cols;
	int count = 0;
	for (int y=h/4; y<h; y++){
		for (int x=0; x<w/2; x++){
			if (y >= (-h*x/(2*w))+(h/2)){
				int pixel = img.at<int>(y,x,0);
				if (pixel == 0) {
					count++;
				}
			}
		}
	}
	return count;
}

int countRight(Mat img){
	int h = img.rows;
	int w = img.cols;
	int count = 0;
	for (int y=h/4; y<h; y++){
		for (int x=w/2; x<w; x++){
			if (y >= h*x/(2*w)){
				int pixel = img.at<int>(y,x,0);
				if (pixel == 0) {
					count++;
				}
			}
		}
	}
	return count;
}

int placement (Mat img){
	const int front = 0;
	const int right = 1;
	const int rightCrit = 2;
	const int left = 3;
	const int leftCrit = 4;
	
	const int surface = (img.rows*img.cols*5)/16;
	
	const int low = surface*0.25;
	const int critical = surface * 0.35;
	const int filled = surface * 0.7;
	//const int side = surface * 0.6;

	int l = countLeft(img);
	int r = countRight(img);
	if (r < low && l < low){
		return front;
	}
	else if (r < low){
		if (l < critical){
			return front;
		} else if (l < filled) {
			return left;
		} else {
			return leftCrit;
		}
	}
	else if (l < low){
		if (r < critical){
			return front;
		} else if (r < filled) {
			return right;
		} else {
			return rightCrit;
		}
	}
	/*else if (r > critical && l > side){
		return leftCrit;
	}
	else if (l > critical && r > side){
		return rightCrit;
	}*/
	/*else if (r < critical && l < critical){
		return front;
	}*/
	else {
		if (r > l){
			return rightCrit;
		} else {
			return leftCrit;
		}
	}
}

int main(int argc, char **argv ){
	
	VideoCapture cap(0); // open the default camera
	if(!cap.isOpened()){  // check if we succeeded
        	cout << "Could not open camera" << endl;
		return -1;
	}
	for(;;){
		Mat img, hsv, val[3], hue, sat;
		cap >> img; // get a new frame from camera
		/* Convert from Red-Green-Blue to Hue-Saturation-Value */
		cvtColor(img, hsv, CV_BGR2HSV);

		/* Split hue, saturation and value of hsv in val, get hue*/
		split(hsv, val);
		hue = val[0];
		
		inRange(hue, 60, 255, hue);
		int place = placement (hue);
		if (place == 0){
			cout << "Front"  << endl;
		}
		else if (place == 1){
			cout << "Right" << endl;
		}
		else if (place == 2){
			cout << "RightCrit " << endl;
		}
		else if (place == 3){
				cout << "Left" << endl;
		}
		else if (place == 4){
				cout << "LeftCrit" << endl;
		}
		else{
			cout << "error place" << endl;
		}
		//imwrite("hue.jpg", hue);

		//cout << "place " << place << endl;
		//usleep(200000);
	}

return 0;
}
