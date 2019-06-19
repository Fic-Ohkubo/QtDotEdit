#include "stdafx.h"
//#include <glog/logging.h>
#include "caffe/llogging_mod.h"
#include "Logger.h"
#include "stringConvert.h"
#include <mpi.h>

using namespace std;

	// 
	// ** note: MPI 並列処理プログラムの鉄則 ** (Ficha Inc. K.Ohkubo)
	//
	//		並列処理プログラムでは 送信と受信の数を必ず合わせる。
	//		node番号の条件以外に絶対に if-block の中に入れてはならない(違反すると必ずデッドロックになって落ちる)
	//		固定長で通信を行い, 送信バッファと受信バッファの大きさのつじつまを合わせる
	//		Multi-GPU の場合、元のコードが許可しているかどうか確かめる。
	//		Caffeの場合、大元のコードが許可しないから Multi-GPU は禁止
	//		注: OpenMP の初期化は stdafx.h の中に書いた
	//


// 元々あったCellDetect のパラメータ
static const int DEFAULT_WIDTH = 1600;
static const int DEFAULT_HEIGHT = 1200;
static const int ARGC_NUM = 7;	//コマンドラインでの引数の総数
static const int MAX_ARG = 30;

//int wmain(int argc, WCHAR* argv[], WCHAR* envp[])
int main(int argc, char *argv[])
{
	int par_rank, par_size;
	int rcode = 0;
	std::string st1, st2, st3, st4, st5, st6, st7;

	//** Init MPI (MPI並列処理の初期化) */
	MPI_Init(&argc, &argv);	// 初期化(プロセスのクローン:必ずmain()の先頭で実行)
	MPI_Comm_rank(MPI_COMM_WORLD, &par_rank); // 自分のノード番号取得
	MPI_Comm_size(MPI_COMM_WORLD, &par_size); // 並列数(ノード数)

	char pname[MPI_MAX_PROCESSOR_NAME]; // マシン名
	int plen = 0;
	memset(pname, 0, sizeof(char)*MPI_MAX_PROCESSOR_NAME);
	MPI_Get_processor_name(pname, &plen);
	int myid = par_rank;
	int NodeSize = par_size;
	std::string myNodeName = string(pname);

	MPI_Barrier(MPI_COMM_WORLD); // Initialize の結果出力
	
	if (par_size < 4) {
		printf("Error: proc size too small\n");
		MPI_Finalize(); // 並列数が足りないときは異常終了させる(並列処理不可ケース)
		return(-999);	// 異常終了値を戻り値にする
	}

	//コマンド読込 (全ノードで行う) (note:引数だけは全ノードに自動的に送られる)
	st1 = string(argv[CELL_ARG_IN_BRIGH]);	//明視野画像の読込
	st2 = string(argv[CELL_ARG_IN_TUMOR]);	//腫瘍領域画像の読込
	st3 = string(argv[CELL_ARG_IN_ROI]);	//ROI画像の読込
	st4 = string(argv[CELL_ARG_OUT_CELL]);	//出力画像の名称読込
	st5 = string(argv[CELL_ARG_IN_INI]);	//INIファイルの読込
	st6 = string(argv[CELL_ARG_IN_LOG]);	//logファイルの出力場所読込の読込

    //UNREFERENCED_PARAMETER(envp);
    _wsetlocale(LC_ALL, L"Japanese");
	// 親ノード(node=0:ルートノード)のみバージョン表示
	if (myid == 0){
		printf_s("software:");
		Logger::GetInstance().WriteLog(SOFT_VERSION);
	}
    // glogの初期化 (念のため全ノードで実行)
    google::InitGoogleLogging("CellDetect.exe");    //ログ出力時のプログラム名を入力（CellDetectではログ出力を行わないので、ソフト名直打ちを仮置き）

    std::wstring input_bFilename;	//明視野画像のファイル名
    std::wstring input_tFilename;	//腫瘍領域画像のファイル名
    std::wstring input_roiFilename;	//ROI画像のファイル名
    std::wstring outputFilename;	//出力画像のファイル名
    std::wstring iniFilename;		//INIファイルのファイル名
    std::wstring logFilename;		//logファイルのファイル名

    //コマンドの引数の数を確認する ( 基本の入出力処理はルートノードのみで実行)
	if (myid == 0){ 
		if (argc != ARGC_NUM){
			Logger::GetInstance().WriteLog(EMSG_ARG);
			Logger::GetInstance().WriteLog(EMSG_ARG_USAGE);
			return ARGUMENT_ERROR;
		}
	}

    //引数変換(wstring convert) (全ノード)
	input_bFilename		= s2ws(st1);
	input_tFilename		= s2ws(st2);
	input_roiFilename	= s2ws(st3);
	outputFilename		= s2ws(st4);
	iniFilename			= s2ws(st5);
	logFilename			= s2ws(st6);

	// ログファイル名を設定 (全ノード)
	Logger::GetInstance().SetFilePath(logFilename.c_str());
	MPI_Barrier(MPI_COMM_WORLD); // 同期を取ってから解析実行
	rcode = func(	// note: func() は全ノードで呼び出す
				brabra,
				myid,
				NodeSize,
				myNodeName
				);
	MPI_Barrier(MPI_COMM_WORLD); // 終了前に同期を取る
	/* Finalize MPI */
	MPI_Finalize(); // 各ノード上のクローンプロセスを成仏させる
	return rcode;	// 正常終了の場合、EXIT_SUCCESSが帰る 
}
