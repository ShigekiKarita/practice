#include "pnm.h"
#include "mosaic.h"

#define MAXDOUBLE 1.79769313486231e+308

/*
  ��̉摜���ł���v����Ƃ��̈ړ���(�E�����A������)�Ɖ�]�p�𑍓���
  ��Ōv�Z����B��v�̊��R�AG�AB�̊e�`�����l���̓��덷�ł���B
  
  �����F
  img1: ��r����摜1
  img2: ��r����摜2
  xmin, xmax: �������̒T���͈�
  dx: �E�����̈ړ��ʁi�P�ʂ̓s�N�Z���j
  ymin, ymax: �c�����̒T���͈�
  dy: �������̈ړ��ʁi�P�ʂ̓s�N�Z���j
  tmin, tmax: ��]�p�̒T���͈�
  tstep: ��]�p�̍X�V��(�T�����x)
  dt: �����v���̉�]�p�i�P�ʂ͓x�j
*/
void mosaicGetMin(IMAGE *img1, IMAGE *img2,
		  int xmin, int xmax, int *dx, int ymin, int ymax, int *dy,
		  double tmin, double tmax, double tstep, double *dt)
{
    int	i, j, u, v, x, y;
    double t, s1, c1;
    /* sum: 2���̉摜���d�Ȃ�̈�̓��덷
       count: 2���̉摜���d�Ȃ�̈�̃s�N�Z���� */
    int	sum, tmp, count;
    /* ave: 2���̉摜���d�Ȃ�̈�̕��ϓ��덷(sum/count)
       min: 2���̉摜���d�Ȃ�̈�̕��ϓ��덷�̍ŏ��l */
    double ave, min;

    min = MAXDOUBLE;
    *dx = *dy = 0;
    *dt = 0.0;

    for(t=tmin; t<=tmax; t+=tstep) {
	s1 = sin(t * 3.14159265358979323846 / 180.0);
	c1 = cos(t * 3.14159265358979323846 / 180.0);
	for(y = ymin; y <= ymax; y++) {
	    for(x = xmin; x <= xmax; x++) {
		sum = 0;
		count = 0;
		for(i=0; i<img2->height; i++) {
		    for(j=0; j<img2->width; j++) {
		        /* �摜2����]�����Ƃ��̃s�N�Z���̈ʒu */
			u = (int)floor((s1 * i + c1 * j) + x + 0.5);
			v = (int)floor((c1 * i - s1 * j) + y + 0.5);
			/* �摜���d�Ȃ�Ȃ������͌덷���v�Z���Ȃ� */
			if(u < 0 || u >= img1->width ||
			   v < 0 || v >= img1->height)
			    continue;
			/* ���덷�̌v�Z */
			tmp = img1->pixel[v][u].r - img2->pixel[i][j].r;
			sum += tmp * tmp;
			tmp = img1->pixel[v][u].g - img2->pixel[i][j].g;
			sum += tmp * tmp;
			tmp = img1->pixel[v][u].b - img2->pixel[i][j].b;
			sum += tmp * tmp;
			count++;
		    }
		}
		/* ���ϓ��덷�̌v�Z */
		ave = (double)sum / count;
		/* ���ϓ��덷���ŏ��l��菬�����ꍇ�̓p�����[�^�̍X�V */
		if(min > ave) {
		    min = ave;
		    *dy = y;
		    *dx = x;
		    *dt = t;
		}
	    }
	}
    }
}

/*
  �摜�̑傫����ύX���A�Ԃ��܂��B
  
  �����F
  image1: �傫����ύX����摜
  width:  �摜�̑傫��(���F��������)
  height: �摜�̑傫��(�c�F��������)
 */
static IMAGE *mosaicResizeImage(IMAGE *image1, int width, int height)
{
    int	i, j, k, l, count;
    int	r, g, b, stepx, stepy;
    /* �傫����ύX�����摜 */
    IMAGE *image2;

    /* �傫���ύX��̉摜���쐬 */
    if((image2 = pnmAllocMemory(width, height, image1->maxval)) == NULL) {
	fprintf(stderr, "can't allocate memory\n");
	exit(1);
    }
    image2->width = width;
    image2->height = height;

    /* ���̉摜�ƐV�����摜�̑傫���̔���v�Z */
    stepx = image1->width / width;
    stepy = image1->height / height;

    /* �s�N�Z���̃R�s�[ */
    for(i=0; i<height; i++) {
	for(j=0; j<width; j++) {
	    r = g = b = 0;
	    count = 0;
	    for(k=stepy*i; k<=stepy*(i+1)-1; k++) {
		for(l=stepx*j; l<=stepx*(j+1)-1; l++) {
		    if(k >= image1->height || l >= image1->width)
			continue;
		    count++;
		    r += image1->pixel[k][l].r;
		    g += image1->pixel[k][l].g;
		    b += image1->pixel[k][l].b;
		}
	    }
	    image2->pixel[i][j].r = r / count;
	    image2->pixel[i][j].g = g / count;
	    image2->pixel[i][j].b = b / count;
	}
    }
    /* �傫����ύX�����摜��Ԃ� */
    return image2;
}

/*
  �摜 image1 ����Ƃ����ꍇ�ɁA2���̉摜���d�Ȃ�悤�� image2 �̐�
  ���A���������̈ړ��ʁA��]�p����ъg�嗦���v�Z���ĕԂ��܂��B
  ���̊֐�����ʂ̊֐����Ăяo���Ă��\���܂���B

  ���̃T���v���v���O�����ł́AmosaicGetMin �� mosaicResizeImage ���Ă�
  �o���Ă��܂��B

  �T���v���摜�Z�b�g�P�̏ꍇ�� dt �� r�A�T���v���摜�Z�b�g�Q�̊֐��̏ꍇ�� r ��
  �l�͕ύX���Ȃ��ł��������B

  �����F
  image1: ���͉摜1
  image2: ���͉摜2
  dx: �E�����̈ړ��ʁi�P�ʂ̓s�N�Z���j
  dy: �������̈ړ��ʁi�P�ʂ̓s�N�Z���j
  dt: �����v���̉�]�p�i�P�ʂ͓x�j
  r:  �g�嗦�i�{�j(pixels)
*/
void mosaic(IMAGE *image1, IMAGE *image2,
	    double *dx, double *dy, double *dt, double *r)
{
    int	i_min1, j_min1, i_min2, j_min2;
    double t_min1, t_min2;
    /* ���͉摜���k�������摜 */
    IMAGE *image3, *image4;

    /* ���͉摜�𐅕��A������������ 1/5 �ɏk�������摜���쐬���� */
    image3 = mosaicResizeImage(image1, image1->width/5, image1->height/5);
    image4 = mosaicResizeImage(image2, image2->width/5, image2->height/5);
    /* �k�������摜�ŁA�摜���ł��d�Ȃ�Ƃ��̃p�����[�^���T�Z */
    mosaicGetMin(image3, image4,
		 2, image1->width/5-3, &j_min1, 2, image1->height/5-3, &i_min1,
		 -15.0, 15.0, 2.0, &t_min1);
    /* �k�������摜�̃������̈���J�� */
    pnmFreeMemory(image3);
    pnmFreeMemory(image4);

    /* ���̉摜�Ńp�����[�^���v�Z */
    mosaicGetMin(image1, image2, (j_min1-1)*5, (j_min1+1)*5-1, &j_min2,
		 (i_min1-1)*5, (i_min1+1)*5-1, &i_min2,
		 t_min1-1.0, t_min1+1.0, 1.0, &t_min2);
    /* �p�����[�^��Ԃ� */
    *dx = (double)j_min2; *dy = (double)i_min2; *dt = t_min2;
}
