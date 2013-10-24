# Copyright (c) 2011-2012 Nokia Corporation.

QT += core
CONFIG += mobility

TARGET = matchempoker
TEMPLATE = app
VERSION = 1.2

# Include the Qt GameEnabler source
include(./qtgameenabler/qtgameenabler.pri)

INCLUDEPATH += \
    spritebatch/src_general/ \
    spritebatch/src_gles2spritebatch/

SOURCES += \
    src/main.cpp \
    src/npclevel.cpp \
    src/ParticleEngine.cpp \
    src/QtTileGameEngine.cpp \
    src/tilegameinterfaces.cpp \
    src/tilesnpc.cpp \
    src/tilerenderer.cpp \
    spritebatch/src_general/SpriteBatch.cpp \
    spritebatch/src_gles2spritebatch/GLES2SpriteBatch.cpp

HEADERS += \
    src/npclevel.h \
    src/ParticleEngine.h \
    src/QtTileGameEngine.h \
    src/tilegameinterfaces.h \
    src/tilenpc.h \
    src/tilerenderer.h \
    spritebatch/src_general/SpriteBatch.h \
    spritebatch/src_gles2spritebatch/GLES2SpriteBatch.h

RESOURCES += \
    sounds.qrc \
    textures.qrc

symbian {
    message(Symbian build)

    TARGET = MatchemPoker
    TARGET.UID3 = 0xee69daed
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0xf00000

    MOBILITY += multimedia

    ICON = icons/matchempoker.svg

    # FOR UGLY VOLUMEHACK
    INCLUDEPATH += /epoc32/include/mmf/common
    INCLUDEPATH += /epoc32/include/mmf/server
    #LIBS += -lmmfdevsound

    LIBS += -lcentralrepository
    LIBS += -llibEGL -llibGLESv2 -lcone -leikcore -lavkon

    backup.sources = backup_registration.xml
    backup.path = !:/private/EE69dAED
}

contains(MEEGO_EDITION, harmattan) {
    message(Harmattan build)

    DEFINES += Q_WS_HARMATTAN
    MOBILITY += multimedia
    LIBS += -lEGL -lGLESv2

    target.path = /opt/$${TARGET}/bin
    desktopfile.files = qtc_packaging/debian_harmattan/$${TARGET}.desktop
    desktopfile.path = /usr/share/applications
    icon.files = icons/$${TARGET}80.png
    icon.path = /usr/share/icons/hicolor/64x64/apps

    # Classify the application as a game to support volume keys on Harmattan.
    gameclassify.files += qtc_packaging/debian_harmattan/$${TARGET}.conf
    gameclassify.path = /usr/share/policy/etc/syspart.conf.d
    
    INSTALLS += \
        target \
        desktopfile \
        icon \
        gameclassify
}

unix:!symbian {
    contains(DEFINES, DESKTOP) {
        message(Unix based desktop build)
        QT += multimedia

        INCLUDEPATH += ../SDKPackage_OGLES2/Builds/OGLES2/Include
        INCLUDEPATH += $(HOME)/Downloads/qt-mobility-opensource-src-1.2.0/install/include
        INCLUDEPATH += $(HOME)/Downloads/qt-mobility-opensource-src-1.2.0/install/include/QtMultimedia

        LIBS +=  -L../SDKPackage_OGLES2/Builds/OGLES2/LinuxPC/Lib
        LIBS +=  -L$(HOME)/Downloads/qt-mobility-opensource-src-1.2.0/install/lib
        LIBS += -lX11 -lEGL -lGLESv2
    }
}

windows {
    message(Windows build)
    QT += multimedia
    INCLUDEPATH += /PowerVRSDK/Builds/OGLES2/Include
    LIBS +=  -L/PowerVRSDK/Builds/OGLES2/WindowsPC/Lib
    LIBS += -llibEGL -llibGLESv2
}
