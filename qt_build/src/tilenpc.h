/**
 * Copyright  2011 Nokia Corporation.
 *
 */

#ifndef __CTILE__
#define __CTILE__


#include "npclevel.h"
#include "ParticleEngine.h"


#define TILE_GRIDWIDTH 6
#define TILE_GRIDHEIGHT 8



class CTileNpc : public ITileGame
{
public:
    CTileNpc( TileRenderer *renderer, int x,int y, int width, int height );
    ~CTileNpc();

    int gameRun( int fixedFrameTime );
    int gameDraw();
    void handleGameMouseEvent(MouseEvent eventType, int fixedX, int fixedY);
    void gameStateChanged();

    int getIntAttribute( TileGameAttribute att, unsigned int arg );
    void setIntAttribyte( TileGameAttribute att, int set );

    void endGame();

    void nextLevel( int restartAt = -1 );


    int save();
    int load();

    void pause();
    void resume();

protected:	

    void writeEffectText(  int y, const char *text, int size, int fade );
    void writeText(  int x,int y, const char *text, int fade, int charSize, int charSpace  );
    void writeLevelCompletedString();



    void levelCompleted();
    void gameOver();




    CNpcLevel *m_level;



    // NÄMÄ arvot tallennetaan / ladataan
    int m_difficulty;
    int m_blockTimerEffect;				// kuinka paljon blokin tuhoaminen kasvattaa progressia.
    int m_timeTimerEffect;				// kuinka paljon aika hidastaa progressia
    int m_currentLevel;
    int m_targetTimer;
    int m_timer;
    int m_score;
    int m_displayScore;
    int m_highScore;
    int m_gameIsOn;
    int m_waitBeforeTimerStartsTime;

    int m_scoreRollPos[7];
    int m_scoreRollTargetPos[7];
    int m_scoreRollExTargetPos[7];
    int m_scoreRollInc[7];






    // game specific drawstuff
    int m_infoScreenDisplayCounter;
    int m_timeSinceLastScore;
    int m_logoWobble;
    int m_logoWobbleInc;
    int m_bgAngle;
    int m_completedTextCounter;
    int m_completedTextAngle;
    const char *m_levelCompletedPoem;
    int m_levelCompletedCounter;
    int m_gameOverCounter;
    int m_pauseCounter;
    int m_eventCounter;

    int m_menuCounter;
    int m_menuModeCounter;
    int m_effectAngle;

    void changeBg( int newBg = -1);
    int m_bgIndex1,m_bgIndex2;
    int m_fadingBgCounter;


};


#endif
