// cv util function for MPI
// 0_MPIcv.h   :2018 Feb 

#include "stdafx.h"
#include "KM_MPI_data.h"
#pragma once

//
// ** Konica Minolta 2018 March
// OpenCV & MPI functions for Parallel-CellDetect
//

// ������摜�𕪉����ăX���[�u�m�[�h�֑��M����
int KM_MPI_SendBright2AllNode(cv::Mat img, int nx, int ny, int nrank);


// �C�ӂ̏ꏊ�̉摜��؂�o���֐�
int MatCrop(cv::Mat image, int startX, int startY,
			int width, int height, cv::Mat &croppedImage);

// predict(���F�̈�) ���ʉ摜�̎󂯎��
int KM_MPI_RecvStainResult(std::vector<cv::Mat> &StainResult, int Node);
// predict(���F�̈�) ���ʉ摜��e�m�[�h�ɑ���Ԃ�
int KM_MPI_SendBackStainImg(std::vector<cv::Mat> &StainResult);

// predict(�זE�j) ���ʉ摜�̎󂯎��
int KM_MPI_RecvNucleusResult(std::vector<cv::Mat> &NucleusResult, int Node);
// predict(�זE�j) ���ʉ摜��e�m�[�h�ɑ���Ԃ�
int KM_MPI_SendBackNucleusImg(std::vector<cv::Mat> &NucleusResult);


int KM_MPI_Mono8Send(cv::Mat & mi, int dest);	// ���m�N���摜���M(��K�̓f�[�^�Ή�)
cv::Mat KM_MPI_Mono8Recv(int node);				// ���m�N���摜��M(��K�̓f�[�^�Ή�)
int Mono2mpidata(cv::Mat img, uchar data[]);	// MPI ���M�f�[�^�փ��m�N���摜�f�[�^��ϊ�
int Bright2mpidata(cv::Mat img, ushort data[]);	// MPI ���M�f�[�^�֖�����摜�f�[�^��ϊ�

// send BrightImage to AllSlave(from RootNode)
int KM_MPI_BrightSendALL(cv::Mat & mi, int NodeSize);
// MPI send Mono8Image to AllSlaveNode
int KM_MPI_Mono8SendAll(cv::Mat & mi, int NodeSize);


int KM_MPI_BrightSend(cv::Mat &mi, int dest);	// ������摜���M(��K�̓f�[�^�Ή�)
cv::Mat KM_MPI_BrightImageRecv(int node);		// ������摜��M(��K�̓f�[�^�Ή�)

int ShowMatProp(cv::Mat img     ); // �摜�����\��
int ShowMatType(cv::Mat inputMat); // �摜type�\��

// ** �f�o�b�O�p�r�b�g�ϊ��֐�
cv::Mat Bright2ColorImg(cv::Mat Bright); // debug function


//���K�̓f�[�^�p(��{�̊֐�)
void umatsnd(const cv::Mat & mi, int dest);
cv::Mat umatrcv(int src);

void matsnd(const cv::Mat & mi, int dest);
cv::Mat matrcv(int src);
