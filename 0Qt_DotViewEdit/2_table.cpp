#include "MyHead.h"

using namespace std;
using namespace cv;

// TableWidgetの関数

//table widget の初期化とレイアウトセットアップ(この関数はコンストラクタ以外で使用してはならない)
void MyDialog::LeftTableSetup(QTableWidget *table) 
{
	//tableのLayout 指定
	int nrow = MyDialog::LayerMax; // Layer 最大値
	table->setRowCount(nrow);
	table->setColumnCount(10);
	//table->setMinimumHeight(140);
	table->setColumnWidth(0, 240); // ファイ名表示の横幅指定

	// ボタン名称のfont
	QFont bfont;
	bfont.setFamily(QStringLiteral("MS UI Gothic"));
	bfont.setBold(false); //set style bold if is true
	bfont.setItalic(false); //set style italic if is true
	bfont.setPixelSize(12); // Sets the font size to pixelSize pixels.

	// Mapper を使ったウィジェットの生成と動作定義
	//std::unique_ptr<QSignalMapper[]> mapper(new QSignalMapper[5]);
	mapper = new QSignalMapper(); // mapper for readImage
	mapper2= new QSignalMapper(); // mapper for draw on/off
	mapper3= new QSignalMapper(); // mapper for color change

	int bCol1 = 2; // ReadImage button position
	int bCol2 = 4; // ON/Off switch position
	int bCol3 = 6; // setColor position
	int bheight = 10;
	//table->verticalHeader()->setFixedHeight(22);
	table->horizontalHeader()->setFixedHeight(24);
	table->setColumnWidth(0, 200); //FileName area width setup
	table->setColumnWidth(1, 48); // PID or not
	table->setColumnWidth(2, 50); // Load
	table->setColumnWidth(3, 60); // Button area width setup(show on/off)
	table->setColumnWidth(4, 64);  //Button area width setup
	table->setFocusPolicy(Qt::NoFocus);
	for (int i = 0; i<nrow; i++){
		// reading button（note: tabbtn[] は mydaialog.h でstaticとして宣言)
		tabbtn[i] = new QPushButton("LOAD");
		tabbtn[i]->setParent(table);
		tabbtn[i]->setIconSize((QSize(20, bheight)));
		tabbtn[i]->setFocusPolicy(Qt::NoFocus);
		tabbtn[i]->setFont(bfont);
		// select button
		swbtn[i] = new QPushButton("ON/OFF");
		swbtn[i]->setParent(table);
		swbtn[i]->setIconSize((QSize(60, bheight)));
		swbtn[i]->setFocusPolicy(Qt::NoFocus);
		swbtn[i]->setFont(bfont);
		// color change button
		layercolor[i] = new QPushButton("SET COLOR");
		layercolor[i]->setParent(table);
		layercolor[i]->setIconSize((QSize(70, bheight)));
		layercolor[i]->setFocusPolicy(Qt::NoFocus);
		layercolor[i]->setFont(bfont);

		// map for image read
		table->setRowHeight(i, 24); // 高さの指定(共通)
		connect(tabbtn[i], SIGNAL(clicked()), mapper, SLOT(map())); // ボタンアクションの処理設定
		mapper->setMapping(tabbtn[i], i); // widget mapping
		table->setCellWidget(i, bCol1, tabbtn[i]);
		// map for selector-switch
		connect(swbtn[i], SIGNAL(clicked()), mapper2, SLOT(map())); // ボタンアクションの処理設定
		mapper2->setMapping(swbtn[i], i); // widget mapping
		table->setCellWidget(i, bCol2, swbtn[i]);
		// map for colorset
		connect(layercolor[i], SIGNAL(clicked()), mapper3, SLOT(map())); // ボタンアクションの処理設定
		mapper3->setMapping(layercolor[i], i); // widget mapping
		table->setCellWidget(i, bCol3, layercolor[i]);
	}
	connect(mapper, SIGNAL(mapped(int)), this, SLOT(PlaneRead(int)));
	connect(mapper2, SIGNAL(mapped(int)), this, SLOT(DrawSwitch(int)));
	connect(mapper3, SIGNAL(mapped(int)), this, SLOT(setColor(int)));
	// ** end of TableWidget setup
	return;
}


// ** setup color function
void MyDialog::setColor(int i)
{
	if (!MyDialog::Layer[i].dataload) return;
	int idx = MyDialog::Layer[i].idx;
	int	colorR=MyDialog::rootPlane[idx].colorR  ;
	int	colorG=MyDialog::rootPlane[idx].colorG  ;
	int	colorB=MyDialog::rootPlane[idx].colorB  ;
	QColor ccolor=QColor(colorR,colorG,colorB,255); //set current color
	QColor color = QColorDialog::getColor(ccolor, this );
	if( color.isValid() ){
		color.getRgb(&colorR, &colorG, &colorB);
		//std::string str = "Color:["+to_string(i)+"] "+ to_string(colorR) + "," + 
		//	to_string(colorG) + "," + to_string(colorB);
		//this->ui->MyTxtBrowse->append(str.c_str());
		// set color entry data
		MyDialog::rootPlane[idx].colorR = colorR;
		MyDialog::rootPlane[idx].colorG = colorG;
		MyDialog::rootPlane[idx].colorB = colorB;
		MyDialog::setCellColor(i);
		cv::Mat src = MyDialog::rootPlane[idx].raw_image;
		MyDialog::rootPlane[idx].image=ColorChange(src, colorR, colorG, colorB);
		MyDialog::mkBaseDataAndView();
	}
	return;
}

// draw color with specified color
void MyDialog::setCellColor(int i)
{
	int colorCol = 5; // color display cell column
	int colorR(0), colorG(0), colorB(0);
	if(MyDialog::Layer[i].isPID){
		colorR = 255;colorG = 255;colorB = 255;
	} else {
		int idx = MyDialog::Layer[i].idx; // set plane daya array index
		colorR = MyDialog::rootPlane[idx].colorR;
		colorG = MyDialog::rootPlane[idx].colorG;
		colorB = MyDialog::rootPlane[idx].colorB;
	}
	QColor cellcolor = QColor();
	cellcolor.setRed(colorR); cellcolor.setGreen(colorG); cellcolor.setBlue(colorB);
	//this->ui->MyTxtBrowse->append("test");
	QPixmap pixmap(100, 24);
	pixmap.fill(cellcolor);
	QLabel *clabel=new QLabel;
	clabel->setPixmap(pixmap);
	this->ui->tableWidget->setCellWidget(i, colorCol, clabel);
	return;
}


// Layer display update (post process after changing layer sequence)
// (Table contents(cell) updating)
void MyDialog::TableLayerUpdate(int i)
{
	// update display of each table
	int nameCol = 0;
	int PIDCol = 1;
	int swCol = 3;
	if (i < 0 || i >= MyDialog::LayerMax) return;	// check layer number
	std::string tiffname = MyDialog::Layer[i].name;
	this->ui->tableWidget->setItem(i, nameCol,new QTableWidgetItem(QString(tiffname.c_str())));
	// ** PID flagg
	if(MyDialog::Layer[i].isPID){
		this->ui->tableWidget->setItem(i, PIDCol, new QTableWidgetItem(QString("*")));
	} else {
		this->ui->tableWidget->setItem(i, PIDCol, new QTableWidgetItem(QString(""))); // set-empty
	}
	// draw switch flagg
	if(MyDialog::Layer[i].use){
		this->ui->tableWidget->setItem(i, swCol, new QTableWidgetItem(QString("ON")));
	} else {
		this->ui->tableWidget->setItem(i, swCol, new QTableWidgetItem(QString("OFF")));
	}
	// empty data case
	if(!MyDialog::Layer[i].dataload) {
		this->ui->tableWidget->setItem(i, nameCol,new QTableWidgetItem(QString("")));
		this->ui->tableWidget->setItem(i, swCol, new QTableWidgetItem(QString("")));
		this->ui->tableWidget->setItem(i, PIDCol, new QTableWidgetItem(QString(""))); // set-empty
	}
	if(MyDialog::Layer[i].name.empty())this->ui->tableWidget->setItem(i, swCol, new QTableWidgetItem(QString("")));

	// ** button activate control
	if(MyDialog::Layer[i].isPID){
		MyDialog::tabbtn[i]->setEnabled(false);
		MyDialog::layercolor[i]->setEnabled(false);
		//this->ui->tableWidget->cellWidget(i, setColorCol)->setEnabled(false);
	} else {
		MyDialog::tabbtn[i]->setEnabled(true);
		MyDialog::layercolor[i]->setEnabled(true);
	}
	MyDialog::setCellColor(i);
	return;
}


// read Layer-image 
void MyDialog::PlaneRead(int i)
{
	if (!MyDialog::rootData.dataload) {
		//MessageBeep(-1); // windows beep
		QMessageBox *msgBox = new QMessageBox(this);
		std::string errortxt="** Error **\n\n First of all, you must load PID image data!";
		msgBox->setText(errortxt.c_str() );
		//this->ui->MyTxtBrowse->textCursor().movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
		msgBox->setDefaultButton(QMessageBox::Ok);
		msgBox->exec();
		return; //PID data を読み込む前はエラーで返す
	}
	if (MyDialog::Layer[i].isPID) {
		this->ui->MyTxtBrowse->append("** Use ReadPID button");
		return;
	}
	// ** PlaneData and LayerData update
	int idx = MyDialog::Layer[i].idx;
	MyDialog::LayerRead(idx); // *** reading image ***
	if (!MyDialog::rootPlane[idx].use) return; // 読み込みが有効でなければそのまま抜ける
	MyDialog::Layer[i].use = true;
	MyDialog::Layer[i].dataload = true;
	MyDialog::rootPlane[idx].LayerID = i;
	MyDialog::Layer[i].name=getImagename(MyDialog::rootPlane[idx].filename); // get filename without path
	// ** table and image update
	MyDialog::TableLayerUpdate(i);// update table widget
	MyDialog::mkBaseDataAndView();
	return;
}


// toggle switch (like a flip-flop circuit)
// 画像描画ON/OFFの切り替え(Table widget)
void MyDialog::DrawSwitch(int num )
{
	int swCol = 3; // draw_switch status column
	if (!MyDialog::Layer[num].dataload) return;
	// toggle switch
	if (MyDialog::Layer[num].use){
		MyDialog::Layer[num].use = false;
		this->ui->tableWidget->item(num, swCol)->setText("OFF");
	} else {
		MyDialog::Layer[num].use = true;
		this->ui->tableWidget->item(num, swCol)->setText("ON");
	}
	//MyDialog::TableLayerUpdate(num);// update table widget
	MyDialog::mkBaseDataAndView();
	return;
}

