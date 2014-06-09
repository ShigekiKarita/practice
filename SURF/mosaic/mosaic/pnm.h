#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

typedef	unsigned char BYTE;

/* �s�N�Z���̒�` */
typedef	struct {
    BYTE r;
    BYTE g;
    BYTE b;
}   PIXEL;

/* �摜�̒�` */
typedef	struct {
    int width;
    int height;
    int	maxval;
    PIXEL **pixel;
}   IMAGE;

/*
  �w�肳�ꂽ�傫���̉摜���쐬���ĕԂ��܂��B���̍ہA�摜�Ƀ�����������
  �Ă܂��̂ŁA�g�p��A�֐� pnmFreeMemory �Ń��������J�����Ă��������B
  
  �����F
  width:  �摜�̑傫��(���F��������)
  height: �摜�̑傫��(�c�F��������)
  maxval: �摜�̊K���̍ő�l
 */
IMAGE *pnmAllocMemory(int, int, int);

/*
  �摜�Ɋ����Ă��������̈���J�����܂��B

  �����F
  image: �������̈���J������摜
 */
void pnmFreeMemory(IMAGE*);

/*
  �t�@�C������ pbm�Apgm�Appm�`��(�����𑍏̂���pnm�`���Ƃ����܂�)��
  �摜��ǂݍ��݂܂��B

  �����F
  fname: pnm�`���̉摜�t�@�C����
 */
IMAGE *pnmReadFile(char*);

/*
  pnm�`���̉摜���o�͂��܂��B
  (�o�C�i���^ppm�̂ݎ���)

  �����F
  image: �����o���摜
  fname: �t�@�C����
*/
void pnmWriteFile(IMAGE*, char*);


IMAGE *pnmConstructImage(IMAGE*, IMAGE*, double, double, double, double);

/*
  �g�嗦���w�肵�ĉ摜�̑傫����ύX���A�Ԃ��܂��B

  �����F
  i1: �傫����ύX����摜
  ratio: �摜�̊g�嗦
 */
IMAGE *pnmResizeImage(IMAGE*, double);
