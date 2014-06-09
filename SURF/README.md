
## 動かし方

* ```Makefile``` のOpenCVパスを動かす環境に合わせる

```
INCPATH = -I/usr/local/Cellar/opencv/2.4.9/include
LIBPATH = -L/usr/local/Cellar/opencv/2.4.9/lib
```

* このディレクトリで次のコマンドを打つ

```
make
```

* 実行

```
./main
```

画像が表示されたら成功


## 注意

Visual Studioなど特殊な環境では mykeypoints.hpp 22行目

```c++:mykeypoints.hpp
//cv::initModule_nonfree();
```

上のコメントアウトを消せば動くかもしれません


## その他

shigekikarita@gmail.com まで
