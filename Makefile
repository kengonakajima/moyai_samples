UNAME=$(shell uname)

SNAPPYOBJS=moyai/snappy/snappy-sinksource.o moyai/snappy/snappy-c.o moyai/snappy/snappy.o


ifeq ($(UNAME),Darwin)
CFLAGS=-I./moyai/ -I./moyai/libuv-1.20.2/include/ -I./moyai/glfw-3.2/include #-g
FREETYPELIB=moyai/freetype-2.4.10/objs/.libs/libfreetype.a
BZ2LIB=moyai/bzip2-1.0.6/libbz2.a
ZLIBLIB=moyai/zlib-1.2.7/libz.a
JPEGLIB=moyai/jpeg-8d/.libs/libjpeg.a
LIBFLAGS=moyai/libmoyaicl.a moyai/libuv-1.20.2/.libs/libuv.a -framework Cocoa -framework IOKit -framework OpenGL -framework CoreFoundation ./moyai/glfw-3.2/src/libglfw3.a -framework CoreVideo $(JPEGLIB) -L/usr/local/lib $(SNAPPYOBJS) moyai/libftgl.a $(FREETYPELIB) $(BZ2LIB) $(ZLIBLIB) -framework OpenAL moyai/libalut.a
else
CFLAGS=-I./moyai/ -std=c++11 -g -I./moyai/libuv-1.20.2/include
JPEGLIB=moyai/jpeg-8d/libjpeg.a
LIBFLAGS=moyai/libmoyaicl.a -L/usr/local/lib $(ZLIBLIB) $(SNAPPYOBJS) $(JPEGLIB) ./moyai/libuv-1.20.2/.libs/libuv.a -lpthread
endif


all : min dm rv duel bench scroll

min : min.cpp sample_common.cpp moyai/libmoyaicl.a
	g++ $(CFLAGS) min.cpp sample_common.cpp $(LIBFLAGS) -o min

dm : danmaku.cpp sample_common.cpp moyai/libmoyaicl.a
	g++ $(CFLAGS) danmaku.cpp sample_common.cpp $(LIBFLAGS) -o dm

rv : reversi.cpp sample_common.cpp moyai/libmoyaicl.a
	g++ $(CFLAGS) reversi.cpp sample_common.cpp $(LIBFLAGS) -o rv

duel : duel.cpp sample_common.cpp moyai/libmoyaicl.a
	g++ $(CFLAGS) duel.cpp sample_common.cpp $(LIBFLAGS) -o duel

bench : bench.cpp sample_common.cpp moyai/libmoyaicl.a
	g++ $(CFLAGS) bench.cpp sample_common.cpp $(LIBFLAGS) -o bench

scroll : scroll.cpp sample_common.cpp moyai/libmoyaicl.a
	g++ $(CFLAGS) scroll.cpp sample_common.cpp $(LIBFLAGS) -o scroll

clean :
	rm min dm rv duel bench scroll
	rm -rf *.dSYM
