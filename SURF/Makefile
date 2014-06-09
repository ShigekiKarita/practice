
# FIT TO YOUR ENVIRONMENT
INCPATH = -I/usr/local/Cellar/opencv/2.4.9/include
LIBPATH = -L/usr/local/Cellar/opencv/2.4.9/lib
CXXFLAGS =  -std=c++11 -O2 -g -Wall -Wextra -stdlib=libc++ -fmessage-length=0 

TARGET =    main
OBJS =      main.o

LIBS =  -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_gpu


$(TARGET):  $(OBJS)
	g++ $(INCPATH) $(LIBPATH) -o $(TARGET) $(OBJS) $(LIBS)


all:    $(TARGET)


clean:
	rm -f $(OBJS) $(TARGET)
