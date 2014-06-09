#include "pnm.h"

static double s1, c1;

typedef struct {
    double x;
    double y1;
    double y2;
} CUT;


/*
  (画像)ファイルの読み込みに失敗した場合にエラーメッセージを出力し、終
  了コード 1 を返して終了する。

  引数：
  fname: 読み込みに失敗したファイル名
 */
static void pnmReadError(char *fname)
{
    fprintf(stderr, "%s: this file type is not supported.\n", fname);
    exit(1);
}


/*
  指定された大きさの画像を作成して返します。その際、画像にメモリを割当
  てますので、使用後、関数 pnmFreeMemory でメモリを開放してください。
  
  引数：
  width:  画像の大きさ(横：水平方向)
  height: 画像の大きさ(縦：垂直方向)
  maxval: 画像の階調の最大値
 */
IMAGE *pnmAllocMemory(int width, int height, int maxval)
{
    int	i;
    IMAGE *image;	
    PIXEL *pixelbuf;

    if((image = (IMAGE *)calloc(sizeof(IMAGE), 1)) == NULL ||
       (pixelbuf = (PIXEL *)calloc(sizeof(PIXEL), width * height)) == NULL ||
       (image->pixel = (PIXEL **)calloc(sizeof(PIXEL *), height)) == NULL) {
	return NULL;
    }
    /* 画像の大きさと階調の最大値を設定*/
    image->width = width;
    image->height = height;
    image->maxval = maxval;

    for(i=0; i<height; i++) {
	image->pixel[i] = pixelbuf + i * width;
    }
    /* 画像を返す */
    return image;
}

/*
  画像に割当てたメモリ領域を開放します。

  引数：
  image: メモリ領域を開放する画像
 */
void pnmFreeMemory(IMAGE *image)
{
    free(image->pixel[0]);
    free(image->pixel);
    free(image);
}

/*
  pnm画像を実際に読み込み、返します。

  引数：
  fp: 読み込んだファイルのポインタ
  fname: 読み込んだファイルの名前
  type: マジックナンバ
 */
static IMAGE *pnmReadFileRGB(FILE *fp, char *fname, char type)
{
    int	i, j, k;
    int	c, mode, maxmode, length;
    int	width, height, maxval;
    int	r, g, b;
    int	tmp;
    IMAGE *image;

    /* 画像の大きさ、階調の最大値の読み込み */
    width = height = maxval = 0;
    if(type == '1' || type == '4') {
        maxmode = 1;	/* 階調を読み込まない */
	maxval = 255;
    } else {
        maxmode = 2;	/* 階調を読み込む */
    }
    mode = 0;
    c = fgetc(fp);
    while(mode <= maxmode) {
	if(c == EOF) {
	    break;
	} else if(c >= '0' && c <= '9') {
	    /* 数字の読み込み */
	    tmp = 0;
	    while(c >= '0' && c <= '9') {
		tmp = tmp * 10 + c - '0';
		c = fgetc(fp);
	    }
	    if(mode == 0) {
		width = tmp;
	    } else if(mode == 1) {
		height = tmp;
	    } else {
		maxval = tmp;
	    }
	    mode++;
	} else if(c == '#') {	/* コメントの読み飛ばし */
	    while(c != '\n')
		c = fgetc(fp);	
	} else {
	    c = fgetc(fp);
	}
    }
    /* 階調の最大値が 255を超えた場合はエラーを出して終了 */
    if(maxval > 255) {
	pnmReadError(fname);
    }

    /* 画像のメモリ領域の割り当て */
    if((image = pnmAllocMemory(width, height, maxval)) == NULL) {
	fprintf(stderr, "can't allocate memory\n");
	exit(1);
    }

    /* アスキー型の場合 */
    if(type >= '1' && type <= '3') {
	for(i=0; i<height; i++) {
	    for(j=0; j<width; j++) {
		fscanf(fp, "%d", &r);
		if(type == '1')
		    r = g = b = (r == 0)? 255 : 0;
		else if(type == '2')
		    g = b = r;
		else
		    fscanf(fp, "%d%d", &g, &b);
		image->pixel[i][j].r = r;
		image->pixel[i][j].g = g;
		image->pixel[i][j].b = b;
	    }
	}
    } else {
      /* ファイルから読み込むバイト数の調整
	 (バイナリ型 pbm の場合のみ約1/8にする) */
	if(type == '4') {
	    length = width / 8;
	    if(length * 8 < width)
		length++;
	} else {
	    length = width;
	}
	/* 画像データの読み込み */
	for(i=0; i<height; i++) {
	    for(j=0; j<length; j++) {
		r = fgetc(fp);
		if(type == '4') {
		  /* バイナリ型 pbm の場合 */
		    for(k=0; k<8; k++) {
			if(j * 8 + k >= width)
			    break;
			g = (r & (0x80 >> k))? 0 : 255;
			image->pixel[i][j * 8 + k].r = g;
			image->pixel[i][j * 8 + k].g = g;
			image->pixel[i][j * 8 + k].b = g;
		    }
		} else if(type == '5') {
		  /* バイナリ型 pgm の場合 */
		    image->pixel[i][j].r = r;
		    image->pixel[i][j].g = r;
		    image->pixel[i][j].b = r;
		} else {
		  /* バイナリ型 ppm の場合 */
		    image->pixel[i][j].r = r;
		    image->pixel[i][j].g = fgetc(fp);
		    image->pixel[i][j].b = fgetc(fp);
		}
	    }
	}
    }
    return image;
}


/*
  ファイルから pbm、pgm、ppm形式(これらを総称してpnm形式という)の画像
  を読み込みます。

  引数：
  fname: pnm形式の画像ファイル名
 */
IMAGE *pnmReadFile(char *fname)
{
    int	c;
    FILE *fp, *fopen();
    IMAGE *image = NULL;

    /* ファイルを開く。ファイルを開けない場合はエラーを返し、終了コー
       ド 1 を返して終了 */
    if((fp = fopen(fname, "r")) == NULL) {
	fprintf(stderr, "can't open %s\n", fname);
	exit(1);
    }

    /* 画像の最初の文字が 'P' でない場合、 関数 pnmReadError でエラー
       メッセージを表示し、終了 */
    if(fgetc(fp) != 'P') {
	pnmReadError(fname);
    }
    /* pnm形式の画像ファイルのマジックナンバー(1～6)を読み込む */
    c = fgetc(fp);
    /* 関数 pnmReadFileRGB を呼び、画像を読み込む。もし、マジックナン
       バーが異常なら、エラーを返す */
    if(c >= '1' && c <= '6')
	image = pnmReadFileRGB(fp, fname, c);
    else
        pnmReadError(fname);	/* マジックナンバーが異常の場合 */

    /* ファイルを閉じる */
    fclose(fp);
    /* 読み込んだ画像を返す */
    return image;
}


/*
  pnm形式の画像を出力します。
  (バイナリ型ppmのみ実装)

  引数：
  image: 書き出す画像
  fname: ファイル名
*/
void pnmWriteFile(IMAGE *image, char *fname)
{
    int	i, j;
    FILE *fp, *fopen();

    /* ファイルを開く。ファイルを開けない場合はエラーを返し、終了コー
       ド 1 を返して終了 */
    if((fp = fopen(fname, "w")) == NULL) {
	fprintf(stderr, "can't create %s\n", fname);
	exit(1);
    }

    /* ヘッダの書き出し */
    fprintf(fp, "P6\n%d %d\n%d\n", image->width, image->height, image->maxval);
    /* 画像の書き出し */
    for(i=0; i<image->height; i++) {
	for(j=0; j<image->width; j++) {
	    fputc(image->pixel[i][j].r, fp);
	    fputc(image->pixel[i][j].g, fp);
	    fputc(image->pixel[i][j].b, fp);
	}
    }
}

int xcompare(const void* a, const void* b)
{
  CUT *i, *j;
  double sa;

  i = (CUT*) a;
  j = (CUT*) b;

  sa = i->x - j->x;
    if (sa>0) return 1;
    else if (sa==0) return 0;
    else return -1;
}


static int pnmMakeCut(CUT *cut, int u, int v, double x1, double y1)
{
    double X[4], Y[4], xx, yy1, yy2 = 0;
    int	ind[4][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    int	c, flag;
    int	i, j, k[4];
    
    for(i=0; i<4; i++) {
	X[i] = c1 * (u + ind[i][0]) - s1 * (v + ind[i][1]) + x1;
	Y[i] = s1 * (u + ind[i][0]) + c1 * (v + ind[i][1]) + y1;
    }
    c = 0;
    if(X[0] == X[3]) {
	cut[c].x = X[0]; cut[c].y1 = Y[0]; cut[c].y2 = Y[3]; c++;
	cut[c].x = X[1]; cut[c].y1 = Y[1]; cut[c].y2 = Y[2]; c++;
    } else if(X[0] < X[3]) {
	cut[c].x = X[0]; cut[c].y1 = cut[c].y2 = Y[0]; c++;
	cut[c].x = X[2]; cut[c].y1 = cut[c].y2 = Y[2]; c++;
	if(X[1] == X[3]) {
	    cut[c].x = X[1]; cut[c].y1 = Y[1]; cut[c].y2 = Y[3]; c++;
	} else if(X[1] > X[3]) {
	    cut[c].x = X[1]; cut[c].y1 = Y[1];
	    cut[c].y2 = (X[1]-X[2])*(Y[3]-Y[2])/(X[3]-X[2])+Y[2]; c++;
	    cut[c].x = X[3]; cut[c].y2 = Y[3];
	    cut[c].y1 = (X[3]-X[0])*(Y[1]-Y[0])/(X[1]-X[0])+Y[0]; c++;
	} else {
	    cut[c].x = X[1]; cut[c].y1 = Y[1];
	    cut[c].y2 = (X[1]-X[0])*(Y[3]-Y[0])/(X[3]-X[0])+Y[0]; c++;
	    cut[c].x = X[3]; cut[c].y2 = Y[3];
	    cut[c].y1 = (X[3]-X[1])*(Y[2]-Y[1])/(X[2]-X[1])+Y[1]; c++;
	}
    } else {			     
	cut[c].x = X[3]; cut[c].y1 = cut[c].y2 = Y[3]; c++;
	cut[c].x = X[1]; cut[c].y1 = cut[c].y2 = Y[1]; c++;
	if(X[0] == X[2]) {
	    cut[c].x = X[0]; cut[c].y1 = Y[0]; cut[c].y2 = Y[2]; c++;
	} else if(X[0] > X[2]) {
	    cut[c].x = X[0]; cut[c].y1 = Y[0];
	    cut[c].y2 = (X[0]-X[1])*(Y[2]-Y[1])/(X[2]-X[1])+Y[1]; c++;
	    cut[c].x = X[2]; cut[c].y2 = Y[2];
	    cut[c].y1 = (X[2]-X[0])*(Y[3]-Y[0])/(X[3]-X[0])+Y[0]; c++;
	} else {
	    cut[c].x = X[0]; cut[c].y1 = Y[0];
	    cut[c].y2 = (X[0]-X[2])*(Y[3]-Y[2])/(X[3]-X[2])+Y[2]; c++;
	    cut[c].x = X[2]; cut[c].y2 = Y[2];
	    cut[c].y1 = (X[2]-X[0])*(Y[1]-Y[0])/(X[1]-X[0])+Y[0]; c++;
	}
    }		

    for(i=0; i<4; i++) {
	for(j=0; j<4; j++) {
	    k[j] = (i + j) % 4;
	}
	if(floor(Y[k[3]]) == floor(Y[k[0]]))
	    continue;
	yy1 = (Y[k[3]] > Y[k[0]])? floor(Y[k[3]]) : floor(Y[k[0]]);
	xx = (yy1 - Y[k[0]])
	    * (X[k[3]] - X[k[0]]) / (Y[k[3]] - Y[k[0]]) + X[k[0]];
	flag = 0;
	for(j=0; j<c; j++) {
	    if(xx == cut[j].x) {
		flag = 1;
		break;
	    }
	}
	if(flag)
	    continue;

	for(j=0; j<3; j++) {
	    if((X[k[j]] < xx && xx < X[k[j+1]]) ||
	       (X[k[j+1]] < xx && xx < X[k[j]])) {
		yy2 = (xx - X[k[j]]) *
		    (Y[k[j+1]] - Y[k[j]]) / (X[k[j+1]]-X[k[j]]) + Y[k[j]];
		break;
	    }
	}
	cut[c].x = xx;
	if(yy1 < yy2) {
	    cut[c].y1 = yy1; cut[c].y2 = yy2;
	} else {
	    cut[c].y1 = yy2; cut[c].y2 = yy1;
	}
	c++;
    }

    for(i=0; i<4; i++) {
	for(j=0; j<4; j++) {
	    k[j] = (i + j) % 4;
	}
	if(floor(X[k[3]]) == floor(X[k[0]]))
	    continue;
	xx = (X[k[3]] > X[k[0]])? floor(X[k[3]]) : floor(X[k[0]]);
	flag = 0;
	for(j=0; j<c; j++) {
	    if(xx == cut[j].x) {
		flag = 1;
		break;
	    }
	}
	if(flag)
	    continue;
	yy1 = (xx - X[k[0]])
	    * (Y[k[3]] - Y[k[0]]) / (X[k[3]] - X[k[0]]) + Y[k[0]];
	for(j=0; j<3; j++) {
	    if((X[k[j]] < xx && xx < X[k[j+1]]) ||
	       (X[k[j+1]] < xx && xx < X[k[j]])) {
		yy2 = (xx - X[k[j]]) *
		    (Y[k[j+1]] - Y[k[j]]) / (X[k[j+1]]-X[k[j]])	+Y[k[j]];
		break;
	    }
	}
	cut[c].x = xx;
	if(yy1 < yy2) {
	    cut[c].y1 = yy1; cut[c].y2 = yy2;
	} else {
	    cut[c].y1 = yy2; cut[c].y2 = yy1;
	}
	c++;
    }

    qsort((char *)cut, c, sizeof(CUT), xcompare);

    return c;
}

static PIXEL pnmGetPixel(IMAGE *input, int u, int v, double x1, double y1)
{
    int	i, xx, yy;
    int c;
    double t1, t2, t3, t4;
    double s, ssum, r, g, b;
    double ymax, ymin;
    PIXEL pixel;	
    CUT	cut[12];

    c = pnmMakeCut(cut, u, v, x1, y1);

    ssum = 0.0;
    r = g = b = 0.0;
    for(i=1; i<c; i++) {
	ymin = (cut[i-1].y1 < cut[i].y1)? cut[i-1].y1 : cut[i].y1;
	ymax = (cut[i-1].y2 > cut[i].y2)? cut[i-1].y2 : cut[i].y2;
	yy = (int)floor(ymax);
	xx = (int)floor(cut[i-1].x);

	for(; yy >= (int)floor(ymin); yy--) {
	    if(cut[i-1].y1 > (double)(yy+1) || cut[i-1].y2 < (double)yy)
		continue;
	    if(cut[i].y1 > (double)(yy+1) || cut[i].y2 < (double)yy)
		continue;
	    t1 = (cut[i-1].y1 > (double)yy)? cut[i-1].y1 : (double)yy;
	    t2 = (cut[i-1].y2 < (double)(yy+1))? cut[i-1].y2 : (double)(yy+1);
	    t3 = (cut[i].y1 > (double)yy)? cut[i].y1 : (double)yy;
	    t4 = (cut[i].y2 < (double)(yy+1))? cut[i].y2 : (double)(yy+1);
	    s = (t2 - t1 + t4 - t3) * (cut[i].x - cut[i-1].x) / 2.0;
	    ssum += s;
	    if(cut[i-1].x < 0 || cut[i].x > input->width ||
	       yy < 0 || yy >= input->height) {
		r += s * 255.0;
		g += s * 255.0;
		b += s * 255.0;
	    } else {
		r += s * input->pixel[yy][xx].r;
		g += s * input->pixel[yy][xx].g;
		b += s * input->pixel[yy][xx].b;
	    }
	}
    }

    if(r > 255.0) r = 255.0;
    if(g > 255.0) g = 255.0;
    if(b > 255.0) b = 255.0;
    pixel.r = (int)r;
    pixel.g = (int)g;
    pixel.b = (int)b;
    return pixel;
}

/*
  拡大率を指定して画像の大きさを変更し、返します。

  引数：
  i1: 大きさを変更する画像
  ratio: 画像の拡大率
 */
IMAGE *pnmResizeImage(IMAGE *i1, double ratio)
{
    int	i, j, k, l;
    int	width, height;
    double xs, xe, ys, ye;
    int	xsi, xei, ysi, yei;
    double r, g, b, area, total;
    IMAGE *image;

    /* 変更後の画像を作成 */
    width = (int)floor(i1->width * ratio);
    height = (int)floor(i1->height * ratio);
    image = pnmAllocMemory(width, height, 255);

    for(i=0; i<height; i++) {
	for(j=0; j<width; j++) {
	    r = g = b = 0.0;
	    total = 0.0;

	    xs = (double)j / ratio;
	    xe = (double)(j + 1) / ratio;
	    ys = (double)i / ratio;
	    ye = (double)(i + 1) / ratio;
	    xsi = (int)ceil(xs);
	    xei = (int)floor(xe);
	    ysi = (int)ceil(ys);
	    yei = (int)floor(ye);

	    if((double)xsi != xs && (double)ysi != ys) {
		area = ((double)xsi - xs) * ((double)ysi - ys);
		r += (double)i1->pixel[ysi-1][xsi-1].r * area;
		g += (double)i1->pixel[ysi-1][xsi-1].g * area;
		b += (double)i1->pixel[ysi-1][xsi-1].b * area;
		total += area;
	    }
	    if((double)xei != xe && (double)ysi != ys) {
		area = (xe - (double)xei) * ((double)ysi - ys);
		r += (double)i1->pixel[ysi-1][xei].r * area;
		g += (double)i1->pixel[ysi-1][xei].g * area;
		b += (double)i1->pixel[ysi-1][xei].b * area;
		total += area;
	    }
	    if((double)xsi != xs && (double)yei != ye) {
		area = ((double)xsi - xs) * (ye - (double)yei);
		r += (double)i1->pixel[yei][xsi-1].r * area;
		g += (double)i1->pixel[yei][xsi-1].g * area;
		b += (double)i1->pixel[yei][xsi-1].b * area;
		total += area;
	    }
	    if((double)xei != xe && (double)yei != ye) {
		area = (xe - (double)xei) * (ye - (double)yei);
		r += (double)i1->pixel[yei][xei].r * area;
		g += (double)i1->pixel[yei][xei].g * area;
		b += (double)i1->pixel[yei][xei].b * area;
		total += area;
	    }

	    for(k=xsi; k<xei; k++) {
		if((double)ysi != ys) {
		    area = ((double)ysi - ys);
		    r += (double)i1->pixel[ysi-1][k].r * area;
		    g += (double)i1->pixel[ysi-1][k].g * area;
		    b += (double)i1->pixel[ysi-1][k].b * area;
		    total += area;
		}
		if((double)yei != ye) {
		    area = (ye - (double)yei);
		    r += (double)i1->pixel[yei][k].r * area;
		    g += (double)i1->pixel[yei][k].g * area;
		    b += (double)i1->pixel[yei][k].b * area;
		    total += area;
		}
	    }
	    for(k=ysi; k<yei; k++) {
		if((double)xsi != xs) {
		    area = ((double)xsi - xs);
		    r += (double)i1->pixel[k][xsi-1].r * area;
		    g += (double)i1->pixel[k][xsi-1].g * area;
		    b += (double)i1->pixel[k][xsi-1].b * area;
		    total += area;
		}
		if((double)xei != xe) {
		    area = (xe - (double)xei);
		    r += (double)i1->pixel[k][xei].r * area;
		    g += (double)i1->pixel[k][xei].g * area;
		    b += (double)i1->pixel[k][xei].b * area;
		    total += area;
		}
	    }

	    for(k=ysi; k<yei; k++) {
		for(l=xsi; l<xei; l++) {
		    r += (double)i1->pixel[k][l].r;
		    g += (double)i1->pixel[k][l].g;
		    b += (double)i1->pixel[k][l].b;
		    total += 1.0;
		}
	    }
	    image->pixel[i][j].r = r / total;
	    image->pixel[i][j].g = g / total;
	    image->pixel[i][j].b = b / total;
	}
    }
    /* 画像を返す */
    return	image;
}

/*
  2枚の画像を合成し、返す。

  引数：
  i1: 合成する画像1
  i2: 合成する画像2
  dx: 右方向の移動量（単位はピクセル）
  dy: 下方向の移動量（単位はピクセル）
  dt: 反時計回りの回転角（単位は度）
  r:  拡大率（倍）(pixels)
*/
IMAGE *pnmConstructImage(IMAGE *i1, IMAGE *i2,
			 double dx, double dy, double dt, double r)
{
    int	i, j, t;
    int	top, bottom, left, right;
    int	width, height;
    int	x, y;
    double x1, y1;
    /* image: 合成画像
       i3: i2を拡大率 r で拡大した画像 */
    IMAGE *image, *i3;

    /* i2を拡大率 r で拡大 */
    i3 = pnmResizeImage(i2, r);

    /* 回転角を度からラジアンに変換 */
    dt = dt * 3.14159265358979323846 / 180.0;
    /* sin, cos */
    s1 = sin(dt);
    c1 = cos(dt);

    /* 合成画像を含む最小の矩形を計算(画像 i1 の左上が座標の原点) */
    /* 上 */
    top = 0;
    if(top > (t = (int)floor(dy)))
	top = t;
    if(top > (t = (int)floor(dy - i3->width * s1)))
	top = t;
    if(top > (t = (int)floor(dy - i3->width * s1 + i3->height * c1)))
	top = t;
    if(top > (t = (int)floor(dy + i3->height * c1)))
	top = t;

    /* 下 */
    bottom = i1->height - 1;
    if(bottom < (t = (int)ceil(dy) - 1))
	bottom = t;
    if(bottom < (t = (int)ceil(dy - i3->width * s1) - 1))
	bottom = t;
    if(bottom < (t = (int)ceil(dy - i3->width * s1 + i3->height * c1) - 1))
	bottom = t;
    if(bottom < (t = (int)ceil(dy + i3->height * c1) - 1))
	bottom = t;

    /* 左 */
    left = 0;
    if(left > (t = (int)floor(dx)))
	left = t;
    if(left > (t = (int)floor(dx + i3->width * c1)))
	left = t;
    if(left > (t = (int)floor(dx + i3->width * c1 + i3->height * s1)))
	left = t;
    if(left > (t = (int)floor(dx + i3->height * s1)))
	left = t;

    /* 右 */
    right = i1->width - 1;
    if(right < (t = (int)ceil(dx) - 1))
	right = t;
    if(right < (t = (int)ceil(dx + i3->width * c1) - 1))
	right = t;
    if(right < (t = (int)ceil(dx + i3->width * c1 + i3->height * s1) - 1))
	right = t;
    if(right < (t = (int)ceil(dx + i3->height * s1) - 1))
	right = t;

    /* 合成画像の大きさを計算 */
    width = right - left + 1; 
    height = bottom - top + 1;

    /* 合成画像のメモリ領域を確保 */
    image = pnmAllocMemory(width, height, 255);

    /* 合成画像の初期化(白で埋める) */
    for(i=0; i<height; i++) {
	for(j=0; j<width; j++) {
	    image->pixel[i][j].r = 255;
	    image->pixel[i][j].g = 255;
	    image->pixel[i][j].b = 255;
	}
    }

    x1 = ( - dx + left) * c1 - ( - dy + top) * s1;
    y1 = ( - dx + left) * s1 + ( - dy + top) * c1;
    /* ループ：合成画像の各ピクセルについて */
    for(i=0; i<image->height; i++) {
	for(j=0; j<image->width; j++) {
	    x = (int)((j - dx + left) * c1 - (i - dy + top) * s1);
	    y = (int)((j - dx + left) * s1 + (i - dy + top) * c1);
	    if(x >= 0 && x < i3->width && y >= 0 && y < i3->height) {
/*
		image->pixel[i][j] = i3->pixel[y][x];
*/
	        /* i3 を合成画像に書き出す */
		image->pixel[i][j] = pnmGetPixel(i3, j, i, x1, y1);
		/* i1 の画像領域の場合、合成画像とi1の平均で上書きする */
		if(i+top >= 0 && i+top < i1->height &&
		   j+left >= 0 && j+left < i1->width) {
		    image->pixel[i][j].r = (image->pixel[i][j].r
					    + i1->pixel[i+top][j+left].r) / 2;
		    image->pixel[i][j].g = (image->pixel[i][j].g
					    + i1->pixel[i+top][j+left].g) / 2;
		    image->pixel[i][j].b = (image->pixel[i][j].b
					    + i1->pixel[i+top][j+left].b) / 2;
		}
	    } else if(i+top >= 0 && i+top < i1->height &&
		      j+left >= 0 && j+left < i1->width) {
	        /* i1 の画像領域の場合、i1を上書きする */
		image->pixel[i][j] = i1->pixel[i+top][j+left];
	    }
	}
    }
    /* 合成画像を返す */
    return image;
}
