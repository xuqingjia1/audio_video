# Automatically generated by configure - do not modify!
shared=yes
build_suffix=
prefix=/usr/local/ffmpeg
libdir=${prefix}/lib
incdir=${prefix}/include
rpath=
source_path=.
LIBPREF=lib
LIBSUF=.a
extralibs_avutil="-pthread -lm -framework VideoToolbox -framework CoreFoundation -framework CoreMedia -framework CoreVideo -framework CoreServices"
extralibs_avcodec="-liconv -lm -llzma -lz -framework AudioToolbox -L/usr/local/Cellar/fdk-aac/2.0.1/lib -lfdk-aac -L/usr/local/Cellar/x264/r3049/lib -lx264 -L/usr/local/Cellar/x265/3.5/lib -lx265 -pthread -framework VideoToolbox -framework CoreFoundation -framework CoreMedia -framework CoreVideo -framework CoreServices"
extralibs_avformat="-lm -lbz2 -lz -Wl,-framework,CoreFoundation -Wl,-framework,Security"
extralibs_avdevice="-framework Foundation -lm -framework AVFoundation -framework CoreVideo -framework CoreMedia -pthread -framework CoreGraphics -L/usr/local/Cellar/libxcb/1.14_1/lib -lxcb -L/usr/local/Cellar/libxcb/1.14_1/lib -lxcb-shm -L/usr/local/Cellar/libxcb/1.14_1/lib -lxcb-shape -L/usr/local/Cellar/libxcb/1.14_1/lib -lxcb-xfixes -L/usr/local/lib -lSDL2"
extralibs_avfilter="-framework OpenGL -framework OpenGL -pthread -lm -framework CoreImage -framework AppKit"
extralibs_avresample="-lm"
extralibs_postproc="-lm"
extralibs_swscale="-lm"
extralibs_swresample="-lm"
avdevice_deps="avfilter swscale postproc avformat avcodec swresample avutil"
avfilter_deps="swscale postproc avformat avcodec swresample avutil"
swscale_deps="avutil"
postproc_deps="avutil"
avformat_deps="avcodec swresample avutil"
avcodec_deps="swresample avutil"
swresample_deps="avutil"
avresample_deps="avutil"
avutil_deps=""
