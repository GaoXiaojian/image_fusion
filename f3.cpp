#include "opencv2/opencv.hpp"
#include <iostream>

using namespace cv;
using namespace std;
/************************************************************************/
/* 说明：
*金字塔从下到上依次为 [0,1，...，level-1] 层
*blendMask 为图像的掩模
*maskGaussianPyramid为金字塔每一层的掩模
*resultLapPyr 存放每层金字塔中直接用左右两图Laplacian变换拼成的图像
*/
/************************************************************************/


class LaplacianBlending {
private:
	Mat_<Vec3f> left;
	Mat_<Vec3f> right;
	Mat_<float> blendMask;

	vector<Mat_<Vec3f> > leftLapPyr,rightLapPyr,resultLapPyr;//Laplacian Pyramids
	Mat leftHighestLevel, rightHighestLevel, resultHighestLevel;
	vector<Mat_<Vec3f> > maskGaussianPyramid; //masks are 3-channels for easier multiplication with RGB

	int levels;

	void buildPyramids() {
		buildLaplacianPyramid(left,leftLapPyr,leftHighestLevel);
		buildLaplacianPyramid(right,rightLapPyr,rightHighestLevel);
	//	buildGaussianPyramid();
	}

	void buildGaussianPyramid() {//金字塔内容为每一层的掩模
		assert(leftLapPyr.size()>0);

		maskGaussianPyramid.clear();
		Mat currentImg;
		cvtColor(blendMask, currentImg, CV_GRAY2BGR); //store color img of blend mask into maskGaussianPyramid
		maskGaussianPyramid.push_back(currentImg); //0-level

		currentImg = blendMask;
		for (int i=1; i<levels+1; i++) {
			Mat _down;
			if (leftLapPyr.size() > i)
				pyrDown(currentImg, _down, leftLapPyr[i].size());
			else
				pyrDown(currentImg, _down, leftHighestLevel.size()); //lowest level

			Mat down;
			cvtColor(_down, down, CV_GRAY2BGR);
			maskGaussianPyramid.push_back(down);//add color blend mask into mask Pyramid
			currentImg = _down;
		}
	}

	void buildLaplacianPyramid(const Mat& img, vector<Mat_<Vec3f> >& lapPyr, Mat& HighestLevel) {
		lapPyr.clear();
		Mat currentImg = img;
		for (int l=0; l<levels; l++) {
			Mat down,up;
			pyrDown(currentImg, down);
			pyrUp(down, up,currentImg.size());
			Mat lap = currentImg - up;
			lapPyr.push_back(lap);
			currentImg = down;
		}
		currentImg.copyTo(HighestLevel);
	}

	Mat_<Vec3f> reconstructImgFromLapPyramid() {
		//将左右laplacian图像拼成的resultLapPyr金字塔中每一层
		//从上到下插值放大并相加，即得blend图像结果
		Mat currentImg = resultHighestLevel;
		for (int i=levels-1; i>=0; i--) {
			Mat up;

			pyrUp(currentImg, up, resultLapPyr[i].size());
			currentImg = up + resultLapPyr[i];
		}
		return currentImg;
	}

	void blendLapPyrs() {
#if 0
		//获得每层金字塔中直接用左右两图Laplacian变换拼成的图像resultLapPyr
		// A.mul(B) 为对应位置元素相乘
		resultHighestLevel = leftHighestLevel.mul(maskGaussianPyramid.back()) +
			rightHighestLevel.mul(Scalar(1.0,1.0,1.0) - maskGaussianPyramid.back()); // 选取左边一半，右边一半
		for (int i=0; i<levels; i++) {
			Mat A = leftLapPyr[i].mul(maskGaussianPyramid[i]);
			Mat antiMask = Scalar(1.0,1.0,1.0) - maskGaussianPyramid[i];
			Mat B = rightLapPyr[i].mul(antiMask);
			Mat_<Vec3f> blendedLevel = A + B;

			resultLapPyr.push_back(blendedLevel);
		}
#else
		// 缩小图取平均
		resultHighestLevel = leftHighestLevel/2 + rightHighestLevel/2;
		for (int i = 0; i < levels; i++)
		{
			Mat A = (leftLapPyr[i] > rightLapPyr[i])/255;
		//	cout << A.channels() << endl;
		//	cout << A.depth() << endl;
			Mat B;
			A.convertTo(B, CV_32FC3);
		//	cout << A;
		//	cvtColor(A, A, CV_GRAY2BGR);
		//	Mat C = leftLapPyr[0].mul(B); //+ rightLapPyr[0].mul(Scalar(1,1,1)-B);
			Mat_<Vec3f> blendedLevel = leftLapPyr[i].mul(B) + rightLapPyr[i].mul(Scalar(1,1,1)-B);  //取大混合
			resultLapPyr.push_back(blendedLevel);
		}
			
		
#endif
	}

public:
	LaplacianBlending(const Mat_<Vec3f>& _left, const Mat_<Vec3f>& _right, const Mat_<float>& _blendMask, int _levels)://construct function, used in LaplacianBlending lb(l,r,m,4);
	  left(_left),right(_right),blendMask(_blendMask),levels(_levels)
	  {
		  assert(_left.size() == _right.size());
		  assert(_left.size() == _blendMask.size());
		  buildPyramids();	//construct Laplacian Pyramid and Gaussian Pyramid
		  blendLapPyrs();	//blend left & right Pyramids into one Pyramid
	  };

	  Mat_<Vec3f> blend() {
		  return reconstructImgFromLapPyramid();//reconstruct Image from Laplacian Pyramid
	  }
};

Mat_<Vec3f> LaplacianBlend(const Mat_<Vec3f>& l, const Mat_<Vec3f>& r, const Mat_<float>& m) {
	LaplacianBlending lb(l,r,m,4);
	return lb.blend();
}

int main() {

#if 0
	Mat M(3,3,CV_32F);
	Mat_<float>& M1 = (Mat_<float>&) M;
	M1(0,0) = 1.f;
	cout << M1;
/*
	Mat M(10000, 1000, CV_8UC3, Scalar(0,255,0));

	double duration;
	duration = static_cast<double>(getTickCount());

	Mat_<Vec3b> M1 = M;
	for (auto it = M1.begin(); it != M1.end(); it++)
	{
		(*it)[0] = 250;
	}
	
/ *
	unsigned char *ptr = M.data;
	int ranges = M.cols * M.rows;
	for (int i = 0; i < ranges; i++)
	{
		ptr[0] = 250;
		ptr+=3;
	}* /
	duration = static_cast<double>(getTickCount()) - duration;
	duration /= getTickFrequency();
	cout << "Last Time:" << duration << "ms" << endl;*/


#else

	Mat l8u = imread("cola1.bmp");
	Mat r8u = imread("cola2.bmp");

	imshow("left",l8u); 
	imshow("right",r8u);

	Mat_<Vec3f> l; l8u.convertTo(l,CV_32F,1.0/255.0);//Vec3f表示有三个通道，即 l[row][column][depth]
	Mat_<Vec3f> r; r8u.convertTo(r,CV_32F,1.0/255.0);
	/*****************    void convertTo( OutputArray m, int rtype, double alpha=1, double beta=0 ) const;******************/
	/* Performs linear transformation on every source array element:
	dst(x,y,c) = scale*src(x,y,alpha)+beta.
	Arbitrary combination of input and output array depths are allowed
	(number of channels must be the same), thus the function can be used
	for type conversion */

	//create blend mask matrix m
	Mat_<float> m(l.rows,l.cols,0.0);					// 将m全部赋值为0
	m(Range::all(),Range(0,m.cols/2)) = 1.0;	// 取m全部行&[0,m.cols/2]列，赋值为1.0

	LaplacianBlending A(l,r,m, 2);
	Mat_<Vec3f> blend = A.blend();
//	Mat_<Vec3f> blend = LaplacianBlend(l, r, m);
	imshow("blended",blend);

	waitKey(0);
	destroyAllWindows();

#endif

	return 0;
}