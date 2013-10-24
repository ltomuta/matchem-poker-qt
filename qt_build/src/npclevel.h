/**
 * Copyright (c) 2011-2012 Nokia Corporation.
 *
 */


#ifndef __CTILELEVEL__
#define __CTILELEVEL__


#include <stdio.h>
#include "ParticleEngine.h"




#define GRIDFLAG_SELECTED 1			// selected by user.
#define GRIDFLAG_CALCULATION_TEMP 2		// temporarely used for calculations.
#define GRIDFLAG_MARKED 4			// marked
#define GRIDFLAG_HIDDEN 8			// is the tile visible at all
#define GRIDFLAG_PLUS_STRAIGHT	16	
#define GRIDFLAG_MINUS_STRAIGHT 32
#define GRIDFLAG_SAME_EARTH 64
#define GRIDFLAG_SAME_NUMBER 128


enum LevelState {
    LevelStateNormal,
    LevelStateBeginning,
    LevelStateGameOver,
    LevelStateCompleted,
    LevelStateIdle
};


/**
 *
 * Single item in a puzzlegrid
 *
 */
struct SNpcGridItem {
    int flags;
    int index;                                  // index to be rendered.
    short gridxpos, gridypos;

    int wobble;
    int wobbleinc;

    int genCount;

    int dropping;
    int destroying;                             // -1 if not

    int posoffsetx;
    int posoffsety;

    int lposx;
    int lposy;

    int animationCount;
};



class CNpcLevel {	
public:
    CNpcLevel( TileRenderer *renderer, ParticleEngine *pengine, int width, int height );
    virtual ~CNpcLevel();

    inline LevelState getState() { return m_state; }
    void setLevelState(LevelState state);

    void createLevel( int levelIndex );

    void run( int fixedFrameTime16Bit );
    void draw( TileRenderer &renderer );

    inline int getGridWidth() { return m_gridWidth; }
    inline int getGridHeight() { return m_gridHeight; }


    void handleMouseEvent(int type, int fixedX, int fixedY);




    void setGameArea( int x,int y, int width, int height ) {
        m_areaX = x;
        m_areaY = y;
        m_areaWidth = width;
        m_areaHeight = height;
    };



    int takeLevelScore() { int rval = m_levelScore; m_levelScore = 0; return rval; }
    int takeLevelProgressChange() { int rval = m_levelProgressChange; m_levelProgressChange = 0; return rval; }


    inline int xtoGameArea( const int sourcex );
    inline int ytoGameArea( const int sourcey );


    void saveToFile( FILE *file );
    void loadFromFile( FILE *file );

    int m_difficulty;

    int m_hasMoves;
    int *getCosineTable() { return m_cosineTable; }

    // Particlesprays, public so the game can access them as well
    SParticleSprayType m_scoreSpray;
    SParticleSprayType m_smokeSpray;
    SParticleSprayType m_sparkleSpray;
    SParticleSprayType m_fruitSpray;
    SParticleSprayType m_morphSpray;


    bool doingNothing() { return m_doingNothing; }
    void wobbleHint();

protected:
    bool m_doingNothing;
    static void scoreParticleRenderFunction( void *data, SParticle *p );
    void cancelSelection( int index );
    int randomBlock();


    int tryApplyChange( SNpcGridItem *i );
    int hasMovesLeft();			// return 1 if some move would work .. 0 otherwize
    int tryMoveWith( SNpcGridItem *i1, SNpcGridItem *i2 );

    void renderTileCaller( SNpcGridItem *i, TileRenderer &renderer );
    int m_areaX, m_areaY, m_areaWidth, m_areaHeight;


    void applyDestroy(unsigned int flag);
    int calculateSimilar(  int index, int x, int y, int checkmode, int dir );
    int checkDestroyAt( int blockx, int blocky , char apply = 1);			// return 1 if something is to be destroyed.
    int checkDestroyWholeLevel( char apply = 1);



    void zeroMask( unsigned int maskToZero );
    SNpcGridItem *getGridItemAt( int blockx, int blocky );

    void resetGrid( int width, int height );


    int m_destroyingRound;
    SNpcGridItem *m_changing[2];
    int m_dragBegan;
    int m_changingCounter;
    int m_illegalMoveCounter;

    SNpcGridItem *m_hint1, *m_hint2;

    int m_cosineTable[4096];
    int m_startupCounter;
    int m_gridWidth;
    int m_gridHeight;

    int m_itemWidth;
    int m_itemHeight;

    SNpcGridItem *m_grid;

    LevelState m_state;

    TileRenderer *m_renderer;
    ParticleEngine *m_particleEngine;

    int m_levelScore;
    int m_levelProgressChange;

    int m_floatingAngle;
    int m_currentLevel;

    int m_deckVisibility;

};



#endif
