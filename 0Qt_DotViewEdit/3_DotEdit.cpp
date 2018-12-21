#include "MyHead.h"

using namespace std;
using namespace cv;




// change status of DotImage visiblity (visible/invisible)
void MyDialog::SetDotView()
{
	if (!MyDialog::DotImageData.dataload) return; // 読み込んでない場合
	if (MyDialog::DotImageData.DotVisible) { // VISIBLE \flip-flop
		MyDialog::DotImageData.DotVisible = false;
		this->ui->DotDrawLabel->setText("OFF");
	} else {
		MyDialog::DotImageData.DotVisible = true;
		this->ui->DotDrawLabel->setText("ON");
	}

	int cx = this->ui->scrollArea->horizontalScrollBar()->value();
	int cy = this->ui->scrollArea->verticalScrollBar()->value();
	MyDialog::DrawImage(); // draw visualize image with new DotImage
	this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
	this->ui->scrollArea->verticalScrollBar()->setValue(cy);

	return;
}

// delete point
int MyDialog::delPoint(int x, int y)
{
	int delsize = MyDialog::DotImageData.EraserSize;
	Vec3b DotVal(0,0,0);
	cv::Scalar black(0,0,0);
	cv::Mat DotImage = MyDialog::DotImageData.image.clone();
	int xleft(0),yleft(0),xright(0),yright(0);

	if(delsize==1){
		if(x<DotImage.cols && y<DotImage.rows){
			MyDialog::DotImageData.image.at<Vec3b>(Point(x, y))=DotVal;
			return 0;
			MyDialog::DotImageData.UndoDone=false;
		} else {
			return -1;
		}
	}
	int xmax=DotImage.cols;
	int ymax=DotImage.rows;
	if (x < xmax && y < ymax && x>0 && y>0) {
		delsize--; delsize /= 2;
		// set point-1
		xleft = x - delsize;
		yleft = y - delsize;
		xright = x + delsize;
		yright = y + delsize;
		if (xleft < 0 && x >= 0)  xleft = 0;
		if (yleft < 0 && y >= 0)  yleft = 0;
		if (xright > xmax && x <= xmax)  xright = xmax;
		if (yright > ymax && y <= ymax)  yright = ymax;
		cv::Point p1 = cv::Point(xleft, yleft);
		cv::Point p2 = cv::Point(xright, yright);
		cv::rectangle(MyDialog::DotImageData.image, p1, p2, black, -1, 8); // Erase points by black-rectangle
		MyDialog::DotImageData.UndoDone=false;
		return 0;
	} else {
		return -1;
	}
	return 0;
}

int MyDialog::addPoint(int x, int y)
{
	cv::Mat DotImage=MyDialog::DotImageData.image;
	int w = static_cast<int>(DotImage.cols);
	int h = static_cast<int>(DotImage.rows);
	int R=MyDialog::DotImageData.DotColor.val[2];
	int G=MyDialog::DotImageData.DotColor.val[1];
	int B=MyDialog::DotImageData.DotColor.val[0];
	if(x<w && y<h){
		 Vec3b DotPoint;
		 DotPoint[0]=B;
		 DotPoint[1]=G;
		 DotPoint[2]=R;
		 MyDialog::DotImageData.image.at<Vec3b>(Point(x, y))=DotPoint;
		 MyDialog::DotImageData.UndoDone=false;
		 return 0;
	}
	return -1;
}


void MyDialog::DotImageLoad()
{
	// *** Dot data loading and begin DotEditMode
	if(!MyDialog::rootData.dataload) return; // PID data を読んでない時はそのまま返す
	QString qfname = QFileDialog::getOpenFileName(this,
						tr("Open Dot Image File (8bit 3ch)"), QDir::currentPath(), tr("Image Files (*.tif)"));
	if (qfname.isEmpty() ) return; //Assertion(cancel case)
	std::string filename = qfname.toUtf8().constData();//(UTF8 convert)
	cv::Mat DotRawimage;
	int read_result=loadDotTiff(filename,DotRawimage);

	// ******* Data check *******
	if(read_result==DOTREAD_SUCCESS){ // file IO
		string str="Load Dot image file:"+filename+" (success)";
		this->ui->MyTxtBrowse->append(str.c_str());
	} else {
		//MessageBeep(-1); // windows beep
		QMessageBox *msgBox = new QMessageBox(this);
		std::string errortxt="** Error **\n\n Invalid data type was detected. Check your DotImage data channel and depth";
		errortxt=errortxt+"\n Only 8bit 3channel data will be accepted as DotImage"; 
		msgBox->setText(errortxt.c_str() );
		msgBox->setDefaultButton(QMessageBox::Ok);
		msgBox->exec();
		return; // データタイプのエラー
	}
	int width=DotRawimage.cols;
	int height=DotRawimage.rows;
	int PIDwidth=MyDialog::rootData.baseImageMat.cols;
	int PIDheight=MyDialog::rootData.baseImageMat.rows;
	if(width!=PIDwidth || height!=PIDheight){ // size mismatch error
		//MessageBeep(-1); // windows beep
		QMessageBox *msgBox = new QMessageBox(this);
		std::string errortxt="** Error **\n\n Size mismatch error was detected. Check your data size.";
		errortxt=errortxt+"\n DotImage size:"+to_string(width)+"x"+to_string(height); 
		msgBox->setText(errortxt.c_str() );
		msgBox->setDefaultButton(QMessageBox::Ok);
		msgBox->exec();
		return;
	}
	// ********* End of Data check ************
	DotRawimage=mkInitialDotImage(DotRawimage);//white image としてload
	if (MyDialog::DotImageData.dataload) {
		MyDialog::DotImageData.bkup_image = MyDialog::DotImageData.image;
	} else {
		MyDialog::DotImageData.bkup_image = DotRawimage.clone();
	}
	// ** Create DotImage data entry 
	//** color Icon
	int W = 100;	int H = 100;
	QPixmap pixmap(W, H);
	pixmap.fill(QColor("white"));
	this->ui->dotColorLabel->setPixmap(pixmap);
	MyDialog::DotImageData.filename=filename;
	MyDialog::DotImageData.dataload=true;
	MyDialog::DotImageData.image=DotRawimage;

	// DrawImage
	int cx = this->ui->scrollArea->horizontalScrollBar()->value();
	int cy = this->ui->scrollArea->verticalScrollBar()->value();
	MyDialog::DrawImage(); // draw visualize image with new DotImage
	this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
	this->ui->scrollArea->verticalScrollBar()->setValue(cy);
	// mkBaseDataAndView();
	MyDialog::SetEditPanel(true);
	return;
}

void MyDialog::mkNewDotImage()
{
	if(!MyDialog::rootData.dataload) return; // PID data を読んでない時はそのまま返す
	cv::Size dotsize=MyDialog::rootData.baseImageMat.size();
	int width = MyDialog::rootData.baseImageMat.cols;
	int height= MyDialog::rootData.baseImageMat.rows;
	cv::Mat DotRawimage=BlackPlateImage(width,height);
	// ** Create DotImage data entry 
	//** color Icon
	int W = 100;	int H = 100;
	QPixmap pixmap(W, H);
	pixmap.fill(QColor("white"));
	if (MyDialog::DotImageData.dataload) {
		MyDialog::DotImageData.bkup_image = MyDialog::DotImageData.image;
	} else {
		MyDialog::DotImageData.bkup_image = DotRawimage.clone();
	}
	this->ui->dotColorLabel->setPixmap(pixmap);
	MyDialog::DotImageData.filename=std::string("newDotImage.tif");
	MyDialog::DotImageData.dataload=true;
	MyDialog::DotImageData.image=DotRawimage;
	string str="** New DotImage [newDotImage.tif]";
	this->ui->MyTxtBrowse->append(str.c_str());
	int cx = this->ui->scrollArea->horizontalScrollBar()->value();
	int cy = this->ui->scrollArea->verticalScrollBar()->value();
	MyDialog::DrawImage(); // draw visualize image with new DotImage
	this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
	this->ui->scrollArea->verticalScrollBar()->setValue(cy);
	MyDialog::SetEditPanel(true);
	MyDialog::DotImageData.UndoDone=false;
	return;
}

void MyDialog::DotEditUndo()
{
	if(MyDialog::DotImageData.UndoDone) return;
	MyDialog::DotImageData.image = MyDialog::DotImageData.bkup_image;
	int cx = this->ui->scrollArea->horizontalScrollBar()->value();
	int cy = this->ui->scrollArea->verticalScrollBar()->value();
	MyDialog::DrawImage(); // draw visualize image with new DotImage
	this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
	this->ui->scrollArea->verticalScrollBar()->setValue(cy);
	MyDialog::DotImageData.UndoDone=true;
	return;
}

void MyDialog::DotImageSave()
{
	QString QfileName = QFileDialog::getSaveFileName(this,
        tr("** Save DotImage data **"), MyDialog::DotImageData.filename.c_str(),
        tr("ImageFile (*.tif);;All Files (*)"));
	if (QfileName.isEmpty())	return;
	std::string filename = QfileName.toUtf8().constData();//(UTF8 convert)
	std::string str="** DotImage was saved to ["+filename+"] **";
	this->ui->MyTxtBrowse->append(str.c_str());
	std::string savefile = QfileName.toLocal8Bit().constData();
	cv::Mat DotRawimage=mkInitialDotImage(MyDialog::DotImageData.image);//white image としてsave
	imwrite(savefile.c_str(),DotRawimage);
	return;
}


void MyDialog::DotColorChange()
{
	int W=50;
	int H=40;
	if(!MyDialog::DotImageData.dataload) return;
	cv::Scalar color=MyDialog::DotImageData.DotColor;
	int Rcolor = color[2]; // note: CVcolor is "BGR"  
	int Gcolor = color[1];
	int Bcolor = color[0];
	int Alpha(0); // this is dummy
	QColor ccolor=QColor(Rcolor,Gcolor,Bcolor,255); //set current color
	QColor newcolor = QColorDialog::getColor(ccolor, this );
	if( newcolor.isValid() ){
		newcolor.getRgb(&Rcolor, &Gcolor, &Bcolor,&Alpha);
		MyDialog::DotImageData.DotColor=cv::Scalar(Bcolor,Gcolor,Rcolor); // note: drop alpha
		MyDialog::DotImageData.Rval=Rcolor;
		MyDialog::DotImageData.Gval=Gcolor;
		MyDialog::DotImageData.Bval=Bcolor;
		QPixmap pixmap(W, H);
		pixmap.fill(newcolor);
		this->ui->dotColorLabel->setPixmap(pixmap);
		//note: dot Image の場合、常に一度white-image に戻してから色づけ(カレント編集画像への色付けなので)
		cv::Mat DotRawimage=MyDialog::DotImageData.image;
		DotRawimage=mkInitialDotImage(DotRawimage);//white DotImage に一旦変換
		MyDialog::DotImageData.image=ColorChange(DotRawimage,Rcolor,Gcolor,Bcolor);
		int cx = this->ui->scrollArea->horizontalScrollBar()->value();
		int cy = this->ui->scrollArea->verticalScrollBar()->value();
		MyDialog::DrawImage(); // draw visualize image with new DotImage
		this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
		this->ui->scrollArea->verticalScrollBar()->setValue(cy);
	}

	return;
}


// Do not use (this is test code)
void MyDialog::DotImageTest()
{
	this->ui->imgLabel = new QLabel;
	this->ui->imgLabel->setMouseTracking( true ); // set Label property again
	this->ui->imgLabel->installEventFilter(this); // set Label property again

	this->ui->imgLabel->setBackgroundRole(QPalette::Base);
	this->ui->imgLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	this->ui->imgLabel->setScaledContents(true);
	// [PixMap generate and set] = drawing 
	//int ngrid = MyDialog::ViewDataLeft.ngrid;
	//cv::Scalar gridcolor=MyDialog::ViewDataLeft.GridColor;
	//cv::Mat viewimage = drawGrid(MyDialog::ViewDataLeft.currentMat, ngrid,gridcolor);
	QPixmap currentpix = Mat2QPixmap(MyDialog::ViewDataLeft.currentMat);
	//QPixmap currentpix = Mat2QPixmap(viewimage);
	int W = MyDialog::ViewDataLeft.currentMat.cols;
	int H = MyDialog::ViewDataLeft.currentMat.rows;

	QPixmap result(W, H);
	result.fill(Qt::transparent); // force alpha channel 
	QPainter paintOver(&result);
    paintOver.drawPixmap(0, 0, currentpix);
    //paintOver.drawPixmap(0, 0, overlay);
	paintOver.setBrush(Qt::red);
	paintOver.drawEllipse(20, 20, 40, 40);
	paintOver.setBrush(Qt::red);
	paintOver.drawLine(100,100,200,200);
	this->ui->imgLabel->setGeometry(QRect(0, 0, W, H));
	this->ui->imgLabel->setPixmap(result.scaled(W,H,Qt::KeepAspectRatio));
	// reset scroolArea property
	this->ui->scrollArea->setWidget(this->ui->imgLabel);
	return;

}

void MyDialog::eraserBTNupdate(int selected)
{
	for(int i=0;i<Eraser_max;i++){
		MyDialog::EraserBTN[i]->setChecked(false);
	}
	MyDialog::EraserBTN[selected]->setChecked(true);
	return;
}

void MyDialog::eraser11set()
{
	// choose 1x1 
	MyDialog::DotImageData.EraserSize=1;
	MyDialog::eraserBTNupdate(0);
	return;
}

void MyDialog::eraser33set()
{
	MyDialog::DotImageData.EraserSize=3;
	MyDialog::eraserBTNupdate(1);
	return;
}

void MyDialog::eraser55set()
{
	MyDialog::DotImageData.EraserSize=5;
	MyDialog::eraserBTNupdate(2);
	return;
}

void MyDialog::eraser99set()
{
	MyDialog::DotImageData.EraserSize=9;
	MyDialog::eraserBTNupdate(3);
	return;
}