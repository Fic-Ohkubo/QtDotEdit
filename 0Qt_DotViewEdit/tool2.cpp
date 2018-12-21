#include "MyHead.h"
// tool function group2

using namespace cv;
using namespace std;



// Mat overwrite module (Calling sequence MatOverwrite(base, writingImage))
// DotImageMat=CV_8UC1 (gray scale)
cv::Mat DrawDotImage(const cv::Mat basemat, const cv::Mat DotImageMat,
					const int r, const int g, const int b)
{
	// note: 画像１(mymat1)は画像２(mymat2)と同じサイズ or 大きいサイズでなくてはならない
	// output size is equal to mymat1 size 
	int width = static_cast<int>(basemat.cols);
	int height = static_cast<int>(basemat.rows);

	int width2 = static_cast<int>(DotImageMat.cols);
	int height2 = static_cast<int>(DotImageMat.rows);

	cv::Mat dst = basemat.clone();
	int ny = min(height, height2);  // crash 対策
	int nx = min(width, width2);	// crash 対策

	int pixel(0);
	#pragma omp parallel
	{
		#pragma omp for
		for(int y = 0; y < ny; y++) {
			//cv::Vec3b *pixel = (cv::Vec3b*)mymat2.ptr<cv::Vec3b>(y);
			uchar *pix = (uchar*)DotImageMat.ptr<uchar>(y);
			cv::Vec3b *out = (cv::Vec3b*)dst.ptr<cv::Vec3b>(y);
			#pragma omp critical
			{
				for(int x = 0; x < nx; x++) {
					pixel = (*pix++);
					if(pixel > 0) { // ピクセル値がゼロでなければ指定色で上書き
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

//  data load module by libtiff (read 8bit flat image (known as BlackField))
// (輝点位置情報) ポイントデータ(tif 8bit3channel の読み込み)
int loadDotTiff(std::string &filename, cv::Mat &dotImage)
{
	// note: 輝点位置画像は 8bit 3channel 画像のみ許容　（後は不可）
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wfilename = converter.from_bytes(filename);
	TIFF *image = TIFFOpenW(wfilename.c_str(), "r"); // 読み込みモードでオープン
	uint32 W(0), H(0);
	uint16 Bands(0), imagebit(0);
	TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &W);

	TIFFGetField(image, TIFFTAG_IMAGELENGTH, &H);
	TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &Bands);// 色の数に対応（一応マルチページでも可能）	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
	TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &imagebit);
	//qDebug("Tiff info %d %d ch:%d bit:%d", W, H, Bands, imagebit);

	int channel = static_cast<int>(Bands);
	int width	= static_cast<int>(W);
	int height	= static_cast<int>(H);
	int depth	= static_cast<int>(imagebit);

	// *** check Data for confirm processing
	if(channel !=3){
		return DOTC_ERROR;
	}
	if(depth!=8){
		return DOTD_ERROR;
	}

	dotImage = cv::Mat(height, width, CV_8UC3);
	// color CASE
	tdata_t buf = _TIFFmalloc(TIFFScanlineSize(image));
	if (depth == 8) { // 8bit case
		for (int e = 0; e < height; e++) {
			TIFFReadScanline(image, buf, e);
			cv::Vec3b *data = (cv::Vec3b*)dotImage.ptr<cv::Vec3b>(e);
			for (int c = 0; c < width; c++) {
				uint8 red  = static_cast<uint8*>(buf)[c * 3 + 0];
				uint8 green= static_cast<uint8*>(buf)[c * 3 + 1];
				uint8 blue = static_cast<uint8*>(buf)[c * 3 + 2];
				data[c][0] = saturate_cast<uchar>(blue);
				data[c][1] = saturate_cast<uchar>(green);
				data[c][2] = saturate_cast<uchar>(red);
			}//col
		}//row
		//
		_TIFFfree(buf); // 読み終わったらすぐに破棄
	}
	TIFFClose(image);
	return DOTREAD_SUCCESS; //読まれない場合”空データ”が帰る
}


// 単色化関数(PID画像以外)
cv::Mat mkGrayImage(const cv::Mat &src)
{
	if (src.empty()) return src;
	cv::Mat Bimage, Gimage, Rimage;
	// note: src=CV_8UC3 
	//		 grey scale adjust max of r,g,b
	cv::Mat dst(src.size(), CV_8UC3);
	cv::Mat plane[3];
	cv::split(src, plane);
	plane[0].copyTo(Bimage);
	plane[1].copyTo(Gimage);
	plane[2].copyTo(Rimage);
	int width  = src.cols;
	int height = src.rows;
	for(int y=0;y<height;y++){
		uchar *Bpix = (uchar*)Bimage.ptr<uchar>(y);
		uchar *Gpix = (uchar*)Gimage.ptr<uchar>(y);
		uchar *Rpix = (uchar*)Rimage.ptr<uchar>(y);
		cv::Vec3b *out = (cv::Vec3b*)dst.ptr<cv::Vec3b>(y);
		for(int x=0;x<width;x++){
			int b = (*Bpix++); int g = (*Gpix++); int r = (*Rpix++);
			int val = pickMAX(b, g, r);
			out[x][0] = saturate_cast<uchar>(val); //B
			out[x][1] = saturate_cast<uchar>(val); //G
			out[x][2] = saturate_cast<uchar>(val); //R
		}
	}
	return dst;
}

// DotImage 初期化関数(generate white dotImage) 
cv::Mat mkInitialDotImage(const cv::Mat &src)
{
	if (src.empty()) return src;
	cv::Mat dst(src.size(), CV_8UC3);
	int width  = src.cols;
	int height = src.rows;
	for(int y=0;y<height;y++){
		cv::Vec3b *in = (cv::Vec3b*)src.ptr<cv::Vec3b>(y);
		cv::Vec3b *out = (cv::Vec3b*)dst.ptr<cv::Vec3b>(y);
		for(int x=0;x<width;x++){
			int Rpix=in[x][2];
			int Gpix=in[x][1];
			int Bpix=in[x][0];
			if (Rpix>0 || Gpix>0 || Bpix>0) {
				out[x][0] = static_cast<uchar>(255); //B
				out[x][1] = static_cast<uchar>(255); //G
				out[x][2] = static_cast<uchar>(255); //R
			} else {
				out[x][0] = static_cast<uchar>(0); //B
				out[x][1] = static_cast<uchar>(0); //G
				out[x][2] = static_cast<uchar>(0); //R
			}
		}
	}
	return dst;
}


int pickMAX(int a, int b, int c)
{
	int val=a;
	if (b > val) val = b;
	if (c > val) val = c;
	return val;
}

// 単色化関数(PID画像)
cv::Mat PIDimage1chTo3ch(const cv::Mat &src)
{
	if (src.type()!=CV_8UC1) return src;
	cv::Mat dst(src.size(), CV_8UC3);
	int width  = src.cols;
	int height = src.rows;
	for(int y=0;y<height;y++){
		uchar *pixel = (uchar*)src.ptr<uchar>(y);
		cv::Vec3b *out = (cv::Vec3b*)dst.ptr<cv::Vec3b>(y);
		for(int x=0;x<width;x++){
			int val = (*pixel++);
			out[x][0] = saturate_cast<uchar>(val); //B
			out[x][1] = saturate_cast<uchar>(val); //G
			out[x][2] = saturate_cast<uchar>(val); //R
		}
	}
	return dst;
}

// extract BrightSpot position XY
void ExtractOriginXY(BspotMap &bsdata)
{
	cv::Mat workmat=bsdata.origin.clone();
	int width=workmat.cols;
	int height=workmat.rows;

	for(int y=0;y<height;y++){
		//uchar *pixel = (uchar*)workmat.ptr<uchar>(y); // case monochrome
		for(int x=0;x<width;x++){
			Vec3b pixel=workmat.at<Vec3b>(y,x);
			int val =saturate_cast<int>(pixel[2]+pixel[1]+pixel[0]);
			if(val>0){
				bsdata.x.push_back(x);
				bsdata.y.push_back(y);
			}
		}
	}
	return;
}


// ** obtain search Range
void getDotSearchRange(int x,int y,BspotMap &bsdata,DotRegion &DotRange)
{
	int xtrueMAX = bsdata.origin.cols;
	int ytrueMAX = bsdata.origin.rows;
	int size=bsdata.size;
	size=(size-1)/2;
	int xcmin= x-size; if(xcmin<0) xcmin=0;
	int xcmax= x+size; if(xcmax>=xtrueMAX) xcmax=xtrueMAX;
	int ycmin= y-size; if(ycmin<0) ycmin=0;
	int ycmax= y+size; if(ycmax>=ytrueMAX) ycmax=ytrueMAX;
	DotRange.xmin=xcmin;
	DotRange.xmax=xcmax;
	DotRange.ymin=ycmin;
	DotRange.ymax=ycmax;
	return;
}

// check: is target pixel empty or not? [map =Mat(col,row,CV_8UC3)]
bool isEmptyPixel(int x, int y, const cv::Mat map)
{
	cv::Vec3b pixel = map.at<cv::Vec3b>(y,x);
	int value = saturate_cast<int>(pixel[2]+pixel[1]+pixel[0]);
	if(value>0){
		return false;
	} else {
		return true;
	}
}