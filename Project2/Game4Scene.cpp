#include "GameMain.h"
#include "GameSceneMain.h"
#include "Game4Scene.h"
#include <math.h>
#include <stdio.h>
#include <string>

int SqBoardA[12][12];  //�t�B�[���h��12�~12�}�X
//�Ȃ��A�Ֆʏ�̃f�[�^�́F0=�u����Ă��Ȃ��@1=���@2=��

std::string msg2;	  //���b�Z�[�W�i�[�p�ϐ�
int msg_wait2;		  //���b�Z�[�W�\���̎���

int CurrentRound = 1;     //���E���h�Ǘ��p�ϐ�
int excuted = 0;        //BGM���[�v�΍��p�ϐ�

//�e��t�H���g�ݒ�
unsigned int ColorWhite2 = GetColor(255, 255, 255);
unsigned int ColorRed2 = GetColor(255, 0, 0);
unsigned int ColorSky2 = GetColor(40, 235, 255);

//�O����`(GameMain.cpp�ɂĐ錾)
extern int Input, EdgeInput;

// �V�[���J�n�O�̏��������s��
BOOL initGame4Scene(void)
{
	return TRUE;
}

// �w�肵���ʒu�ɃR�}��u��
int putPiece2(int x, int y, int turn, bool put_flag)
{
	//X,Y���R�}��u�����W�@TURN�����ԁi1�͍��A2�͔��j
	//PUT_FLAG���u���邩�̊m�F����FALSE�A�u���Ȃ��TRUE

	//�Ȃ��A�߂�l�͗��Ԃ����R�}�̐��B
	//���Ő錾�Ə��������s��
	int sum = 0;

	//�u����ꏊ�ɃR�}������΁A���̎��_�Ŗ߂�
	if (SqBoardA[y][x] > 0) return 0;

	//�u�����ꏊ����㉺���E�A�΂߂�8�����ɔՖʂ��`�F�b�N���Ă���
	//dx, dy�̓`�F�b�N�������������
	for (int dy = -1; dy <= 1; dy++) for (int dx = -1; dx <= 1; dx++)
	{
		//���Ԃ����Ƃ��ł���G�R�}�̈ʒu���ꎞ�i�[���Ă����z��
		int wx[12] = { 0 }, wy[12] = { 0 };

		for (int wn = 0;; wn++)
		{
			//kx, ky�Ń`�F�b�N����ꏊ������
			int kx = x + dx * (wn + 1); int ky = y + dy * (wn + 1);

			//�`�F�b�N�ʒu���Ֆʂ���͂ݏo������A�󂫏�Ԃ̏ꍇ�͗��Ԃ��Ȃ��̂Ń��[�v�E�o
			if (kx < 0 || kx > 11 || ky < 0 || ky > 11 || SqBoardA[ky][kx] == 0) break;

			//�Ԃɋ��܂ꂽ�R�}�����ۂɗ��Ԃ�
			//���Ԃ������̍��v��SUM�ɉ��Z�����
			if (SqBoardA[ky][kx] == turn)
			{
				if (put_flag) for (int i = 0; i < wn; i++) SqBoardA[wy[i]][wx[i]] = turn;

				sum += wn;
				break;
			}

			//�G�R�}�Ȃ�Η��Ԃ���̂ŁA�ꎞ�I�Ɉʒu���i�[���Ă���
			wx[wn] = kx; wy[wn] = ky;
		}
	}

	if (sum > 0 && put_flag) SqBoardA[y][x] = turn;

	return sum;
}

// �p�X�`�F�b�N���s��
bool isPass2(int turn)
//�^�[�����s���̃v���C���[�ɉ����đΉ��R�}���`�F�b�N����F1=���A2=��
{
	for (int y = 0; y < 12; y++) for (int x = 0; x < 12; x++) //�u����}�X�����邩�ǂ���������
	{
		if (putPiece2(x, y, turn, false)) return false;      //�p�X�s�v�Ȃ��FALSE���Ԃ����
	}
	return true;  //�p�X�Ȃ��TRUE���Ԃ����
}

/*

�v�l���[�`���ɂ��Ă̋��ʍ�

�����FTURN��1�Ȃ�΍��A2�Ȃ�Δ��̃^�[���ɂȂ�B
�߂�l�F�R�}��u�����Ȃ��TRUE�A�R�}��u���Ă��Ȃ����FALSE

*/

// �v�l���[�`��1 �v���C���[�̑���
bool think01(int turn)
{
	static bool mouse_flag = false;         //�N���b�N����Ă��邩�̃t���O

	if (GetMouseInput() & MOUSE_INPUT_LEFT) //�}�E�X���N���b�N�����m�����ꍇ
	{
		if (!mouse_flag)
		{
			mouse_flag = true;   //�t���O�𗧂Ă�

			int mx, my;

			GetMousePoint(&mx, &my);  //�}�E�X�|�C���^�[�̏ꏊ���擾

			//�|�C���^�[�̂���ꏊ�̃}�X�ɒu��
			if (putPiece2(mx / 48, my / 48, turn, true)) return true;
		}
	}
	else mouse_flag = false;  //�����łȂ���΃t���O�͂��܂�

	return false;
}

// �v�l���[�`��2 �ł���������Ƃ���ɒu��
bool think02(int turn)
{
	//MAX���擾�ł���R�}�̍ő吔�@wx, wy����ԑ�������R�}��u���ꏊ
	int max = 0, wx, wy;

	for (int y = 0; y < 12; y++) for (int x = 0; x < 12; x++)
	{
		//�Ֆʂ����Ƀ`�F�b�N���āA�擾�\�ȃR�}���𓾂�
		int num = putPiece2(x, y, turn, false);

		//����R�}�����݂̍ő�����傫����Ζ���������ւ��B
		//��������΁A1/2�̊m���œ���ւ���B
		//����̌J��Ԃ��ŁA��ԑ�������ꏊ���i�荞��ł����B
		if (max < num || (max == num && GetRand(1) == 0))
		{
			max = num; wx = x; wy = y;
		}
	}
	putPiece2(wx, wy, turn, true);
	return true;
}

// ���b�Z�[�W�Z�b�g����

// turn ... 1:BLACK 2:WHITE 3:DRAW�@1�ō��ɑ΂���A2�Ŕ��ɑ΂��郁�b�Z�[�W
// type ... 0:TURN 1:PASS 2:WIN!    0�Ń^�[���J�n�A1�Ńp�X�A2�ŏ������b�Z�[�W
void setMsg2(int turn, int type)
{
	std::string turn_str[] = { "BLACK", "WHITE", "DRAW" };
	std::string type_str[] = { "TURN", "PASS", "WINS!" };
	msg2 = turn_str[turn - 1];
	if (turn != 3) msg2 += " " + type_str[type];
	msg_wait2 = 60;
}

// ���s�`�F�b�N
int checkResult2()
{
	//���v���C���[�̏����R�}�����i�[����ϐ��ƁA
	//���҂𔻒肷��ϐ���錾����
	int pnum[2] = {};
	int result = 0;

	//�Ֆʂ̃R�}���𐔂���B���̃R�}��pnum[0]�ɁA����pnum[1]�ɃZ�b�g����B
	for (int y = 0; y < 12; y++) for (int x = 0; x < 12; x++)
	{
		if (SqBoardA[y][x] > 0) pnum[SqBoardA[y][x] - 1]++;
	}

	//�o���Ƃ��p�X���I���̍��}�B�@���s�`�F�b�N���n�܂�B
	if (isPass2(1) && isPass2(2))
	{
		//Result=1�Ȃ�΍��̏����A2�Ȃ�Δ��̏����A3�Ȃ�Έ��������ƂȂ�B
		if (pnum[0] > pnum[1]) result = 1;
		else if (pnum[0] < pnum[1]) result = 2;
		else result = 3;
	}

	if (result) setMsg2(result, 2);
	return result;
}

//�Ֆʂ�3����2�������A��2���E���h�֌�����
void removePiece()
{
	for (int j = 0; j < 96; j++)
	{
		int rmx, rmy;
		rmx = GetRand(11);
		rmy = GetRand(11);

		SqBoardA[rmy][rmx] = 0;
	}
}

//BGM�ύX�֐�
void changeBGM()
{
	StopSoundFile();
	PlaySoundFile("res/loop_68.wav", DX_PLAYTYPE_LOOP);
}

//	�t���[������
void moveGame4Scene()
{
	int pieces[2];  //�摜�ۑ��p�ϐ��@0=���@1=��
	//int back;
	int status = 2; // 1:�v���C�� 2:TURN���b�Z�[�W�� 3:�p�X���b�Z�[�W�� 4:�I��
	int turn = 1;   // 1:���^�[�� 2:���^�[��

	//��ʕ`��̏���
	SetDrawScreen(DX_SCREEN_BACK);
	ChangeFont("�l�r ����");

	LoadDivGraph("res/piece.png", 2, 2, 1, 47, 47, pieces); //�摜�ǂݍ��݁F�R�}�i1�𕪊��j
	//back = LoadGraph("res/board1212.png");  //�w�i�摜�ǂݍ���

	 //SetDrawBlendMode(DX_BLENDMODE_PMA_INVSRC, 255);  //�{�[�h�摜���]�̏���

	SqBoardA[5][5] = SqBoardA[6][6] = 1;      //�����R�}��Տ�ɃZ�b�g
	SqBoardA[6][5] = SqBoardA[5][6] = 2;

	setMsg2(turn, 0);

	//���[�vBGM���Z�b�g����i�󋵂ɉ����āj
	PlaySoundFile("res/loop_95.wav", DX_PLAYTYPE_LOOP);

	while (!ProcessMessage())
	{
		ClearDrawScreen();

		switch (status)
		{
		case 1:               //�v���C���[�h�ɓ���
			if (isPass2(turn)) //�p�X�Ɣ��f���ꂽ�ꍇ
			{
				setMsg2(turn, 1); //���̎��_�̃v���C���[�Ƀp�X��鍐����
				status = 3;      //�p�X�t�F�C�Y�ڍs
			}
			else
			{
				/*

				�v�l���[�`����think�H�̑g�ݍ��킹��ς���ƁA������`�����y���߂�I
				��FThink1���m�Ńv���C���[���m�̑ΐ�
				�@�@Think2,3������CPU���m�̑ΐ�

				*/
				bool (*think[])(int) = { think01, think02 };  //�v�l���[�`���I���̏���

				if ((*think[turn - 1])(turn))//�v�l���[�`�����Ăяo��
				{
					turn = 3 - turn; status = 2; //���A���̃^�[�������ւ��Ď���
					setMsg2(turn, 0);
				}
			}

			if (checkResult2()) status = 4;
			break;

		case 2:     //TURN���b�Z�[�W�\�����̏ꍇ
			if (msg_wait2 > 0) msg_wait2--;   //msg_wait�̕������J�E���g����
			else status = 1;                //�J�E���g���I��������v���C�Ɉڍs����
			break;

		case 3:     //PASS�Ɣ��肳��A���b�Z�[�W���\�������
			if (msg_wait2 > 0) msg_wait2--;   //msg_wait�̕������J�E���g����
			else
			{
				turn = 3 - turn; status = 2;//�J�E���g���I��������^�[�����ڍs�A����
				setMsg2(turn, 0);
			}
			break;
		case 4:
			if (CurrentRound == 1)
			{
				WaitTimer(4000); //������҂�(�~���b)
				//���E���h�ڍs�������s��
				removePiece();	//�s�[�X������
				CurrentRound++;	//���E���h�J�E���g�A�b�v
				status = 2;		//�X�e�[�^�X���v���C�������ɖ߂�
			}
			else if (CurrentRound == 2)
			{
				//���E���h2�I����
				//�I���ҋ@��Ԃ֔��
				releaseGame4Scene();
			}
			break;
		}

		//ESC�L�[�������ꂽ�烋�[�v���甲����
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)break;

		//DrawGraph(100, 0, back, FALSE);

		//�v���O�����Ŋi�q��`���i��ł͂��܂��s���Ȃ��j
		//��1�i�K�F�l���̘g���ƃt�B�[���h�Ή�
		{
			DrawBox(5, 5, 580, 580, GetColor(0, 140, 20), TRUE);
			int i;
			for (i = 0; i < 3; i++)
			{
				DrawLine(5, 5, 580, 5, ColorWhite2);
				DrawLine(5, 5, 5, 580, ColorWhite2);
				DrawLine(580, 5, 580, 580, ColorWhite2);
				DrawLine(5, 580, 580, 580, ColorWhite2);
			}
		}

		//��2�i�K�F�c���̊i�q
		int i, j;
		int DrawGap = 48;   //�`��Ԋu��50
		int DrawX = 5, DrawY = 5;
		for (i = 0; i < 11; i++)
		{
			DrawX = DrawX + DrawGap;
			DrawLine(DrawX, 5, DrawX, 580, ColorWhite2);
		}
		for (j = 0; j < 11; j++)
		{
			DrawY = DrawY + DrawGap;
			DrawLine(5, DrawY, 580, DrawY, ColorWhite2);
		}

		/*�e�X�g�p�F�}�E�X���W�擾
		int mcx, mcy;
		char StrBuf[128], StrBuf2[32];
		int calcx, calcy;
		char StrBuf3[128], StrBuf4[32];
		GetMousePoint(&mcx, &mcy);
		calcx = (mcx / 48);
		calcy = (mcy / 48);
		{
			lstrcpy(StrBuf, "���W �w"); // ������"���W �w"��StrBuf�ɃR�s�[
			itoa(mcx, StrBuf2, 10); // MouseX�̒l�𕶎���ɂ���StrBuf2�Ɋi�[
			lstrcat(StrBuf, StrBuf2); // StrBuf�̓��e��StrBuf2�̓��e��t������
			lstrcat(StrBuf, "�@�x "); // StrBuf�̓��e�ɕ�����"�x"��t������
			itoa(mcy, StrBuf2, 10); // MouseY�̒l�𕶎���ɂ���StrBuf2�Ɋi�[
			lstrcat(StrBuf, StrBuf2); // StrBuf�̓��e��StrBuf2�̓��e��t������

			lstrcpy(StrBuf3, "�Z�o�l X");
			itoa(calcx, StrBuf4, 10);
			lstrcat(StrBuf3, StrBuf4);
			lstrcat(StrBuf3, " Y ");
			itoa(calcy, StrBuf4, 10);
			lstrcat(StrBuf3, StrBuf4);
		}
		DrawString(0, 0, StrBuf, GetColor(255, 255, 255));
		DrawString(0, 40, StrBuf3, GetColor(255, 255, 255));
		//�e�X�g�p�����܂�*/

		for (int y = 0; y < 12; y++) for (int x = 0; x < 12; x++)
		{
			//�R�}�\����
			if (SqBoardA[y][x]) DrawGraph(x * 48 + 5, y * 48 + 5, pieces[SqBoardA[y][x] - 1], TRUE);
		}

		if (status > 1)
		{
			//�^�[�����b�Z�[�W�\����
			int mw = GetDrawStringWidth(msg2.c_str(), msg2.size());

			DrawBox(192 - mw / 2 - 30, 630, 192 + mw / 2 + 30, 655, GetColor(150, 150, 150), TRUE);
			DrawString(192 - mw / 2, 620, msg2.c_str(), ColorWhite2);
		}

		//���ƍ��̃R�}���𐔂��A���҂̎擾����\������i���łɗD���ȕ���Ԃ��j
		{
			int pcnum[2] = {};
			char blackC[32], whiteC[32];
			for (int y = 0; y < 12; y++) for (int x = 0; x < 12; x++)
			{
				if (SqBoardA[y][x] > 0) pcnum[SqBoardA[y][x] - 1]++;
			}
			itoa(pcnum[0], blackC, 10);
			itoa(pcnum[1], whiteC, 10);

			//�D������
			int winning = 0;

			if (pcnum[0] > pcnum[1]) winning = 1;
			else if (pcnum[0] < pcnum[1]) winning = 2;
			else winning = 3;

			//�D���ȕ��͕������Ԃ��Ȃ�
			DrawString(590, 5, "BLACK", ColorWhite2);
			if (winning == 1)
			{
				DrawString(590, 40, blackC, ColorRed2);
			}
			else if (winning == 2 || winning == 3)
			{
				DrawString(590, 40, blackC, ColorWhite2);
			}

			DrawString(590, 90, "WHITE", ColorWhite2);
			if (winning == 2)
			{
				DrawString(590, 125, whiteC, ColorRed2);
			}
			else if (winning == 1 || winning == 3)
			{
				DrawString(590, 125, whiteC, ColorWhite2);
			}
		}

		{
			//�N�̃^�[������\������
			if (turn == 1)
			{
				DrawString(680, 40, "�� Now", ColorSky2);
			}
			else if (turn == 2)
			{
				DrawString(680, 125, "�� Now", ColorSky2);
			}
		}

		//���݂̃��E���h����\��
		DrawString(590, 220, "Round: ", ColorWhite2);
		if (CurrentRound == 2)
		{
			DrawFormatString(710, 220, ColorRed2, "%d", CurrentRound);
		}
		else if (CurrentRound == 1)
		{
			DrawFormatString(710, 220, ColorWhite2, "%d", CurrentRound);
		}

		//�Q�[���I�����A���Ă��炤�悤�v������
		if (status == 4 && CurrentRound == 2)
		{
			DrawString(590, 330, "ESC�L�[��\n�����ďI��\n���Ă�������", ColorWhite2);
		}
		else if (status == 4 && CurrentRound == 1)
		{
			//�\�����郁�b�Z�[�W�͗����Ō��肷��
			int msgRand = GetRand(2);
			if (msgRand == 0)
			{
				DrawString(590, 330, "�܂����c�c�I\n�܂��I����\n��I", ColorWhite2);
			}
			else if (msgRand == 1)
			{
				DrawString(590, 330, "�����܂ł��H\n�c�c����I\n�܂����I", ColorWhite2);
			}
			else if (msgRand == 2)
			{
				DrawString(590, 330, "�m�[�J�E���g\n�m�[�J�E���g\n�Ȃ񂾁c�I�I", ColorWhite2);
			}
		}

		//BGM��ύX����
		if (CurrentRound == 2)
		{
			if (excuted == 0)
			{
				changeBGM();
				excuted++;
			}
			else if (excuted >= 1)
			{
			}
		}

		ScreenFlip();
	}
}

//	�����_�����O����
void renderGame4Scene(void)
{
}

//	�V�[���I�����̌㏈��
void releaseGame4Scene(void)
{
	//ESC�L�[����������I������
	if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)DxLib_End();
}

// ���蔻��R�[���o�b�N �@�@�@�����ł͗v�f���폜���Ȃ����ƁI�I
void  Game4SceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}