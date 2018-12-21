#include "MyHead.h"
// UI component (define action of each item)

using namespace std;
using namespace cv;

//LayerDOWNfnc ���C���[�K�w��������
void MyDialog::LayerDOWNfnc()
{
	QItemSelectionModel *select = this->ui->tableWidget->selectionModel();
	if (!select->hasSelection())return;
	// ** caution final row item is inhibit 
	int searchRange = MyDialog::LayerMax - 1; // final row is inhibit
	// �����ɑI�����ڂ�T��
	for(int i=0;i<searchRange;i++){
		if( this->ui->tableWidget->item(i, 0) !=NULL &&
			this->ui->tableWidget->item(i, 0)->isSelected() ){
			//std::string mesg = "Down Layer [" + to_string(i)+"]";
			//this->ui->MyTxtBrowse->append(mesg.c_str());
			int j = i + 1;
			MyDialog::LayerSwap(i, j);
			MyDialog::TableLayerUpdate(i);// update table widget
			MyDialog::TableLayerUpdate(j);// update table widget
			this->ui->tableWidget->selectColumn(0);
			this->ui->tableWidget->selectRow(j);
			MyDialog::mkBaseDataAndView();
			break; // swap ���s��item�͈����
		}
	}
	return;
}


//LayerUPfnc ���C���[�K�w���グ��
void MyDialog::LayerUPfnc()
{
	QItemSelectionModel *select = this->ui->tableWidget->selectionModel();
	if (!select->hasSelection()){
		//this->ui->MyTxtBrowse->append("no");
		return;
	}
	// ** caution starting row item is inhibit 
	int searchRange = MyDialog::LayerMax - 1; // final row is inhibit
	// �~���ɑI�����ڂ�T��
	for(int i=searchRange;i>0;i--){
		if( this->ui->tableWidget->item(i, 0) !=NULL &&
			this->ui->tableWidget->item(i, 0)->isSelected() ){
			//std::string mesg = "Up Layer [" + to_string(i)+"]";
			//this->ui->MyTxtBrowse->append(mesg.c_str());
			int j = i -1;
			MyDialog::LayerSwap(i, j);
			MyDialog::TableLayerUpdate(i);// update table widget
			MyDialog::TableLayerUpdate(j);// update table widget
			this->ui->tableWidget->selectColumn(0);
			this->ui->tableWidget->selectRow(j);
			MyDialog::mkBaseDataAndView();
			break; // swap ���s��item�͈����
		}
	}
	return;
}


// �I���{�^��
void MyDialog::myQuit() // �S�I��
{
	delete ui;
	this->close();
	exit(0);
}

//PID open
void MyDialog::loadPIDimageData()
{
	openImage(); //PID �f�[�^�̃I�[�v��
	//this->ui->scrollArea->update();
	if (MyDialog::rootData.dataload) { // �����ɃI�[�v�����ꂽ��\�����X�V
		this->ui->ReadDotImageBTN->setEnabled(true); // permit DotImage Edit 
		this->ui->NewDotImageBTN->setEnabled(true); // permit DotImage Edit 
		this->ui->BrightSlide->setValue(100);
		this->ui->MaxSpin->setValue(4095);// spinbox �̃��Z�b�g(12bit MAX �ɑ�����)
		this->ui->MinSpin->setValue(0); //  default value(zero)
		MyDialog::mkBaseDataAndView();
	}
	return;
}

//zoom1
void MyDialog::zoomIn() {
	// wheel
	ScaleByCV(1); // zoom in
}

//zoom2
void MyDialog::zoomOut() {
	ScaleByCV(-1); // zoom out
}


//Image Size Reset
void MyDialog::resetsize() 
{
	//this->ui->MyTxtBrowse->append("*******pushed********"); //**debug
	std::string scaleDisp="Scale 1.00 x";
	this->ui->ScaleLabel->setText(scaleDisp.c_str() );
	SizeResetByCV();
}


// PID image �P�x�l�X�P�[�����O �i�X���C�h�o�[�j
void MyDialog::BrightScaling(int BrightScale)
{
	MyDialog::BrightChange(BrightScale);
}


// PID extraction-range maximum change (spinBox 1)
void MyDialog::ChangingPIDimageMax(int Max)
{
	MyDialog::rootData.Max = Max; // update range parameter(Max)
	BaseDataPID source = MyDialog::rootData; // set original data
	int status(0);
	cv::Mat modifiedImage=GetModifiedPIDimage(source, status);// reloading PID iamge
	if(status==0){
		MyDialog::rootData.baseImageMat = modifiedImage.clone();
	} else {
		ui->MyTxtBrowse->append("PID bright range modification failed.(probably parameter error exist)");
	}
	MyDialog::ViewDataLeft.BrightScale = 1.0;
	this->ui->BrightSlide->setValue(100);
	MyDialog::mkBaseDataAndView();
	this->ui->MinSpin->editingFinished();
	this->ui->MaxSpin->clearFocus();
	return;
}


// PID extraction-range minimum change (Spin Box2)
void MyDialog::ChangingPIDimageMin(int Min)
{
	if (Min < 0 || Min >= MyDialog::rootData.Max) {
		ui->MinSpin->setValue(0);
		return; // assertion(invalid value)
	}
	MyDialog::rootData.Min = Min; // update range parameter(Max)
	BaseDataPID source = MyDialog::rootData; // set original data
	int status(0);
	cv::Mat modifiedImage=GetModifiedPIDimage(source, status); // reloading PID iamge
	if(status==0){
		MyDialog::rootData.baseImageMat = modifiedImage.clone();
	} else {
		ui->MyTxtBrowse->append("PID bright range modification failed.(probably parameter error exist)");
	}
	MyDialog::ViewDataLeft.BrightScale = 1.0;
	this->ui->BrightSlide->setValue(100);
	MyDialog::mkBaseDataAndView();
	this->ui->MinSpin->editingFinished();
	this->ui->MinSpin->clearFocus();
	return;
}