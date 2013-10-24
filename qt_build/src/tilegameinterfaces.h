/**
 * Copyright (c) 2011-2012 Nokia Corporation.
 */

#ifndef TILEGAMEINTERFACES_H
#define TILEGAMEINTERFACES_H

// Forward declarations
class ParticleEngine;
class TileRenderer;


enum TextureID
{
    TexLogo = 1,
    TexPieces,
    TexPiecesSelected,
    TexFontScore,
    TexGradient,
    TexMeter,
    TexMeterBase,
    TexParticle,
    TexFont,
    TexBackground,
    TexExtra1,
    TexEndOfList
};


struct GameTexture
{
    TextureID id;
    const char *name;
    int tilesX;
    int tilesY;
    unsigned int glId;
    float matrix[16];
};


extern GameTexture gameTextures[];


enum TileGameEffect {
    ClickEffect = 1,
    ChangingEffect,
    ChangeCompletedEffect,
    LevelCompletedEffect,
    NewLevelEffect,
    GameOverEffect,
    IllegalMoveEffect,
    DestroyingEffect,
    DestroyingBonusEffect,
    MenuEffect,
    BuyMoreEffect,
    ExitEffect,
    BlockBeginFinishedEffect,
    BlockVanishStartedEffect,
    XBonusEffect,
    ScoreChangedEffect,
    TileGameEffectMax // Marker for max. number of items
};


enum TileGameAttribute {
    AttributeScore = 1,
    AttributeHighScore,
    AttributeCurrentLevel,
    AttributeDifficulty,
    TimeLimitCurrent,
    TimeLimitMax
};


enum TileGameState {
    NotSetState = 1,
    MenuState,
    RunningState,
    GameOverState,
    PausedState,
    ShowingInfoScreenState
};


enum MouseEvent {
    ButtonUp = 1,
    ButtonDown,
    MouseDrag
};


class ITileGame
{
public:
    ITileGame(TileRenderer *renderer,
              int x = 0,
              int y = 0,
              int width = 65536,
              int height = 65536);
    virtual ~ITileGame();

public: // Pure virtual methods
    virtual void handleGameMouseEvent(MouseEvent eventType,
                                      int fixedX,
                                      int fixedY) = 0;
    virtual int gameRun(int fixedFrameTime16Bit) = 0;
    virtual int gameDraw() = 0;
    virtual void gameStateChanged() = 0;
    virtual int getIntAttribute(TileGameAttribute att, unsigned int arg) = 0;
    virtual void setIntAttribyte(TileGameAttribute att, int set) = 0;

    /*!
      The following methods should return non-zero if the game is ongoing.
    */
    virtual int load() = 0;
    virtual int save() = 0;

    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void endGame() = 0;

public:
    int run(int fixedFrameTime16Bit);
    int draw();
    void setGameState(TileGameState state);
    void setGameArea(int x, int y, int width, int height);
    void handleMouseEvent(MouseEvent eventType, int fixedX, int fixedY);

protected: // Data
    ParticleEngine *m_particleEngine; // Owned
    TileRenderer *m_renderer; // Not owned
    int m_areaX;
    int m_areaY;
    int m_areaWidth;
    int m_areaHeight;

    // Basic functionality of any game (which we are doing)
    TileGameState m_state;
    int	m_logoState; // states: 0 - invisible, 65536 - completely visible
    int m_hudState;
};


extern "C" ITileGame *createTileGame(TileRenderer *renderer,
                                     int x = 0,
                                     int y = 0,
                                     int width = 65536,
                                     int height=65536);

#endif // TILEGAMEINTERFACES_H
