/**
 * Copyright (c) 2011-2014 Microsoft Mobile.
 */

#include "QtTileGameEngine.h"

#include <QApplication>
#include <QDebug>
#include <QMouseEvent>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "audiobufferplayinstance.h"
#include "GLES2SpriteBatch.h"

// Constants
const char *AudioSamples[AUDIO_SAMPLE_COUNT] = {
    "click.wav",
    "change.wav",           // by milton
    "change_completed.wav", // by milton
    "destroy.wav",          // by milton
    "fanfare.wav",          // by milton
    "game_over.wav",        // by milton
    "illegal_move.wav",     // by milton
    "next_level.wav",       // by milton @ freesound
    "exp_bonus.wav",        // by mattwasser
    "xbonus.wav",           // by Christianjinnyzoe
};

#define AUDIO_SAMPLE_CLICK 0
#define AUDIO_SAMPLE_CHANGE 1
#define AUDIO_SAMPLE_CHANGE_COMPLETED 2
#define AUDIO_SAMPLE_DESTROY 3
#define AUDIO_LEVEL_COMPLETED 4
#define AUDIO_GAME_OVER 5
#define AUDIO_ILLEGAL_MOVE 6
#define AUDIO_NEXT_LEVEL 7
#define AUDIO_SAMPLE_EXP_BONUS 8
#define AUDIO_SAMPLE_X_BONUS 9

/*!
  Custom projection matrix used for rendering our virtual
  65536 x 65536 * aspectRatio screen into our actual resolution.
*/
const float MatrixProjection[16] =
{
#ifdef Q_WS_HARMATTAN
    0, 2.0f / (1.5f * 16384.0f), 0, 0,
    2.0f / (16384.0f), 0, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
#else
    2 / 65536.0f, 0, 0, -1,
    0, -2 / (1.5f * 65536.0f), 0, 1,
    0, 0, 1, 0,
    0, 0, 0, 1,
#endif
};



/*!
  \class QtTileGameEngine
  \brief The tile game engine for Qt port.
*/


/*!
  Constructor.
*/
QtTileGameEngine::QtTileGameEngine(QWidget *parent)
    : GE::GameWindow(parent),
      m_game(0),
      m_spriteBatch(0),
      m_mousePressTime(0.0f),
      m_previousMode(0),
      m_mouseOn(false)
{
    for (int i = 0; i < AUDIO_SAMPLE_COUNT; ++i) {
        m_audioBuffers[i] = 0;
    }
}


/*!
  Destructor.
*/
QtTileGameEngine::~QtTileGameEngine()
{
    delete m_game;
    delete m_spriteBatch;
}


/*!
  Updates the tile game.
*/
void QtTileGameEngine::onUpdate(const float fFrameDelta)
{
    float ft = fFrameDelta;

    if (ft > 0.3f) {
        ft = 0.3f;
    }

    if (ft <0.00001f) {
        ft = 0.00001f;
    }

    if (m_game) {
        m_game->run((int)(ft * 65536.0f));
    }
}


/**
 *
 * Render the ITileGame
 *
 */
void QtTileGameEngine::onRender()
{
     if (m_game)
        m_game->draw();
}


/*!
  Creates the system. OpenGL is initialized when arriving here.
*/
void QtTileGameEngine::onCreate()
{
    // Load the textures
    for (int i = 0; i < TexEndOfList; ++i) {
        load((TextureID)i);
    }

    delete m_spriteBatch;
    m_spriteBatch = new GLES2SpriteBatch(100,100);

    // Load the audio samples
    for (int f = 0; f < AUDIO_SAMPLE_COUNT; ++f) {
        QString str = ":/" + QString(AudioSamples[f]);
        m_audioBuffers[f] = GE::AudioBuffer::loadWav(str);
        qDebug() << "Loaded audio sample (" << str << "):" << m_audioBuffers[f];
    }

    m_game = createTileGame(this, -2500, 0,
                            65536 + 5000,
                            65536 * 3 / 2 + 2500);
    getMixer().setAbsoluteVolume(0.5f);
}


/**
 *
 * Destroy the system. OpenGL is about to be destroyed when entering here
 *
 */
void QtTileGameEngine::onDestroy()
{

    if (m_spriteBatch) delete m_spriteBatch;
    m_spriteBatch = 0;

    for (int f=0; f<AUDIO_SAMPLE_COUNT; f++) {
        qDebug() << "Releasing audiosample: " << m_audioBuffers[f];
        if (m_audioBuffers[f]) delete m_audioBuffers[f];
        m_audioBuffers[f] = 0;
    };

    if (m_game) {
        delete m_game;
        m_game = 0;
    };
}


GLuint QtTileGameEngine::loadGLTexture(const char *fn) {
    if (fn==0 || fn[0]==0) {
        qDebug() << "emphty texture: Ignoring.";
        return 0;
    }

    QString filename = QString(fn);
    QImage image = QImage(filename);
    if (image.width() < 1 || image.height() < 1) {
        qDebug() << "Error: Cannot find " << filename;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLuint* pTexData = new GLuint[image.width() * image.height()];
    GLuint* sdata = (GLuint*)image.bits();
    GLuint* tdata = pTexData;

    for (int y=0; y<image.height(); y++)
        for (int x=0; x<image.width(); x++) {
            //*tdata = *sdata;
            *tdata = ((*sdata&255) << 16) | (((*sdata>>8)&255) << 8)
                    | (((*sdata>>16)&255) << 0) | (((*sdata>>24)&255) << 24);
            sdata++;
            tdata++;
        };

    qDebug() << "texture : " << image.width() << " " << image.height();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pTexData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);

    delete [] pTexData;
    return texture;
};



void QtTileGameEngine::load(TextureID id)
{
    GameTexture *t=&gameTextures[0];
    while(1)
    {
        if(t->id==TexEndOfList) return;
        if(t->id==id)
        {
            glBindTexture(GL_TEXTURE_2D, t->id);
            char s[256];

            sprintf(s, ":/%s.png", t->name);
            t->glId=loadGLTexture(s);
            qDebug() << "Loading texture:" << s << " success("<< t->glId << ")";


            if(!t->tilesX) t->tilesX=1;
            if(!t->tilesY) t->tilesY=1;


            for(int i=0;i<16;i++) t->matrix[i]=0;
            t->matrix[0]=1.0f/t->tilesX;
            t->matrix[4*1+1]=1.0f/t->tilesY;
            t->matrix[4*2+2]=1.0f;
            t->matrix[4*3+3]=1.0f;

            return;
        }
        t++;
    }
}


/*!
*/
void QtTileGameEngine::mousePressEvent(QMouseEvent *event)
{
    if (!m_game) {
        return;
    }

#ifdef Q_WS_HARMATTAN
    int y = (event->x() << 16) / width();
    int x = 65536 - (event->y() << 16) / height();

#else
    int x = (event->x() << 16) / width();
    int y = (event->y() << 16) / height();
#endif

    y = y * 3 / 2;
    qDebug() << "Mouse press at [" << x << "," << y << "]";
    m_game->handleMouseEvent(ButtonDown, x, y);
}


/*!
*/
void QtTileGameEngine::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_game) {
        return;
    }

#ifdef Q_WS_HARMATTAN
    int y = (event->x() << 16) / width();
    int x = 65536 - (event->y() << 16) / height();

#else
    int x = (event->x() << 16) / width();
    int y = (event->y() << 16) / height();
#endif

    y = y * 3 / 2;
    m_game->handleMouseEvent(ButtonUp, x, y);

}


/*!
*/
void QtTileGameEngine::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_game) {
        return;
    }

#ifdef Q_WS_HARMATTAN
    int y = (event->x() << 16) / width();
    int x = 65536 - (event->y() << 16) / height();

#else
    int x = (event->x() << 16) / width();
    int y = (event->y() << 16) / height();
#endif

    y = y * 3 / 2;
    m_game->handleMouseEvent(MouseDrag, x, y);
}


void QtTileGameEngine::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void QtTileGameEngine::onPause() {

    m_game->pause();
}

void QtTileGameEngine::onResume()
{
    m_game->resume();
}


void QtTileGameEngine::setSize(int w, int h)
{
    qDebug() << "setSize: viewport to:" << w << "," << h;
    glViewport(0, 0, w, h);
}


void QtTileGameEngine::effectNotify(TileGameEffect effect,
                                    unsigned int arg1,
                                    unsigned int arg2)
{
    Q_UNUSED(arg2);

    switch (effect) {
    case ExitEffect: {
        qApp->exit(0);
        break;
    }
    case DestroyingEffect:
    case DestroyingBonusEffect: {
        if (audioEnabled()) {
            m_audioBuffers[AUDIO_SAMPLE_DESTROY]->playWithMixer(getMixer());

            if (arg1 > 0) {
                GE::AudioBufferPlayInstance *i =
                        m_audioBuffers[AUDIO_SAMPLE_EXP_BONUS]->playWithMixer(getMixer());
                i->setSpeed(1.0f + (float)arg1 / 12.0f);
            }
        }

        break;
    }
    case GameOverEffect: {
        if (audioEnabled()) {
            m_audioBuffers[AUDIO_GAME_OVER]->playWithMixer(getMixer());
        }

        break;
    }
    case NewLevelEffect: {
        if (audioEnabled()) {
            m_audioBuffers[AUDIO_NEXT_LEVEL]->playWithMixer(getMixer());
        }

        break;
    }
    case LevelCompletedEffect: {
        if (audioEnabled()) {
            m_audioBuffers[AUDIO_LEVEL_COMPLETED]->playWithMixer(getMixer());
        }

        break;
    }
    case XBonusEffect: {
        if (audioEnabled()) {
            m_audioBuffers[AUDIO_SAMPLE_X_BONUS]->playWithMixer(getMixer());
        }

        break;
    }
    case BlockBeginFinishedEffect: {
        if (audioEnabled()) {
            GE::AudioBufferPlayInstance *i =
                    m_audioBuffers[AUDIO_SAMPLE_CHANGE]->playWithMixer(getMixer());
            i->setLeftVolume(0.1f);
            i->setRightVolume(0.1f);
        }

        break;
    }
    case BlockVanishStartedEffect: {
        if (audioEnabled()) {
            GE::AudioBufferPlayInstance *i =
                    m_audioBuffers[AUDIO_SAMPLE_CHANGE_COMPLETED]->playWithMixer(getMixer());
            i->setLeftVolume(0.1f);
            i->setRightVolume(0.1f);
        }

        break;
    }
    case ChangingEffect: {
        if (audioEnabled()) {
            GE::AudioBufferPlayInstance *i =
                    m_audioBuffers[AUDIO_SAMPLE_CHANGE]->playWithMixer(getMixer());
            i->setSpeed(0.8 + (float)(rand() & 255) / 255.0f * 0.4f);
        }

        break;
    }
    case ChangeCompletedEffect: {
        if (audioEnabled()) {
            GE::AudioBufferPlayInstance *i =
                    m_audioBuffers[AUDIO_SAMPLE_CHANGE_COMPLETED]->playWithMixer(getMixer());
            i->setSpeed(0.8 + (float)(rand() & 255) / 255.0f * 0.4f);
        }

        break;
    }
    case IllegalMoveEffect: {
        if (audioEnabled()) {
            GE::AudioBufferPlayInstance *i =
                    m_audioBuffers[AUDIO_ILLEGAL_MOVE]->playWithMixer(getMixer());
            i->setSpeed(0.9 + (float)(rand() & 255) / 255.0f * 0.2f);
        }

        break;
    }
    case ClickEffect:
    case MenuEffect: {
        if (audioEnabled()) {
            m_audioBuffers[AUDIO_SAMPLE_CLICK]->playWithMixer(getMixer());
        }
        break;
    }
    default: {
        break;
    }
    } // switch (effect)
}


void QtTileGameEngine::renderTile(int x, int y,
                                   int w, int h,
                                   int angle,
                                   int mode,
                                   unsigned int tileIndex, unsigned int arg)
{
    GameTexture *gt = gameTextures + ((tileIndex>>16)&0xff) -1;
    SpriteDrawInfo sdi;
    sdi.textureHandle = gt->glId;


    // Change the blending mode if required
    if (mode != m_previousMode) {
        m_spriteBatch->end();

        if (mode == 0) {
            m_spriteBatch->begin(SpriteBatch::eALPHA,
                                 SpriteBatch::eCUSTOMPROJECTION,
                                 (float*)MatrixProjection);
        }
        else {
            m_spriteBatch->begin(SpriteBatch::eADDITIVE,
                                 SpriteBatch::eCUSTOMPROJECTION,
                                 (float*)MatrixProjection);
        }

        m_previousMode = mode;
    }

    int t(tileIndex & 65535);

    // Since difficulties with custom projection, the harmattan version is aligned correctly here
#ifdef Q_WS_HARMATTAN
    w>>=2;
    h>>=2;
    x>>=2;
    y>>=2;
    sdi.setTargetPos(x+(w>>1)-16384/2,y+(h>>1)-16384*3/4);
#else
    sdi.setTargetPos(x+(w>>1),y+(h>>1));
#endif

    sdi.angle = (float)angle / 65536.0f * 3.14159f*2.0f;
    sdi.a = 1-((tileIndex>>24)&0xff)/255.0f;
    sdi.setScale(w,-h);

    sdi.setSourceRect((t%gt->tilesX)/(float)gt->tilesX,
                      (t/gt->tilesX%gt->tilesY)/(float)gt->tilesY,
                      1.0f/gt->tilesX,
                      1.0f/gt->tilesY);


    m_spriteBatch->draw(&sdi);
}


/*!
  Renders the background.
*/
void QtTileGameEngine::renderBackground(unsigned int index)
{
    Q_UNUSED(index);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_previousMode = 0;
    glDisable(GL_CULL_FACE);

    if (m_spriteBatch) {
        m_spriteBatch->begin(SpriteBatch::eALPHA,
                             SpriteBatch::eCUSTOMPROJECTION,
                             (float*)MatrixProjection);
    }
}


/*!
  Renders the foreground.
*/
void QtTileGameEngine::renderForeground(unsigned int index)
{
    Q_UNUSED(index);

    if (m_spriteBatch) {
        m_spriteBatch->end();
    }
}


/*!
*/
int QtTileGameEngine::run(int fixedFrameTime16Bit)
{
    return 1;
}
