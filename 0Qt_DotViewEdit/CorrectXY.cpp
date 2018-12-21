// dynamic Dot position refinement procedure
// Dialog-form Created by: Qt User Interface Compiler version 5.10.0
#include "MyHead.h"

using namespace std;
using namespace cv;


// BrightSpot position correct control panel (this is test code)
void MyDialog::CorrectXYpanel()
{
	// Dynamic contents in this UI-box
	QVBoxLayout *verticalLayout_2;
	QVBoxLayout *verticalLayout;
	QGroupBox *groupBox;
	QPushButton *pushButton;
	QComboBox *comboBox;
	QLabel *label;

	// note: called by correct-button clicked
	this->ui->MyTxtBrowse->append("correct invoked");
	QDialog *CorrectDialog = new QDialog(this);
	MyDialog::correctDiag =(QObject*)CorrectDialog;
    CorrectDialog->setWindowTitle
          (QApplication::translate("Correctwidget", "DotXY correct widget"));
	if(CorrectDialog->objectName().isEmpty())
		CorrectDialog->setObjectName(QStringLiteral("CorrectDialog"));
	CorrectDialog->resize(462, 310);
	verticalLayout_2 = new QVBoxLayout(CorrectDialog);
	verticalLayout_2->setSpacing(6);
	verticalLayout_2->setContentsMargins(11, 11, 11, 11);
	verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
	verticalLayout = new QVBoxLayout();
	verticalLayout->setSpacing(6);
	verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
	groupBox = new QGroupBox(CorrectDialog);
	groupBox->setObjectName(QStringLiteral("groupBox"));
	QFont font;
	font.setFamily(QStringLiteral("MS Sans Serif"));
	font.setPointSize(10);
	font.setBold(true);
	font.setWeight(75);
	groupBox->setFont(font);
	groupBox->setAutoFillBackground(true);
	pushButton = new QPushButton(groupBox);
	pushButton->setObjectName(QStringLiteral("pushButton"));
	pushButton->setGeometry(QRect(50, 190, 309, 28));
	comboBox = new QComboBox(groupBox);
	comboBox->addItem(QString());
	comboBox->addItem(QString());
	comboBox->addItem(QString());
	comboBox->addItem(QString());
	comboBox->setObjectName(QStringLiteral("comboBox"));
	comboBox->setGeometry(QRect(50, 130, 309, 31));
	comboBox->setContextMenuPolicy(Qt::NoContextMenu);
	comboBox->setLayoutDirection(Qt::LeftToRight);
	comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
	label = new QLabel(groupBox);
	label->setObjectName(QStringLiteral("label"));
	label->setGeometry(QRect(80, 50, 266, 40));
	verticalLayout->addWidget(groupBox);
	verticalLayout_2->addLayout(verticalLayout);

	// *** correct dialog form contents Text setting

	groupBox->setTitle(QApplication::translate("Dialog", "Dot position refine parameter", nullptr));
	pushButton->setFont(font);
	pushButton->setText(QApplication::translate("Dialog", "Submit positioning refinement", nullptr));
	comboBox->setFont(font);
	comboBox->setItemText(0, QApplication::translate("Dialog", "\tsize3", nullptr));
	comboBox->setItemText(1, QApplication::translate("Dialog", "\tsize5", nullptr));
	comboBox->setItemText(2, QApplication::translate("Dialog", "\tsize7", nullptr));
	comboBox->setItemText(3, QApplication::translate("Dialog", "\tsize9", nullptr));
	comboBox->setCurrentIndex(1);
	comboBox->setProperty("Test1", QVariant(QApplication::translate("Dialog", "Test1", nullptr)));
	label->setFont(font);
	label->setText(QApplication::translate("Dialog", "Choose size of shift range for refinement. \n"
										   " Then, push [submit] button", nullptr));
	connect(pushButton, SIGNAL(clicked()),this, SLOT(correctSubmit()));
	MyDialog::rangesize = comboBox;
	//QMetaObject::connectSlotsByName(CorrectDialog);
	// **************
	CorrectDialog->exec();
	return;
}

void MyDialog::correctSubmit()
{
	QObject *object= MyDialog::correctDiag;
	QComboBox *rangesize=MyDialog::rangesize;

	int val=rangesize->currentIndex(); // ** obtain user choice
	val=val*2+3;
	string outstring="** Correct submit ** set range ("+to_string(val)+")";
	this->ui->MyTxtBrowse->append(outstring.c_str());
	MyDialog::correctProcedure(val);
	if(object)	object->~QObject(); // self Release correctDialog
	return;

}


void MyDialog::correctProcedure(int rangeSZ)
{
	BspotMap workData;
	clock_t start = clock();
	workData.origin = MyDialog::DotImageData.image.clone();
	MyDialog::DotImageData.bkup_image = MyDialog::DotImageData.image.clone();
	workData.size=rangeSZ; //set search range
	ExtractOriginXY(workData);// pickup current point XY list
	MyDialog::RefineXY(workData); // correction procedure
	MyDialog::DotImageData.refinedMap=workData.refinedMap.clone(); // *********
	MyDialog::DotImageData.image=workData.refinedMap.clone();
	clock_t end = clock();
	double seconds = (double)(end - start) / CLOCKS_PER_SEC;
	string cputime = " Elapsed time:"+to_string(seconds)+"sec";
	this->ui->MyTxtBrowse->append(cputime.c_str() );

	// ** DotEdit panel masking
	MyDialog::SetEditPanel(false);
	this->ui->ReadDotImageBTN->setEnabled(false);
	this->ui->NewDotImageBTN->setEnabled(false);
	

	// ** Draw refined data
	int cx = this->ui->scrollArea->horizontalScrollBar()->value();
	int cy = this->ui->scrollArea->verticalScrollBar()->value();
	MyDialog::DrawImage(); // draw visualize image with new DotImage
	this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
	this->ui->scrollArea->verticalScrollBar()->setValue(cy);
	MyDialog::legendSet();
	// set accept button and reject button
	this->ui->acceptBTN->setEnabled(true);
	this->ui->rejectBTN->setEnabled(true);

	return;
}


// descend compare (caution: didnt write in include-file)
bool Dotcmp(const DotInfo &a, const DotInfo &b)
{
    return a.intensity > b.intensity;
}

// PID Dot image positioning correction (位置補正処理)
void MyDialog::RefineXY(BspotMap &bsdata)
{
	cv::Mat PIDintensityMap = MyDialog::rootData.baseMat.clone();
	DotRegion DotRange;
	Vec3b magenta = Vec3b(255,255,0); // same
	Vec3b green   = Vec3b(85,255,0);  // shift new xy
	Vec3b yellow  = Vec3b(0,255,255); // duplicate point (neighbor)

	int width = bsdata.origin.cols;
	int height = bsdata.origin.rows;
	cv::Mat destMap=BlackPlateImage(width,height);
	int same=0; int shifted=0; int duplicate=0;// counter-setup

	// note: 元画像の輝点位置の登録リストに対して探索範囲を設定する
	//		設定した探索範囲の輝点強度と位置を登録する(DotInfo)
	//		登録リストを強度の大きい順にソートする
	//		ソートしたリストに対し同じ位置ならシアン(空色), 新しい位置(green), 
	//		重複によりずらした位置(黄色)をプロットする
	//		プロットが終わったら、bsdata.refinedMap に転写して終わり
	int npoint = static_cast<int>(bsdata.x.size() );
	std::vector <DotInfo> dotinfo;
	for(int i=0;i<npoint;i++){
		// set point XY initialize
		int xc=bsdata.x[i];
		int yc=bsdata.y[i];
		int obsc=saturate_cast<int>(PIDintensityMap.at<ushort>(yc, xc));
		// ** mapping in search region
		getDotSearchRange(xc, yc, bsdata, DotRange);
		for(int y=DotRange.ymin;y<DotRange.ymax;y++){
			for(int x=DotRange.xmin;x<DotRange.xmax;x++){
				int obs= saturate_cast<int>(PIDintensityMap.at<ushort>(y, x));
				dotinfo.push_back(DotInfo(x,y,obs)); // add xy entry
			}
		} // end of Range-loop
		std::sort(dotinfo.begin(),dotinfo.end(),Dotcmp);
		// ** plotting corrected Dot position
		if (obsc == dotinfo[0].intensity && isEmptyPixel(xc,yc,destMap)) {
		    //if(dotinfo[0].x == xc && dotinfo[0].y ==yc){ 
			destMap.at<Vec3b>(yc,xc) = magenta; // same position
			same++;
		} else {
			Vec3b plotcolor=green;
			bool PlotExected(false);
			for(int j=0;j<static_cast<int>(dotinfo.size()) && !PlotExected ;j++){
				int xt=dotinfo[j].x;
				int yt=dotinfo[j].y;
				if(isEmptyPixel(xt,yt,destMap)){
					destMap.at<Vec3b>(yt,xt) = plotcolor;
					PlotExected=true; // set ending condition
					shifted++;
				} else {
					plotcolor = yellow; // represent double booking position
					duplicate++;
				}
			}
		} // end of plotting
		dotinfo.clear(); // clear for next loop
	}
	bsdata.refinedMap=destMap.clone();
	string wText= to_string(npoint)+" points detected ";
	wText = wText + " same:"+to_string(same)+" shift:"+to_string(shifted)
			+" Duplicate:"+to_string(duplicate);
	this->ui->MyTxtBrowse->append(wText.c_str() );
	return;
}

void MyDialog::acceptFnc()
{
	// set DotImage color 
	cv::Mat DotRawimage=MyDialog::DotImageData.image;
	DotRawimage=mkInitialDotImage(DotRawimage);//white DotImage に一旦変換
	int Rcolor=	MyDialog::DotImageData.Rval;
	int Gcolor=	MyDialog::DotImageData.Gval;
	int	Bcolor=MyDialog::DotImageData.Bval;
	MyDialog::DotImageData.image=ColorChange(DotRawimage,Rcolor,Gcolor,Bcolor);

	// Draw corrected image as New DotImage
	int cx = this->ui->scrollArea->horizontalScrollBar()->value();
	int cy = this->ui->scrollArea->verticalScrollBar()->value();
	MyDialog::DrawImage(); // draw visualize image with new DotImage
	this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
	this->ui->scrollArea->verticalScrollBar()->setValue(cy);

	// ** masking tune
	MyDialog::SetEditPanel(true);
	this->ui->ReadDotImageBTN->setEnabled(true);
	this->ui->NewDotImageBTN->setEnabled(true);
	this->ui->acceptBTN->setEnabled(false);
	this->ui->rejectBTN->setEnabled(false);
	MyDialog::legendUnset();
	return;
}

void MyDialog::rejectFnc()
{
	// fail_back copy backup-image to current image
	MyDialog::DotImageData.image=MyDialog::DotImageData.bkup_image.clone();

	// Draw original DotImage
	int cx = this->ui->scrollArea->horizontalScrollBar()->value();
	int cy = this->ui->scrollArea->verticalScrollBar()->value();
	MyDialog::DrawImage(); // draw visualize image with new DotImage
	this->ui->scrollArea->horizontalScrollBar()->setValue(cx);
	this->ui->scrollArea->verticalScrollBar()->setValue(cy);

	// ** masking tune
	MyDialog::SetEditPanel(true);
	this->ui->ReadDotImageBTN->setEnabled(true);
	this->ui->NewDotImageBTN->setEnabled(true);
	this->ui->acceptBTN->setEnabled(false);
	this->ui->rejectBTN->setEnabled(false);
	MyDialog::legendUnset();
	return;
}

void MyDialog::legendUnset()
{
	// set "invisible"(hide label)
	this->ui->LegendBox->setStyleSheet("background-color: lightGray");
	this->ui->leg1label->setStyleSheet("QLabel { background-color : lightGray; color : lightGray; }");
	this->ui->leg2label->setStyleSheet("QLabel { background-color : lightGray; color : lightGray; }");
	this->ui->leg3label->setStyleSheet("QLabel { background-color : lightGray; color : lightGray; }");
	this->ui->LegendBox->setEnabled(false);
	return;
}


void MyDialog::legendSet()
{
	// set "visible label"
	this->ui->LegendBox->setEnabled(true);
	this->ui->leg1label->setStyleSheet("QLabel { background-color : black; color : cyan; }");
	this->ui->leg2label->setStyleSheet("QLabel { background-color : black; color : lightgreen; }");
	this->ui->leg3label->setStyleSheet("QLabel { background-color : black; color : yellow; }");
	return;
}
