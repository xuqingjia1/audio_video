TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        h264-util.c \
        main.c \
        rtp-packet.c

HEADERS += \
    h264-util.h \
    rtp-header.h \
    rtp-packet.h \
    rtp-util.h
