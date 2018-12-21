#pragma once
#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QDialog>
#include "basedata.h"
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include <QSignalMapper>
#include <QLCDNumber>
#include "MyHead.h"

namespace Ui {
	class MyDialog;
}


class MyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MyDialog(QWidget *parent = 0);
    ~MyDialog();
	void DataInitialize();
	// ユーザーデータ
	// ** 左画像と右画像を宣言(当面は左画像のみ使う)
	ViewData ViewDataLeft; // MainView
	zoomingView zooming;

	int PlaneNumberSUM;
	BaseDataPID rootData;				//PID image rootData
	PlaneData   rootPlane[PlaneNumber];	// image data of non PID data
	LayerData   Layer[PlaneNumber];		// Layer data(index of image-data)
	DotData		DotImageData; // BrightSpot positioning dot image data

private slots:
	void loadPIDimageData();	// Open PID image data
	void zoomIn();		// UI-action zoom
	void zoomOut();		// UI-action zoom
	void resetsize();	// UI-action zoom reset
	void myQuit();		// Quit 
	void openImage();	//open BrightSpot Image function to the project
	// Table button
	void PlaneRead(int);		// ** Image reader( background(LayerMask) image)
	void DrawSwitch(int);		// ** Layer Draw Selector
	void setColor(int);			// ** Set Layer color

	void mkBaseDataAndView();	// PID data load (inner action)
	void BrightScaling(int);	// ImageBright
	void ChangingPIDimageMax(int); // extraction range change
	void ChangingPIDimageMin(int); // extraction range change
	void LayerUPfnc();		// UI-action
	void LayerDOWNfnc();	// UI-action

	void gridsetZ();		//grid setzero (empty)
	void gridset3();		// Grid 3x3
	void gridset4();		// grid 4x4
	void gridset5();		// grid 5x5
	void gridset8();		// grid 8x8
	void GridColorChange();	// Grid lineColor

	void DotImageLoad();	// DotImage file loader
	void mkNewDotImage();	// Create New DotImage
	void DotImageSave();	// Save current DotImage
	void DotColorChange();	// DotColor change
	void eraser11set();		// Eraser 1x1
	void eraser33set();		// Eraser 3x3
	void eraser55set();		// Eraser 5x5
	void eraser99set();		// Eraser 7x7
	void DotEditUndo();		// Undo 1 operation
	void eraserBTNupdate(int); // selector_Button update
	void SetDotView();		// DotView (visible/invisible) change
	void CorrectXYpanel();	// correct XY function for DotEdit
	void acceptFnc();		// accept correct XY result
	void rejectFnc();		// reject correct XY result

	// ** shortcut-Keys slot entry 
	void F1key();
	void F2key();
	void F3key();
	void F4key();
	void F5key();
	void F6key();
	void F7key();
	void F8key();
	void F9key();
	void F10key();
	void SF1key();
	void SF2key();
	void SF3key();
	void SF4key();
	void SF5key();
	void SF6key();
	void SF7key();
	void SF8key();
	void SF9key();
	void SF10key();
	void sHelp(); // shortcut-key help
	void helpend();			// shortcut-key help end
	void correctSubmit();	// BrightSpot XY correction submit


protected:
	// component function
	void mkViewData();
	void mkViewPlane(BaseDataPID &PIDimage, cv::Mat &dst);
	void DrawImage();
	bool eventFilter(QObject *obj, QEvent *event); // call back function(danger)
	void BrightChange(int BrightScale);
	void SizeResetByCV();

	void ScaleByCV(int); // zoom scaling 
	void InitLCD(); // 表示パネルレイアウト初期化
	void InitDrawWidget(); // 描画ウイジェットの初期化(左)
	void InitDotEditPanel();
	void SetEditPanel(bool);
	void gridsetup(); // grid-button ui setup 
	void gridButtonUpdate(int);
	//** proto-type functions

	void LayerSwap(int, int);	// Layer data swapping
	void TableLayerUpdate(int);	// table widget updater
	void LeftTableSetup(QTableWidget *table);
	void LayerRead(int);
	void setCellColor(int); // cellcolor change by QLabel (note: this is not general technic in tableWidget)

	void DotImageTest();
	void EraserSetup();
	void DotEdit(const QMouseEvent *event,int x,int y);
	int addPoint(int x, int y);
	int	delPoint(int x, int y);
	void createShortcutKey(); // generate shortcut key function

private:
	//QImage image;
	//MyimageLabel *imageLabel;

	// GUI component
	QGridLayout* gridLayout; // grid button groupBox
	QScrollArea *scrollArea;
	QTableWidget *tableWidget;
	QPushButton *LoadPID;
	QPushButton *Push_Zin;
	QPushButton *Push_Zout;
    QPushButton *ReloadViewBTN;
	QSlider *BrightSlide;
	QLCDNumber	*lcdX;
	QLCDNumber	*lcdY;

	// ** DotImageEditor object
	QPushButton *ReadDotImageBTN;
	QPushButton *NewDotImageBTN;
    QPushButton *DotSaveBTN;
	QPushButton *DotColorBTN;
    QLabel *dotColorLabel;
	QPushButton *EraserBTN[5]; // for eraser size check(select) operation
    QPushButton *Eraser1x1BTN;
    QPushButton *Eraser3x3BTN;
    QPushButton *Eraser5x5BTN;
    QPushButton *Eraser9x9BTN;
	int Eraser_max;
    QPushButton *DotViewSW; // toggle switch Dot Image visible/invisible

	// ** Viewer grid setting
	QPushButton *gridBtn[5];  // zero,3,4,5,8 (grid button [for check(select) operation])
	int GridMax; // Number of Gridbutton

	// ** table-widget component (Left table Button)
	QSignalMapper *mapper;	// signal mapper (Read button)
	QSignalMapper *mapper2; // signal mapper (ON/OFF switch)
	QSignalMapper *mapper3; // signal mapper (color change)
	QPushButton *tabbtn[PlaneNumber];		// number for array size equal to max size
	QPushButton *swbtn[PlaneNumber];		// number for array size equal to max size
	QPushButton *layercolor[PlaneNumber];	// set layer color
	int LayerMax;

	// Inner interfacxe data
	double scaleFactor;

	// etc.
	QLabel *crossLabel;
	QPixmap *tpix;
	void drawcross(int,int);


	// for help destruction
	QObject *destObj;

	// XY correct panel
	QObject *correctDiag;
	QComboBox *rangesize;
	void correctProcedure(int); // MyDialog クラスからデータを貰うのでメンバーとして登録
	void RefineXY(BspotMap &bsdata);// refinement procedure
	void legendUnset();
	void legendSet();

	// ** action
	QAction *copyAct;
	QAction *fitToWindowAct;

	//interface
    Ui::MyDialog *ui;
};

#endif // MYDIALOG_H
