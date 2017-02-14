TEMPLATE = app

QT += qml quick  widgets
contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    QT += androidextras
}

SOURCES += main.cpp \
    wifi-client.cpp \
    image-item.cpp \
    decode_process.cpp \
    video_decode.cpp \
    ms-net/IRProtocol.cpp \
    ms-net/msendian.cpp \
    image-widget.cpp

HEADERS += \
    wifi-client.h \
    image-item.h \
    decode_process.h \
    video_decode.h \
    video_decode_api.h \
    ms-net/IRProtocol.h \
    ms-net/msendian.h \
    image-widget.h


RESOURCES += qml.qrc \
    src.qrc

INCLUDEPATH += "$$PWD/net_discover/include" "$$PWD/libevent/include" "$$PWD/ffmpeg/include"

linux{
    LIBS += -L"$$PWD/net_discover/libs/linux" -lyf-net-discover
    LIBS += -L"$$_PRO_FILE_PWD_/libevent/libs/linux" -levent
    LIBS += -L"$$PWD/ffmpeg/libs/linux" -lavformat-57 -lavcodec-57 -lavdevice-57 -lavfilter-6 -lavutil-55 -lswresample-2 -lswscale-4
}


#contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
#    LIBS += -L"$$PWD/libevent/libs/android" -levent
#    LIBS += "$$PWD/net_discover/libs/android/libyf-net-discover.so"
#    LIBS += "$$PWD/ffmpeg/libs/android/libavformat-57.so" \
#            "$$PWD/ffmpeg/libs/android/libavcodec-57.so" \
#            "$$PWD/ffmpeg/libs/android/libavdevice-57.so" \
#            "$$PWD/ffmpeg/libs/android/libavfilter-6.so" \
#            "$$PWD/ffmpeg/libs/android/libavutil-55.so" \
#            "$$PWD/ffmpeg/libs/android/libswresample-2.so" \
#            "$$PWD/ffmpeg/libs/android/libswscale-4.so" \
#            "$$PWD/ffmpeg/libs/android/libpostproc-54.so"
#}
#contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
#    ANDROID_EXTRA_LIBS = \
#        $$PWD/ffmpeg/libs/android/libavcodec-57.so \
#        $$PWD/ffmpeg/libs/android/libavdevice-57.so \
#        $$PWD/ffmpeg/libs/android/libavfilter-6.so \
#        $$PWD/ffmpeg/libs/android/libavformat-57.so \
#        $$PWD/ffmpeg/libs/android/libavutil-55.so \
#        $$PWD/ffmpeg/libs/android/libpostproc-54.so \
#        $$PWD/ffmpeg/libs/android/libswresample-2.so \
#        $$PWD/ffmpeg/libs/android/libswscale-4.so \
#        $$PWD/net_discover/libs/android/libyf-net-discover.so \
#}

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
#include(deployment.pri)

#DISTFILES += \
#    ImageDisplay.qml \
#    main.qml \
#    MainForm.ui.qml




