#include "MyHead.h"
// UI component (define action of each item)

using namespace std;
using namespace cv;

// switch for changing DotImage eraser size
void MyDialog::EraserSetup()
{
	int eraser_max(4);
	MyDialog::Eraser_max = eraser_max;
	MyDialog::EraserBTN[0] = this->ui->Eraser1x1BTN;
	MyDialog::EraserBTN[1] = this->ui->Eraser3x3BTN;
	MyDialog::EraserBTN[2] = this->ui->Eraser5x5BTN;
	MyDialog::EraserBTN[3] = this->ui->Eraser9x9BTN;
	// setup grid select status
	for(int i=1;i<Eraser_max;i++){
		MyDialog::EraserBTN[i]->setChecked(false);
	}
	MyDialog::EraserBTN[0]->setChecked(true);
	// setup ui signal-slot
	connect(this->ui->Eraser1x1BTN, SIGNAL(clicked()), this, SLOT(eraser11set()));
	connect(this->ui->Eraser3x3BTN, SIGNAL(clicked()), this, SLOT(eraser33set()));
	connect(this->ui->Eraser5x5BTN, SIGNAL(clicked()), this, SLOT(eraser55set()));
	connect(this->ui->Eraser9x9BTN, SIGNAL(clicked()), this, SLOT(eraser99set()));
	return;
}

// grid switch initialize
void MyDialog::gridsetup()
{
	int grid_max(5);
	MyDialog::GridMax = grid_max;
	MyDialog::gridBtn[0] = this->ui->gridZero;
	MyDialog::gridBtn[1] = this->ui->grid3;
	MyDialog::gridBtn[2] = this->ui->grid4;
	MyDialog::gridBtn[3] = this->ui->grid5;
	MyDialog::gridBtn[4] = this->ui->grid8;

	// setup grid select status500
	for(int i=0;i<grid_max;i++){
		MyDialog::gridBtn[i]->setChecked(false);
	}
	MyDialog::gridBtn[0]->setChecked(true);
	// setup ui signal-slot
	connect(this->ui->gridZero, SIGNAL(clicked()), this, SLOT(gridsetZ()));
	connect(this->ui->grid3, SIGNAL(clicked()), this, SLOT(gridset3()));
	connect(this->ui->grid4, SIGNAL(clicked()), this, SLOT(gridset4()));
	connect(this->ui->grid5, SIGNAL(clicked()), this, SLOT(gridset5()));
	connect(this->ui->grid8, SIGNAL(clicked()), this, SLOT(gridset8()));
	return;
}

// initialize viewport for image display
void MyDialog::InitDrawWidget()
{
	// ** resize draw area of UI
	this->ui->scrollArea->setMinimumSize(QSize(1100, 700)); // viewer 画面size(min)
	this->ui->scrollArea->setMaximumSize(QSize(1100, 700)); // viewer 画面size(max)
	// ** resize draw area of virtual
	int width0 = 1600;
	int height0 = 1200;
	//* image area(scroll area) initialize (やっておかないと落ちる！:描画エリアに空の画像をセット)
	this->ui->label->setScaledContents(true);
	this->ui->imgLabel = new QLabel;
	this->ui->imgLabel->setStyleSheet("QLabel {background-color: blue; color: white;}");
	QPixmap pixmap(width0, height0);
	pixmap.fill(QColor("blue"));
	this->ui->imgLabel->setPixmap(pixmap);
	this->ui->scrollArea->setWidget(ui->imgLabel);
	QImage newimg = pixmap.toImage();

	MyDialog::crossLabel = new QLabel(this);
	this->ui->imgLabel->stackUnder(crossLabel);
	crossLabel->raise();

	// **basic data initialize
	InitStartWindBaseData(MyDialog::ViewDataLeft, newimg);
	MyDialog::rootData.baseImageMat = BlackPlateImage(width0, height0); 
	MyDialog::rootData.baseMat = BlackPlateImage16(width0, height0);
	return;
}

// DotImageEditor Panel initialize
void MyDialog::InitDotEditPanel()
{
	MyDialog::EraserSetup();
	
	// initialize panel widget layout
	this->ui->ReadDotImageBTN->setEnabled(false);
	this->ui->NewDotImageBTN->setEnabled(false);

	this->ui->NewDotImageBTN->setFocusPolicy(Qt::NoFocus);
	this->ui->ReadDotImageBTN->setFocusPolicy(Qt::NoFocus);
	this->ui->DotSaveBTN->setFocusPolicy(Qt::NoFocus);

	MyDialog::SetEditPanel(false);
	MyDialog::legendUnset();

	int W=70; // indicator label width
	int H=30; // indicator label height
	QPixmap pixmap(W, H);
	pixmap.fill("white");
	this->ui->DotDrawLabel->setPixmap(pixmap);
	this->ui->DotDrawLabel->setText("ON");

	//initialize correct panel
	this->ui->acceptBTN->setEnabled(false);
	this->ui->rejectBTN->setEnabled(false);

	// ** DotImage Editor panel action setup [Signal-Slot setting]
	connect(this->ui->ReadDotImageBTN , SIGNAL(clicked()), this, SLOT(DotImageLoad()));
	connect(this->ui->NewDotImageBTN , SIGNAL(clicked()), this, SLOT(mkNewDotImage()));
	connect(this->ui->DotSaveBTN , SIGNAL(clicked()), this, SLOT(DotImageSave())   );
	connect(this->ui->DotColorBTN ,SIGNAL(clicked()), this, SLOT(DotColorChange()) );
	connect(this->ui->DotViewSW,   SIGNAL(clicked()), this, SLOT(SetDotView())     );
	connect(this->ui->CorrectBTN,  SIGNAL(clicked()), this, SLOT(CorrectXYpanel()) );
	connect(this->ui->acceptBTN,   SIGNAL(clicked()), this, SLOT(acceptFnc())      );
	connect(this->ui->rejectBTN,   SIGNAL(clicked()), this, SLOT(rejectFnc())      );
	//DotImageSave()
	return;
}

// set Edit panel component activate
void MyDialog::SetEditPanel(bool status)
{
	this->ui->DotSaveBTN->setEnabled(status);
	this->ui->Eraser1x1BTN->setEnabled(status);
	this->ui->Eraser3x3BTN->setEnabled(status);
	this->ui->Eraser5x5BTN->setEnabled(status);
	this->ui->Eraser9x9BTN->setEnabled(status);
	this->ui->groupBox_5->setEnabled(status);
	this->ui->DotColorBTN->setEnabled(status);
	this->ui->groupBox_6->setEnabled(status);
	this->ui->DotViewSW->setEnabled(status);
	this->ui->CorrectBTN->setEnabled(status);
	return;
}

void MyDialog::InitLCD() // LCDパネルの初期化
{
	// 座標と強度(PID輝度)の測定表示パネルのレイアウトスタイル初期化

	// ** set LCD number panel color(輝度測定表示パネル)
	this->ui->lcdBox->setStyleSheet("background:darkgrey;");
	QPalette *Pal = new QPalette();
    Pal->setColor(QPalette::WindowText, QColor(0, 255, 0));
	this->ui->lcdBox->setPalette(*Pal);
    this->ui->lcdX->setPalette(*Pal);
	this->ui->lcdY->setPalette(*Pal);
	this->ui->lcdI->setPalette(*Pal);
	this->ui->lcdX->setStyleSheet("background:black;");
	this->ui->lcdY->setStyleSheet("background:black;");
	this->ui->lcdI->setStyleSheet("background:black;");

	// 計測表示(スタイル表示で色を指定)
	this->ui->lcdBox->setStyleSheet("background:black;");
	QPalette *PalObs = new QPalette();
	PalObs->setColor(QPalette::WindowText, QColor(255, 255, 0));
	this->ui->obsX->setPalette(*PalObs);
	this->ui->obsY->setPalette(*PalObs);
	this->ui->PIDintensity->setPalette(*PalObs);

	this->ui->obsX->setStyleSheet("background:black;");
	this->ui->obsY->setStyleSheet("background:black;");
	this->ui->PIDintensity->setStyleSheet("background:black;");

	return;
}

// create shortcut-key entry and their control
void MyDialog::createShortcutKey()
{
	// ** ZoomInOut control
	QShortcut *shortcut1 = new QShortcut(QKeySequence("+"), this);
	QShortcut *shortcut2 = new QShortcut(QKeySequence("-"), this);
	QShortcut *shortcut1b = new QShortcut(QKeySequence("PgUp"), this);
	QShortcut *shortcut2b = new QShortcut(QKeySequence("PgDown"), this);
	connect(shortcut1, SIGNAL(activated()), this, SLOT(zoomIn()));
	connect(shortcut2, SIGNAL(activated()), this, SLOT(zoomOut()));
	connect(shortcut1b, SIGNAL(activated()), this, SLOT(zoomIn()));
	connect(shortcut2b, SIGNAL(activated()), this, SLOT(zoomOut()));

	// shortcut function key (Layer ON/OFF)
	QShortcut *shortcutF1 = new QShortcut(QKeySequence("F1"), this);
	QShortcut *shortcutF2 = new QShortcut(QKeySequence("F2"), this);
	QShortcut *shortcutF3 = new QShortcut(QKeySequence("F3"), this);
	QShortcut *shortcutF4 = new QShortcut(QKeySequence("F4"), this);
	QShortcut *shortcutF5 = new QShortcut(QKeySequence("F5"), this);
	QShortcut *shortcutF6 = new QShortcut(QKeySequence("F6"), this);
	QShortcut *shortcutF7 = new QShortcut(QKeySequence("F7"), this);
	QShortcut *shortcutF8 = new QShortcut(QKeySequence("F8"), this);
	QShortcut *shortcutF9 = new QShortcut(QKeySequence("F9"), this);
	QShortcut *shortcutF10= new QShortcut(QKeySequence("F10"), this);

	connect(shortcutF1, SIGNAL(activated()), this, SLOT(F1key()));
	connect(shortcutF2, SIGNAL(activated()), this, SLOT(F2key()));
	connect(shortcutF3, SIGNAL(activated()), this, SLOT(F3key()));
	connect(shortcutF4, SIGNAL(activated()), this, SLOT(F4key()));
	connect(shortcutF5, SIGNAL(activated()), this, SLOT(F5key()));
	connect(shortcutF6, SIGNAL(activated()), this, SLOT(F6key()));
	connect(shortcutF7, SIGNAL(activated()), this, SLOT(F7key()));
	connect(shortcutF8, SIGNAL(activated()), this, SLOT(F8key()));
	connect(shortcutF9, SIGNAL(activated()), this, SLOT(F9key()));
	connect(shortcutF10,SIGNAL(activated()), this, SLOT(F10key()));

	QShortcut *shortcutSF1 = new QShortcut(QKeySequence("Shift+F1"), this);
	QShortcut *shortcutSF2 = new QShortcut(QKeySequence("Shift+F2"), this);
	QShortcut *shortcutSF3 = new QShortcut(QKeySequence("Shift+F3"), this);
	QShortcut *shortcutSF4 = new QShortcut(QKeySequence("Shift+F4"), this);
	QShortcut *shortcutSF5 = new QShortcut(QKeySequence("Shift+F5"), this);
	QShortcut *shortcutSF6 = new QShortcut(QKeySequence("Shift+F6"), this);
	QShortcut *shortcutSF7 = new QShortcut(QKeySequence("Shift+F7"), this);
	QShortcut *shortcutSF8 = new QShortcut(QKeySequence("Shift+F8"), this);
	QShortcut *shortcutSF9 = new QShortcut(QKeySequence("Shift+F9"), this);
	QShortcut *shortcutSF10= new QShortcut(QKeySequence("Shift+F10"), this);

	connect(shortcutSF1, SIGNAL(activated()), this, SLOT(SF1key()));
	connect(shortcutSF2, SIGNAL(activated()), this, SLOT(SF2key()));
	connect(shortcutSF3, SIGNAL(activated()), this, SLOT(SF3key()));
	connect(shortcutSF4, SIGNAL(activated()), this, SLOT(SF4key()));
	connect(shortcutSF5, SIGNAL(activated()), this, SLOT(SF5key()));
	connect(shortcutSF6, SIGNAL(activated()), this, SLOT(SF6key()));
	connect(shortcutSF7, SIGNAL(activated()), this, SLOT(SF7key()));
	connect(shortcutSF8, SIGNAL(activated()), this, SLOT(SF8key()));
	connect(shortcutSF9, SIGNAL(activated()), this, SLOT(SF9key()));
	connect(shortcutSF10,SIGNAL(activated()), this, SLOT(SF10key()));

	// ** DotImage Editor shortcut
	QShortcut *shortcut3 = new QShortcut(QKeySequence("Tab"), this);
	QShortcut *shortcut3b = new QShortcut(QKeySequence("Backspace"), this);
	connect(shortcut3, SIGNAL(activated()), this, SLOT(SetDotView()));
	connect(shortcut3b, SIGNAL(activated()), this, SLOT(SetDotView()));
	QShortcut *shortcut4 = new QShortcut(QKeySequence("Ctrl+z"), this);
	connect(shortcut4, SIGNAL(activated()), this, SLOT(DotEditUndo()));
	QShortcut *shortcutSave = new QShortcut(QKeySequence("Ctrl+w"), this);
	connect(shortcutSave, SIGNAL(activated()), this, SLOT(DotImageSave()));

	// ** special function
	QShortcut *shortcutResize = new QShortcut(QKeySequence("Ctrl+*"), this);
	connect(shortcutResize,SIGNAL(activated()),this, SLOT(resetsize()) );
	QShortcut *shortcut5 = new QShortcut(QKeySequence("Ctrl+F11"), this);
	connect(shortcut5, SIGNAL(activated()), this, SLOT(mkBaseDataAndView()));
	QShortcut *shortcut6 = new QShortcut(QKeySequence("Ctrl+F12"), this);
	connect(shortcut6, SIGNAL(activated()), this, SLOT(myQuit()));
	QShortcut *shortcut7 = new QShortcut(QKeySequence("Ctrl+h"), this);
	connect(shortcut7, SIGNAL(activated()), this, SLOT(sHelp()));
	return;
}

void MyDialog::sHelp()
{
	int Bottom=static_cast<int>(640);
	QDialog *HDialog = new QDialog(this);
	if (HDialog->objectName().isEmpty())
		HDialog->setObjectName(QStringLiteral("HelpDialog"));
	HDialog->resize(700, Bottom);
	//HDialog->setLayoutDirection(
	QTextBrowser *helpBrowse = new QTextBrowser(HDialog);
	helpBrowse->resize(680,Bottom-60);
	QLocale jloc= QLocale(QLocale::Japanese);
	QFont newfont("FixedSys",10);
	newfont.setStyleHint(QFont::TypeWriter);
	helpBrowse->setFont(newfont);
	helpBrowse->setLocale(jloc);
	MyDialog::destObj = (QObject*)HDialog;


	std::string msg = "\t\t\t*** ショートカットキー・ガイド ***\n\n\n";
	msg=msg+"** 拡大／縮小\n\n";
	msg=msg+"\t[+][-]......................... ズームイン, アウト\n";
	msg=msg+"\t[PgUp][PgDown]................. (同じ)\n\n\n";
	msg=msg+"** レイヤー画像の ON/OFF\n\n";
	msg=msg+"\tLayer1 から Layer10 \t= [F1]から[F10]\n";
	msg=msg+"\tLayer11 から Layer20 \t= [Shift]+[F1]から[Shift]+[F10]\n\n\n";
	msg=msg+"** 点画像の編集\n\n";
	msg=msg+"\t[Tab],[Backspace]...... 描画ON/OFF\n";
	msg=msg+"\t[Ctrl]+[z]............. ひとつ前の画像に戻る \n";
	msg=msg+"\t[Ctrl]+[w]............. 点画像の保存 \n\n\n";
	msg=msg+"** その他\n\n";
	msg=msg+"\t[Ctrl]+[F11]............. 画像の書き直し\n";
	msg=msg+"\t[Ctrl]+[F12]............. 強制終了\n";
	msg=msg+"\t[Ctrl]+[*]............... 画像サイズをリセット\n";
	msg=msg+"\t[Ctrl]+[h]............... ショートカットキーヘルプ(この画面)\n";
	QString message = QString::fromLocal8Bit(msg.c_str());
	helpBrowse->setText(message);
	helpBrowse->show();
	QPushButton *closeButton=new QPushButton(HDialog);
	QFont bfont;
	bfont.setBold(true);
	closeButton->setFont(bfont);
	closeButton->setText("close");
	closeButton->setGeometry(QRect(260, Bottom-40, 80, 28));
	connect(closeButton, SIGNAL(clicked()),this, SLOT(helpend()));
	HDialog->exec();
	return;
}


void MyDialog::helpend()
{
	QObject *object= MyDialog::destObj;
	if(object)	object->~QObject();
	return;
}
