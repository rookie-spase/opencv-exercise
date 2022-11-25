#include<opencv2/opencv.hpp>
#include<iostream>
#include<opencv2/highgui.hpp>

using namespace std;
using namespace cv;


////////////////////////////////////////////////////////////////////////////////////////////////
VideoCapture cap("Resources/test_video.mp4");
Mat img;
vector<vector<int>> mycolor{					// 要寻找的颜色
								{124,48,117,143,170,255},		// perpel
								{68,72,156,102,126,255},		// green
								{0,62,0,35,255,255}				// 橙色
};
vector<Scalar> mycolorValues{					// 要显示的颜色
	{255,0,255},												// purple
	{0,255,0},													// green
	{51,153,255}												// 橙色
};
vector<vector<int>> newpoints;					// 找到的目标点

////////////////////////////////////////////////////////////////////////////////////////////////

/*		Project 1
*		说明：这个项目是通过摄像头跟踪指定的颜色画笔，画笔经过的地方会在屏幕上画出一条线(线由圆组成)
*			
* 
*		流程说明：  (每一步都是一个函数)
*			1.通过hsv检测出颜色(mask)
*					检测的颜色不止绿色，也有可能是蓝色和紫色。
*					那么检测的范围就不是只有一个，由多个范围(蓝色，绿色等)的范围
*						表示这个范围: vector<vector<int>> mycolor{{},{}};
*						使用这个范围：第一次时检测绿色，第二次检测蓝色... 应该用for
*				> cvtColor(img, imgHsv, COLOR_BGR2HSV)  从bgr到hsv的转换
*				> inRange(imgHsv, lower, upper, mask) with for loop  检测颜色
*				> call step2 funciton to find contour in for loop
*				> return step2 
*			2.根据检测出的mask绘制矩形将对象选中
*					这个就是第7章的知识,找轮廓点，然后找尖点再绘制矩形.
*					这个函数应该返回矩形左上角坐标,因为需要绘制圆;
*					检测出多个矩形的情况是很常见的，所以点就不应该是一个Point
*						表示: vector<vector<int>> {x,y,颜色编号}
*					
*				> 和第7章一样，但是只需要一个参数，再原图上画就可以
*				> 返回的时候是返回一个点
*			3.从矩形的左上角开始绘制圆
*					这个函数接收点和在这个点需要画什么颜色
*				> circle(img, Point(newPoints[i][0],newPoints[i][1]), 10, myColorValues[newPoints[i][2]], FILLED);
*/

Point getContours(Mat imgDil) {

	vector<vector<Point>> countor;
	vector<Vec4i> hierarchy;

	findContours(imgDil, countor, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);	


	vector<vector<Point>> conPoly(countor.size());			// 用来保存尖点
	vector<Rect> boundrect(countor.size());					// 保存范围
	Point mypoints(0,0);

	for (int i = 0; i < countor.size(); ++i) {
		int area = contourArea(countor[i]);			// 计算区域的面积
		cout << area << endl;

		if (area > 1000) {
			int arlen = arcLength(countor[i], true);		// 获取第i个图形的轮廓长度
			approxPolyDP(countor[i], conPoly[i], 0.02 * arlen, true);		// 计算第i个形状的尖点


			boundrect[i] = boundingRect(conPoly[i]);									// 从尖点中获取范围

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
		newpoints.push_back({ mypoint.x,mypoint.y,i });	// i就是颜色编号
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