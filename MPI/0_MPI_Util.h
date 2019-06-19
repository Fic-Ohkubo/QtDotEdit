//

#include "stdafx.h"
#include "KM_MPI_data.h"
#pragma once




const int KM_MPI_str_len = 2048; // define MAX length of MPI-string buffer

// Konica Minolta MPI utilize function

// 全スレーブノードに PreParam を送信する
int KM_MPI_sendAllPreparam(PreParam sendparam, int NodeSize);

// 全スレーブノードにCaffeParam を送信する
int KM_MPI_SendCaffeParamAll(CaffeParam SendParam, int NodeSize);

// CaffeModelファイルのクローンを作成する
// この関数は全ノードで実行する (** 集団通信を利用 **)
int KM_MPI_CaffeModelClone(CaffeParam param, int myid, int nodeSize);

// Weight(caffemodel)クローンファイルを作成する
// この関数は全ノードで実行する (** 集団通信を利用 **)
int KM_MPI_WeightClone(CaffeParam param, int myid, int subject);


// MPI_comm_function for caffe_parameter
int KM_MPI_SendCaffeParam(CaffeParam SendParam, int node);
CaffeParam KM_MPI_RecvCaffeParam(int node);

// MPI_comm for PreParam
int KM_MPI_sendPreParam(PreParam sendparam, int node);
PreParam KM_MPI_RecvPreParam(int node);

int PrintCparam(CaffeParam param);


// 文字列送受信
int KM_MPI_stringSend(std::string sendstr, int dest);
std::string KM_MPI_stringRecv(int node);

// 引数=nul について、後で気がついて書き足した関数
int set_MPI_confirmParameter(DLPostProcParam &param, KM_confirm_stat &root_arg_status, int myid);