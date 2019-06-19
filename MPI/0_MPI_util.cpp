//
// 2018 MPI utilize function
//

#include <thread>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "stdafx.h"
#include <locale>
#include <codecvt>
#include "Iniparam.h"

//#include "CellDetect.h"
#include "stringConvert.h"
#include "Iniparam.h"
#include "mpi.h"
#include "0_MPI_Util.h"

//#include "mpicxx.h"

using namespace std;
using namespace cv;

//	*** Konica Minolta MPI utility function lib **
//	
//	program note:	MPI通信では** wstring 型は使えない** (言語国際対応無し、全てマルチバイトで処理する)
//					必ずchar型に変換して通信する
//

// Weight(caffemodel)クローンファイルを作成する
// この関数は全ノードで実行する (** 集団通信を利用 **)
int KM_MPI_WeightClone(CaffeParam param, int myid, int subject)
{
	// ** model-file (protobuf-text) clone **
	std::vector<char> streamBuffer;
	ifstream ifs;	// ** ストリームの宣言は全ノードでやっておく
	ofstream ofs;	// ** ストリームの宣言は全ノードでやっておく
	string textline;
	long bsize;

	// ** ファイルオープン
	if (myid == 0){
		ifs.open(param.weightsFileName ,std::ifstream::binary);	// 入力(バイナリーファイル)
		//wprintf_s(L"%s open\n", param.weightsFileName.c_str()); // debug
	}else{
		string cloneFile = ws2s(NodeWeightFile(subject, myid)); //クローンファイル名設定(重要)
		ofs.open(cloneFile, std::ofstream::binary);	// バイナリーファイル
	}

	if (myid == 0){
		ifs.seekg(0, ifs.end);
		bsize = (long)ifs.tellg(); // 読込みのため終了位置取得
		ifs.seekg(0);
	}
	MPI_Bcast(&bsize, 1, MPI_LONG, 0, MPI_COMM_WORLD); // ファイルサイズを全スレーブに送信
	streamBuffer.reserve(bsize);	// 全領域でバッファサイズ確保
	if(myid==0) ifs.read(streamBuffer.data(), bsize);
	MPI_Barrier(MPI_COMM_WORLD);	// 同期を待つ(要らないかも)

	MPI_Bcast(&streamBuffer[0], bsize, MPI_CHAR, 0, MPI_COMM_WORLD); // データを全スレーブに送信
	//MPI_Bcast(&streamBuffer[0], bsize*sizeof(char), MPI_BYTE, 0, MPI_COMM_WORLD); // (low speed)
	if (myid != 0)	{
		ofs.write(streamBuffer.data(), bsize);
		ofs.close(); //クローンファイル close
	}
	return 0;
}


// CaffeModel(prototext)クローンファイルを作成する
// この関数は全ノードで実行する (** 集団通信を利用 **)
int KM_MPI_CaffeModelClone(CaffeParam param, int myid, int subject)
{
	// ** model-file (protobuf-text) clone **
	char buff[2048]; // 送受信バッファ(全ノードで使う) 最大長は2040文字とする
	std::vector<std::string> streamBuffer;
	ifstream ifs;	// ** ストリームの宣言は全ノードでやっておく
	ofstream ofs;	// ** ストリームの宣言は全ノードでやっておく
	string textline;
	int col;

	// ** ファイルオープン
	if(myid==0){ 
		ifs.open(param.modelFileName);	// 入力
		//wprintf_s(L"%s open\n", param.modelFileName.c_str()); //**debug
	}else{ 
		string cloneFile = ws2s(NodeModelFile(subject, myid)); //クローンファイル名設定（重要）
		ofs.open(cloneFile);
	}

	if (myid == 0){
		while (getline(ifs,textline)){
			textline.erase(remove(textline.begin(), textline.end(), L'\n'), textline.end());// 改行削除
			//wprintf_s(L"%s\n",txtline.c_str()); //** debug **
			if (textline.empty()) continue;	// 空打ち行チェック
			streamBuffer.push_back(textline);
		}
		ifs.close();
	}
	MPI_Barrier(MPI_COMM_WORLD); // ルートノードの読み込みを待つ(要らないかも)
	int lines = (int)streamBuffer.size();
	MPI_Bcast(&lines,  1, MPI_INT, 0, MPI_COMM_WORLD); // 行数を全スレーブに送信

	for(int i=0;i<lines;i++){
		if(myid==0) {
			sprintf_s(buff, "%s", streamBuffer[i].c_str());
			col = (int)streamBuffer[i].length();
		}
		MPI_Bcast(&buff[0], 2048, MPI_CHAR, 0, MPI_COMM_WORLD); // 内容を全スレーブに送信
		textline = string(buff)+string("\n");
		if(myid!=0)	ofs.write(textline.c_str(),textline.size());	//全スレーブ上で書き込み
	}

	if(myid!=0)	ofs.close(); //クローンファイル close
	return 0;
}

// 送受信パラメータ内容確認用の関数(for debugging) ** 消さない方が良い
int PrintCparam(CaffeParam param)
{
	printf_s("*** print CaffeParam\n");
	printf_s("kind				=%d\n", param.kind);
	printf_s("tumore_use		=%d\n", param.tumor_use);
	wprintf_s(L"model			=[%s]\n",param.modelFileName.c_str());
	wprintf_s(L"weight			=[%s]\n", param.weightsFileName.c_str());
	wprintf_s(L"GPUid			=[%s]\n", param.gpuId.c_str());
	printf_s("std_threshold[]=\n");
	for (int i = 0; i < 3; i++) printf_s(" %+3.4f", param.stain_th[i]);
	wprintf_s(L"\nColorSpace	=[%s]\n", param.colorSpaceName.c_str());
	printf_s("convert_matrix=\n");
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++) printf_s(" %+4.4f", param.convert_matrix[i][j]);
		printf_s("\n");
	}
	printf_s("white_level		=%.3f\n",param.white_level);
	printf_s("stain_th			=\n");
	for (int i = 0; i < 3; i++) printf_s(" %3.4f", param.stain_th[i]);
	printf_s("\ntrain_patch_size	=%d\n", param.train_patch_size);
	printf_s("image_scale		=%.3f\n", param.image_scale);
	return 0;
}


// ** MPI string utility **
// ** string send
int KM_MPI_stringSend(std::string sendstr, int dest)
{
	// ** KM MPI string send function
	// dest = node number of send-destination
	//MPI_Request MPIreq;
	char sendarray[KM_MPI_str_len]; // Max length will be set as 2048
	int strlen = (int)sendstr.length();	// ** store string-length
	//printf_s("*** DEBUG:%s\n", sendstr.c_str());
	sprintf_s(sendarray, "%s", sendstr.c_str());	// ** string copy to fix-length data
	sendarray[sendstr.length() + 1] = '\0'; 			// write string-end as undertaking
	//printf_s("*** DEBUG:%s\n", sendarray);
	MPI_Send(&strlen, 1, MPI_INT, dest, 0, MPI_COMM_WORLD); // send string length
	MPI_Send(&sendarray[0], KM_MPI_str_len, MPI_BYTE, dest, 0, MPI_COMM_WORLD);
	
	return 0; // do not return err_code for avoid lock
}


// ** string receive
std::string KM_MPI_stringRecv(int node)
{
	MPI_Status status;
	char rString[KM_MPI_str_len]; // receive string 
	char workStr[KM_MPI_str_len];
	int rstringLength, rcount;

	MPI_Recv(&rstringLength, 1, MPI_INT, node, 0, MPI_COMM_WORLD, &status); // Get string length
	MPI_Recv(&rString, KM_MPI_str_len, MPI_BYTE, node, 0, MPI_COMM_WORLD, &status);	// Get String as fixed-size data
	strncpy_s(workStr, _countof(workStr), rString, rstringLength);
	MPI_Get_count(&status, MPI_BYTE, &rcount); // check receive data number

	if (rcount != KM_MPI_str_len) fprintf_s(stderr, "ERR: string %d\n", rcount);
	std::string rcvstring = string(workStr);
	return rcvstring;
}


cv::Mat vec2flat(vector<cv::Mat> vm) // input
{
	Mat flat;       // output

	for (size_t i = 0; i < vm.size(); i++)
	{
		// you said, those would be 1xM, but let's make sure.
		flat.push_back(vm[i].reshape(1, 1));
	}

	flat = flat.reshape(1, 1); // all in one row, 1xN*M now
	return flat;
}


//  ******* MPI Data Type Index *******
//	MPI_SHORT			short int
//	MPI_INT				int
//	MPI_LONG			long int
//	MPI_LONG_LONG		long long int	(倍々精度)

//	MPI_UNSIGNED_CHAR	unsigned char
//	MPI_UNSIGNED_SHORT	unsigned short int
//	MPI_UNSIGNED		unsigned int
//	MPI_UNSIGNED_LONG	unsigned long int

//	MPI_UNSIGNED_LONG_LONG	unsigned long long int (倍々精度)
//	MPI_FLOAT			float
//	MPI_DOUBLE			double
//	MPI_LONG_DOUBLE		long double
//	MPI_BYTE			char


// UTF8 file read(後の拡張のため残した関数)
int UTFfileRead(
	std::wstring filename,
	std::vector<std::wstring> &streamlines)
{
	wifstream wifs;
	wstring txtline;
	string sbuf;
	int line_count = 0;
	string sfilename = ws2s(filename);

	wifs.open(filename);
	if (!wifs.is_open())
	{
		wcerr << L"Unable to open file" << endl;
		return -1;
	}
	// We are going to read an UTF-8 file
	wifs.imbue(locale(wifs.getloc(), new codecvt_utf8<wchar_t, 0x10ffff, consume_header>()));
	while (getline(wifs, txtline)){
		txtline.erase(remove(txtline.begin(), txtline.end(), L'\n'), txtline.end());// 改行削除
		//wprintf_s(L"%s\n",txtline.c_str()); //** debug **
		if (txtline.empty()) continue;	// 空打ち行チェック
		streamlines.push_back(txtline);
		line_count++;
	}
	wifs.close();
	return line_count; //normal end
}