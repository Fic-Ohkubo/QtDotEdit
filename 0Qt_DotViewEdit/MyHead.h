// ÉãÅ[ÉgÉwÉbÉ_
#pragma once
#pragma warning (disable:4996)	//printfÇ»Ç«
#pragma warning (disable:4244)	//å^ïœä∑Ç»Ç«
#pragma warning (disable:4267)	//å^ïœä∑Ç»Ç«
#pragma warning (disable:4819)	//opencv char error

#include <iostream>			// C++ setup

// opencv header
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>

// Qt header group
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <QApplication>
#include <QColor>
#include <QColorDialog>
#include <QGraphicsScene>
#include <QDebug>
#include <QEvent>
#include <QLCDNumber>
#include <QMouseEvent>
#include <QMainWindow>
#include <Qpicture>
#include <QPainter>
#include <QPixmap>
#include <QScrollBar>
#include <QShortcut>
#include <QSignalMapper>
#include <QSizePolicy>
#include <QString>
#include <QStyle>
#include <QThread>
#include <QtDebug>
#include <QTextEdit>


#define NOMINMAX			// reject windows.h max macro 
#include <vector>
#include <algorithm>
#include "tiffio.h" // Handle Tiff file by utilize libtiff
#include <locale>
#include <codecvt>
#include <string>
#include <memory>
#include <time.h>
#include <ctime>
#include <fstream>
#include <Windows.h>

// private header 
#include "ui_mydialog.h"	// UI layout 
#include "mydialog.h"		// UI,data,slot definition
#include "Tool.h"			// original functiuon group
#include "ErrorDef.h"		// Define Error

// parallel environment
// openMP setup
#include <omp.h>
#define OMP_NUM_THREADS 12
#define OMP_NESTED TRUE