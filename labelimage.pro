TARGET = Image" "Labeler
QT += widgets
VERSION_MAJOR = 2
VERSION_MINOR = 1
VERSION_BUILD = 2


DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"
INCLUDEPATH += D:/OpenCV4.4_dl/install/include
CONFIG(debug,debug|release){
LIBS += D:\OpenCV4.4_dl\install\x64\vc15\lib\opencv_world440d.lib
}
CONFIG(realese,debug|release){
LIBS += D:\OpenCV4.4_dl\install\x64\vc15\lib\opencv_world440.lib
}
#Target version
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

HEADERS       = \
    augDialog.h \
    detwindow.h \
    boxitem.h \
    FreeImage.h \
    mainwindow.h \
    pointitem.h \
    pointitemmimedata.h \
    segwindow.h \
    commands.h \
    customview.h \
    customscene.h \
    boxitemmimedata.h \
    mfile.h \
    task.h \
    ocrwindow.h \
    classwindow.h
SOURCES       = \
    augDialog.cpp \
    detwindow.cpp \
    main.cpp \
    boxitem.cpp \
    mainwindow.cpp \
    pointitem.cpp \
    pointitemmimedata.cpp \
    segwindow.cpp \
    commands.cpp \
    customview.cpp \
    customscene.cpp \
    boxitemmimedata.cpp \
    mfile.cpp \
    task.cpp \
    ocrwindow.cpp \
    classwindow.cpp

# install
# target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgets/labelimage
# INSTALLS += target

RESOURCES += \
    labelimage.qrc

win32 {
    LIBS += -L$$PWD/lib/ -lFreeImage
}
unix {
    LIBS += -L$$PWD/lib/ -lfreeimage -ltiff
    # install
    target.source = $$TARGET
    target.path = /usr/bin
    INSTALLS = target
}


DEPENDPATH += $$PWD/.

TRANSLATIONS += $$PWD/languages/zh_CN.ts \
               $$PWD/languages/en_US.ts

CODECFORSRC = UTF-8

win32 {
    RC_ICONS += images/draw.ico
}

DISTFILES += \
    images/128.png \
    images/about.png \
    images/actual-size.png \
    images/augment.png \
    images/bb.png \
    images/copy.png \
    images/cut.png \
    images/draw.ico \
    images/draw.png \
    images/edit.png \
    images/en_US.png \
    images/fit-to-window.png \
    images/folder.png \
    images/fullscreen.png \
    images/help.png \
    images/home.png \
    images/language.png \
    images/magic.png \
    images/next.png \
    images/pan.png \
    images/paste.png \
    images/prev.png \
    images/previous.png \
    images/quit.png \
    images/redo.png \
    images/resize.png \
    images/tt.png \
    images/undo.png \
    images/zh_CN.png \
    images/zoom-in.png \
    images/zoom-out.png

FORMS += \
    task.ui
