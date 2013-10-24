/**
 * Copyright (c) 2011-2012 Nokia Corporation.
 */

#ifndef QTTILEGAMEENGINE_H
#define QTTILEGAMEENGINE_H

#include "gamewindow.h"
#include "tilegameinterfaces.h"
#include "tilerenderer.h"

#include <GLES2/gl2.h>

// Forward declarations
namespace GE {
    class AudioBuffer;
}

class SpriteBatch;

// Constants
#define AUDIO_SAMPLE_COUNT 10


class QtTileGameEngine : public GE::GameWindow, public TileRenderer
{
    Q_OBJECT

public:
    explicit QtTileGameEngine(QWidget *parent = 0);
    virtual ~QtTileGameEngine();

public: // From TileRenderer
    void renderTile(int fixedx,
                    int fixedy,
                    int fixedWidth,
                    int fixedHeight,
                    int angle,
                    int mode,
                    unsigned int tileIndex,
                    unsigned int arg);

    void renderBackground(unsigned int index);
    void renderForeground(unsigned int index) ;
    void effectNotify(TileGameEffect effect, unsigned int arg1, unsigned int arg2);
    int run(int fixedFrameTime16Bit);

public:
    void setSize(int w, int h);

protected: // From QWidget
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

protected: // From GE::GameWindow
    virtual void onCreate();
    virtual void onDestroy();
    void onUpdate(const float fFrameDelta);
    void onRender();
    void onPause();
    void onResume();

protected:
    GLuint loadGLTexture(const char *fn);
    void load(TextureID id);
    void toggleMute();
    void coordsToScreen(QMouseEvent *event, float &x, float &y);

protected: // Data
    ITileGame *m_game; // Owned
    SpriteBatch *m_spriteBatch; // Owned
    GE::AudioBuffer *m_audioBuffers[AUDIO_SAMPLE_COUNT]; // Audio samples
    float m_mousePressPos[2];
    float m_mousePressTime;
    int m_previousMode;
    bool m_mouseOn;
};

#endif // QTTILEGAMEENGINE_H
