TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        aac-util.c \
        h264-util.c \
        main.c \
        rtp-h264-pack.c \
        rtp-h264-unpack.c \
        rtp-mpeg4-generic-pack.c \
        rtp-mpeg4-generic-unpack.c \
        rtp-packet.c \
        rtp-paylaod-helper.c \
        rtp-payload.c

HEADERS += \
    aac-util.h \
    h264-util.h \
    rtp-header.h \
    rtp-packet.h \
    rtp-param.h \
    rtp-payload-helper.h \
    rtp-payload-internal.h \
    rtp-payload.h \
    rtp-profile.h \
    rtp-util.h
