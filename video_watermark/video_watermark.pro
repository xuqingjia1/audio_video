TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c



macx {
    FFMPEG_HOME = /usr/local/Cellar/ffmpeg/4.4_1
#    FFMPEG_HOME = /usr/local/ffmpeg
}
INCLUDEPATH += $${FFMPEG_HOME}/include

LIBS += -L$${FFMPEG_HOME}/lib \
        -lavcodec \
        -lavformat \
        -lavutil \
        -lswresample \
        -lswscale \
        -lavdevice \
        -lavfilter \
        -lpostproc \
