// mouse event handler
#include "MyHead.h"

using namespace std;
using namespace cv;

// ******************* mouse monitoring function ****
// Caution: you must coding carefully. this function has risk to crash
// *** observation intensity
// PID 画像データの強度モニタリング、測定
bool MyDialog::eventFilter(QObject *watched, QEvent *event)
{
	//int cx = this->ui->scrollArea->horizontalScrollBar()->value();
	//int cy = this->ui->scrollArea->verticalScrollBar()->value();
	// mouse position
	//if(event->type() == QEvent::EnterWhatsThisMode) ui->MyTxtBrowse->append("clicked help");
	if ( watched != this->ui->imgLabel )return false;
	const QMouseEvent* const me = static_cast<const QMouseEvent*>( event);
	QPoint p = me->pos();
	int x = static_cast<int>(p.x());
	int y = static_cast<int>(p.y());
	double scalefactor = MyDialog::ViewDataLeft.scaleFactor;
	double fx = static_cast<double>(x);
	double fy = static_cast<double>(y);
	x =  static_cast<int>(fx / scalefactor);
	y =  static_cast<int>(fy / scalefactor);
	std::string Info("");
	update();
	if (event->type() == QEvent::MouseButtonPress){ // mouse button press event
		int intensity = ObserveIntensity(MyDialog::rootData, x, y);
		//Info = "XY: " + to_string(p.x()) + " , " + to_string(p.y());
		//ui->MyTxtBrowse->append(Info.c_str());
		this->ui->obsX->display(x);
		this->ui->obsY->display(y);
		this->ui->PIDintensity->display(intensity);
		if(MyDialog::DotImageData.dataload && MyDialog::DotImageData.DotVisible) DotEdit(me,x,y);
		return true;
	} else if (event->type() == QEvent::MouseMove) { // with(click)
		//std::string Info = "Move: " + std::to_string(x) + " , " + std::to_string(y);
		int intensity = ObserveIntensity(MyDialog::rootData, x, y);
		this->ui->lcdX->display(x);
		this->ui->lcdY->display(y);
		this->ui->lcdI->display(intensity);
		MyDialog::drawcross(p.x(),p.y());
		update();
		return true;
	} else {
	   return false;
   }
	
 }


// Execute DotEdit operation by mouse event(DotEdit interface)
void MyDialog::DotEdit(const QMouseEvent *event,int x, int y)
{
	if(MyDialog::ViewDataLeft.scaleFactor<1.0) return;
	int val(0);
	cv::Mat bkup_image;
	if (event->button() == Qt::LeftButton){
		//this->ui->MyTxtBrowse->append("*** Left mouse button clicked");
		#pragma omp parallel sections
		{
			#pragma omp section
			bkup_image = MyDialog::DotImageData.image.clone();
			#pragma omp section
			val=MyDialog::addPoint( x,  y);
		}
		//if(val==0) 	mkBaseDataAndView();
		if (val == 0) {
			int cx = this->ui->scrollArea->horizontalScrollBar()->value();
			int cy = this->ui->scrollArea->verticalScrollBar()->value();
			MyDialog::DrawImage(); // draw visualize image with new DotImage
			MyDialog::DotImageData.bkup_image=bkup_image.clone();
			this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
			this->ui->scrollArea->verticalScrollBar()->setValue(cy);
		}
		return;
	} else if(event->button() == Qt::RightButton){
		//this->ui->MyTxtBrowse->append("*** Right mouse button clicked");
		// delPoint
		bkup_image = MyDialog::DotImageData.image.clone();
		val=MyDialog::delPoint(x,y);
		if (val == 0) {
			int cx = this->ui->scrollArea->horizontalScrollBar()->value();
			int cy = this->ui->scrollArea->verticalScrollBar()->value();
			MyDialog::DotImageData.bkup_image=bkup_image.clone();
			MyDialog::DrawImage(); // draw visualize image with new DotImage
			this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
			this->ui->scrollArea->verticalScrollBar()->setValue(cy);
		}
		return;
	}
	return;
}

// ** event change example code
//void MyDialog::changeEvent(QEvent *e)
//{
//	QDialog::changeEvent(e);
//	switch (e->type()) {
//	case QEvent::LanguageChange:
//		ui->retranslateUi(this);
//		break;
//	default:
//		break;
//	}
//}

