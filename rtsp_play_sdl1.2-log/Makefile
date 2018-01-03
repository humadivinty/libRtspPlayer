
#目标文件
SRC    :=  $(shell find . -iname '*.c' -o -iname '*.cpp')

OBJ := $(patsubst %.c,%.o, $(patsubst %.cpp,%.o, $(SRC))) 

#程序名
PROGRAM = libRtsp_play_sdl1_2.so

CC := gcc
CXX := g++
AR := ar
STRIP := strip
#C/C++代码的宏定义
CFLAG := -DLINUX  -D_GNU_SOURCE -Wno-psabi -O2 -D__STDC_CONSTANT_MACROS

CXXFLAG := $(CFLAG)

#连接库路径
FFMPEG_DIR = /home/qinjj/FFmpeg_3.4

LIBPATH := -L$(FFMPEG_DIR)
LIBPATH += -L$(FFMPEG_DIR)/libavcodec
LIBPATH += -L$(FFMPEG_DIR)/libavformat
LIBPATH += -L$(FFMPEG_DIR)/libavutil
LIBPATH += -L$(FFMPEG_DIR)/libswscale

OUT_PUT_PATH := .
	 
LDFLAG := $(LIBPATH)
 
#STATIC_LIB :=  libavcodec.a libavformat.a libavutil.a libswscale.a
DYNAMIC_LIB := -lavcodec -lavformat -lavutil -lswscale `sdl-config --cflags --libs`
	   
#编译规则
all:$(PROGRAM)

INC := -I.\
		-I$(FFMPEG_DIR)

#依赖关系
Q := @
W := -w
#编译规则
.c.o:
	@echo "Compile $@"
	@echo $(Q)$(CC) $(CFLAG) -o $@ -g -c $< $(INC) $(W)

.cpp.o:
	@echo "Compile $@"
	$(Q)$(CXX) $(CXXFLAG) -o $@ -g -c $< $(INC) $(W)
	
	
$(PROGRAM):$(OBJ)
	$(Q)$(CXX) $(OBJ)  $(DYNAMIC_LIB)  $(LDFLAG)  -shared -fPIC  -o $(PROGRAM) $(W)
#	ar r $(PROGRAM) $(OBJ)
#	$(Q)rm *.o
	$(Q)mkdir -p $(OUT_PUT_PATH)
#	$(Q)cp $@ $(OUT_PUT_PATH)/$@

clean:
	$(Q)rm -rf $(PROGRAM) $(OUT_PUT_PATH)/$(PROGRAM) $(OBJ)  *.o *.bak
	
	
	
	
	
	
	
	
	
