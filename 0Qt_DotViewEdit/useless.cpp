#include "MyHead.h"
// junk code group

using namespace cv;
using namespace std;


// crosshair-line (testing) (note: very slow. dont use)
void MyDialog::drawcross(int x, int y)
{
	QPixmap currentpix = Mat2QPixmap(MyDialog::ViewDataLeft.currentMat);
	//QPixmap currentpix = Mat2QPixmap(viewimage);
	int W = MyDialog::ViewDataLeft.currentMat.cols;
	int H = MyDialog::ViewDataLeft.currentMat.rows;
	
	QPixmap result(W, H);
	//result.fill(); // force alpha channel 
	QPainter paintOver(&result);

	QLine line1=QLine(x,0,x,H);
	QLine line2=QLine(0,y,W,y);
	paintOver.setPen(Qt::lightGray);
	result.fill(Qt::transparent); // force alpha channel 
	paintOver.drawPixmap(0, 0, currentpix);
	paintOver.drawLine(line1);
	paintOver.drawLine(line2);
    
	this->ui->imgLabel->setGeometry(QRect(0, 0, W, H));
	//this->ui->imgLabel->lower();
	this->ui->imgLabel->setPixmap(result);
	//this->ui->imgLabel->setPixmap(result.scaled(W,H,Qt::KeepAspectRatio));
	// reset scroolArea property
	this->ui->scrollArea->setWidget(this->ui->imgLabel);
	return;
}




// alpha blend
cv::Mat OverlayAlpha(const cv::Mat &rgba, const cv::Mat &base) // stacking with alpha channel
{
	// note: rgba =CV_8UC4
	//       base =CV_8UC3
	//       return=CV_8UC3
	//		base.size() = rgba.size()
	if (rgba.empty()) return base;
	if (base.empty()) return base;

	// ** channel extraction 
	cv::Mat Bimage(rgba.size(), CV_8UC1);
	cv::Mat Gimage(rgba.size(), CV_8UC1);
	cv::Mat Rimage(rgba.size(), CV_8UC1);
	cv::Mat Alpha(rgba.size(), CV_8UC1);
	splitBGRA(rgba,  Bimage, Gimage, Rimage, Alpha); //channel split

	cv::Mat dst = base.clone();
	double alphaVal(0.0), betaVal(0.0);

	int width = min(base.cols, rgba.cols);  // crash 対策
	int height= min(base.rows, rgba.rows);	// crash 対策
	// Normalize the alpha mask to keep intensity between 0 and 1
    Alpha.convertTo(Alpha, CV_32FC3, 1.0/255); // 

	// conversion by  pixel access
	for(int y=0;y<height;y++){
		// set position RGBA
		uchar *Bpixel = (uchar*)Bimage.ptr<uchar>(y);
		uchar *Gpixel = (uchar*)Gimage.ptr<uchar>(y);
		uchar *Rpixel = (uchar*)Rimage.ptr<uchar>(y);
		float *alpha = (float*)Alpha.ptr<float>(y);
		// set base and destination point
		cv::Vec3b *basepixel = (cv::Vec3b*)base.ptr<cv::Vec3b>(y);
		cv::Vec3b *out = (cv::Vec3b*)dst.ptr<cv::Vec3b>(y);
		for(int x=0;x<width;x++){
			alphaVal = (*alpha++);
			betaVal = 1.0 - alphaVal;
			if (Bpixel[x] > 0 || Gpixel[x]>0 || Rpixel[x] > 0) {
				out[x][0] = saturate_cast<uchar>(Bpixel[x] + betaVal*basepixel[x][0]);
				out[x][1] = saturate_cast<uchar>(Gpixel[x] + betaVal*basepixel[x][1]);
				out[x][2] = saturate_cast<uchar>(Rpixel[x] + betaVal*basepixel[x][2]);
			}
		} // x-loop
	} //y-loop
	return dst;
}

//4チャンネル版　チャネルスプリッター
void splitBGRA(const cv::Mat &src, 
			   cv::Mat &Bimage, cv::Mat &Gimage,cv::Mat &Rimage, cv::Mat &Alpha)
{
	// note: src=CV_8UC4 
	//		 &Bimage,  &Gimage, &Rimage, &Alpha= CV_8UC1 
	if (src.empty()) return;
	cv::Mat plane[4];
	cv::split(src, plane);
	plane[0].copyTo(Bimage);
	plane[1].copyTo(Gimage);
	plane[2].copyTo(Rimage);
	plane[3].copyTo(Alpha);
	return;
}


// アルファチャネル付加
cv::Mat addAlpha(const cv::Mat &src)
{
	if (src.empty()) return src;
	//int from_to[] = { 0,0, 1,1, 2,2, 3,3 };
	// fill src as you prefer
	//std::vector<cv::Mat> channels;
	//cv::split(src, channels); // put each channel in a mat in the vector
	// swap or add channels in the vector
	//cv::Mat alpha(src.rows, src.cols, CV_8U, cv::Scalar(255));
	//channels.push_back(alpha);
	//std::reverse(channels.begin(), channels.end()); //needs <algorithm>
	// merge the channels in one new image
	//cv::merge(channels, final_image); 


	cv::Mat dst(src.size(), CV_8UC4);
	int width = src.cols; // crash 対策
	int height= src.rows;	// crash 対策
	
	// conversion by  pixel access
	for(int y=0;y<height;y++){
		// set position RGBA

		// set base and destination point
		cv::Vec3b *basepixel = (cv::Vec3b*)src.ptr<cv::Vec3b>(y);
		cv::Vec4b *out = (cv::Vec4b*)dst.ptr<cv::Vec4b>(y);
		for(int x=0;x<width;x++){
			int alpha = 0;
			int max = pickMAX(basepixel[x][0] ,basepixel[x][1] , basepixel[x][2]);
			if (max > 0) alpha = 255;
			out[x][0] = saturate_cast<uchar>(basepixel[x][0]); //B
			out[x][1] = saturate_cast<uchar>(basepixel[x][1]); //G
			out[x][2] = saturate_cast<uchar>(basepixel[x][2]); //R
			out[x][3] = saturate_cast<uchar>(alpha);
		} // x-loop
	} //y-loop
	return dst;
}
