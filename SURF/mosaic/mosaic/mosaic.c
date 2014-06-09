#include "pnm.h"
#include "mosaic.h"

#define MAXDOUBLE 1.79769313486231e+308

/*
  二つの画像が最も一致するときの移動量(右方向、下方向)と回転角を総当た
  りで計算する。一致の基準はR、G、Bの各チャンネルの二乗誤差である。
  
  引数：
  img1: 比較する画像1
  img2: 比較する画像2
  xmin, xmax: 横方向の探索範囲
  dx: 右方向の移動量（単位はピクセル）
  ymin, ymax: 縦方向の探索範囲
  dy: 下方向の移動量（単位はピクセル）
  tmin, tmax: 回転角の探索範囲
  tstep: 回転角の更新量(探索精度)
  dt: 反時計回りの回転角（単位は度）
*/
void mosaicGetMin(IMAGE *img1, IMAGE *img2,
		  int xmin, int xmax, int *dx, int ymin, int ymax, int *dy,
		  double tmin, double tmax, double tstep, double *dt)
{
    int	i, j, u, v, x, y;
    double t, s1, c1;
    /* sum: 2枚の画像が重なる領域の二乗誤差
       count: 2枚の画像が重なる領域のピクセル数 */
    int	sum, tmp, count;
    /* ave: 2枚の画像が重なる領域の平均二乗誤差(sum/count)
       min: 2枚の画像が重なる領域の平均二乗誤差の最小値 */
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
		        /* 画像2を回転したときのピクセルの位置 */
			u = (int)floor((s1 * i + c1 * j) + x + 0.5);
			v = (int)floor((c1 * i - s1 * j) + y + 0.5);
			/* 画像が重ならない部分は誤差を計算しない */
			if(u < 0 || u >= img1->width ||
			   v < 0 || v >= img1->height)
			    continue;
			/* 二乗誤差の計算 */
			tmp = img1->pixel[v][u].r - img2->pixel[i][j].r;
			sum += tmp * tmp;
			tmp = img1->pixel[v][u].g - img2->pixel[i][j].g;
			sum += tmp * tmp;
			tmp = img1->pixel[v][u].b - img2->pixel[i][j].b;
			sum += tmp * tmp;
			count++;
		    }
		}
		/* 平均二乗誤差の計算 */
		ave = (double)sum / count;
		/* 平均二乗誤差が最小値より小さい場合はパラメータの更新 */
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
  画像の大きさを変更し、返します。
  
  引数：
  image1: 大きさを変更する画像
  width:  画像の大きさ(横：水平方向)
  height: 画像の大きさ(縦：垂直方向)
 */
static IMAGE *mosaicResizeImage(IMAGE *image1, int width, int height)
{
    int	i, j, k, l, count;
    int	r, g, b, stepx, stepy;
    /* 大きさを変更した画像 */
    IMAGE *image2;

    /* 大きさ変更後の画像を作成 */
    if((image2 = pnmAllocMemory(width, height, image1->maxval)) == NULL) {
	fprintf(stderr, "can't allocate memory\n");
	exit(1);
    }
    image2->width = width;
    image2->height = height;

    /* 元の画像と新しい画像の大きさの比を計算 */
    stepx = image1->width / width;
    stepy = image1->height / height;

    /* ピクセルのコピー */
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
    /* 大きさを変更した画像を返す */
    return image2;
}

/*
  画像 image1 を基準とした場合に、2枚の画像が重なるように image2 の水
  平、垂直方向の移動量、回転角および拡大率を計算して返します。
  この関数から別の関数を呼び出しても構いません。

  このサンプルプログラムでは、mosaicGetMin と mosaicResizeImage を呼び
  出しています。

  サンプル画像セット１の場合は dt と r、サンプル画像セット２の関数の場合は r の
  値は変更しないでください。

  引数：
  image1: 入力画像1
  image2: 入力画像2
  dx: 右方向の移動量（単位はピクセル）
  dy: 下方向の移動量（単位はピクセル）
  dt: 反時計回りの回転角（単位は度）
  r:  拡大率（倍）(pixels)
*/
void mosaic(IMAGE *image1, IMAGE *image2,
	    double *dx, double *dy, double *dt, double *r)
{
    int	i_min1, j_min1, i_min2, j_min2;
    double t_min1, t_min2;
    /* 入力画像を縮小した画像 */
    IMAGE *image3, *image4;

    /* 入力画像を水平、垂直方向共に 1/5 に縮小した画像を作成する */
    image3 = mosaicResizeImage(image1, image1->width/5, image1->height/5);
    image4 = mosaicResizeImage(image2, image2->width/5, image2->height/5);
    /* 縮小した画像で、画像が最も重なるときのパラメータを概算 */
    mosaicGetMin(image3, image4,
		 2, image1->width/5-3, &j_min1, 2, image1->height/5-3, &i_min1,
		 -15.0, 15.0, 2.0, &t_min1);
    /* 縮小した画像のメモリ領域を開放 */
    pnmFreeMemory(image3);
    pnmFreeMemory(image4);

    /* 元の画像でパラメータを計算 */
    mosaicGetMin(image1, image2, (j_min1-1)*5, (j_min1+1)*5-1, &j_min2,
		 (i_min1-1)*5, (i_min1+1)*5-1, &i_min2,
		 t_min1-1.0, t_min1+1.0, 1.0, &t_min2);
    /* パラメータを返す */
    *dx = (double)j_min2; *dy = (double)i_min2; *dt = t_min2;
}
