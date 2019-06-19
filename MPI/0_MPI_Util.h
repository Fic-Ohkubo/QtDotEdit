//

#include "stdafx.h"
#include "KM_MPI_data.h"
#pragma once




const int KM_MPI_str_len = 2048; // define MAX length of MPI-string buffer

// Konica Minolta MPI utilize function

// �S�X���[�u�m�[�h�� PreParam �𑗐M����
int KM_MPI_sendAllPreparam(PreParam sendparam, int NodeSize);

// �S�X���[�u�m�[�h��CaffeParam �𑗐M����
int KM_MPI_SendCaffeParamAll(CaffeParam SendParam, int NodeSize);

// CaffeModel�t�@�C���̃N���[�����쐬����
// ���̊֐��͑S�m�[�h�Ŏ��s���� (** �W�c�ʐM�𗘗p **)
int KM_MPI_CaffeModelClone(CaffeParam param, int myid, int nodeSize);

// Weight(caffemodel)�N���[���t�@�C�����쐬����
// ���̊֐��͑S�m�[�h�Ŏ��s���� (** �W�c�ʐM�𗘗p **)
int KM_MPI_WeightClone(CaffeParam param, int myid, int subject);


// MPI_comm_function for caffe_parameter
int KM_MPI_SendCaffeParam(CaffeParam SendParam, int node);
CaffeParam KM_MPI_RecvCaffeParam(int node);

// MPI_comm for PreParam
int KM_MPI_sendPreParam(PreParam sendparam, int node);
PreParam KM_MPI_RecvPreParam(int node);

int PrintCparam(CaffeParam param);


// �����񑗎�M
int KM_MPI_stringSend(std::string sendstr, int dest);
std::string KM_MPI_stringRecv(int node);

// ����=nul �ɂ��āA��ŋC�����ď����������֐�
int set_MPI_confirmParameter(DLPostProcParam &param, KM_confirm_stat &root_arg_status, int myid);