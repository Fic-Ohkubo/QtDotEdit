// cv util function for MPI
// 0_MPIcv.h   :2018 Feb 

#include "stdafx.h"
#include "KM_MPI_data.h"
#pragma once

//
// ** Konica Minolta 2018 March
// OpenCV & MPI functions for Parallel-CellDetect
//

// 明視野画像を分解してスレーブノードへ送信する
int KM_MPI_SendBright2AllNode(cv::Mat img, int nx, int ny, int nrank);


// 任意の場所の画像を切り出す関数
int MatCrop(cv::Mat image, int startX, int startY,
			int width, int height, cv::Mat &croppedImage);

// predict(染色領域) 結果画像の受け取り
int KM_MPI_RecvStainResult(std::vector<cv::Mat> &StainResult, int Node);
// predict(染色領域) 結果画像を親ノードに送り返す
int KM_MPI_SendBackStainImg(std::vector<cv::Mat> &StainResult);

// predict(細胞核) 結果画像の受け取り
int KM_MPI_RecvNucleusResult(std::vector<cv::Mat> &NucleusResult, int Node);
// predict(細胞核) 結果画像を親ノードに送り返す
int KM_MPI_SendBackNucleusImg(std::vector<cv::Mat> &NucleusResult);


int KM_MPI_Mono8Send(cv::Mat & mi, int dest);	// モノクロ画像送信(大規模データ対応)
cv::Mat KM_MPI_Mono8Recv(int node);				// モノクロ画像受信(大規模データ対応)
int Mono2mpidata(cv::Mat img, uchar data[]);	// MPI 送信データへモノクロ画像データを変換
int Bright2mpidata(cv::Mat img, ushort data[]);	// MPI 送信データへ明視野画像データを変換

// send BrightImage to AllSlave(from RootNode)
int KM_MPI_BrightSendALL(cv::Mat & mi, int NodeSize);
// MPI send Mono8Image to AllSlaveNode
int KM_MPI_Mono8SendAll(cv::Mat & mi, int NodeSize);


int KM_MPI_BrightSend(cv::Mat &mi, int dest);	// 明視野画像送信(大規模データ対応)
cv::Mat KM_MPI_BrightImageRecv(int node);		// 明視野画像受信(大規模データ対応)

int ShowMatProp(cv::Mat img     ); // 画像属性表示
int ShowMatType(cv::Mat inputMat); // 画像type表示

// ** デバッグ用ビット変換関数
cv::Mat Bright2ColorImg(cv::Mat Bright); // debug function


//小規模データ用(手本の関数)
void umatsnd(const cv::Mat & mi, int dest);
cv::Mat umatrcv(int src);

void matsnd(const cv::Mat & mi, int dest);
cv::Mat matrcv(int src);
