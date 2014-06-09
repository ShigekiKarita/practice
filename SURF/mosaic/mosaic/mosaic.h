/*
  画像 image1 を基準とした場合に、2枚の画像が重なるように image2 の水
  平、垂直方向の移動量、回転角および拡大率を計算して返します。
  この関数から別の関数を呼び出しても構いません。

  引数：
  image1: 入力画像1
  image2: 入力画像2
  dx: 右方向の移動量（単位はピクセル）
  dy: 下方向の移動量（単位はピクセル）
  dt: 反時計回りの回転角（単位は度）
  r:  拡大率（倍）(pixels)
*/
void mosaic(IMAGE*, IMAGE*, double*, double*, double*, double*);
