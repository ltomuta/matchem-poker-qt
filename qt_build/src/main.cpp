/**
 * Copyright (c) 2011-2014 Microsoft Mobile.
 */

#include <QtGui/QApplication>
#include <QPaintEngine>

#ifdef Q_OS_SYMBIAN
#include <eikenv.h>
#include <eikappui.h>
#include <aknenv.h>
#include <aknappui.h>
#endif

#include "gamewindow.h"
#include "QtTileGameEngine.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Lock orientation to landscape
#ifdef Q_OS_SYMBIAN
    CAknAppUi* appUi = dynamic_cast<CAknAppUi*>(CEikonEnv::Static()->AppUi());
    TRAPD(error,
          if (appUi) {
            appUi->SetOrientationL(CAknAppUi::EAppUiOrientationPortrait);
          }
    );
#endif

    GE::GameWindow *gameWindow = new QtTileGameEngine();
    gameWindow->create();
    gameWindow->setWindowState(Qt::WindowNoState);

    qApp->installEventFilter(gameWindow);

#ifdef Q_OS_WIN32
    gameWindow->setGeometry(0, 0, 360, 640);
    gameWindow->show();
#else
    gameWindow->showFullScreen();
#endif

    int result = a.exec();
    gameWindow->destroy();
    delete gameWindow;
    return result;
}
