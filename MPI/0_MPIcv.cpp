//
// 2018 MPI with image util
//
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "stdafx.h"
#include "mpi.h"
#include "0_MPIcv.h"

using namespace std;
using namespace cv;

// ******************************************
// *** MPI function for OpenCV
// ******************************************


// ���ʉ摜�̎󂯎��Ɖ摜�č\��
int KM_MPI_RecvResultNucleus(std::vector<cv::Mat> &NucleusResult, int nrank,int width, int height,
							 std::vector<KM_MPI_NodeData> &nodeInfo)
{
	// prepare result image
	for(int i=0;i<6;i++){
		NucleusResult[i] = cv::Mat::zeros(height, width, CV_8UC3);
	}

	std::vector<cv::Mat> workDlImgs;		    //DL�摜(�זE�j)
	int bx, by;
	for(int i=1;i<nrank;i++){
		if(nodeInfo[i].nucleus){ // i���g�p���Ă���m�[�h�ł���Ό��ʃf�[�^�擾
			KM_MPI_RecvNucleusResult(workDlImgs, i);//�m�[�hi����f�[�^��M
			bx = nodeInfo[i].ix_cell;
			by = nodeInfo[i].iy_cell;
			// �摜�č\��(�^�C���C���[�W�\��t��)
			for(int j=0;j<6;j++){
				workDlImgs[j].copyTo(NucleusResult[j](Rect(bx, by, width, height)));
			}
		}
	}
	return 0;
}

// ������摜�̕������M
int KM_MPI_SendBright2AllNode(cv::Mat img, int nx, int ny, int nrank, 
							  std::vector<KM_MPI_NodeData> &nodeInfo)
{
	//string imagedata = "indata.tif"; //1600x1200

	int slave_node_total = nrank - 1;
	int ntotal_devided_image = nx*ny;


	if (slave_node_total < ntotal_devided_image){
		return -1; // �G���[�P�[�X�F���蓖�Ă�m�[�h��������Ȃ�
	}
	//
	// 4x4 case
	//int nx = 4;
	//int ny = 4;

	int src_width = img.cols;
	int src_height = img.rows;

	int width = src_width / nx;		//�����摜��
	int height = src_height / ny;	//�����摜����
	int current_node = 1; // ���M��m�[�h(slave�Ȃ̂�1����J�n)
	Mat cimage;// ��ƗpMat(�����摜)
	for (int j = 0; j<ny; j++){
		for (int i = 0; i<nx; i++){
			int bx = i*width;
			int by = j*height;
			MatCrop(img, bx, by, width, height, cimage);//�^�C������
			//string outdata = prefix + int2s(j) + string("x") + int2s(i) + ".tif";
			//imwrite(outdata.c_str(), cimage);
			KM_MPI_BrightSend(cimage, current_node);// MPI_Send (������摜���M)
			nodeInfo[current_node].nucleus = true; // �m�[�h�g�p�̃t���O
			nodeInfo[current_node].ix_cell = bx; // �摜�ʒu�i�[
			nodeInfo[current_node].iy_cell = by; // �摜�ʒu�i�[
			current_node++;
		}
	}

	return 0;
}




// �C�ӂ̏ꏊ�̉摜��؂�o���֐�
int MatCrop(cv::Mat image, int startX, int startY, 
			int width, int height, cv::Mat &croppedImage)
{
	// image = source image
	Mat ROI(image, Rect(startX, startY, width, height));

	croppedImage = ROI.clone();// clone the data into output matrix
	//imwrite("newImage.png",croppedImage); // * debug
	return 0;
}

// predict(���F�̈�) ���ʉ摜�̎󂯎��
int KM_MPI_RecvStainResult(std::vector<cv::Mat> &StainResult, int Node)
{
	// Predict�̌��ʓ���ꂽ�摜�� �X���[�u�m�[�h(Node)�����M(Vector<mat>�ɕϊ�)
	// note:���̊֐���OpenMP�K�p�֎~
	// note:StainResult�͏�ʊ֐��Ő錾����������vector�ϐ�
	for (int i = 0; i < 3; i++){ // ���F�̈�͂R�摜
		Mat recv_image = KM_MPI_Mono8Recv(Node);
		StainResult.push_back(recv_image);//�i�[
	}
	return 0;
}


// predict(���F�̈�) ���ʉ摜��e�m�[�h�ɑ���Ԃ�
int KM_MPI_SendBackStainImg(std::vector<cv::Mat> &StainResult)
{
	// Predict�̌��ʓ���ꂽMonochrome 8bit �摜��e�m�[�h�֑���Ԃ�
	// note: ���̊֐���OpenMP�K�p�֎~
	// note:StainResult�͏�ʊ֐��Ńf�[�^�i�[�ς�vector�ϐ�
	for (int i = 0; i<3; i++){// ���F�̈�͂R�摜
		KM_MPI_Mono8Send(StainResult[i], 0);
	}
	return 0;
}


// predict(�זE�j) ���ʉ摜�̎󂯎��
int KM_MPI_RecvNucleusResult(std::vector<cv::Mat> &NucleusResult, int Node)
{
	// Predict�̌��ʓ���ꂽ�摜�� �X���[�u�m�[�h(Node)�����M(Vector<mat>�ɕϊ�)
	// note:���̊֐���OpenMP�K�p�֎~
	// note:NucleusResult�͏�ʊ֐��Ő錾����������vector�ϐ�
	for (int i = 0; i < 6; i++){ // �זE�j�͂U�摜
		Mat recv_image = KM_MPI_Mono8Recv(Node);
		NucleusResult.push_back(recv_image);//�i�[
	}
	return 0;
}

// predict(�זE�j) ���ʉ摜��e�m�[�h�ɑ���Ԃ�
int KM_MPI_SendBackNucleusImg(std::vector<cv::Mat> &NucleusResult)
{
	// Predict�̌��ʓ���ꂽMonochrome 8bit �摜��e�m�[�h�֑���Ԃ�
	// note: ���̊֐���OpenMP�K�p�֎~
	// note:NucleusResult�͏�ʊ֐��Ńf�[�^�i�[�ς�vector�ϐ�
	for(int i=0;i<6;i++){// �זE�j�͂U�摜
		KM_MPI_Mono8Send(NucleusResult[i], 0);
	}
	return 0;
}




// MPI send Mono8Image to AllSlaveNode
int KM_MPI_Mono8SendAll(cv::Mat & mi, int NodeSize)
{
	// �摜���������̂����R�R�ɓ����

#pragma omp parallel // OpenMP ���g���č�����
	{
#pragma omp for
		for (int i = 1; i < NodeSize; i++){
			KM_MPI_Mono8Send(mi, i);
		}
	} // openmp block
	return 0;
}
 
// send monochrome image function
int KM_MPI_Mono8Send(cv::Mat & mi, int dest)
{
	// ** Bright image send function
	int rows = mi.rows;				// height
	int cols = mi.cols;				// width
	int data_size = rows*cols;
	// image depth ���قȂ�Ƃ��́�������������
	vector<uchar> mpidata;// = (uchar*)malloc(data_size);
	mpidata.resize(data_size); //���m�N��1channel
	Mono2mpidata(mi, mpidata.data() );
	MPI_Send(&cols, 1, MPI_INT, dest, 0, MPI_COMM_WORLD); // send width
	MPI_Send(&rows, 1, MPI_INT, dest, 0, MPI_COMM_WORLD); // send height
	MPI_Send(&mpidata[0], data_size, MPI_UNSIGNED_CHAR, dest, 0, MPI_COMM_WORLD); // send image
	//std::free(mpidata);
	return 0;
}

// Konica Minolta MPI utilize function
// MPI_recv_BrightImage
// 
// cv::Mat recv function
cv::Mat KM_MPI_Mono8Recv(int node)
{
	cv::Mat recv_img;
	MPI_Status rstat, status;
	bool do_debug = false; // debug mode 
	int rows = 0;
	int cols = 0;
	int count; // for debugging

	MPI_Recv(&cols, 1, MPI_INT, node, 0, MPI_COMM_WORLD, &rstat); // Get width
	MPI_Recv(&rows, 1, MPI_INT, node, 0, MPI_COMM_WORLD, &rstat); // Get height
	//cout << "MonoRecv: rows=" << rows << endl;
	//cout << "MonoRecv: cols=" << cols << endl;
	int array_size = rows*cols; // ���m�N���P�`�����l��
	vector<uchar>mpidata; //8bit uchar
	mpidata.resize(array_size); // �̈�Z�b�g
	MPI_Recv(&mpidata[0], array_size, MPI_UNSIGNED_CHAR, node, 0, MPI_COMM_WORLD, &status); // send image
	MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &count);
	if (do_debug ) fprintf(stderr, "Received array size:%d\n", count);
	recv_img = Mat(rows, cols, CV_8UC1, (uchar *)mpidata.data() ).clone(); // set Bright image property
	return recv_img;
}

// ���m�N���C���[�W��MPI���M�f�[�^�ɕϊ�
// �g�p����: �o�b�t�@data�T�C�Y���C���[�W�T�C�Y�Ɠ���ł��邱��
int Mono2mpidata(cv::Mat img, uchar data[])
{
	// Monochrome(CV_8UC1) image to MPI-data
	// �����I�� 
	// HighSpeed copy by pointer operation
	// �f�[�^�ɑ΂��A�|�C���^�A�N�Z�X�ō����R�s�[���s��

#pragma omp parallel // OpenMP ���g���č�����
	{
#pragma omp for
		for (int i = 0; i < img.rows; ++i){
#pragma omp critical// �����u���b�N�͕��񉻗}��
			{
				uchar* pixel = img.ptr<uchar>(i);  // point to first color in row
				for (int j = 0; j < img.cols; ++j){
					(*data++) = *pixel++;
				}
			}
		}// openmp inhibition block
	}// openmp block
	return 0;
}


// 16bit ������C���[�W��MPI���M�f�[�^�ɕϊ�
// �g�p����: �o�b�t�@data�T�C�Y���C���[�W�T�C�Y�Ɠ���ł��邱��
int Bright2mpidata(cv::Mat img, ushort data[])
{
	// HighSpeed copy by pointer operation
#pragma omp parallel // OpenMP ���g���č�����
	{
#pragma omp for
		for (int i = 0; i < img.rows; ++i){
#pragma omp critical // �����u���b�N�͕��񉻗}��
			{
				ushort* pixel = img.ptr<ushort>(i);  // point to first color in row
				for (int j = 0; j < img.cols; ++j){
					for (int k = 0; k < img.channels(); ++k){
						(*data++) = *pixel++;
					}
				}
			}
		}
	}
	return 0;
}
// �z��A�N�Z�X
//data[k++] = *pixel++; // B
//data[k++] = *pixel++; // G
//data[k++] = *pixel++; // R

// BrightImage send All(from RootNode)
int KM_MPI_BrightSendALL(cv::Mat & mi, int NodeSize)
{
	// note: ���̊֐��͕��񉻉\
	//
	// �����ɉ摜����������


	// ���݂͓����f�[�^�𑗐M���� // single task 
#pragma omp parallel // OpenMP ���g���č�����
	{
#pragma omp for
		for (int i = 1; i < NodeSize; i++){
			KM_MPI_BrightSend(mi, i);
		}
	}
	return 0;
}




// MPI_send_BrightImage
// 
// cv::Mat send function
int KM_MPI_BrightSend(cv::Mat & mi, int dest)
{
	// ��K�̓f�[�^�Ή� ������ 16bit MPI send function
	int rows = mi.rows;
	int cols = mi.cols;
	int channels = mi.channels();
	int type = mi.type();

	int data_size = rows*cols*channels;
	vector<ushort>mpidata;
	mpidata.resize(data_size); // MPI-data �̈�̓��I�m��
	// ** param print
	//cout << "BrightSend: rows=" << rows << endl;
	//cout << "BrightSend: cols=" << cols << endl;
	//cout << "BrightSend: channels=" << channels << endl;

	Bright2mpidata(mi, mpidata.data()); // �|�C���^���p�̍����R�s�[
	// printf_s("*** BrightImage memcpy done\n");
	MPI_Send(&cols, 1, MPI_INT, dest, 0, MPI_COMM_WORLD); // send width
	MPI_Send(&rows, 1, MPI_INT, dest, 0, MPI_COMM_WORLD); // send height
	MPI_Send(&type, 1, MPI_INT, dest, 0, MPI_COMM_WORLD); // send height
	MPI_Send(&mpidata[0], data_size, MPI_UNSIGNED_SHORT, dest, 0, MPI_COMM_WORLD); // send image
	// printf_s("*** BrightImage sent\n");
	return 0;
}

// Konica Minolta MPI utilize function
// MPI_recv_BrightImage
// 
// cv::Mat recv function
cv::Mat KM_MPI_BrightImageRecv(int node)
{
	// MPI BrightImage receive function(support huge data)
	cv::Mat recv_img;
	Mat retImage;
	MPI_Status rstat, status;
	int rows, cols, type;
	int channels = 3;

	MPI_Recv(&cols, 1, MPI_INT, node, 0, MPI_COMM_WORLD, &rstat); // Get width
	MPI_Recv(&rows, 1, MPI_INT, node, 0, MPI_COMM_WORLD, &rstat); // Get height
	MPI_Recv(&type, 1, MPI_INT, node, 0, MPI_COMM_WORLD, &rstat); // Get height
	int array_size = rows*cols*channels;
	// ** param print
	//cout << "BrightRecv: rows=" << rows << endl;
	//cout << "BrightRecv: cols=" << cols << endl;
	//cout << "BrightRecv: type=" << type << endl;
	//cout << "BrightRecv: channels=" << channels << endl;

	vector<ushort>mpidata;
	mpidata.resize(array_size); // �̈�Z�b�g
	//printf_s("MPI_RCV row:%d col:%d\n", rows, cols);
	MPI_Recv(&mpidata[0], array_size, MPI_UNSIGNED_SHORT, node, 0, MPI_COMM_WORLD, &status); // send image
	//printf_s("*** BrightImage Received\n");
	recv_img = Mat(rows, cols, type, mpidata.data()).clone(); // vector �Ŏ擾����image�ɂ�clone���g��
	return recv_img;
}


// Mat data �����\��
int ShowMatProp(cv::Mat img)
{
	printf_s("w: %d\n", img.cols);
	printf_s("h: %d\n", img.rows);
	printf_s("data_size: %d\n", img.size );
	printf_s("elem_size: %d\n", img.elemSize());
	printf_s("total: %d\n", img.total());
	printf_s("ch:%d\n", img.channels());
	printf_s("depth:%d\n", img.depth());
	printf_s("type:%d\n", img.type());
	ShowMatType(img);
	return 0;
}



// data type check fnc for programming check 
int ShowMatType(cv::Mat inputMat)
{
	int inttype = inputMat.type();

	string r, a;
	uchar depth = inttype & CV_MAT_DEPTH_MASK;
	uchar chans = (uchar)(1 + (inttype >> CV_CN_SHIFT));
	switch (depth) {
	case CV_8U:  r = "8U";   a = "Mat.at<uchar>(y,x)"; break;
	case CV_8S:  r = "8S";   a = "Mat.at<schar>(y,x)"; break;
	case CV_16U: r = "16U";  a = "Mat.at<ushort>(y,x)"; break;
	case CV_16S: r = "16S";  a = "Mat.at<short>(y,x)"; break;
	case CV_32S: r = "32S";  a = "Mat.at<int>(y,x)"; break;
	case CV_32F: r = "32F";  a = "Mat.at<float>(y,x)"; break;
	case CV_64F: r = "64F";  a = "Mat.at<double>(y,x)"; break;
	default:     r = "User"; a = "Mat.at<UKNOWN>(y,x)"; break;
	}
	r += "C";
	r += (chans + '0');
	cout << "Mat is of type " << r << " and should be accessed with " << a << endl;
	return 0;
}


// ** �f�o�b�O�p�r�b�g�ϊ��֐�
cv::Mat Bright2ColorImg(cv::Mat Bright) // debug function
{
	// *** Debug ������摜��8bit�ɕϊ����ăt�@�C���Z�[�u ---//
	Mat imgBrightTmp = Mat(Bright.rows, Bright.cols, CV_8UC3);	// ��Ɨp(����ِ��͖�����摜�ɍ��킹��)
	int activeBit = 12;
	double	imgBit = pow(2.0, (double)activeBit) - 1.0; // �摜���ޯĐ�
	double	coefBit = 255.0 / imgBit;					// �ϊ��W��(��:12bit��8bit�̂Ƃ���255/4095=0.062)
	Bright.convertTo(imgBrightTmp, CV_8U, coefBit);  // �r�b�g�ϊ�
	// imwrite("dbgBright.tif", imgBrightTmp);
	return imgBrightTmp;
}






// ******** �ȉ��͏����ɍ쐬�����摜�ʐM�֐�(����{�������̂ŁA�c���Ēu��)

// ���K�͉摜����M�̐ݒ�
const int UMAXBYTES = 16 * 1024 * 1024;
ushort ubuffer[UMAXBYTES];
// ���K�̓f�[�^�pMPI����M�֐�
// 16bit cv::Mat send function
void umatsnd(const cv::Mat & mi, int dest)
{
	Mat m;
	int rows = mi.rows;
	int cols = mi.cols;
	int type = mi.type();
	int channels = mi.channels();
	memcpy(&ubuffer[0 * sizeof(ushort)], (ushort *)& rows, sizeof(int));
	memcpy(&ubuffer[1 * sizeof(ushort)], (ushort *)& cols, sizeof(int));
	memcpy(&ubuffer[2 * sizeof(ushort)], (ushort *)& type, sizeof(int));
	// See note at end of answer about "bytes" variable below!!!
	int bytespersample = sizeof(short);// 1;	// change if using shorts or floats
	int bytes = mi.rows * mi.cols * channels * bytespersample;
	cout << "matsnd: rows=" << rows << endl;
	cout << "matsnd: cols=" << cols << endl;
	cout << "matsnd: type=" << type << endl;
	cout << "matsnd: channels=" << channels << endl;
	cout << "matsnd: bytes=" << bytes << endl;
	if (!mi.isContinuous()) {
		m = Mat::zeros(mi.rows, mi.cols, CV_8U);
	}
	else {
		m = mi.clone();
	}
	memcpy(&ubuffer[3 * sizeof(ushort)], m.data, bytes);
	MPI_Send(&ubuffer, bytes + 3 * sizeof(ushort), MPI_UNSIGNED_SHORT, dest, 0,
			 MPI_COMM_WORLD);
}

// ���K�̓f�[�^�p
// 16bit Mat data receive function
cv::Mat umatrcv(int src)
{
	MPI_Status status;
	int count, rows, cols, type, channels;
	ushort urows, ucols, utype;
	channels = 3;
	MPI_Recv(&ubuffer, sizeof(ubuffer), MPI_UNSIGNED_SHORT, src, 0,
			 MPI_COMM_WORLD, &status);
	MPI_Get_count(&status, MPI_UNSIGNED_SHORT, &count);
	memcpy((ushort *)& urows, &ubuffer[0 * sizeof(ushort)], sizeof(ushort));
	memcpy((ushort *)& ucols, &ubuffer[1 * sizeof(ushort)], sizeof(ushort));
	memcpy((ushort *)& utype, &ubuffer[2 * sizeof(ushort)], sizeof(ushort));

	rows = (int)urows;
	cols = (int)ucols;
	type = (int)utype;

	cout << "matrcv: Count=" << count << endl;
	cout << "matrcv: rows=" << rows << endl;
	cout << "matrcv: cols=" << cols << endl;
	cout << "matrcv: type=" << type << endl;
	// Make the mat
	Mat received =
		Mat(rows, cols, type, (ushort *)& ubuffer[3 * sizeof(ushort)]);
	return received;
}

const int MAXBYTES = 8 * 1024 * 1024;
uchar buffer[MAXBYTES];

// cv::Mat send function
void matsnd(const cv::Mat & mi, int dest)
{
	Mat m;
	int rows = mi.rows;
	int cols = mi.cols;
	int type = mi.type();
	int channels = mi.channels();
	memcpy(&buffer[0 * sizeof(int)], (uchar *)& rows, sizeof(int));
	memcpy(&buffer[1 * sizeof(int)], (uchar *)& cols, sizeof(int));
	memcpy(&buffer[2 * sizeof(int)], (uchar *)& type, sizeof(int));
	// See note at end of answer about "bytes" variable below!!!
	int bytespersample = sizeof(short);// 1;	// change if using shorts or floats
	int bytes = mi.rows * mi.cols * channels * bytespersample;
	cout << "matsnd: rows=" << rows << endl;
	cout << "matsnd: cols=" << cols << endl;
	cout << "matsnd: type=" << type << endl;
	cout << "matsnd: channels=" << channels << endl;
	cout << "matsnd: bytes=" << bytes << endl;
	if (!mi.isContinuous()) {
		m = Mat::zeros(mi.rows, mi.cols, CV_8U);
	}
	else {
		m = mi.clone();
	}
	memcpy(&buffer[3 * sizeof(int)], m.data, bytes);
	MPI_Send(&buffer, bytes + 3 * sizeof(int), MPI_UNSIGNED_CHAR, dest, 0,
			 MPI_COMM_WORLD);
}


// Mat data receive function
cv::Mat matrcv(int src)
{
	MPI_Status status;
	int count, rows, cols, type, channels;
	channels = 0;
	MPI_Recv(&buffer, sizeof(buffer), MPI_UNSIGNED_CHAR, src, 0,
			 MPI_COMM_WORLD, &status);
	MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &count);
	memcpy((uchar *)& rows, &buffer[0 * sizeof(int)], sizeof(int));
	memcpy((uchar *)& cols, &buffer[1 * sizeof(int)], sizeof(int));
	memcpy((uchar *)& type, &buffer[2 * sizeof(int)], sizeof(int));
	cout << "matrcv: Count=" << count << endl;
	cout << "matrcv: rows=" << rows << endl;
	cout << "matrcv: cols=" << cols << endl;
	cout << "matrcv: type=" << type << endl;
	// Make the mat
	Mat received =
		Mat(rows, cols, type, (uchar *)& buffer[3 * sizeof(int)]);
	return received;
}
