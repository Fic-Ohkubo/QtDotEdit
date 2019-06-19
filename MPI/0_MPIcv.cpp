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


// 結果画像の受け取りと画像再構成
int KM_MPI_RecvResultNucleus(std::vector<cv::Mat> &NucleusResult, int nrank,int width, int height,
							 std::vector<KM_MPI_NodeData> &nodeInfo)
{
	// prepare result image
	for(int i=0;i<6;i++){
		NucleusResult[i] = cv::Mat::zeros(height, width, CV_8UC3);
	}

	std::vector<cv::Mat> workDlImgs;		    //DL画像(細胞核)
	int bx, by;
	for(int i=1;i<nrank;i++){
		if(nodeInfo[i].nucleus){ // iが使用しているノードであれば結果データ取得
			KM_MPI_RecvNucleusResult(workDlImgs, i);//ノードiからデータ受信
			bx = nodeInfo[i].ix_cell;
			by = nodeInfo[i].iy_cell;
			// 画像再構成(タイルイメージ貼り付け)
			for(int j=0;j<6;j++){
				workDlImgs[j].copyTo(NucleusResult[j](Rect(bx, by, width, height)));
			}
		}
	}
	return 0;
}

// 明視野画像の分割送信
int KM_MPI_SendBright2AllNode(cv::Mat img, int nx, int ny, int nrank, 
							  std::vector<KM_MPI_NodeData> &nodeInfo)
{
	//string imagedata = "indata.tif"; //1600x1200

	int slave_node_total = nrank - 1;
	int ntotal_devided_image = nx*ny;


	if (slave_node_total < ntotal_devided_image){
		return -1; // エラーケース：割り当てるノード数が足りない
	}
	//
	// 4x4 case
	//int nx = 4;
	//int ny = 4;

	int src_width = img.cols;
	int src_height = img.rows;

	int width = src_width / nx;		//分割画像幅
	int height = src_height / ny;	//分割画像高さ
	int current_node = 1; // 送信先ノード(slaveなので1から開始)
	Mat cimage;// 作業用Mat(分割画像)
	for (int j = 0; j<ny; j++){
		for (int i = 0; i<nx; i++){
			int bx = i*width;
			int by = j*height;
			MatCrop(img, bx, by, width, height, cimage);//タイル分割
			//string outdata = prefix + int2s(j) + string("x") + int2s(i) + ".tif";
			//imwrite(outdata.c_str(), cimage);
			KM_MPI_BrightSend(cimage, current_node);// MPI_Send (明視野画像送信)
			nodeInfo[current_node].nucleus = true; // ノード使用のフラグ
			nodeInfo[current_node].ix_cell = bx; // 画像位置格納
			nodeInfo[current_node].iy_cell = by; // 画像位置格納
			current_node++;
		}
	}

	return 0;
}




// 任意の場所の画像を切り出す関数
int MatCrop(cv::Mat image, int startX, int startY, 
			int width, int height, cv::Mat &croppedImage)
{
	// image = source image
	Mat ROI(image, Rect(startX, startY, width, height));

	croppedImage = ROI.clone();// clone the data into output matrix
	//imwrite("newImage.png",croppedImage); // * debug
	return 0;
}

// predict(染色領域) 結果画像の受け取り
int KM_MPI_RecvStainResult(std::vector<cv::Mat> &StainResult, int Node)
{
	// Predictの結果得られた画像を スレーブノード(Node)から受信(Vector<mat>に変換)
	// note:この関数はOpenMP適用禁止
	// note:StainResultは上位関数で宣言しただけのvector変数
	for (int i = 0; i < 3; i++){ // 染色領域は３画像
		Mat recv_image = KM_MPI_Mono8Recv(Node);
		StainResult.push_back(recv_image);//格納
	}
	return 0;
}


// predict(染色領域) 結果画像を親ノードに送り返す
int KM_MPI_SendBackStainImg(std::vector<cv::Mat> &StainResult)
{
	// Predictの結果得られたMonochrome 8bit 画像を親ノードへ送り返す
	// note: この関数はOpenMP適用禁止
	// note:StainResultは上位関数でデータ格納済のvector変数
	for (int i = 0; i<3; i++){// 染色領域は３画像
		KM_MPI_Mono8Send(StainResult[i], 0);
	}
	return 0;
}


// predict(細胞核) 結果画像の受け取り
int KM_MPI_RecvNucleusResult(std::vector<cv::Mat> &NucleusResult, int Node)
{
	// Predictの結果得られた画像を スレーブノード(Node)から受信(Vector<mat>に変換)
	// note:この関数はOpenMP適用禁止
	// note:NucleusResultは上位関数で宣言しただけのvector変数
	for (int i = 0; i < 6; i++){ // 細胞核は６画像
		Mat recv_image = KM_MPI_Mono8Recv(Node);
		NucleusResult.push_back(recv_image);//格納
	}
	return 0;
}

// predict(細胞核) 結果画像を親ノードに送り返す
int KM_MPI_SendBackNucleusImg(std::vector<cv::Mat> &NucleusResult)
{
	// Predictの結果得られたMonochrome 8bit 画像を親ノードへ送り返す
	// note: この関数はOpenMP適用禁止
	// note:NucleusResultは上位関数でデータ格納済のvector変数
	for(int i=0;i<6;i++){// 細胞核は６画像
		KM_MPI_Mono8Send(NucleusResult[i], 0);
	}
	return 0;
}




// MPI send Mono8Image to AllSlaveNode
int KM_MPI_Mono8SendAll(cv::Mat & mi, int NodeSize)
{
	// 画像分割をそのうちココに入れる

#pragma omp parallel // OpenMP を使って高速化
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
	// image depth が異なるときは↓ここをいじる
	vector<uchar> mpidata;// = (uchar*)malloc(data_size);
	mpidata.resize(data_size); //モノクロ1channel
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
	int array_size = rows*cols; // モノクロ１チャンネル
	vector<uchar>mpidata; //8bit uchar
	mpidata.resize(array_size); // 領域セット
	MPI_Recv(&mpidata[0], array_size, MPI_UNSIGNED_CHAR, node, 0, MPI_COMM_WORLD, &status); // send image
	MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &count);
	if (do_debug ) fprintf(stderr, "Received array size:%d\n", count);
	recv_img = Mat(rows, cols, CV_8UC1, (uchar *)mpidata.data() ).clone(); // set Bright image property
	return recv_img;
}

// モノクロイメージをMPI送信データに変換
// 使用条件: バッファdataサイズがイメージサイズと同一であること
int Mono2mpidata(cv::Mat img, uchar data[])
{
	// Monochrome(CV_8UC1) image to MPI-data
	// 部分的に 
	// HighSpeed copy by pointer operation
	// データに対し、ポインタアクセスで高速コピーを行う

#pragma omp parallel // OpenMP を使って高速化
	{
#pragma omp for
		for (int i = 0; i < img.rows; ++i){
#pragma omp critical// 内側ブロックは並列化抑制
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


// 16bit 明視野イメージをMPI送信データに変換
// 使用条件: バッファdataサイズがイメージサイズと同一であること
int Bright2mpidata(cv::Mat img, ushort data[])
{
	// HighSpeed copy by pointer operation
#pragma omp parallel // OpenMP を使って高速化
	{
#pragma omp for
		for (int i = 0; i < img.rows; ++i){
#pragma omp critical // 内側ブロックは並列化抑制
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
// 配列アクセス
//data[k++] = *pixel++; // B
//data[k++] = *pixel++; // G
//data[k++] = *pixel++; // R

// BrightImage send All(from RootNode)
int KM_MPI_BrightSendALL(cv::Mat & mi, int NodeSize)
{
	// note: この関数は並列化可能
	//
	// ここに画像分割を入れる


	// 現在は同じデータを送信する // single task 
#pragma omp parallel // OpenMP を使って高速化
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
	// 大規模データ対応 明視野 16bit MPI send function
	int rows = mi.rows;
	int cols = mi.cols;
	int channels = mi.channels();
	int type = mi.type();

	int data_size = rows*cols*channels;
	vector<ushort>mpidata;
	mpidata.resize(data_size); // MPI-data 領域の動的確保
	// ** param print
	//cout << "BrightSend: rows=" << rows << endl;
	//cout << "BrightSend: cols=" << cols << endl;
	//cout << "BrightSend: channels=" << channels << endl;

	Bright2mpidata(mi, mpidata.data()); // ポインタ利用の高速コピー
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
	mpidata.resize(array_size); // 領域セット
	//printf_s("MPI_RCV row:%d col:%d\n", rows, cols);
	MPI_Recv(&mpidata[0], array_size, MPI_UNSIGNED_SHORT, node, 0, MPI_COMM_WORLD, &status); // send image
	//printf_s("*** BrightImage Received\n");
	recv_img = Mat(rows, cols, type, mpidata.data()).clone(); // vector で取得したimageにはcloneを使う
	return recv_img;
}


// Mat data 属性表示
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


// ** デバッグ用ビット変換関数
cv::Mat Bright2ColorImg(cv::Mat Bright) // debug function
{
	// *** Debug 明視野画像を8bitに変換してファイルセーブ ---//
	Mat imgBrightTmp = Mat(Bright.rows, Bright.cols, CV_8UC3);	// 作業用(ﾁｬﾝﾈﾙ数は明視野画像に合わせる)
	int activeBit = 12;
	double	imgBit = pow(2.0, (double)activeBit) - 1.0; // 画像のﾋﾞｯﾄ数
	double	coefBit = 255.0 / imgBit;					// 変換係数(例:12bit→8bitのときは255/4095=0.062)
	Bright.convertTo(imgBrightTmp, CV_8U, coefBit);  // ビット変換
	// imwrite("dbgBright.tif", imgBrightTmp);
	return imgBrightTmp;
}






// ******** 以下は初期に作成した画像通信関数(お手本だったので、残して置く)

// 小規模画像送受信の設定
const int UMAXBYTES = 16 * 1024 * 1024;
ushort ubuffer[UMAXBYTES];
// 小規模データ用MPI送受信関数
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

// 小規模データ用
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
