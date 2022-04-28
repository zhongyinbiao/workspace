
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
    cv::Mat frame;

    // from video
    cv::VideoCapture capture("./v_test.mp4");
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

    waitKey(200);

    while (1)
    {
        // delay for show frame
        waitKey(50);
        imshow("frame", frame);

        if(!capture.read(frame))
        {
            cout << " read video failed.." << endl;
	        return -1;
        }
    }

    capture.release();

    return 0;
}

