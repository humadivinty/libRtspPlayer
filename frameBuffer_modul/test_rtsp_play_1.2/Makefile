
TARGET := test_rtsp_play_1.2

RTSPDIR = /home/Ats/rtsp_play_frameBuffer/rtsp_play_fb

INC := -I$(RTSPDIR)

FFMPEG_DIR = /usr/local/lib


LIB_PATH = -L$(RTSPDIR)
LIB_PATH += -L$(FFMPEG_DIR)
LIB_PATH += -L$(FFMPEG_DIR)/libavcodec
LIB_PATH += -L$(FFMPEG_DIR)/libavformat
LIB_PATH += -L$(FFMPEG_DIR)/libavutil
LIB_PATH += -L$(FFMPEG_DIR)/libswscale
LIB_PATH += -L$(FFMPEG_DIR)/libavfilter
LIB_PATH += -L$(FFMPEG_DIR)/libswresample


CFLAG := -DLINUX  -D_GNU_SOURCE -Wno-psabi -O2

LIBS := -lpthread -ldl -lz -lm -lrt -lswresample -lavcodec -lavformat -lavutil -lswscale  -lRtsp_play_fb

objects := main.o


test_rtsp_play : $(objects)
	g++ $(CFLAG) -o $(TARGET) $(objects) $(INC) $(LIB_PATH) $(LIBS) 

main.o : main.cpp
	g++ -c  main.cpp $(INC) $(CFLAG)

clean :
	-rm $(objects) $(TARGET)
