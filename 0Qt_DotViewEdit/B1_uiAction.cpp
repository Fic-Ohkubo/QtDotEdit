#include "MyHead.h"
// UI component (define action of each item)

using namespace std;
using namespace cv;


void MyDialog::GridColorChange()
{
	int Rcolor = MyDialog::ViewDataLeft.gridR; // note: CVcolor is "BGR"  
	int Gcolor = MyDialog::ViewDataLeft.gridG;
	int Bcolor = MyDialog::ViewDataLeft.gridB;
	QColor ccolor=QColor(Rcolor,Gcolor,Bcolor,255); //set current color
	QColor newcolor = QColorDialog::getColor(ccolor, this );
	if( newcolor.isValid() ){
		newcolor.getRgb(&Rcolor, &Gcolor, &Bcolor);
		MyDialog::ViewDataLeft.gridR=Rcolor;
		MyDialog::ViewDataLeft.gridG=Gcolor;
		MyDialog::ViewDataLeft.gridB=Bcolor;
		MyDialog::mkBaseDataAndView();
	}

	return;
}



void MyDialog::gridsetZ()
{
	MyDialog::ViewDataLeft.ngrid = 0;
	MyDialog::gridButtonUpdate(0);
	return;
}

void MyDialog::gridset3()
{
	MyDialog::ViewDataLeft.ngrid = 3;
	MyDialog::gridButtonUpdate(1);
	return;
}

void MyDialog::gridset4()
{
	MyDialog::ViewDataLeft.ngrid = 4;
	MyDialog::gridButtonUpdate(2);
	return;
}

void MyDialog::gridset5()
{
	MyDialog::ViewDataLeft.ngrid = 5;
	MyDialog::gridButtonUpdate(3);
	return;
}

void MyDialog::gridset8()
{
	MyDialog::ViewDataLeft.ngrid = 8;
	MyDialog::gridButtonUpdate(4);
	return;
}

void MyDialog::gridButtonUpdate(int j)
{
	int cx = this->ui->scrollArea->horizontalScrollBar()->value();
	int cy = this->ui->scrollArea->verticalScrollBar()->value();
	for(int i=0;i<MyDialog::GridMax;i++){
		if (i == j) continue;
		MyDialog::gridBtn[i]->setChecked(false); // ”ñ‘I‘ð‚É‚·‚é
	}
	MyDialog::gridBtn[j]->setChecked(true); // ‘I‘ð‚É‚·‚é
	MyDialog::DrawImage();
	this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
	this->ui->scrollArea->verticalScrollBar()->setValue(cy);
	return;
}