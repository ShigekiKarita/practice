#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

typedef	unsigned char BYTE;

/* ピクセルの定義 */
typedef	struct {
    BYTE r;
    BYTE g;
    BYTE b;
}   PIXEL;

/* 画像の定義 */
typedef	struct {
    int width;
    int height;
    int	maxval;
    PIXEL **pixel;
}   IMAGE;

/*
  指定された大きさの画像を作成して返します。その際、画像にメモリを割当
  てますので、使用後、関数 pnmFreeMemory でメモリを開放してください。
  
  引数：
  width:  画像の大きさ(横：水平方向)
  height: 画像の大きさ(縦：垂直方向)
  maxval: 画像の階調の最大値
 */
IMAGE *pnmAllocMemory(int, int, int);

/*
  画像に割当てたメモリ領域を開放します。

  引数：
  image: メモリ領域を開放する画像
 */
void pnmFreeMemory(IMAGE*);

/*
  ファイルから pbm、pgm、ppm形式(これらを総称してpnm形式といいます)の
  画像を読み込みます。

  引数：
  fname: pnm形式の画像ファイル名
 */
IMAGE *pnmReadFile(char*);

/*
  pnm形式の画像を出力します。
  (バイナリ型ppmのみ実装)

  引数：
  image: 書き出す画像
  fname: ファイル名
*/
void pnmWriteFile(IMAGE*, char*);


IMAGE *pnmConstructImage(IMAGE*, IMAGE*, double, double, double, double);

/*
  拡大率を指定して画像の大きさを変更し、返します。

  引数：
  i1: 大きさを変更する画像
  ratio: 画像の拡大率
 */
IMAGE *pnmResizeImage(IMAGE*, double);
