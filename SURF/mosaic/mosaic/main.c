#include <time.h>
#include <sys/time.h>
#include "pnm.h"
#include "mosaic.h"

double my_clock(void) 
{ 
  struct timeval tv; 
  gettimeofday(&tv, NULL); 
  return tv.tv_sec + (double)tv.tv_usec*1e-6; 
}

int main(int argc, char *argv[])
{
    /* �摜�̒�`
       image1, image2: ���͉摜
       image3: �o�͉摜
       ------------------------------------------------------------
       image->width�F�摜�̕�
       image->height�F�摜�̍���
       image->pixel[i][j].r�F i �s j ��̐Ԃ̉�f�l
       image->pixel[i][j].g�F i �s j ��̗΂̉�f�l
       image->pixel[i][j].b�F i �s j ��̐̉�f�l
       �������A0�� i ��image->height-1�C0�� j ��image->width-1 */
    IMAGE *image1, *image2, *image3;
    /* �摜 image1 ����Ƃ����ꍇ�ɁA2�̉摜���d�Ȃ�悤�� image2
       �̐����A���������̈ړ��ʁA��]�p����ъg�嗦

       dx: �E�����̈ړ��ʁi�P�ʂ̓s�N�Z���j
       dy: �������̈ړ��ʁi�P�ʂ̓s�N�Z���j
       dt: �����v���̉�]�p�i�P�ʂ͓x�j
       r:  �g�嗦�i�{�j(pixels) */
    double dx, dy, dt, r;
    double t1, t2, my_time;
    
    if(argc != 4) {
	fprintf(stderr, "Usage: %s input1 input2 output\n", argv[0]);
	exit(1);
    }

    /* ���͉摜(image1, image2)�̓ǂݍ��� */
    image1 = pnmReadFile(argv[1]);
    image2 = pnmReadFile(argv[2]);

    /* �p�����[�^�̏����� */
    dx = dy = 0.0;
    dt = 0.0;
    r = 1.0;

    /* mosaic.c �ɂ���֐� mosaic ���Ăяo���A
       �摜 image1 ����Ƃ����ꍇ�ɁA2���̉摜���d�Ȃ�悤�� image2
       �̐����A���������̈ړ��ʁA��]�p����ъg�嗦���v�Z���܂��B
       ���̊֐������ǂ��邢�́A�V�����쐬���Ă��������B */

    t1 = my_clock();

    mosaic(image1, image2, &dx, &dy, &dt, &r);

    t2 = my_clock();

    my_time = t2-t1;

    /* �������Ԃ̏o�� */
    printf("processing time: %.6f\n", my_time);

    /* �o�͉摜�̍���*/
    image3 = pnmConstructImage(image1, image2, dx, dy, dt, r);
    /* �ړ��ʂ̏o�� */
    printf("dx = %f, dy = %f, dt = %f, r = %f\n", dx, dy, dt, r);
    /* �o�͉摜�̏����o�� */
    pnmWriteFile(image3, argv[3]);

    /* ����ɏI�� */
    return 0;
}
