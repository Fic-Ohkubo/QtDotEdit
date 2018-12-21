#include "basedata.h"
#include "MyHead.h"

using namespace std;
using namespace cv;

// main contructor for UI class
// ** GUIレイアウト設定と機能のセットアップ(起動時に最初に実行される初期化関数)
// ** Action と connect-signal-Slotの設定
MyDialog::MyDialog(QWidget *parent):
QDialog(parent),
ui(new Ui::MyDialog)
{
	ui->setupUi(this); // UI base layout setting
	this->setStyleSheet(QString("windows"));
	QString message = QString::fromLocal8Bit("[Ctrl] + [h] を押すとショートカットキーのヘルプ表示");
	QFont bfont;
	bfont.setBold(true);
	this->setFont(bfont);
	QLocale jloc= QLocale(QLocale::Japanese);
	this->setLocale(jloc);
	this->setWhatsThis(message);
	MyDialog::DataInitialize();
	MyDialog::LayerMax = PlaneNumber;
	//this->ui->gridLayoutWidget->setMouseTracking(true);

	// note: Signal-Slot 通信の指定によるアクション定義（Qtの動作指定はコールバックとは異なる事に注意）
	connect(this->ui->LoadPID, SIGNAL(clicked()), this, SLOT(loadPIDimageData()));
	// ** image operation and basic operation
	connect(this->ui->Push_Zin, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(this->ui->Push_Zout, SIGNAL(clicked()), this, SLOT(zoomOut()));
	connect(this->ui->LayerDown, SIGNAL(clicked()), this, SLOT(LayerDOWNfnc()) );
	connect(this->ui->LayerUP, SIGNAL(clicked()), this, SLOT(LayerUPfnc()) );
	connect(this->ui->gridColorButton, SIGNAL(clicked()), this, SLOT(GridColorChange()));

	// Layout constrain
	MyDialog::InitDrawWidget(); // (左描画エリアの初期化)起動時必須
	MyDialog::LeftTableSetup(this->ui->tableWidget);// ** table widgetnのレイアウトとアクション定義 - 起動時必須
	MyDialog::gridsetup(); //グリッド選択スイッチ
	MyDialog::InitDotEditPanel(); // PIDdata を読み込まない状態ではEditorを触らせない

	// set BaseImage create and view button
	connect(ui->BrightSlide, SIGNAL(valueChanged(int)), this, SLOT(BrightScaling(int)) );

	// ** mouse position monitoring setup
	this->ui->imgLabel->setMouseTracking( true );
	this->ui->imgLabel->installEventFilter(this);
	MyDialog::InitLCD(); //LCD panel setup
	// ** setup PID extraction range
	this->ui->BrightSlide->setValue(100);// setValue(100) = BrightScale(1.0)
	this->ui->MaxSpin->setValue(4095);
	this->ui->MinSpin->setValue(0);
	connect(ui->MaxSpin, SIGNAL(valueChanged(int)), this, SLOT(ChangingPIDimageMax(int)) );
	connect(ui->MinSpin, SIGNAL(valueChanged(int)), this, SLOT(ChangingPIDimageMin(int)) );
	MyDialog::createShortcutKey(); // create entry of Keybord ShortCut-key entry
	return; // end of invoking initialize(constructor)
}

MyDialog::~MyDialog()
{
	delete ui;
}

// データ初期化関数（重要機能なので、あえてコンストラクタの近くに配置）
void MyDialog::DataInitialize() // データの初期化
{
	int PlaneNum = PlaneNumber;
	MyDialog::rootData.LayerID = 0;

	for (int i = 0; i<PlaneNum; i++){
		MyDialog::rootPlane[i].use = false;
		MyDialog::rootPlane[i].LayerID = i; //逆引きインデックス
		MyDialog::Layer[i].idx = i;// 正引きインデックス (起動時は 正引き=逆引き)
		MyDialog::Layer[i].use = false;
	}
	return;
}