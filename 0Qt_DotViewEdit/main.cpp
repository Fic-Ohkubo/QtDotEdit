#include "MyHead.h"

// * 
// * File:   main.cpp
// * Author: konicaminolta
// *
// * Created on 2018/08/02, 10:46

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	// a.setStyle("motif");
    MyDialog w;
    w.show();

    return a.exec();
}
