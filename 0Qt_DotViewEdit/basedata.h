#pragma once
// include file for tools 
#pragma warning (disable:4996)	//printfなど
#pragma warning (disable:4244)	//型変換など
#pragma warning (disable:4267)	//型変換など
#pragma warning (disable:4819)	//opencv char error

#include <iostream>			// C++ setup
// opencv header
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>


#include <vector>
#include <array>
#include "tiffio.h" // Handle Tiff file by utilize libtiff
#include <locale>
#include <codecvt>
#include <string>
#include <memory>
#include <vector>
#include <time.h>
#include <ctime>
#include <fstream>
#include <Windows.h>

#define PlaneNumber 20  // Plane data layer number
#define BrightMAX 65535 // Image Bright Max value



struct DotInfo {
	int x;
	int y;
    int intensity;
    DotInfo(int xin, int yin, int obs) // entry operation 
    {
		x=xin;
		y=yin;
		intensity=obs;
    }
};



class DotRegion {
public:
	int xmin;
	int xmax;
	int ymin;
	int ymax;
	DotRegion(){
		xmin=xmax=ymin=ymax=0;
	}
};


class BspotMap{
public:
	cv::Mat origin;
	cv::Mat refinedMap;
	std::vector<int> x;
	std::vector<int> y;
	int size;
	BspotMap(){
		x.reserve(500);
		y.reserve(500);
		size=3; // set minimum range as initialize size
	}
};



// ドット画像データ管理クラス
class DotData
{
public:
	cv::Mat image;		// current data
	cv::Mat bkup_image; // backup image for undo
	cv::Mat refinedMap; // correctXY result
	std::string filename; //fullpath filename
	bool dataload; // read or not flagg
	bool UndoDone; // for cheking already done
	bool DotVisible;
	int EraserSize;
	cv::Scalar DotColor;
	int Rval;
	int Gval;
	int Bval;
	DotData(){
		filename=std::string("");
		dataload = false;
		UndoDone = false;
		DotVisible=true;
		DotColor=CV_RGB(255,255,255); // default は白
		EraserSize=1; // default eraser size
		Rval=255;
		Gval=255;
		Bval=255;
	}
};


#define ZSTEP 26 // zoomspep number 
class zoomingView
{
public:
	double scale[ZSTEP]; // zoom Scale array
	int		step; // current zoom-step
	int		zoom_step_max; // acceptable max step number
	int		zoom_step_min; // acceptable min step number
	int		default_step;
	zoomingView(){
		double base[ZSTEP] ={ 0.1, 0.2, 0.25, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0,
		1.2, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
		12.0, 14.0, 16.0 } ;
		default_step=static_cast<int>(10);
		step = static_cast<int>(10); // element 1.0
		zoom_step_max = static_cast<int>(25);
		zoom_step_min = static_cast<int>(0);
		for (int i = 0; i < ZSTEP; i++) {
			scale[i] = base[i];
		}
	};
};


// *** 表示画像データ
class ViewData 
{
public:
	cv::Mat PIDimage;		// 1ch grayscale
	cv::Mat PlaneImage;		//3ch colorImage
	cv::Mat baseImageMat;
	cv::Mat currentMat;
	int width;				// image width (based on PID bright spot data)
	int height;				// image height(based on PID bright spot data)
	int ngrid;
	// property
	double scaleFactor; // image size scale parameter
	double BrightScale; // image intensity-scale parameter
	// grid color property
	int gridR;
	int gridG;
	int gridB;

	ViewData(){
		width = 0;			
		height = 0;			
		ngrid = 0;			// default grid number 
		scaleFactor = 1.0;	// default size scale 
		BrightScale = 1.0;	// default bright scale
		gridR=100;
		gridG=100;
		gridB=100;
	};

};

// レイヤー画像表示シーケンス管理
class LayerData
{
public:
	int  idx;   // plane data array number
	bool isPID; // check flagg (if set PID data, this is  true)
	std::string name;	// data name (optional)
	bool use;			// data sw (optional)
	bool dataload;
	LayerData(){
		idx = 0;
		name=std::string("");
		use = false;	// default status
		isPID = false;	// default data
		dataload = false;
	}

};

// *** PID ベースデータ定義
// PID: bright spot data class （輝点画像データ管理)
class BaseDataPID
{
public:
	cv::Mat baseMat;		// raw data(16bit 1ch data)
	cv::Mat baseImageMat;	// PID image(3ch 8bit image)
	std::string filename;
	int width;				// image width
	int height;				// image height
	int channel;			// image channel(optional utilize)
	int depth;				// image depth(optional utilize)
	// property
	int Max; // extraction range Maximum
	int Min; // extraction range mINIMUM
	int LayerID; //reverse search index
	
	bool dataload; // read or not flagg	
	BaseDataPID(){
		dataload = false;
		width  = 0;
		height  = 0;
		channel = 0;
		depth = 0;
		Max = 4095;
		Min = 0;
		LayerID = 0;
	};
};

// PID 以外のレイヤー画像データ
// note:PlaneData はフラットなデータという程度の意味
class PlaneData
{
public:
	cv::Mat image; 
	cv::Mat raw_image; //original image(read only)
	std::string filename; //fullpath filename
	bool use;
	bool dataload; // read or not flagg
	int LayerID; //reverse search index
	int colorR;
	int colorG;
	int colorB;
	PlaneData(){
		filename=std::string("");
		use = false;
		dataload = false;
		LayerID = 0;
		colorR = 255; // initialize RGB=white 
		colorG = 255; // initialize RGB=white 
		colorB = 255; // initialize RGB=white 
	}
};

// *********************************************
// 
