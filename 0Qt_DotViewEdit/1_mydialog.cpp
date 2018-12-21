#include "MyHead.h"
// UI function 

using namespace std;
using namespace cv;

// Layer の入れ替え
void MyDialog::LayerSwap(int i, int j)
{
	if (i < 0 || i >= MyDialog::LayerMax) return;	// imvalid array number
	if (j < 0 || j >= MyDialog::LayerMax) return;	// invalid array number
	LayerData tmp = MyDialog::Layer[j];
	MyDialog::Layer[j] = MyDialog::Layer[i];
	MyDialog::Layer[i] = tmp;
 	// ** PID data case (flagg update)
	if (MyDialog::Layer[i].isPID) MyDialog::rootData.LayerID = i;
	if (MyDialog::Layer[j].isPID) MyDialog::rootData.LayerID = j;
	return; 
}

//Open image function call(primitive data for background imagiung)
void MyDialog::LayerRead(int id) // obtain PlaneData array number (i)
{
	int i = id;
	if (!MyDialog::rootData.dataload){ // 念のため、ここでもAssertion
		return;
	}
	QString qfname = QFileDialog::getOpenFileName(this,
						tr("** Load Layer image File **"), QDir::currentPath(), tr("Image Files (*.tif)"));
	if (qfname.isEmpty() ) return; //Assertion
	std::string filename = qfname.toUtf8().constData();
	std::string tiffname = getImagename(filename);
	MyDialog::rootPlane[i].filename = filename;
	MyDialog::rootPlane[i].colorB=255; //reset RGB
	MyDialog::rootPlane[i].colorG=255; //reset RGB
	MyDialog::rootPlane[i].colorR=255; //reset RGB
	int width(0),  height(0),  channel(0),  depth(0);
	ReadPlaneData(MyDialog::rootPlane[i], qfname, width, height, channel, depth);// PID 以外の画像データ読み込み
	MyDialog::rootPlane[i].use = true;
	MyDialog::rootPlane[i].dataload = true;

	//MyDialog::TableLayerUpdate(i);// update table widget
	std::string log = "Layer[" + std::to_string(i+1) + "]\t"+
		" - Info W:"+to_string(width)+" H:"+to_string(height)+
		" Ch:"+to_string(channel)+" depth:"+to_string(depth)+"bit "+tiffname;
	this->ui->MyTxtBrowse->append(log.c_str());
	return;
}

//Open BrightSpot image (PID image data)
void MyDialog::openImage()
{
	QString qfname = QFileDialog::getOpenFileName(this,
					  tr("** Load PID image data File **"), QDir::currentPath(), tr("Image Files (*.tif)"));
	if (qfname.isEmpty() ) return; //Assertion
	bool isThisFirstTime = MyDialog::rootData.dataload;
	ReadBaseData(MyDialog::rootData, qfname); // *** Reading PID image data
	int LayerId(0); //
	if(isThisFirstTime){
		//over writeing
		LayerId = MyDialog::rootData.LayerID;
		MyDialog::Layer[LayerId].isPID = true; // 念のため
		MyDialog::Layer[LayerId].use = true;
		MyDialog::Layer[LayerId].name=MyDialog::rootData.filename;
		MyDialog::Layer[LayerId].dataload = true;
		MyDialog::TableLayerUpdate(LayerId);// update table widget
	} else {
		// initial reading at invoking program
		MyDialog::rootData.LayerID = 0;
		MyDialog::Layer[0].idx = 0;
		MyDialog::Layer[0].isPID = true;// 念のため
		MyDialog::Layer[0].use = true;
		MyDialog::Layer[0].dataload = true;
		MyDialog::Layer[0].name=MyDialog::rootData.filename;
	}
	
	MyDialog::rootData.dataload = true;
	int width(0), height(0), bitDepth(0), channel(0);
	width=MyDialog::rootData.width;
	height=MyDialog::rootData.height;
	bitDepth=MyDialog::rootData.depth;
	channel=MyDialog::rootData.channel;
	std::string imageinfo = "[PiD Image info] ";
	imageinfo = imageinfo + "w:" + to_string(width) + " h:" + to_string(height)
		+ " ch:" + to_string(channel) + " Depth:" + to_string(bitDepth) + "bit";
	imageinfo = imageinfo+" Name:  " + std::string(rootData.filename.c_str());
	this->ui->MyTxtBrowse->append(imageinfo.c_str()); // Logging Information of PID
	MyDialog::TableLayerUpdate(0);// update table widget
	MyDialog::ViewDataLeft.scaleFactor = 1.0;

	return;
}

// Size-Reset function
void MyDialog::SizeResetByCV()
{
	Q_ASSERT(ui->imgLabel->pixmap());
	MyDialog::zooming.step=MyDialog::zooming.default_step;
	MyDialog::ViewDataLeft.scaleFactor = 1.0;
	MyDialog::ViewDataLeft.currentMat = MyDialog::ViewDataLeft.baseImageMat.clone();
	MyDialog::DrawImage(); // draw visualize image
	return;
}

//Open image function call (arbitraly zooming operation)
void MyDialog::ScaleByCV(int factor)
{
	QPoint viewcenter=this->ui->scrollArea->viewport()->rect().center();
	int offsetX0 = viewcenter.x(); // offset x [Scale=1.0]
	int offsetY0 = viewcenter.y(); // offset y [Scale=1.0]
	Q_ASSERT(ui->imgLabel->pixmap());

	if(factor>0) MyDialog::zooming.step++;
	if(factor<0) MyDialog::zooming.step--;
	int step= MyDialog::zooming.step;
	//check scaling-step
	if (step < MyDialog::zooming.zoom_step_min) {// limitter for zoom-out
		step = MyDialog::zooming.zoom_step_min;
		MyDialog::zooming.step = MyDialog::zooming.zoom_step_min;
	}
	if (step > MyDialog::zooming.zoom_step_max) { // limitter for zoom-in
		step = MyDialog::zooming.zoom_step_max;
		MyDialog::zooming.step = MyDialog::zooming.zoom_step_max;
	}
	// Set new scale
	double NewScale =  MyDialog::zooming.scale[step];
	std::string scaleDisp="Scale "+to_string(NewScale).substr(0,4)+" x";
	this->ui->ScaleLabel->setText(scaleDisp.c_str() );
	double OldScale = MyDialog::ViewDataLeft.scaleFactor; // Get old scale factor

	if (NewScale == MyDialog::ViewDataLeft.scaleFactor) {
		std::string	Info = "**Error** Operation reached Zoom-limit ["+
			to_string(NewScale)+"] ";
		this->ui->MyTxtBrowse->append(Info.c_str());
		return; // Reach limit case
	}
	MyDialog::ViewDataLeft.scaleFactor = NewScale;

	int cx = this->ui->scrollArea->horizontalScrollBar()->value();
	int cy = this->ui->scrollArea->verticalScrollBar()->value();
	int tx = cx / OldScale; // current baseX
	int ty = cy / OldScale; // current baseY
	tx = tx + offsetX0/OldScale; // true centerX
	ty = ty + offsetY0/OldScale; // true centerY
	
	cv::Mat src = MyDialog::ViewDataLeft.baseImageMat;
	cv::Mat dst;
	cv::Size dsiz = cv::Size(round(NewScale*src.cols), round(NewScale*src.rows));
	cv::resize(src, dst, dsiz, NewScale, NewScale, INTER_NEAREST);
	dst.copyTo(MyDialog::ViewDataLeft.currentMat);

	MyDialog::DrawImage(); // draw visualize image

	int nx = static_cast<int>(tx*NewScale);// new center x
	int ny = static_cast<int>(ty*NewScale);// new center y
	int sx = nx - offsetX0; // calc adjustment value X
	int sy = ny - offsetY0; // calc adjustment value Y
	this->ui->scrollArea->horizontalScrollBar()->setValue(sx);
	this->ui->scrollArea->verticalScrollBar()->setValue(sy);
	return;
}


// create synthesized image (Layer image and PID-image) by stacking
void MyDialog::mkViewPlane(BaseDataPID &PIDimage, cv::Mat &dst)
{
	cv::Mat PIDmat = PIDimage.baseImageMat;
	int LayerNum = PlaneNumber; // set Max Layer number
	cv::Mat workDST, src;

	//workDSTcv::Mat(PIDmat.rows, PIDmat.cols, CV_8UC3);
	// サイズはPIDdataに合わせる
	int width = static_cast<int>(PIDmat.cols);
	int height= static_cast<int>(PIDmat.rows);
	workDST=BlackPlateImage(width, height);

	//int from_to[] = { 0,0, 1,1, 2,2, 3,3 };
	//cv::mixChannels(&bgra,1,&abgr,1,from_to,4);
	for (int i = 0; i<LayerNum; i++){
		int idx = MyDialog::Layer[i].idx;
		//PlaneData source = MyDialog::rootPlane[idx];
		if (MyDialog::Layer[i].use) {
			if (!MyDialog::Layer[i].isPID) {
				int layerW = static_cast<int>(MyDialog::rootPlane[idx].image.cols);
				int layerH = static_cast<int>(MyDialog::rootPlane[idx].image.rows);
				if (width != layerW || height != layerH) {
					string str="*** Warning: Size mismatch detected: "+
						to_string(width)+"x"+to_string(height)+" vs "+
						to_string(layerW)+"x"+to_string(layerH);
					this->ui->MyTxtBrowse->append(str.c_str() );
				}
				workDST = MatOverwrite(workDST, MyDialog::rootPlane[idx].image); // if DrawSwitch is on execute image overwrite 
			} else {
				PIDmat= MatBrightScaling(PIDmat, MyDialog::ViewDataLeft.BrightScale);
				//workDST = OverlayAlpha(PIDalpha, workDST); //alpha blend
				workDST = MatOverwrite(workDST, PIDmat); // if DrawSwitch is on execute image overwrite 
			}
		}
	}
	dst = workDST.clone();
	return;
}



// display synthesized image
void MyDialog::mkViewData()
{
	// 描画データ作成
	cv::Mat refImg = MyDialog::rootData.baseImageMat;
	cv::Mat planeview = BlackPlateImage(refImg.cols, refImg.rows); 
	MyDialog::mkViewPlane( MyDialog::rootData,planeview);// synthesize PID and other images
	double scaleFactor=MyDialog::ViewDataLeft.scaleFactor;
	// 表示データ作成
	MyDialog::ViewDataLeft.PlaneImage = planeview;
	cv::Mat baseImage=planeview;
	//if(baseImage.type() ==CV_8UC4) 
	//	cvtColor(baseImage,baseImage,COLOR_BGRA2BGR);// 4ch image to 3ch image for QPixmap bug

	MyDialog::ViewDataLeft.baseImageMat = baseImage;
	if(MyDialog::ViewDataLeft.scaleFactor==1.0){
		MyDialog::ViewDataLeft.currentMat = baseImage;
	} else {
		cv::Mat dst;
		cv::Size dsiz = cv::Size(round(scaleFactor*baseImage.cols), round(scaleFactor*baseImage.rows));
		cv::resize(baseImage, dst, dsiz, scaleFactor, scaleFactor, INTER_NEAREST);
		MyDialog::ViewDataLeft.currentMat=dst;
	}
	MyDialog::ViewDataLeft.width = MyDialog::rootData.baseImageMat.cols;
	MyDialog::ViewDataLeft.height = MyDialog::rootData.baseImageMat.rows;
	return;
}

// Drawing ViewData (and DotImage)
// set BrightSpot image and plane image to ViewData
void MyDialog::DrawImage()
{
	this->ui->imgLabel = new QLabel;
	this->ui->imgLabel->setMouseTracking( true ); // set Label property again
	this->ui->imgLabel->installEventFilter(this); // set Label property again
	this->ui->imgLabel->setBackgroundRole(QPalette::Base);
	this->ui->imgLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	this->ui->imgLabel->setScaledContents(true);
	// [PixMap generate and set] = drawing 
	int ngrid = MyDialog::ViewDataLeft.ngrid;
	int R=MyDialog::ViewDataLeft.gridR;
	int G=MyDialog::ViewDataLeft.gridG;
	int B=MyDialog::ViewDataLeft.gridB;
	cv::Scalar gridcolor=CV_RGB(R,G,B);
	cv::Mat viewimage = drawGrid(MyDialog::ViewDataLeft.currentMat, ngrid,gridcolor);
	if (MyDialog::DotImageData.dataload && MyDialog::DotImageData.DotVisible) {
		cv::Mat baseDotImage=MyDialog::DotImageData.image;
		//cv::Mat grayimage=cv::Mat(baseDotImage.size(),CV_8UC1);
		//cvtColor(baseDotImage,grayimage,CV_BGR2GRAY);
		//extractChannel(baseDotImage, grayimage, 0);
		double scaleFactor=MyDialog::ViewDataLeft.scaleFactor;
		cv::Size dsiz = cv::Size(viewimage.cols, viewimage.rows);
		cv::Mat DotImageScaled=cv::Mat(dsiz,CV_8UC3);
		cv::resize(baseDotImage, DotImageScaled, dsiz, scaleFactor, scaleFactor, INTER_NEAREST);
		//viewimage=DrawDotImage(viewimage, DotImageScaled,r,g,b);
		viewimage = MatOverwrite(viewimage, DotImageScaled); // overwrite
	}
	int W=viewimage.cols;
	int H=viewimage.rows;
	QPixmap currentpix(W, H);
	currentpix = Mat2QPixmap(viewimage); // overwrite(avoid color error)
	this->ui->imgLabel->setGeometry(QRect(0, 0, W, H));
	this->ui->imgLabel->setPixmap(currentpix.scaled(W,H,Qt::KeepAspectRatio));
	// reset scroolArea property
	this->ui->scrollArea->setWidget(this->ui->imgLabel);
	return;
}

void MyDialog::mkBaseDataAndView()
{
	// 可視化データの作成と描画
	int cx = this->ui->scrollArea->horizontalScrollBar()->value();
	int cy = this->ui->scrollArea->verticalScrollBar()->value();
	MyDialog::mkViewData(); // make visualize data
	MyDialog::DrawImage(); // draw visualize image
	this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
	this->ui->scrollArea->verticalScrollBar()->setValue(cy);
	return;
}


