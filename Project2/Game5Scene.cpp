#include "GameMain.h"
#include "GameSceneMain.h"
#include "Game5Scene.h"
#include <string>

//�O����`�iGameMain.cpp�ɂĐ錾�j
extern int Input, EdgeInput;
extern char nameTmp[12];

//�V�[���J�n�O�̏��������s��
BOOL initGame5Scene(void)
{
	return TRUE;
}

//�e��t���[������
void moveGame5Scene()
{
	ClearDrawScreen();

	//�\��E�w����`�悷��
	ChangeFont("�l�r ����");

	SetFontSize(45);
	SetFontThickness(9);
	DrawString(280, 50, "NAME ENTRY", GetColor(255, 255, 255));

	SetFontSize(30);
	SetFontThickness(5);
	DrawString(160, 100, "�A���t�@�x�b�g�Ŗ��O����͂��Ă�������\n���͂��I�������AEnter�L�[��\n�����Ă�������", GetColor(255, 255, 255));

	//���͒��̖��O��\������
	SetFontSize(40);
	SetFontThickness(6);
	DrawString(270, 280, nameTmp, GetColor(255, 255, 255));
	//���O���ɐ���\���i11���������x,X�̒���=250�j
	DrawLine(265, 330, 515, 330, GetColor(255, 255, 255));

	//���͂��m�F������AEnter�L�[�ŃQ�[���X�^�[�g
	//���[�v���������B�Ⴆ�΁F1=�ʏ탂�[�h�@4=�܂����ǂ胂�[�h
	if (nameTmp[0] != NULL)
	{
		SetFontSize(30);
		DrawString(50, 370, "���͊����B\nEnter�L�[�������đ΋ǊJ�n�I", GetColor(255, 255, 255));
		ScreenFlip();
		//���͂����悱��
		changeScene(SCENE_GAME4);
	}

	//���͂����m�E���
	KeyInputSingleCharString(270, 280, 32, nameTmp, FALSE);
}

//��ʕ`�揈��
void renderGame5Scene(void)
{
}

//�V�[���I�����̌㏈��
void releaseGame5Scene(void)
{
}

//�����蔻��R�[���o�b�N�i�v�f�폜�֎~�j
void Game5SceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}