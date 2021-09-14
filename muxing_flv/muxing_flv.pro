TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c

macx {
    FFMPEG_HOME = /usr/local/ffmpeg
}

INCLUDEPATH += $${FFMPEG_HOME}/include

LIBS += -L $${FFMPEG_HOME}/lib \
        -lavdevice \
        -lavformat \
        -lavcodec \
        -lavutil \
        -lavfilter \
        -lpostproc\
        -lswresample\
        -lswscale\
