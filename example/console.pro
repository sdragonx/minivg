#QT      += core gui

CONFIG -= qt

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# check is use shadow build
!equals(_PRO_FILE_PWD_, $${OUT_PWD}) {
    message(_PRO_FILE_PWD_: $$_PRO_FILE_PWD_)
    message(OUT_PWD: $$OUT_PWD)
    error(you is use shadow build)
}

# platform
contains(QT_ARCH, i386) {
    PLATFORM = x86
}
else {
    PLATFORM = x64
}

CONFIG += c++11 sse2
CONFIG += console

DEFINES -= UNICODE _UNICODE

# target
DESTDIR = bin
DLLDESTDIR = bin

# include and library directory
INCLUDEPATH += "include"
LIBS += -L./lib
LIBS += -L./lib/$${PLATFORM}

INCLUDEPATH += "x:/include"
LIBS += -Lx:/lib/windows/gcc/$${PLATFORM}

# intermediate directory
CONFIG(debug, debug|release) {
    MOC_DIR = obj/$${PLATFORM}/debug/moc
    OBJECTS_DIR = obj/$${PLATFORM}/debug
    RCC_DIR = obj/$${PLATFORM}/debug/rcc
    UI_DIR = obj/$${PLATFORM}/debug/ui
} else {
    MOC_DIR = obj/$${PLATFORM}/release/moc
    OBJECTS_DIR = obj/$${PLATFORM}/release
    RCC_DIR = obj/$${PLATFORM}/release/rcc
    UI_DIR = obj/$${PLATFORM}/release/ui
}

# windows configure
win32 {
    CONFIG += embed_manifest_dll
    # LIBS += -lopengl32 -lglu32
    LIBS += -luuid -lole32 -lcomdlg32 -ldwmapi -lwinmm
    LIBS += -lcomctl32 -luxtheme
    LIBS += -lgdi32 -lgdiplus
    LIBS += -lminiz
}

SOURCES += \
    main.cpp

HEADERS += \
    pch.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
