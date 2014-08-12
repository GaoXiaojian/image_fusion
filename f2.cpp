/******************************************************
[内容]	
	图像金字塔

*******************************************************/


//-------------------文件包含----------------------
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <stdlib.h>
//------------------命名空间---------------------
using namespace cv;
using namespace std;

//------------------全局变量---------------------

 
//------------------函数声明---------------------


//--------------------类----------------------


//------------------主函数----------------------
int main(int argc, char *argv[])
{
	// load source image
	Mat src_img_1 = imread("cola1.bmp", IMREAD_GRAYSCALE);
	CV_Assert(src_img_1.data != NULL);
	Mat src_img_2 = imread("cola2.bmp", IMREAD_GRAYSCALE);
	CV_Assert(src_img_2.data != NULL);
	

	Mat Temp2, Temp3;
	pyrDown(src_img_2, Temp2);
	pyrDown(Temp2, Temp3);
	


	Mat G1(src_img_1);
	Mat G2;
	Mat G3;
	Mat L1(G1.size(), CV_32F);
	Mat L2;
	Mat temp1, temp2;
	pyrDown(G1, G2);
	pyrDown(G2, G3);
	pyrUp(G2, temp1);
	pyrUp(G3, temp2);

	Mat G1_32F, temp1_32F;
	Mat G2_32F, temp2_32F;
	G1.convertTo(G1_32F, CV_32FC3, 1.0);
	temp1.convertTo(temp1_32F, CV_32FC3, 1.0);
	L1 = G1_32F - temp1_32F; 

	G2.convertTo(G2_32F, CV_32FC3, 1.0);
	temp2.convertTo(temp2_32F, CV_32FC3, 1.0);
	L2 = G2_32F - temp2_32F; 

	Mat g3_(G3.size(), CV_8UC1, Scalar(0));
	Mat g3_left = g3_.colRange(0, g3_.cols/2);
	Mat g3_right = g3_.colRange(g3_.cols/2, g3_.cols);
	Mat myLeft = G3.colRange(0, G3.cols/2);
	Mat myRight = Temp3.colRange(G3.cols/2, G3.cols);


//	g3_left = myLeft;
//	g3_right = myRight;

	myLeft.copyTo(g3_left);
	myRight.copyTo(g3_right);

	/*unsigned char *ptr_left = myLeft.ptr<uchar>();
	unsigned char *ptr_right = myRight.ptr<uchar>();
	unsigned char *pGL = g3_left.ptr<uchar>();
	unsigned char *pGR = g3_right.ptr<uchar>();
	for (int i = 0; i < myLeft.rows; i++)
	{
		memcpy(pGL, ptr_left, myLeft.cols);
		memcpy(pGR, ptr_right, myRight.cols);
		pGL += myLeft.cols;
		ptr_left += myLeft.cols;
		pGR += myRight.cols;
		ptr_right += myRight.cols;
	}*/
	
//	cout << g3_left.cols << " " << myLeft.cols << endl;
//	cout << g3_left.rows << " " << myLeft.rows << endl;
//	cout << g3_right.cols << " " << myRight.cols << endl;
//	cout << g3_right.rows << " " << myRight.rows << endl;


	Mat g2_;
	pyrUp(g3_, g2_);




	Mat _g2_32f;
	g2_.convertTo(_g2_32f, CV_32FC3, 1.0);
	Mat g2_total = _g2_32f + L2;

	Mat g2_total_8u;
	g2_total.convertTo(g2_total_8u, CV_8UC1, 1.0);

	
	Mat g1_;
	pyrUp(g2_total_8u, g1_);
	Mat _g1_32f;
	g1_.convertTo(_g1_32f, CV_32FC3, 1.0);
	Mat g1_total = _g1_32f + L1;
	Mat g1_total_8u;
	g1_total.convertTo(g1_total_8u, CV_8UC1, 1.0);

	imshow("G1", G1);
	imshow("g1_total", g1_total_8u);
	imshow("g3_", g3_);
	imshow("myleft", myLeft);
	imshow("myright", myRight);

	waitKey();
	destroyAllWindows();

}

