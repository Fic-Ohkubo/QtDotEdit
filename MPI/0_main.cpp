#include "stdafx.h"
//#include <glog/logging.h>
#include "caffe/llogging_mod.h"
#include "Logger.h"
#include "stringConvert.h"
#include <mpi.h>

using namespace std;

	// 
	// ** note: MPI ���񏈗��v���O�����̓S�� ** (Ficha Inc. K.Ohkubo)
	//
	//		���񏈗��v���O�����ł� ���M�Ǝ�M�̐���K�����킹��B
	//		node�ԍ��̏����ȊO�ɐ�΂� if-block �̒��ɓ���Ă͂Ȃ�Ȃ�(�ᔽ����ƕK���f�b�h���b�N�ɂȂ��ė�����)
	//		�Œ蒷�ŒʐM���s��, ���M�o�b�t�@�Ǝ�M�o�b�t�@�̑傫���̂��܂����킹��
	//		Multi-GPU �̏ꍇ�A���̃R�[�h�������Ă��邩�ǂ����m���߂�B
	//		Caffe�̏ꍇ�A�匳�̃R�[�h�������Ȃ����� Multi-GPU �͋֎~
	//		��: OpenMP �̏������� stdafx.h �̒��ɏ�����
	//


// ���X������CellDetect �̃p�����[�^
static const int DEFAULT_WIDTH = 1600;
static const int DEFAULT_HEIGHT = 1200;
static const int ARGC_NUM = 7;	//�R�}���h���C���ł̈����̑���
static const int MAX_ARG = 30;

//int wmain(int argc, WCHAR* argv[], WCHAR* envp[])
int main(int argc, char *argv[])
{
	int par_rank, par_size;
	int rcode = 0;
	std::string st1, st2, st3, st4, st5, st6, st7;

	//** Init MPI (MPI���񏈗��̏�����) */
	MPI_Init(&argc, &argv);	// ������(�v���Z�X�̃N���[��:�K��main()�̐擪�Ŏ��s)
	MPI_Comm_rank(MPI_COMM_WORLD, &par_rank); // �����̃m�[�h�ԍ��擾
	MPI_Comm_size(MPI_COMM_WORLD, &par_size); // ����(�m�[�h��)

	char pname[MPI_MAX_PROCESSOR_NAME]; // �}�V����
	int plen = 0;
	memset(pname, 0, sizeof(char)*MPI_MAX_PROCESSOR_NAME);
	MPI_Get_processor_name(pname, &plen);
	int myid = par_rank;
	int NodeSize = par_size;
	std::string myNodeName = string(pname);

	MPI_Barrier(MPI_COMM_WORLD); // Initialize �̌��ʏo��
	
	if (par_size < 4) {
		printf("Error: proc size too small\n");
		MPI_Finalize(); // ���񐔂�����Ȃ��Ƃ��ُ͈�I��������(���񏈗��s�P�[�X)
		return(-999);	// �ُ�I���l��߂�l�ɂ���
	}

	//�R�}���h�Ǎ� (�S�m�[�h�ōs��) (note:���������͑S�m�[�h�Ɏ����I�ɑ�����)
	st1 = string(argv[CELL_ARG_IN_BRIGH]);	//������摜�̓Ǎ�
	st2 = string(argv[CELL_ARG_IN_TUMOR]);	//��ᇗ̈�摜�̓Ǎ�
	st3 = string(argv[CELL_ARG_IN_ROI]);	//ROI�摜�̓Ǎ�
	st4 = string(argv[CELL_ARG_OUT_CELL]);	//�o�͉摜�̖��̓Ǎ�
	st5 = string(argv[CELL_ARG_IN_INI]);	//INI�t�@�C���̓Ǎ�
	st6 = string(argv[CELL_ARG_IN_LOG]);	//log�t�@�C���̏o�͏ꏊ�Ǎ��̓Ǎ�

    //UNREFERENCED_PARAMETER(envp);
    _wsetlocale(LC_ALL, L"Japanese");
	// �e�m�[�h(node=0:���[�g�m�[�h)�̂݃o�[�W�����\��
	if (myid == 0){
		printf_s("software:");
		Logger::GetInstance().WriteLog(SOFT_VERSION);
	}
    // glog�̏����� (�O�̂��ߑS�m�[�h�Ŏ��s)
    google::InitGoogleLogging("CellDetect.exe");    //���O�o�͎��̃v���O����������́iCellDetect�ł̓��O�o�͂��s��Ȃ��̂ŁA�\�t�g�����ł������u���j

    std::wstring input_bFilename;	//������摜�̃t�@�C����
    std::wstring input_tFilename;	//��ᇗ̈�摜�̃t�@�C����
    std::wstring input_roiFilename;	//ROI�摜�̃t�@�C����
    std::wstring outputFilename;	//�o�͉摜�̃t�@�C����
    std::wstring iniFilename;		//INI�t�@�C���̃t�@�C����
    std::wstring logFilename;		//log�t�@�C���̃t�@�C����

    //�R�}���h�̈����̐����m�F���� ( ��{�̓��o�͏����̓��[�g�m�[�h�݂̂Ŏ��s)
	if (myid == 0){ 
		if (argc != ARGC_NUM){
			Logger::GetInstance().WriteLog(EMSG_ARG);
			Logger::GetInstance().WriteLog(EMSG_ARG_USAGE);
			return ARGUMENT_ERROR;
		}
	}

    //�����ϊ�(wstring convert) (�S�m�[�h)
	input_bFilename		= s2ws(st1);
	input_tFilename		= s2ws(st2);
	input_roiFilename	= s2ws(st3);
	outputFilename		= s2ws(st4);
	iniFilename			= s2ws(st5);
	logFilename			= s2ws(st6);

	// ���O�t�@�C������ݒ� (�S�m�[�h)
	Logger::GetInstance().SetFilePath(logFilename.c_str());
	MPI_Barrier(MPI_COMM_WORLD); // ����������Ă����͎��s
	rcode = func(	// note: func() �͑S�m�[�h�ŌĂяo��
				brabra,
				myid,
				NodeSize,
				myNodeName
				);
	MPI_Barrier(MPI_COMM_WORLD); // �I���O�ɓ��������
	/* Finalize MPI */
	MPI_Finalize(); // �e�m�[�h��̃N���[���v���Z�X�𐬕�������
	return rcode;	// ����I���̏ꍇ�AEXIT_SUCCESS���A�� 
}
