CFLAGS=-I./moyai/ -I/usr/local/Cellar/libuv/1.8.0/include/ -I./moyai/glfw-3.2/include
UNTZDEPENDLIB=-framework AudioToolbox /usr/local/lib/libvorbis.a /usr/local/lib/libvorbisfile.a /usr/local/lib/libogg.a
SNAPPYOBJS=moyai/snappy/snappy-sinksource.o moyai/snappy/snappy-c.o moyai/snappy/snappy.o
FREETYPELIB=moyai/freetype-2.4.10/objs/.libs/libfreetype.a
BZ2LIB=moyai/bzip2-1.0.6/libbz2.a
ZLIBLIB=moyai/zlib-1.2.7/libz.a
LIBFLAGS=moyai/libmoyaicl.a /usr/local/Cellar/libuv/1.8.0/lib/libuv.a moyai/untz/libuntz.a $(UNTZDEPENDLIB) -framework Cocoa -framework IOKit -framework OpenGL -framework CoreFoundation ./moyai/glfw-3.2/src/libglfw3.a -framework CoreVideo -ljpeg -L/usr/local/lib $(SNAPPYOBJS) moyai/libftgl.a $(FREETYPELIB) $(BZ2LIB) $(ZLIBLIB)



all : dm

dm : danmaku.cpp
	g++ $(CFLAGS) danmaku.cpp $(LIBFLAGS) -o dm

clean :
	rm dm

