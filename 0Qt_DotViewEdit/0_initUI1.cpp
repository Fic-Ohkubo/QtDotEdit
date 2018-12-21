#include "basedata.h"
#include "MyHead.h"

using namespace std;
using namespace cv;

// main contructor for UI class
// ** GUI���C�A�E�g�ݒ�Ƌ@�\�̃Z�b�g�A�b�v(�N�����ɍŏ��Ɏ��s����鏉�����֐�)
// ** Action �� connect-signal-Slot�̐ݒ�
MyDialog::MyDialog(QWidget *parent):
QDialog(parent),
ui(new Ui::MyDialog)
{
	ui->setupUi(this); // UI base layout setting
	this->setStyleSheet(QString("windows"));
	QString message = QString::fromLocal8Bit("[Ctrl] + [h] �������ƃV���[�g�J�b�g�L�[�̃w���v�\��");
	QFont bfont;
	bfont.setBold(true);
	this->setFont(bfont);
	QLocale jloc= QLocale(QLocale::Japanese);
	this->setLocale(jloc);
	this->setWhatsThis(message);
	MyDialog::DataInitialize();
	MyDialog::LayerMax = PlaneNumber;
	//this->ui->gridLayoutWidget->setMouseTracking(true);

	// note: Signal-Slot �ʐM�̎w��ɂ��A�N�V������`�iQt�̓���w��̓R�[���o�b�N�Ƃ͈قȂ鎖�ɒ��Ӂj
	connect(this->ui->LoadPID, SIGNAL(clicked()), this, SLOT(loadPIDimageData()));
	// ** image operation and basic operation
	connect(this->ui->Push_Zin, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(this->ui->Push_Zout, SIGNAL(clicked()), this, SLOT(zoomOut()));
	connect(this->ui->LayerDown, SIGNAL(clicked()), this, SLOT(LayerDOWNfnc()) );
	connect(this->ui->LayerUP, SIGNAL(clicked()), this, SLOT(LayerUPfnc()) );
	connect(this->ui->gridColorButton, SIGNAL(clicked()), this, SLOT(GridColorChange()));

	// Layout constrain
	MyDialog::InitDrawWidget(); // (���`��G���A�̏�����)�N�����K�{
	MyDialog::LeftTableSetup(this->ui->tableWidget);// ** table widgetn�̃��C�A�E�g�ƃA�N�V������` - �N�����K�{
	MyDialog::gridsetup(); //�O���b�h�I���X�C�b�`
	MyDialog::InitDotEditPanel(); // PIDdata ��ǂݍ��܂Ȃ���Ԃł�Editor��G�点�Ȃ�

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

// �f�[�^�������֐��i�d�v�@�\�Ȃ̂ŁA�����ăR���X�g���N�^�̋߂��ɔz�u�j
void MyDialog::DataInitialize() // �f�[�^�̏�����
{
	int PlaneNum = PlaneNumber;
	MyDialog::rootData.LayerID = 0;

	for (int i = 0; i<PlaneNum; i++){
		MyDialog::rootPlane[i].use = false;
		MyDialog::rootPlane[i].LayerID = i; //�t�����C���f�b�N�X
		MyDialog::Layer[i].idx = i;// �������C���f�b�N�X (�N������ ������=�t����)
		MyDialog::Layer[i].use = false;
	}
	return;
}