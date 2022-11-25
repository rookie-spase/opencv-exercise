#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/objdetect.hpp>
#include<iostream>

using namespace std;
using namespace cv;




void main() {
	VideoCapture cap("Resources/plates.mp4");
	Mat img;

	CascadeClassifier	plateCascade;
	plateCascade.load("Resources/haarcascade_russian_plate_number.xml");


	if (plateCascade.empty()) {
		cout << "model xml file Do not loaded" << endl;
	}

vector<Rect> plates;
	while (cap.read(img))
	{
		plateCascade.detectMultiScale(img, plates);



		for (int i = 0; i < plates.size(); ++i) {
			Mat imgCrop = img(plates[i]);
			//imshow(to_string(i), imgCrop);
			imwrite("Resources/Plates/"+to_string(i) + ".png", imgCrop);
			rectangle(img, plates[i].tl(), plates[i].br(), Scalar(255, 255, 0), 2);
		}
			


		imshow("img", img);


		waitKey(1);
	}
	

}