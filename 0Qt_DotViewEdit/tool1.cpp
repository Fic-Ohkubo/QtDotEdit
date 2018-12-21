#include "MyHead.h"
// tool function group1

using namespace cv;
using namespace std;

// color change function
cv::Mat ColorChange(const cv::Mat &src, int colorR,int colorG,int colorB )
{
	if (src.empty()) return src;
	int width = static_cast<int>(src.cols);
	int height= static_cast<int>(src.rows);

	cv::Mat dst(src.size(),CV_8UC3);
	// fill src as you prefer
	double r(0.0), g(0.0), b(0.0);
	r = static_cast<double>(colorR / 255.0);
	g = static_cast<double>(colorG / 255.0);
	b = static_cast<double>(colorB / 255.0);
	#pragma omp parallel
	{
	#pragma omp for
		for (int y = 0; y < height; y++) {
			cv::Vec3b *pixel = (cv::Vec3b*)src.ptr<cv::Vec3b>(y);
			cv::Vec3b *out = (cv::Vec3b*)dst.ptr<cv::Vec3b>(y);
			for (int x = 0; x < width; x++) {
				out[x][2] = saturate_cast<uchar>(r*pixel[x][2]);
				out[x][1] = saturate_cast<uchar>(g*pixel[x][1]);
				out[x][0] = saturate_cast<uchar>(b*pixel[x][0]);
			} // x-loop
		} //y-loop
	}
	return dst;
}


// �O���b�h�`��(�`��摜�C���[�W�ɃO���b�h������������)
cv::Mat drawGrid(const cv::Mat image, const int ngrid,const cv::Scalar gridcolor)
{
	if (ngrid == 0) return image;
	//cv::Mat mat_img = cv::Mat(image.size(),CV_8UC3);
	cv::Mat mat_img = image.clone(); // ���̉摜�C���[�W�͂��̂܂ܕێ�����
	int width = mat_img.size().width;
	int height = mat_img.size().height;
	int Xstep = width / ngrid;
	int Ystep = height / ngrid;
	for (int i = 0; i<ngrid; i++){
		int Ypos = i*Ystep;
		cv::line(mat_img, Point(0, Ypos), Point(width, Ypos),gridcolor,1,8,0);
	}
	for (int i = 0; i<ngrid; i++){
		int Xpos = i*Xstep;
		cv::line(mat_img, Point(Xpos, 0), Point(Xpos, height), gridcolor,1,8,0);
	}

	// draw final line
	cv::line(mat_img, Point(0, height), Point(width, height),gridcolor,1,8,0);
	cv::line(mat_img, Point(width, 0), Point(width, height), gridcolor,1,8,0);
	return mat_img;
}


// * 
// * Created on 2018/08/28, 16:46
// Reloading PID image from original 16bit data 
// (change extract range)
cv::Mat GetModifiedPIDimage(const BaseDataPID &basedata, int &retCode)
{
	// note: this module extract modified bright imaging of PID-data
	cv::Mat BaseImage = basedata.baseMat.clone(); // Caution: this is 1ch-16bit image
	int width = static_cast<int>(BaseImage.cols);
	int height = static_cast<int>(BaseImage.rows);
	cv::Mat dst = BlackPlateImage(width, height); //create empty image
	int Max = basedata.Max;
	int Min = basedata.Min;

	// range check (assertion)
	if(Min>=Max || Max>BrightMAX || Min < 0) {
		retCode = -1001; // set error code
		return basedata.baseImageMat; // not changed
	} 
	double range = static_cast<double>(Max - Min);
	double coef = 255.0 / range;
	long r(0);

	// create Modified PIDimage data
	for (int y = 0; y<height; y++){
		ushort *pixel = BaseImage.ptr<ushort>(y);
		cv::Vec3b *out = (cv::Vec3b*)dst.ptr<cv::Vec3b>(y);
		for (int x = 0; x<width; x++){
			r = static_cast<long>(pixel[x]);
			if (Min >0){ 
				r -= static_cast<long>(Min); 
			}
			if (r > 0 ){ // �s�N�Z���l���[���łȂ���Ώ㏑��
				double fr = static_cast<double>(r);
				r = static_cast<long>(fr*coef);
				out[x][2] = saturate_cast<uchar>(r);
				out[x][1] = saturate_cast<uchar>(r);
				out[x][0] = saturate_cast<uchar>(r);
			} //endif
		} // x-loop
	} //y-loop
	return dst;
}

// �w��ʒu��16bit PID ���x���ϑ� 
int ObserveIntensity(BaseDataPID &BrightSpotData, int x, int y)
{
	long obs(0);
	int w = static_cast<int>(BrightSpotData.baseMat.cols);
	int h = static_cast<int>(BrightSpotData.baseMat.rows);
	if(x<w && y<h){
		obs= static_cast<long>(BrightSpotData.baseMat.at<ushort>(y, x));
	}
	return static_cast<int>(obs);
}


// Read PID(Bright Spot) data as base image data
void ReadBaseData(BaseDataPID &mydata, QString qfilename)
{
	std::string filename = qfilename.toUtf8().constData();
	//std::string filename = qfilename.toLocal8Bit().constData(); //8bit ascii code 
	if (filename.empty())return;
	// load tiff and create Base image data
	cv::Mat baseMat;
	cv::Mat workMat;
	int width(0), height(0), bitDepth(0), channel(0);
	loadPidImage(filename, baseMat, workMat,
				 &width, &height, &channel, &bitDepth); //load tiff image as gray image
	cv::Mat showMat=PIDimage1chTo3ch(workMat);// 3channel �摜�ɐ؂�ւ���(�P�����ł͕\������Ȃ�)
	//cvtColor(showMat, showMat, CV_GRAY2BGR);  
	mydata.filename = getImagename(filename);
	mydata.baseMat = baseMat.clone(); // store original 16bit gray image to BaseMat
	mydata.baseImageMat = showMat;
	mydata.width = width;
	mydata.height = height;
	mydata.depth = bitDepth;
	mydata.channel = channel;
	//ckcvmat(mydata.currentMat);
	return;
}

// PID �ȊO�̉摜�ǂݍ��� ( color will be changed as gray scale automatically)
void ReadPlaneData(PlaneData &myplane, QString qfilename,
				   int &width, int &height, int &channel, int &depth)
{
	std::string filename = qfilename.toUtf8().constData();
	//std::string filename = qfilename.toLocal8Bit().constData(); //8bit ascii code 
	if (filename.empty())return;
	cv::Mat TempImage;
	cv::Mat gray;
	load8bitColorImage(filename, TempImage,width,  height,  channel,  depth);
		ckcvmat(TempImage);
	if (TempImage.empty()) return;
	//cv::cvtColor(TempImage, gray, CV_BGR2GRAY); // set gray scale
	//cv::cvtColor(gray, TempImage, CV_GRAY2BGR); // data format adjustment to color
	TempImage = mkGrayImage(TempImage);
	//myplane.image = addAlpha(TempImage);;
	myplane.image = TempImage;
	myplane.raw_image = TempImage.clone();
	myplane.use = true; // �������ɓǂݏI�������@TRUE�ɃZ�b�g����
	return;
}

// �N�����̕\���摜�f�[�^(crash ���)
void InitStartWindBaseData(ViewData &mydata, const QImage &qsrc)
{
	// initialize visualize data for invoking program
	mydata.currentMat = qimage2mat(qsrc);
	mydata.baseImageMat = mydata.currentMat.clone();
	int width = mydata.currentMat.cols;
	int height= mydata.currentMat.rows;
	mydata.PIDimage = BlackPlateImage(width, height);
	mydata.PlaneImage=BlackPlateImage(width, height);

	mydata.BrightScale = 1.0;
	mydata.scaleFactor = 1.0;
}


// CV::Mat ���� QPixmap�ւ̕ϊ�
QPixmap Mat2QPixmap(const cv::Mat &inMat)
{
	QImage workImage=Mat2QImage(inMat);
	//workImage.convertToFormat(QImage::Format_RGBA8888);
	return QPixmap::fromImage(workImage);
}


// Mat overwrite module (Calling sequence MatOverwrite(base, writingImage))
cv::Mat MatOverwrite(const cv::Mat mymat1, const cv::Mat mymat2)
{
	// note: �摜�P(mymat1)�͉摜�Q(mymat2)�Ɠ����T�C�Y or �傫���T�C�Y�łȂ��Ă͂Ȃ�Ȃ�
	// output size is equal to mymat1 size 
	int width = static_cast<int>(mymat1.cols);
	int height = static_cast<int>(mymat1.rows);

	int width2 = static_cast<int>(mymat2.cols);
	int height2 = static_cast<int>(mymat2.rows);

	cv::Mat dst = mymat1.clone();
	int ny = min(height, height2);  // crash �΍�
	int nx = min(width, width2);	// crash �΍�

	int r(0), g(0), b(0);
#pragma omp parallel
	{
#pragma omp for
		for (int y = 0; y < ny; y++) {
			if (y > height2)continue; // limitter(y)
			cv::Vec3b *pixel = (cv::Vec3b*)mymat2.ptr<cv::Vec3b>(y);
			cv::Vec3b *out = (cv::Vec3b*)dst.ptr<cv::Vec3b>(y);
#pragma omp critical
			{
				for (int x = 0; x < nx; x++) {
					if (x > width2)continue; // limitter(x)
					r = static_cast<int>(pixel[x][2]);
					g = static_cast<int>(pixel[x][1]);
					b = static_cast<int>(pixel[x][0]);
					if (r > 0 || g > 0 || b > 0) { // �s�N�Z���l���[���łȂ���Ώ㏑��
						out[x][2] = saturate_cast<uchar>(r);
						out[x][1] = saturate_cast<uchar>(g);
						out[x][0] = saturate_cast<uchar>(b);
					} //endif
				} // x-loop
			} // omp critical area
		} //y-loop
	} //omp parallel area
	return dst;
}


// Create Black Image Plate (8bit 3channel)
cv::Mat BlackPlateImage(int width, int height)
{
	cv::Mat dst = Mat(height, width, CV_8UC3);
#pragma omp parallel
	{
#pragma omp for
		for (int y = 0; y < height; y++) {
				cv::Vec3b *out = (cv::Vec3b*)dst.ptr<cv::Vec3b>(y); // set line position
				for (int x = 0; x < width; x++) {
					out[x][2] = static_cast<uchar>(0);
					out[x][1] = static_cast<uchar>(0);
					out[x][0] = static_cast<uchar>(0);
				} // x-loop
		} //y-loop
	} // end of parallel area
	return dst;
}

// Create Black Image Plate (16bit 1-channel version for PID data)
cv::Mat BlackPlateImage16(int width, int height)
{
	cv::Mat dst = Mat(height, width, CV_16UC1);
	for (int y = 0; y<height; y++){
		ushort *out = dst.ptr<ushort>(y); // set line position
		for (int x = 0; x<width; x++){
			(*out++) = static_cast<ushort>(0);
		} // x-loop
	} //y-loop
	return dst;
}

cv::Mat BlackColorImage16(int width, int height)
{
		cv::Mat dst = Mat(height, width, CV_16UC3);
	for (int y = 0; y<height; y++){
		ushort *out = dst.ptr<ushort>(y); // set line position
		for (int x = 0; x<width; x++){
			(*out++) = static_cast<ushort>(0); //B
			(*out++) = static_cast<ushort>(0); //G
			(*out++) = static_cast<ushort>(0); //R
		} // x-loop
	} //y-loop
	return dst;
}


// change Image intensity 
cv::Mat MatBrightScaling(const cv::Mat src,const double BrightScale)
{
	cv::Mat dst = src.clone();
	int height = static_cast<int>(src.rows);
	int width  = static_cast<int>(src.cols);
	double intensityscale = BrightScale;
	if (BrightScale < 0.0) intensityscale = 1.0; // check scaling (scale error collection)
	int r(0), g(0), b(0);

	// bright value change (���񏈗��t��)
	#pragma omp parallel for
	for (int y = 0; y<height; y++){
		#pragma omp critical
		{
			cv::Vec3b *out = (cv::Vec3b*)dst.ptr<cv::Vec3b>(y); // set line position
			for (int x = 0; x < width; x++) {
				r = saturate_cast<int>(out[x][2] * intensityscale); //r = limit255i(r);
				g = saturate_cast<int>(out[x][1] * intensityscale); //g = limit255i(g);
				b = saturate_cast<int>(out[x][0] * intensityscale); //b = limit255i(b);
				out[x][2] = saturate_cast<uchar>(r);
				out[x][1] = saturate_cast<uchar>(g);
				out[x][0] = saturate_cast<uchar>(b);
			} // x-loop
		} // omp critical area
	} //y-loop
	return dst;
}


// PID data load module by libtiff (read Bright Spot image (known as BlackField))
// note: ���݂̂Ƃ���Atiff �摜�̃s�N�Z���l��
//		�m���Ɏ擾�ł���̂� Libtiff�����Ȃ��̂ł�����g��
void loadPidImage(std::string &filename, 
				  cv::Mat &PIDimage, cv::Mat &visiblePID,
				  int *imageWidth, int *imageHeight, int *imageChannel, int *imageDepth)
{
	int type = CV_16UC4; //default type RGB+alpha 
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wfilename = converter.from_bytes(filename);
	TIFF *image = TIFFOpenW(wfilename.c_str(), "r"); // �ǂݍ��݃��[�h�ŃI�[�v��
	uint32 W(0), H(0);
	uint16 Bands(0),imagebit(0);
	tdata_t buf;

	// obtain Tiff information
	TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &W);
	TIFFGetField(image, TIFFTAG_IMAGELENGTH, &H);
	TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &Bands);// �F�̐��ɑΉ��i�ꉞ�}���`�y�[�W�ł��\�j	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
	TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &imagebit);
	//qDebug("Tiff info %d %d ch:%d bit:%d", W, H, Bands, imagebit);
	int channel = static_cast<int>(Bands);
	if (channel == 1) type = CV_16UC1; // �O���[�X�P�[��Colorimetric 
	if (channel == 3) type = CV_16UC3; // �R�`�����l���i�J���[�j
	if (channel != 3 ) return; // ���݂̓J���[�̂ݑΉ�
	// image info return
	int width = static_cast<int>(W);
	int height = static_cast<int>(H);
	*imageWidth = width;
	*imageHeight = height;
	*imageChannel = channel;
	*imageDepth = imagebit;

	//�Ƃ肠�����㗬���W���[���ō���Ȃ��悤�ɋ�̉摜����
	PIDimage = cv::Mat::zeros(height, width, CV_16UC1);
	visiblePID=cv::Mat::zeros(height, width, CV_8UC1);//1ch grayscale image

	cv::Mat gray16(height, width, CV_16UC1); // base�f�[�^�pcv�摜(�ǂݍ��ݎ���Ɨp)
	cv::Mat cvshow(height, width, CV_8UC1);	 // �����p�̃f�[�^(�ǂݍ��ݎ���Ɨp)
		// �ϊ��W���ƍŏ��̋��x�X�P�[�����O
	double c1 = 255.0;
	//double c2 = 65535.0;
	double c12 = 4095.0;
	//double	c2 = pow(2.0, 12.0) - 1.0;// �摜���ޯĐ�
	//		 �ϊ��W��(��:12bit��8bit�̂Ƃ���255/4095=0.062)

	//double coef = c1 / c2; //�ϊ��W��
	double coef2 = c1 / c12;

	// color CASE
	if (channel == 3){
		buf = _TIFFmalloc(TIFFScanlineSize(image));
		for (int e =height-1; e != -1; e--) {
			TIFFReadScanline(image, buf, e);
			ushort *pdest = gray16.ptr<ushort>(e);
			for (int c = 0; c < width; c++) {
				uint16 red = static_cast<uint16*>(buf)[c * 3 + 0];
				(*pdest++) = saturate_cast<ushort>(red);
				// note: static_cast<uint16*>(buf)[c * 3 + 1]; // green
				// note: static_cast<uint16*>(buf)[c * 3 + 2]; // blue
			}//col
		}//row
		_TIFFfree(buf); // �ǂݏI������炷���ɔj��
		gray16.convertTo(cvshow, CV_8UC1, coef2); // visible greyscale image

		// pickup noise level for visualize
		for (int e = 0; e <height; e++){
			ushort *psrc = gray16.ptr<ushort>(e);
			uchar  *pdst = cvshow.ptr<uchar>(e);
			for (int c = 0; c < width; c++)	{
				int red = (int)(*psrc++);
				if (red>0 && red<255){ //pickup noise
					pdst[c] = static_cast<uchar>(1); // set visible noise as "1"
				}
			}//col
		}//row
	}// endif color case
	TIFFClose(image);
	PIDimage = gray16.clone();
	visiblePID = cvshow.clone();
	return ; //�ǂ܂�Ȃ��ꍇ�h��f�[�^�h���A��
}


//  data load module by libtiff (read 8bit flat image (known as BlackField))
// �w�i�摜�̓ǂݍ���(������)
void load8bitColorImage(std::string &filename, cv::Mat &viewImage,
						int &width, int &height, int &channel, int &depth)
{
	//int type = CV_16UC4; //default type RGB+alpha 
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wfilename = converter.from_bytes(filename);
	TIFF *image = TIFFOpenW(wfilename.c_str(), "r"); // �ǂݍ��݃��[�h�ŃI�[�v��
	uint32 W(0), H(0);
	uint16 Bands(0), imagebit(0);
	TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &W);

	TIFFGetField(image, TIFFTAG_IMAGELENGTH, &H);
	TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &Bands);// �F�̐��ɑΉ��i�ꉞ�}���`�y�[�W�ł��\�j	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
	TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &imagebit);
	//qDebug("Tiff info %d %d ch:%d bit:%d", W, H, Bands, imagebit);
	channel = static_cast<int>(Bands);
	width	= static_cast<int>(W);
	height	= static_cast<int>(H);
	depth	= static_cast<int>(imagebit);

	//�Ƃ肠�����㗬���W���[���ō���Ȃ��悤�ɋ�̉摜����
	//viewImage = BlackPlateImage(height, width);// ��̉摜
	viewImage = BlackPlateImage(width, height);// ��̉摜
	if (channel != 3) return; // ���݂̓J���[�̂ݑΉ�
	// color CASE
	if (channel == 3){ // �O�̂��߁A�`�����l���`�F�b�N
		tdata_t buf = _TIFFmalloc(TIFFScanlineSize(image));
		if (depth == 8) { // 8bit case
			for (int e = 0; e != height; e++) {
				//cv::Vec3b *pdest = viewImage.ptr<cv::Vec3b>(e);
				TIFFReadScanline(image, buf, e);
				//unsigned char *data = viewImage.ptr(e);
				cv::Vec3b *data = (cv::Vec3b*)viewImage.ptr<cv::Vec3b>(e);
				for (int c = 0; c < width; c++) {
					uint8 red = static_cast<uint8*>(buf)[c * 3 + 0];
					uint8 green = static_cast<uint8*>(buf)[c * 3 + 1];
					uint8 blue = static_cast<uint8*>(buf)[c * 3 + 2];
					data[c][0] = saturate_cast<uchar>(blue);
					data[c][1] = saturate_cast<uchar>(green);
					data[c][2] = saturate_cast<uchar>(red);
				}//col
			}//row
		} // 8bit case end
		if (depth == 16) { // 16bit case (caution: 16bit mode is slow-speed)
			cv::Mat tmp16view = BlackColorImage16(width, height);
			for (int e = 0; e != height; e++) {
				//cv::Vec3b *pdest = viewImage.ptr<cv::Vec3b>(e);
				TIFFReadScanline(image, buf, e);
				cv::Vec3w *data = (cv::Vec3w*)tmp16view.ptr<cv::Vec3w>(e);
				//uint16_t* pixelPtr = (uint16_t*)tmp16view.data;
				for (int c = 0; c < width; c++) {
					uint16 red	= static_cast<uint16*>(buf)[c * 3 + 0];
					uint16 green= static_cast<uint16*>(buf)[c * 3 + 1];
					uint16 blue = static_cast<uint16*>(buf)[c * 3 + 2];
					data[c][0] = saturate_cast<uint16>(blue);
					data[c][1] = saturate_cast<uint16>(green);
					data[c][2] = saturate_cast<uint16>(red);
				}//col
			}//row
			double coef =  255.0 / 4095.0; // BrightField conversion
			tmp16view.convertTo(viewImage, CV_8UC3, coef); // visible color image
		} // 16bit case end
		//
		_TIFFfree(buf); // �ǂݏI������炷���ɔj��
	}
	TIFFClose(image);
	return; //�ǂ܂�Ȃ��ꍇ�h��f�[�^�h���A��
}

// �t�@�C��������t�H���_�p�X����������
std::string getImagename(const std::string &filename)
{
	return{ std::find_if(filename.rbegin(), filename.rend(),
				[](char c) { return c == '/'; }).base(),
				filename.end() };
}


//�t�@�C���^�C�v���f�o�b�O���b�Z�[�W�Ƃ��ďo��
void ckcvmat(const cv::Mat &inMat)
{
	switch (inMat.type()){
	case CV_8UC4:
	{
		qDebug("CV_8UC4");
		break;
	}
	case CV_8UC3:
	{
		qDebug("CV_8UC3");
		break;
	}
	case CV_8UC1:
	{
		qDebug("CV_8UC1");
		break;
	}
	default:
		qDebug("Other unknown");
		break;
	}
	return;
}


// Utility module (�`�����l������)
// channel splitter module (extract PID grayscale channel)
cv::Mat split16bitPlane(cv::Mat cvdata)
{
	cv::Mat out;
	switch (cvdata.type()){
		// 16-bit, 1 channel
	case CV_16UC1:
	{
		return cvdata.clone(); // ���̂܂ܓ��͉摜��Ԃ�
	}
	case CV_16UC4: //BGRA 4channel
	{
		cv::Mat plane[4];
		cv::split(cvdata, plane);
		plane[2].copyTo(out);
		return out.clone();
	}
	case CV_16UC3: //BGR 3channel
	{
		cv::Mat plane[3];
		cv::split(cvdata, plane);
		plane[2].copyTo(out);
		return out.clone();
	}
	case CV_16UC2: // GA 2channel (grayscale with alpha channel)
	{ // caution: this is rare case
		cv::Mat plane[2];
		cv::split(cvdata, plane);
		plane[0].copyTo(out); // 
		return out.clone();
	}
	default: // fail case (not 16 bit or multi-band data)
		break;
	}
	return out; //may be empty case
}


cv::Mat qimage2mat(const QImage& image)
{
	cv::Mat out; //destination
	switch (image.format()) {
	case QImage::Format_Invalid:
	{
		Mat empty;
		empty.copyTo(out);
		break;
	}
	case QImage::Format_RGB32:
	{
		Mat view(image.height(), image.width(), CV_8UC4, (void *)image.constBits(), image.bytesPerLine());
		view.copyTo(out);
		break;
	}
	case QImage::Format_RGB888:
	{
		Mat view(image.height(), image.width(), CV_8UC3, (void *)image.constBits(), image.bytesPerLine());
		cvtColor(view, out, COLOR_RGB2BGR);
		break;
	}
	default:
		QImage conv = image.convertToFormat(QImage::Format_ARGB32);
		Mat view(conv.height(), conv.width(), CV_8UC4, (void *)conv.constBits(), conv.bytesPerLine());
		view.copyTo(out);
		break;
	}
	return out;
}

// �v��Ȃ��H
long limit255l(long val)
{
	long limit(255);
	if(val>limit){
		return limit;
	} else {
		return val;
	}
}

// �v��Ȃ��H
int limit255i(int val)
{
	int limit(255);
	if(val>limit){
		return limit;
	} else {
		return val;
	}
}


// QImage �� cv::Mat �̕ϊ�(���ڌĂԂ��Ƃ͗]�薳��)
// ���RGB888�ŏo��
QImage Mat2QImage(const cv::Mat &inMat)
{
	switch (inMat.type()){
	case CV_8UC4:// 8-bit, 4 channel
	{
		QImage image(inMat.data,
					 inMat.cols, inMat.rows,
					 static_cast<int>(inMat.step),
					 QImage::Format_ARGB32);
		image.convertToFormat(QImage::Format_RGB888);
		return image;
	}
	case CV_8UC3:// 8-bit, 3 channel
	{
		QImage image(inMat.data,
					 inMat.cols, inMat.rows,
					 static_cast<int>(inMat.step),
					 QImage::Format_RGB888);
		return image.rgbSwapped();
	}
	case CV_8UC1:// 8-bit, 1 channel
	{
		// From Qt 5.5
		QImage image(inMat.data, inMat.cols, inMat.rows,
					 static_cast<int>(inMat.step),
					 QImage::Format_Grayscale8);
		image.convertToFormat(QImage::Format_RGB888); // �������Ȃ���visible�ɂȂ�Ȃ�
	}
	default:
		//qWarning() << "cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
		break;
	}
	return QImage();
}
