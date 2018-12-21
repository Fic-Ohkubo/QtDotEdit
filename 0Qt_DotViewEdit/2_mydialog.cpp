#include "MyHead.h"
#pragma once

using namespace std;
using namespace cv;

// PID display image Bright change
void MyDialog::BrightChange(int BrightScale)
{
	double BrightScaleD = static_cast<double>(BrightScale / 100.0);
	//** change PIDimage with Intensity change
	MyDialog::ViewDataLeft.BrightScale = static_cast<double>(BrightScaleD);
	cv::Mat src = MyDialog::rootData.baseImageMat; // set Original PID image
	cv::Mat scaledImage= MatBrightScaling(src, BrightScaleD);
	MyDialog::ViewDataLeft.PIDimage = scaledImage;
	cv::Mat planeview = MyDialog::ViewDataLeft.PlaneImage;

	//** change baseImage of view
	cv::Mat baseImage = MatOverwrite(planeview, scaledImage);
	MyDialog::ViewDataLeft.baseImageMat = baseImage.clone();
	
	MyDialog::mkBaseDataAndView(); // viewData reconstruct and draw
	return;
}


// ** FunctionKeys shortcut functions (Layer ON/OFF switch)
void MyDialog::F1key(){	MyDialog::DrawSwitch(0);	return;} // Layer 1
void MyDialog::F2key(){	MyDialog::DrawSwitch(1);	return;} // Layer 2
void MyDialog::F3key(){	MyDialog::DrawSwitch(2);	return;} // Layer 3
void MyDialog::F4key(){	MyDialog::DrawSwitch(3);	return;} // Layer 4
void MyDialog::F5key(){	MyDialog::DrawSwitch(4);	return;} // Layer 5
void MyDialog::F6key(){	MyDialog::DrawSwitch(5);	return;} // Layer 6
void MyDialog::F7key(){	MyDialog::DrawSwitch(6);	return;} // Layer 7
void MyDialog::F8key(){	MyDialog::DrawSwitch(7);	return;} // Layer 8
void MyDialog::F9key(){	MyDialog::DrawSwitch(8);	return;} // Layer 9
void MyDialog::F10key(){MyDialog::DrawSwitch(9);	return;} // Layer 10

void MyDialog::SF1key(){	MyDialog::DrawSwitch(10);	return;}// Layer 11
void MyDialog::SF2key(){	MyDialog::DrawSwitch(11);	return;}// Layer 12
void MyDialog::SF3key(){	MyDialog::DrawSwitch(12);	return;}// Layer 13
void MyDialog::SF4key(){	MyDialog::DrawSwitch(13);	return;}// Layer 14
void MyDialog::SF5key(){	MyDialog::DrawSwitch(14);	return;}// Layer 15
void MyDialog::SF6key(){	MyDialog::DrawSwitch(15);	return;}// Layer 16
void MyDialog::SF7key(){	MyDialog::DrawSwitch(16);	return;}// Layer 17
void MyDialog::SF8key(){	MyDialog::DrawSwitch(17);	return;}// Layer 18
void MyDialog::SF9key(){	MyDialog::DrawSwitch(18);	return;}// Layer 19
void MyDialog::SF10key(){	MyDialog::DrawSwitch(19);	return;}// Layer 20
