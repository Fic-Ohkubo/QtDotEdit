#pragma once
// include file for utility tools (original function group)
#include <iostream>
#include <ctime>
#include <time.h>
#include <codecvt>
#include <fstream>
#include "basedata.h"
#include "MyHead.h"


// ** DotXY correct
// Extraction XY and point mapping
void ExtractOriginXY(BspotMap &bsdata);
void getDotSearchRange(int x,int y,BspotMap &bsdata,DotRegion &DotRange);
bool isEmptyPixel(int x, int y, const cv::Mat map);


int loadDotTiff(std::string &filename, cv::Mat &dotImage);// (�P�_�ʒu���) �|�C���g�f�[�^(tif 8bit3channel �̓ǂݍ���)
cv::Mat GetModifiedPIDimage(const BaseDataPID &basedata, int &retCode);// change extract range of PIDdata 
int ObserveIntensity(BaseDataPID &BrightSpotData, int x, int y); // observe intensity from PIDimage data
cv::Mat drawGrid(cv::Mat image, const int ngrid,const cv::Scalar gridcolor); // gridLine draw
cv::Mat ColorChange(const cv::Mat &src, int colorR, int colorG, int colorB);// color change function
cv::Mat mkGrayImage(const cv::Mat &src);
cv::Mat mkInitialDotImage(const cv::Mat &src);


// �摜�f�[�^�ǂݍ���
void ReadBaseData(BaseDataPID &mydata, QString qfilename);//PID image data
void ReadPlaneData(PlaneData &myplane, QString qfilename,
				   int &width, int &height, int &channel, int &depth);//BackGround Image data

cv::Mat MatOverwrite(const cv::Mat mymat1, const cv::Mat mymat2);// Mat overwrite module
cv::Mat BlackPlateImage(int width, int height); // create black image plate
cv::Mat BlackPlateImage16(int width, int height); // create black image plate
cv::Mat BlackColorImage16(int width, int height);// 16bit 3channel
cv::Mat MatBrightScaling(const cv::Mat src,const double BrightScale);

// Qt��OpenCV�̉摜�ϊ�
cv::Mat qimage2mat(const QImage& image);
QImage Mat2QImage(const cv::Mat &inMat);
QPixmap Mat2QPixmap(const cv::Mat &inMat);

void updateBaseDataByCV(BaseDataPID &mydata, const cv::Mat &src);
cv::Mat split16bitPlane(cv::Mat cvdata);   //�`���l������(�O���[�X�P�[���摜���o��)
void InitStartWindBaseData(ViewData &mydata, const QImage &qsrc); // �����グ����window�Z�b�g�A�b�v

// PID image read(Bright Spot data load) utilize LibTiff
void loadPidImage(std::string &filename,
				  cv::Mat &PIDimage, cv::Mat &visiblePID,
				  int *width, int *height, int *channel, int *depth); // image inform

// �w�i�摜�̓ǂݍ���(Libtiff ���g����������)
void load8bitColorImage(std::string &filename, cv::Mat &viewImage,
						int &width, int &height, int &channel, int &depth);

// �t�@�C��������t�H���_�p�X����������
std::string getImagename(const std::string &filename);

// info function(check cv::Mat data format type)
void ckcvmat(const cv::Mat &inMat);

long limit255l(long val); // limitter1
int limit255i(int val); // limitter2

void splitBGRA(const cv::Mat &src,
			   cv::Mat &Bimage, cv::Mat &Gimage, cv::Mat &Rimage, cv::Mat &Alpha);

int pickMAX(int a, int b, int c);

//** useless code
cv::Mat addAlpha(const cv::Mat &src); // utilize reverse() and cv::merge (need <algorithm> )
cv::Mat OverlayAlpha(const cv::Mat &rgba, const cv::Mat &base); // stacking with alpha channel

// �P�F���֐�(PID�摜)
cv::Mat PIDimage1chTo3ch(const cv::Mat &src);

// Mat overwrite module (Calling sequence MatOverwrite(base, writingImage))
// DotImageMat=CV_8UC1 (gray scale)
cv::Mat DrawDotImage(const cv::Mat basemat, const cv::Mat DotImageMat,
					const int r, const int g, const int b);

