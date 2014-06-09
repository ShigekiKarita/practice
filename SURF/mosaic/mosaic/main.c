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
    /* 画像の定義
       image1, image2: 入力画像
       image3: 出力画像
       ------------------------------------------------------------
       image->width：画像の幅
       image->height：画像の高さ
       image->pixel[i][j].r： i 行 j 列の赤の画素値
       image->pixel[i][j].g： i 行 j 列の緑の画素値
       image->pixel[i][j].b： i 行 j 列の青の画素値
       ただし、0≦ i ≦image->height-1，0≦ j ≦image->width-1 */
    IMAGE *image1, *image2, *image3;
    /* 画像 image1 を基準とした場合に、2つの画像が重なるような image2
       の水平、垂直方向の移動量、回転角および拡大率

       dx: 右方向の移動量（単位はピクセル）
       dy: 下方向の移動量（単位はピクセル）
       dt: 反時計回りの回転角（単位は度）
       r:  拡大率（倍）(pixels) */
    double dx, dy, dt, r;
    double t1, t2, my_time;
    
    if(argc != 4) {
	fprintf(stderr, "Usage: %s input1 input2 output\n", argv[0]);
	exit(1);
    }

    /* 入力画像(image1, image2)の読み込み */
    image1 = pnmReadFile(argv[1]);
    image2 = pnmReadFile(argv[2]);

    /* パラメータの初期化 */
    dx = dy = 0.0;
    dt = 0.0;
    r = 1.0;

    /* mosaic.c にある関数 mosaic を呼び出し、
       画像 image1 を基準とした場合に、2枚の画像が重なるように image2
       の水平、垂直方向の移動量、回転角および拡大率を計算します。
       この関数を改良あるいは、新しく作成してください。 */

    t1 = my_clock();

    mosaic(image1, image2, &dx, &dy, &dt, &r);

    t2 = my_clock();

    my_time = t2-t1;

    /* 処理時間の出力 */
    printf("processing time: %.6f\n", my_time);

    /* 出力画像の合成*/
    image3 = pnmConstructImage(image1, image2, dx, dy, dt, r);
    /* 移動量の出力 */
    printf("dx = %f, dy = %f, dt = %f, r = %f\n", dx, dy, dt, r);
    /* 出力画像の書き出し */
    pnmWriteFile(image3, argv[3]);

    /* 正常に終了 */
    return 0;
}
