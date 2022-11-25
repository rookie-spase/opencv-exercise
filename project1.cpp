#include<opencv2/opencv.hpp>
#include<iostream>
#include<opencv2/highgui.hpp>

using namespace std;
using namespace cv;


////////////////////////////////////////////////////////////////////////////////////////////////
VideoCapture cap("Resources/test_video.mp4");
Mat img;
vector<vector<int>> mycolor{					// ҪѰ�ҵ���ɫ
								{124,48,117,143,170,255},		// perpel
								{68,72,156,102,126,255},		// green
								{0,62,0,35,255,255}				// ��ɫ
};
vector<Scalar> mycolorValues{					// Ҫ��ʾ����ɫ
	{255,0,255},												// purple
	{0,255,0},													// green
	{51,153,255}												// ��ɫ
};
vector<vector<int>> newpoints;					// �ҵ���Ŀ���

////////////////////////////////////////////////////////////////////////////////////////////////

/*		Project 1
*		˵���������Ŀ��ͨ������ͷ����ָ������ɫ���ʣ����ʾ����ĵط�������Ļ�ϻ���һ����(����Բ���)
*			
* 
*		����˵����  (ÿһ������һ������)
*			1.ͨ��hsv������ɫ(mask)
*					������ɫ��ֹ��ɫ��Ҳ�п�������ɫ����ɫ��
*					��ô���ķ�Χ�Ͳ���ֻ��һ�����ɶ����Χ(��ɫ����ɫ��)�ķ�Χ
*						��ʾ�����Χ: vector<vector<int>> mycolor{{},{}};
*						ʹ�������Χ����һ��ʱ�����ɫ���ڶ��μ����ɫ... Ӧ����for
*				> cvtColor(img, imgHsv, COLOR_BGR2HSV)  ��bgr��hsv��ת��
*				> inRange(imgHsv, lower, upper, mask) with for loop  �����ɫ
*				> call step2 funciton to find contour in for loop
*				> return step2 
*			2.���ݼ�����mask���ƾ��ν�����ѡ��
*					������ǵ�7�µ�֪ʶ,�������㣬Ȼ���Ҽ���ٻ��ƾ���.
*					�������Ӧ�÷��ؾ������Ͻ�����,��Ϊ��Ҫ����Բ;
*					����������ε�����Ǻܳ����ģ����Ե�Ͳ�Ӧ����һ��Point
*						��ʾ: vector<vector<int>> {x,y,��ɫ���}
*					
*				> �͵�7��һ��������ֻ��Ҫһ����������ԭͼ�ϻ��Ϳ���
*				> ���ص�ʱ���Ƿ���һ����
*			3.�Ӿ��ε����Ͻǿ�ʼ����Բ
*					����������յ�����������Ҫ��ʲô��ɫ
*				> circle(img, Point(newPoints[i][0],newPoints[i][1]), 10, myColorValues[newPoints[i][2]], FILLED);
*/

Point getContours(Mat imgDil) {

	vector<vector<Point>> countor;
	vector<Vec4i> hierarchy;

	findContours(imgDil, countor, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);	


	vector<vector<Point>> conPoly(countor.size());			// ����������
	vector<Rect> boundrect(countor.size());					// ���淶Χ
	Point mypoints(0,0);

	for (int i = 0; i < countor.size(); ++i) {
		int area = contourArea(countor[i]);			// ������������
		cout << area << endl;

		if (area > 1000) {
			int arlen = arcLength(countor[i], true);		// ��ȡ��i��ͼ�ε���������
			approxPolyDP(countor[i], conPoly[i], 0.02 * arlen, true);		// �����i����״�ļ��


			boundrect[i] = boundingRect(conPoly[i]);									// �Ӽ���л�ȡ��Χ

			mypoints.x = boundrect[i].x + boundrect[i].width / 2;
			mypoints.y = boundrect[i].y;

		}

	}


	return mypoints;
}



vector<vector<int>> findColor(Mat img) {
	Mat imgHsv;
	cvtColor(img, imgHsv, COLOR_BGR2HSV);
	

	for (int i = 0; i < mycolor.size(); ++i) {
		Scalar lower(mycolor[i][0], mycolor[i][1], mycolor[i][2]);
		Scalar upper(mycolor[i][3], mycolor[i][4], mycolor[i][5]);
		
		Mat mask;
		inRange(imgHsv, lower, upper, mask);

		Point mypoint = getContours(mask);
		if(mypoint.x != 0 || mypoint.y != 0)
		newpoints.push_back({ mypoint.x,mypoint.y,i });	// i������ɫ���
	}
	
	 return newpoints;
}

void drawonCanvas(vector<vector<int>> Points,vector<Scalar> color) {
	for (int i = 0; i < Points.size(); ++i) {
		circle(img, Point( Points[i][0],Points[i][1]), 10, color[newpoints[i][2]],FILLED);
	}
}

int main() {



	while (cap.read(img)) {
		

		newpoints = findColor(img);
		drawonCanvas(newpoints,mycolorValues);

		imshow("img", img);
		waitKey(1);
	}



	return -1;
}