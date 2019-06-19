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
	// ** �S�X���[�u�m�[�h�� CaffeParam �𑗐M����
#pragma omp parallel sections // OpenMP ���g���č�����
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
	CaffeParam RecvParam; // �󂯎�����p�����[�^�̊i�[��
	MPI_Status status;	// �f�f�p MPI status code

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

// �S�X���[�u�m�[�h�� PreParam �𑗐M����
int KM_MPI_sendAllPreparam(PreParam sendparam, int NodeSize)
{
#pragma omp parallel sections // OpenMP ���g���č�����
	{
#pragma omp parallel for
		for (int i = 1; i < NodeSize; i++){
			KM_MPI_sendPreParam(sendparam, i);// MPI_send (�p�����[�^���M)
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
	PreParam RecvParam; // �󂯎�����p�����[�^�̊i�[��
	MPI_Status status;	// �f�f�p MPI status code
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
	return RecvParam; // ** �����ɕԂ�
}


// ����=nul �ɂ��āA��ŋC�����čQ�Ăď����������֐�
// ����p�����[�^��S�m�[�h�ɑ��M
int set_MPI_confirmParameter(DLPostProcParam &param, KM_confirm_stat &root_arg_status, int myid)
{
	// *** �t�@�C������nul���w�肳��Ă�����Aprediction ���s��Ȃ�����t���O���i�[����֐� ***
	// �i�Ȃ��p�����[�^�`�F�b�N�̌�ɍēx�p�����[�^�`�F�b�N�����邩���v���O�����ɏ��ꎞ�Ԃقǖ₢�l�߂����j
	bool root_stat=false; // define and initialize
	// ** �זE�j
	if (myid == 0){// �m�[�h����
		if (param.GetNucleusCaffeParam().modelFileName != L"nul" ||
			param.GetNucleusCaffeParam().weightsFileName != L"nul"){
			root_stat = false; //���f���t�@�C������Weight�t�@�C������nul�̎��͎��s���Ȃ�
		}
		else root_stat = true;
	}
	MPI_Bcast(&root_stat, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD); // MS_MPI �̕��� (����MPI�̂Ƃ��͏�������)
	if (myid != 0 && root_stat ){// �X���[�u�m�[�h�փR�s�[
		root_arg_status.nucleus = root_stat;
	}

	// ** ���F�̈�P
	root_stat = false; // initialize
	if (myid == 0){// �m�[�h����
		if (param.GetStain1CaffeParam().modelFileName != L"nul" ||
			param.GetStain1CaffeParam().weightsFileName != L"nul"){
			root_stat = false; //���f���t�@�C������Weight�t�@�C������nul�̎��͎��s���Ȃ�
		}
		else root_stat = true;
	}
	MPI_Bcast(&root_stat, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD); // MS_MPI �̕��� (����MPI�̂Ƃ��͏�������)
	if (myid != 0 && root_stat){// �X���[�u�m�[�h�փR�s�[
		root_arg_status.stain1 = root_stat;
	}

	// ** ���F�̈�Q
	root_stat = false; // initialize
	if (myid == 0){// �m�[�h����
		if (param.GetStain2CaffeParam().modelFileName != L"nul" ||
			param.GetStain2CaffeParam().weightsFileName != L"nul"){
			root_stat = false; //���f���t�@�C������Weight�t�@�C������nul�̎��͎��s���Ȃ�
		}
		else root_stat = true;
	}
	MPI_Bcast(&root_stat, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD); // MS_MPI �̕��� (����MPI�̂Ƃ��͏�������)
	if (myid != 0 && root_stat){// �X���[�u�m�[�h�փR�s�[
		root_arg_status.stain2 = root_stat;
	}
	return 0; // ���̊֐��ŃG���[�͕Ԃ��Ȃ��B������i�߂邽�߉������ł��Ԃ��Ȃ�
}