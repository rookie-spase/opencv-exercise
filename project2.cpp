#include<opencv2/imgcodecs.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>
#include<iostream>

using namespace std;
using namespace cv;
/////////////// �ļ�ɨ�� /////////////////////
// ɨ��һ���ļ�ͼƬ��Ȼ��

/*
* 
*	ת�Ҷȣ����ģ�����ұ�Ե������
*	�ҵ����ľ��ε�4����
*	��������
*		���Ͻǵĵ�x+y����С��(0),���½ǵĵ�x+y������(1)
*		���Ͻǵ�x-y������(2)�����½ǵ�x-yӦ������С��(3)
*	���Ƴ�����4����
*	���⼸�������
*	��΢���вü�
*/

Mat imgOriginal, imgGray, imgBulr, imgCanny, imgDil, imgErode, imgWarp, imgCrop;
Mat imgThre;
vector<Point> initialPoints;
vector<Point> docPoints;
float w = 420, h = 596;

Mat preprocessing(Mat img) {

	cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);				// ͼ��ת��
			// ͼ������ void cvtColor(ԭͼ��ԭͼ����֮���������,��θ���ԭͼ);
	cv::GaussianBlur(img, imgBulr, cv::Size(7, 7), 7, 0);  // ��˹ģ��
			// void GaussianBlur(ԭͼ, ԭͼ����֮���������, �ں˴�С/*ʲô����*/, 7, 0); ������������Խ���Խģ��
	cv::Canny(img, imgCanny, 30, 75);		// ����/ͼ�� �����������
			// void Canny(ԭͼ, ԭͼ�����֮���������, ��ⷶΧ����, ��ⷶΧ����);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));		// ��������������Խ������Խǿ
	cv::dilate(imgCanny, imgDil, kernel);		// ����������(���ǰ��������)�����ͼƬ����̫ϸ��ʾ������������
			// void dilate(�Ѿ�������˵�ͼƬ, ������֮���ͼƬ��������, ����ں���Ҫ�ӵ�)
	//cv::erode(imgdil, imgErode, kernel);	// ������ʴ��(���ǰ�������ϸ)
			// void erode(���͵�ͼƬ,��ʴ���������,kernel/*kernel����Ҫ�����͵�һ��*/)

	return imgDil;

}

vector<Point> get_contours(Mat img) {


	vector<vector<Point>> countor;
	vector<Vec4i> hierarchy;

	findContours(imgDil, countor, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);


	vector<vector<Point>> conPoly(countor.size());			// ����������
	vector<Rect> boundrect(countor.size());					// ���淶Χ
	

	vector<Point> biggest;
	int max_area = -1;

	for (int i = 0; i < countor.size(); ++i) {
		int area = contourArea(countor[i]);			// ������������
		

		if (area > 1000) {
			double arlen = arcLength(countor[i], true);		// ��ȡ��i��ͼ�ε���������
			approxPolyDP(countor[i], conPoly[i], 0.02 * arlen, true);		// �����i����״�ļ��

			if (area > max_area && conPoly[i].size() == 4) {
				max_area = area;
				biggest = { conPoly[i][0],conPoly[i][1], conPoly[i][2], conPoly[i][3]};
				//drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 0), 2);
			}
			

		}
	}


	return biggest;

}

void drawpoints(vector<Point> Points,Scalar color) {
	for (int i = 0; i < Points.size(); ++i) {
		circle(imgOriginal, Points[i], 3, color, FILLED);
		putText(imgOriginal, to_string(i), Points[i], FONT_HERSHEY_COMPLEX_SMALL, 2, color, 2);
	}
}

vector<Point> reorder(vector<Point> points) {

	vector<Point> newPoints;
	vector<int> sum, sub;
	
	for (int i = 0; i < 4; ++i) {
		sum.push_back(points[i].x+points[i].y);
		sub.push_back(points[i].x-points[i].y);
	}

	newPoints.push_back(points[min_element(sum.begin(),sum.end()) - sum.begin()]);
	newPoints.push_back(points[max_element(sub.begin(),sub.end()) - sub.begin()]);
	newPoints.push_back(points[min_element(sub.begin(),sub.end()) - sub.begin()]);
	newPoints.push_back(points[max_element(sum.begin(),sum.end()) - sum.begin()]);

	return newPoints;
}

Mat get_warp(Mat img,vector<Point> points, float w, float h) {
	Point2f src[4] = {points[0],points[1], points[2], points[3]};
	Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };
	// ����Ҫд������
	// w,h �ǿ��Ŀ�Ⱥͳ���


	Mat matrix = getPerspectiveTransform(src, dst);		// ͸�ӱ任
					// ������ת�ã���Ϊ��Ҫ����4�����ͼ��ץȡ����Ȼ��ŵ�һ���µĴ���
					// ���ץȡ������ͼ����Ҫ�������Ǿ�Ҫ���߼����ԭ����4�����Ŀ��4����,�ü���������ֵ
					// ��������Ǽ���,����û�н���ץȡ
	warpPerspective(img, imgWarp, matrix, Point(w, h));		// ����͸��
					// void warpPerspective(ԭͼ,�ŵĵط�, ת�ú�ľ���,��Ƭ�Ŀ�Ⱥͳ���/*��ȡ�Ĵ�С*/);
					// ����ǽ���ץȡ�Ͱ����Ĺ�����
	
	return imgWarp;
}

void main() {
	std::string src = "Resources/paper.jpg";
	imgOriginal = imread(src);
	//resize(imgOriginal, imgOriginal,Size(),0.5,0.5);

	// preprocessing
	imgThre = preprocessing(imgOriginal);
	// get COntours -  biggest
	initialPoints = get_contours(imgThre);
	// reorder
	docPoints = reorder(initialPoints);
	// draw points
	//drawpoints(docPoints, Scalar(255, 0, 0));
	// warp
	imgWarp = get_warp(imgOriginal,docPoints,w,h);
	// crop
	int cval = 10;
	Rect roi(cval, cval,w-(2*cval), h - (2 * cval));
	imgCrop = imgWarp(roi);

	cv::imshow("windows", imgOriginal);
	cv::imshow("warp", imgWarp);
	cv::imshow("crop", imgCrop);
	


	cv::waitKey(0);
}
