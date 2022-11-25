#include<opencv2/imgcodecs.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>
#include<iostream>

using namespace std;
using namespace cv;
/////////////// 文件扫描 /////////////////////
// 扫描一个文件图片，然后将

/*
* 
*	转灰度，添加模糊，找边缘，扩张
*	找到最大的矩形的4个点
*	给点排序
*		左上角的点x+y是最小的(0),右下角的点x+y是最大的(1)
*		右上角的x-y是最大的(2)，左下角的x-y应该是最小的(3)
*	绘制出来这4个点
*	将这几个点摆正
*	稍微进行裁剪
*/

Mat imgOriginal, imgGray, imgBulr, imgCanny, imgDil, imgErode, imgWarp, imgCrop;
Mat imgThre;
vector<Point> initialPoints;
vector<Point> docPoints;
float w = 420, h = 596;

Mat preprocessing(Mat img) {

	cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);				// 图像转换
			// 图像设置 void cvtColor(原图，原图改完之后放在哪里,如何更改原图);
	cv::GaussianBlur(img, imgBulr, cv::Size(7, 7), 7, 0);  // 高斯模糊
			// void GaussianBlur(原图, 原图改完之后放在哪里, 内核大小/*什么东西*/, 7, 0); 后三个参数是越大就越模糊
	cv::Canny(img, imgCanny, 30, 75);		// 人物/图像 的轮廓检测器
			// void Canny(原图, 原图检测完之后放在哪里, 检测范围下限, 检测范围上限);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));		// 尽量用奇数，数越大力度越强
	cv::dilate(imgCanny, imgDil, kernel);		// 轮廓膨胀器(就是把轮廓变粗)，解决图片轮廓太细显示不出来的问题
			// void dilate(已经被检测了的图片, 膨胀完之后的图片放在哪里, 这个内核是要加的)
	//cv::erode(imgdil, imgErode, kernel);	// 轮廓侵蚀器(就是把轮廓变细)
			// void erode(膨胀的图片,侵蚀后放在哪里,kernel/*kernel不需要和膨胀的一样*/)

	return imgDil;

}

vector<Point> get_contours(Mat img) {


	vector<vector<Point>> countor;
	vector<Vec4i> hierarchy;

	findContours(imgDil, countor, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);


	vector<vector<Point>> conPoly(countor.size());			// 用来保存尖点
	vector<Rect> boundrect(countor.size());					// 保存范围
	

	vector<Point> biggest;
	int max_area = -1;

	for (int i = 0; i < countor.size(); ++i) {
		int area = contourArea(countor[i]);			// 计算区域的面积
		

		if (area > 1000) {
			double arlen = arcLength(countor[i], true);		// 获取第i个图形的轮廓长度
			approxPolyDP(countor[i], conPoly[i], 0.02 * arlen, true);		// 计算第i个形状的尖点

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
	// 这里要写浮点数
	// w,h 是卡的宽度和长度


	Mat matrix = getPerspectiveTransform(src, dst);		// 透视变换
					// 将矩阵转置，因为需要将这4个点的图形抓取下来然后放到一个新的窗口
					// 这个抓取下来的图形需要摆正，那就要告诉计算机原来的4个点和目标4个点,让计算机计算差值
					// 这个仅仅是计算,还并没有进行抓取
	warpPerspective(img, imgWarp, matrix, Point(w, h));		// 翘曲透视
					// void warpPerspective(原图,放的地方, 转置后的矩阵,卡片的宽度和长度/*截取的大小*/);
					// 这就是进行抓取和摆正的工作了
	
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
