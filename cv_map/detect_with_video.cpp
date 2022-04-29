#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <ctype.h>
#include "opencv2/imgproc/imgproc_c.h"

using namespace cv;
using namespace std;

// 声明全局变量
Mat image;
//bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
Rect selection;
Point origin;

// 鼠标操作回调:鼠标状态每改变一次，此函数就执行一次
void onMouse(int event, int x, int y, int, void*)
{
	//当左键按下，框选标志为真，执行如下程序得到矩形区域selection
	if (selectObject)
	{
		selection.x = MIN(x, origin.x);
		selection.y = MIN(y, origin.y);
		selection.width = std::abs(x - origin.x);
		selection.height = std::abs(y - origin.y);
		//矩形区域与image进行与运算，结果保存到矩形区域中
		selection &= Rect(0, 0, image.cols, image.rows);
	}

	switch (event)
	{
		//此句代码的OpenCV2版为：
		//case CV_EVENT_LBUTTONDOWN:

		//此句代码的OpenCV3版为：
		//当左键按下，记录原点，创建矩形区域，框选标志置为真
		case EVENT_LBUTTONDOWN:
			origin = Point(x, y);
			selection = Rect(x, y, 0, 0);
			selectObject = true;
			break;

	    //此句代码的OpenCV2版为：
		//case CV_EVENT_LBUTTONUP:

		//此句代码的OpenCV3版为：
		//当左键弹起，将框选标志置为假，如果矩形区域的长宽都大于零，令跟踪标志为-1
		case EVENT_LBUTTONUP:
			selectObject = false;
			if (selection.width > 0 && selection.height > 0)
				trackObject = -1;
			break;
	}
}


int main()
{
	int hsize = 16;
	float hranges[] = { 0,180 };
	const float* phranges = hranges;
	Rect trackWindow;

	// 记录轨迹
	vector <Point> follow_points;

	/*
	VideoCapture cap;
	cap.open(0);
	if (!cap.isOpened())
	{
		cout << "初始化摄像头失败\n";
		while (true);
	}
	*/

	Mat frame, hsv, hue, mask, hist, backproj;

	cv::VideoCapture capture("./v_test2.mp4");
    long totalFrameNumber = capture.get(CAP_PROP_FRAME_COUNT);
    cout << "This Video total has:" << totalFrameNumber << "frame" << endl;
    capture.set( CAP_PROP_POS_FRAMES,0);

    double rate = capture.get(CAP_PROP_FPS);
	cout << "fps:" << rate << endl;
    int delay = 1000/rate;
	if(!capture.read(frame))
	{
	    	cout << "read video failed." << endl;
	        return -1;
	}
	cout << "frame width:" << frame.rows << endl;
	cout << "frame height:" << frame.cols << endl;

	cv::Point origin_point = cv::Point(0, 0);
	cv::Point rows_end_point = cv::Point(0, frame.rows);
	cv::Point cols_end_point = cv::Point(frame.cols, 0);
	cv::Point end_point = cv::Point(frame.cols, frame.rows);

	char str_point_xy[20] = "";
	CvFont font_xy;
	cvInitFont(&font_xy, 3, 0.5, 0.5, 1, 2, 8);

	//namedWindow("Histogram", 1);
	namedWindow("Map and Target", 1);
	// 鼠标事件检测
	setMouseCallback("Map and Target", onMouse, 0);

	while (true)
	{
		waitKey(20);
        if(!capture.read(frame))
        {
            cout << " read video failed.." << endl;
	        return -1;
        }
		
		frame.copyTo(image);//将当前帧复制到image中
	
		cvtColor(image, hsv, COLOR_BGR2HSV);//将image转为hsv色彩空间，保存到hsv中
		if (trackObject)//如果有操作，trackobject等于1或-1
		{
			// 亮度范围设置
			int _vmin = 10, _vmax = 256;
			// 色彩范围检测
			// Scalar: 色调、饱和度、亮度，第一个Scalar是最小值，第二个Scalar是最大值
			inRange(hsv, Scalar(0, 30, MIN(_vmin, _vmax)),Scalar(180, 256, MAX(_vmin, _vmax)), mask);

			int ch[] = { 0, 0 };
			hue.create(hsv.size(), hsv.depth());//创建一个与hsv尺寸和深度一样的hue
			// 从输入中拷贝某通道到输出中特定的通道。
			mixChannels(&hsv, 1, &hue, 1, ch, 1);

			if (trackObject < 0)//如果为-1,代表左键弹起，划定了区域
			{
				//hue是视频帧处理后的图像，selection是鼠标选定的矩形区域，同时创建一个感兴趣区域和一个标记感兴趣区域
				Mat roi(hue, selection), maskroi(mask, selection);
				//imshow("ROI", roi);
				//imshow("maskROI", maskroi);
				calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
				normalize(hist, hist, 0, 255, NORM_MINMAX);
				trackWindow = selection;
				trackObject = 1;
			}

			calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
			backproj &= mask;
			RotatedRect trackBox = CamShift(backproj, trackWindow, TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 10, 1));

			if (trackWindow.area() <= 1)
			{
				int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5) / 6;
				trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
					trackWindow.x + r, trackWindow.y + r) &
					Rect(0, 0, cols, rows);
			}

			// 投影视图
			//cvtColor(backproj, image, COLOR_GRAY2BGR);

			// 画 x / y 轴
			//cv::line(image, origin_point, rows_end_point, Scalar(0, 0, 255), 3, LINE_AA);
			//cv::line(image, origin_point, cols_end_point, Scalar(0, 0, 255), 3, LINE_AA);

			// 画目标椭圆
			ellipse(image, trackBox, Scalar(0, 255, 0), 1, LINE_AA);
			// 画目标圆心
			cv::circle(image, trackBox.center, 2, Scalar(0, 0, 255), LINE_4);
			sprintf(str_point_xy, "  [%.1f, %.1f]", trackBox.center.x, trackBox.center.y);

			// 填充并绘制轨迹点
			follow_points.push_back(cv::Point(trackBox.center.x, trackBox.center.y));
			if (follow_points.size() >= 1)
			{
				for(int index = 0; index < follow_points.size(); index++)
				{
					cv::circle(image, follow_points[index], 2, Scalar(0, 0, 255), LINE_4);
				}
			}

			putText(image, str_point_xy, trackBox.center, FONT_HERSHEY_PLAIN, 2.0, Scalar(0, 0, 255), 2);

			// 画连接直线
			cv::line(image, origin_point, trackBox.center, Scalar(0, 0, 255), 1, LINE_4);

		}
		if (selectObject && selection.width > 0 && selection.height > 0)
		{
			Mat roi(image, selection);
			bitwise_not(roi, roi);
		}
		cv::imshow("Map and Target", image);

		char c = (char)waitKey(10);
		if (c == 27) break;
	}

	capture.release();
	return 0;
}