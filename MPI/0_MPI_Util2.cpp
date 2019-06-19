// 2018 Konica Minolta MPI utility function
//
#include <thread>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "stdafx.h"
#include "stringConvert.h"
#include "Iniparam.h"
#include "mpi.h"
#include "0_MPI_Util.h"
//#include "mpicxx.h"

using namespace std;
using namespace cv;

// Konica Minolta MPI function for CellDetect

// MPI Send CaffeParam to ALL node
int KM_MPI_SendCaffeParamAll(CaffeParam SendParam, int NodeSize)
{
	// ** 全スレーブノードへ CaffeParam を送信する
#pragma omp parallel sections // OpenMP を使って高速化
	{
#pragma omp parallel for
		for (int i = 1; i < NodeSize; i++){
			KM_MPI_SendCaffeParam(SendParam, i);
		}
	}
	return 0;
}



// MPI_send_caffe_parameter
int KM_MPI_SendCaffeParam(CaffeParam SendParam, int node)
{
	// *** MPI Caffe parameter sending function [KM util]
	// node = destination node
	int     kind = SendParam.kind;
	int     tumor_use = SendParam.tumor_use;
	std::string	modelFileName = ws2s(SendParam.modelFileName);
	std::string	weightsFileName = ws2s(SendParam.weightsFileName);
	std::string	gpuId = ws2s(SendParam.gpuId);
	double	std_threshold[3] = { 0.0, 0.0, 0.0 };
	for (int i = 0; i<3; i++){
		std_threshold[i] = SendParam.std_threshold[i];
	}
	std::string    colorSpaceName = ws2s(SendParam.colorSpaceName);
	double	convert_matrix[9];
	int k = 0; //counter
	for (int i = 0; i<3; i++){
		for (int j = 0; j<3; j++){
			convert_matrix[k] = SendParam.convert_matrix[i][j];
			k++;}
	}
	double	white_level = SendParam.white_level;
	double	stain_th[3] = { 0.0, 0.0, 0.0 };
	for (int i = 0; i<3; i++){
		stain_th[i] = SendParam.stain_th[i];
	}
	int		train_patch_size = SendParam.train_patch_size;
	double  image_scale = SendParam.image_scale;

	// ****** MPI Caffe Parameter sending section ******
	MPI_Send(&kind, 1, MPI_INT, node, 0, MPI_COMM_WORLD);
	MPI_Send(&tumor_use, 1, MPI_INT, node, 0, MPI_COMM_WORLD);
	KM_MPI_stringSend(modelFileName, node);
	KM_MPI_stringSend(weightsFileName, node);
	KM_MPI_stringSend(gpuId, node);
	MPI_Send(&std_threshold[0], 3, MPI_DOUBLE, node, 0, MPI_COMM_WORLD);
	KM_MPI_stringSend(colorSpaceName, node);
	MPI_Send(&convert_matrix[0], 9, MPI_DOUBLE, node, 0, MPI_COMM_WORLD);
	MPI_Send(&white_level, 1, MPI_DOUBLE, node, 0, MPI_COMM_WORLD);
	MPI_Send(&stain_th[0], 3, MPI_DOUBLE, node, 0, MPI_COMM_WORLD);
	MPI_Send(&train_patch_size, 1, MPI_INT, node, 0, MPI_COMM_WORLD);
	MPI_Send(&image_scale, 1, MPI_DOUBLE, node, 0, MPI_COMM_WORLD);
	return 0;
}


// MPI_Recv_caffe_parameter
CaffeParam KM_MPI_RecvCaffeParam(int node)
{
	// RecvParam = MPI recv parameter class
	// node = source node (transmission origin)
	CaffeParam RecvParam; // 受け取ったパラメータの格納先
	MPI_Status status;	// 診断用 MPI status code

	// ******** KM_Caffe work variable for MPI receive ********
	int     kind;
	int     tumor_use;
	std::string	modelFileName;
	std::string	weightsFileName;
	std::string	gpuId;
	double	std_threshold[3] = { 0.0, 0.0, 0.0 };
	std::string    colorSpaceName;
	double	convert_matrix[9];
	double	white_level;
	double	stain_th[3] = { 0.0, 0.0, 0.0 };
	int	train_patch_size;
	double  image_scale;

	// ****** MPI Caffe Parameter Receive section ******
	MPI_Recv(&kind, 1, MPI_INT, node, 0, MPI_COMM_WORLD, &status);
	MPI_Recv(&tumor_use, 1, MPI_INT, node, 0, MPI_COMM_WORLD, &status);
	modelFileName = KM_MPI_stringRecv(node);
	weightsFileName = KM_MPI_stringRecv(node);
	gpuId = KM_MPI_stringRecv(node);
	MPI_Recv(&std_threshold[0], 3, MPI_DOUBLE, node, 0, MPI_COMM_WORLD, &status);
	colorSpaceName = KM_MPI_stringRecv(node);
	MPI_Recv(&convert_matrix[0], 9, MPI_DOUBLE, node, 0, MPI_COMM_WORLD, &status);
	MPI_Recv(&white_level, 1, MPI_DOUBLE, node, 0, MPI_COMM_WORLD, &status);
	MPI_Recv(&stain_th[0], 3, MPI_DOUBLE, node, 0, MPI_COMM_WORLD, &status);
	MPI_Recv(&train_patch_size, 1, MPI_INT, node, 0, MPI_COMM_WORLD, &status);
	MPI_Recv(&image_scale, 1, MPI_DOUBLE, node, 0, MPI_COMM_WORLD, &status);

	// ***** Construct Caffe Param *****
	RecvParam.kind = kind;
	RecvParam.tumor_use = tumor_use;
	RecvParam.modelFileName = s2ws(modelFileName);
	RecvParam.weightsFileName = s2ws(weightsFileName);
	RecvParam.gpuId = s2ws(gpuId);

	for (int i = 0; i<3; i++){
		RecvParam.std_threshold[i] = std_threshold[i];
	}
	RecvParam.colorSpaceName = s2ws(colorSpaceName);
	int k = 0; //counter
	for (int i = 0; i<3; i++){
		for (int j = 0; j<3; j++){
			RecvParam.convert_matrix[i][j] = convert_matrix[k];
			k++;}
	}
	RecvParam.white_level = white_level;
	for (int i = 0; i<3; i++){
		RecvParam.stain_th[i] = stain_th[i];
	}
	RecvParam.train_patch_size = train_patch_size;
	RecvParam.image_scale = image_scale;

	return RecvParam;
}

// 全スレーブノードに PreParam を送信する
int KM_MPI_sendAllPreparam(PreParam sendparam, int NodeSize)
{
#pragma omp parallel sections // OpenMP を使って高速化
	{
#pragma omp parallel for
		for (int i = 1; i < NodeSize; i++){
			KM_MPI_sendPreParam(sendparam, i);// MPI_send (パラメータ送信)
		}
	}
	return 0;
}


// MPI_send_PreParameter
int KM_MPI_sendPreParam(PreParam sendparam, int node)
{
	// *** MPI PreParameter sending function [KM util]
	// node = destination node
	int		active_bit = sendparam.active_bit;
	int     color_flag = sendparam.color_flag;
	double  white_RGB[3];
	for (int i = 0; i < COLOR_CHANNEL; i++)white_RGB[i] = sendparam.white_RGB[i];
	double  convert_matrix[9];
	int k = 0;
	for (int i = 0; i<3; i++){
		for (int j = 0; j<3; j++){
			convert_matrix[k] = sendparam.convert_matrix[i][j];
			k++; }
	}
	double  white_level = sendparam.white_level;
	// ****** MPI sending section ******
	MPI_Send(&active_bit,		1,MPI_INT,		node, 0, MPI_COMM_WORLD);
	MPI_Send(&color_flag,		1,MPI_INT,		node, 0, MPI_COMM_WORLD);
	MPI_Send(&white_RGB[0],		3,MPI_DOUBLE,	node, 0, MPI_COMM_WORLD);
	MPI_Send(&convert_matrix[0],9,MPI_DOUBLE,	node, 0, MPI_COMM_WORLD);
	MPI_Send(&white_level,		1,MPI_DOUBLE,	node, 0, MPI_COMM_WORLD);
	return 0;
}


// MPI-Recv-PreParameter
PreParam KM_MPI_RecvPreParam(int node)
{
	// RecvParam = MPI recv parameter class
	// node = source node (transmission origin)
	PreParam RecvParam; // 受け取ったパラメータの格納先
	MPI_Status status;	// 診断用 MPI status code
	int		active_bit;
	int     color_flag;
	double  white_RGB[COLOR_CHANNEL];
	double  convert_matrix[9];
	double  white_level;

	// ****** MPI Caffe Parameter Receive section ******
	MPI_Recv(&active_bit, 1, MPI_INT, node, 0, MPI_COMM_WORLD, &status);
	MPI_Recv(&color_flag, 1, MPI_INT, node, 0, MPI_COMM_WORLD, &status);
	MPI_Recv(&white_RGB[0], 3, MPI_DOUBLE, node, 0, MPI_COMM_WORLD, &status);
	MPI_Recv(&convert_matrix[0], 9, MPI_DOUBLE, node, 0, MPI_COMM_WORLD, &status);
	MPI_Recv(&white_level, 1, MPI_DOUBLE, node, 0, MPI_COMM_WORLD, &status);

	// ***** Construct Caffe Param *****
	RecvParam.active_bit = active_bit;
	RecvParam.color_flag = color_flag;
	int k = 0; //counter
	for (int i = 0; i<3; i++){
		for (int j = 0; j<3; j++){
			RecvParam.convert_matrix[i][j] = convert_matrix[k];
			k++;}
	}
	RecvParam.white_level = white_level;
	return RecvParam; // ** 直ちに返す
}


// 引数=nul について、後で気がついて慌てて書き足した関数
// 制御パラメータを全ノードに送信
int set_MPI_confirmParameter(DLPostProcParam &param, KM_confirm_stat &root_arg_status, int myid)
{
	// *** ファイル名にnulが指定されていたら、prediction を行わない制御フラグを格納する関数 ***
	// （なぜパラメータチェックの後に再度パラメータチェックがあるか元プログラムに小一時間ほど問い詰めたい）
	bool root_stat=false; // define and initialize
	// ** 細胞核
	if (myid == 0){// ノード判定
		if (param.GetNucleusCaffeParam().modelFileName != L"nul" ||
			param.GetNucleusCaffeParam().weightsFileName != L"nul"){
			root_stat = false; //モデルファイル名かWeightファイル名がnulの時は実行しない
		}
		else root_stat = true;
	}
	MPI_Bcast(&root_stat, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD); // MS_MPI の方言 (他のMPIのときは書き換え)
	if (myid != 0 && root_stat ){// スレーブノードへコピー
		root_arg_status.nucleus = root_stat;
	}

	// ** 染色領域１
	root_stat = false; // initialize
	if (myid == 0){// ノード判定
		if (param.GetStain1CaffeParam().modelFileName != L"nul" ||
			param.GetStain1CaffeParam().weightsFileName != L"nul"){
			root_stat = false; //モデルファイル名かWeightファイル名がnulの時は実行しない
		}
		else root_stat = true;
	}
	MPI_Bcast(&root_stat, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD); // MS_MPI の方言 (他のMPIのときは書き換え)
	if (myid != 0 && root_stat){// スレーブノードへコピー
		root_arg_status.stain1 = root_stat;
	}

	// ** 染色領域２
	root_stat = false; // initialize
	if (myid == 0){// ノード判定
		if (param.GetStain2CaffeParam().modelFileName != L"nul" ||
			param.GetStain2CaffeParam().weightsFileName != L"nul"){
			root_stat = false; //モデルファイル名かWeightファイル名がnulの時は実行しない
		}
		else root_stat = true;
	}
	MPI_Bcast(&root_stat, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD); // MS_MPI の方言 (他のMPIのときは書き換え)
	if (myid != 0 && root_stat){// スレーブノードへコピー
		root_arg_status.stain2 = root_stat;
	}
	return 0; // この関数でエラーは返さない。処理を進めるため何が何でも返さない
}