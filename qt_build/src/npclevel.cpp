/**
 * Copyright  2011 Nokia Corporation.
 *
 */

#include "npclevel.h"

#include <math.h> // For cosine
#include <memory.h>	// For memcpy() and memset()
#include <stdio.h> // For debug
#include <stdlib.h>	// For random, can be changed if needed
#include <string.h>	// For strlen()

#include "tilegameinterfaces.h"
#include "tilerenderer.h"

// Constants
#define EMPTY_CARD 100
#define JOKER_CARD 101

#define DECK_POSITION_X (52000)
#define DECK_POSITION_Y (14000)

#define TILE_SIZE_ADD 3000
#define DESTROYINGSPEED 1024 // 256 = 1
#define ALLOW_VOID_MOVES // Allow moves that do not result destroying

#define SQUARE_BASED // Square-based or hexagon-based


#ifndef __int64
#define __int64 long long int
#endif



CNpcLevel::CNpcLevel(TileRenderer *renderer, ParticleEngine *pengine, int width, int height) : m_renderer(renderer)
{
    m_particleEngine = pengine;
    m_deckVisibility = 0;
    m_destroyingRound = 0;
    m_dragBegan = 0;
    m_hint1 = 0;
    m_hint2 = 0;
    m_levelScore = 0;
    m_levelProgressChange = 0;

    m_grid = 0;
    m_difficulty = 1;
    m_gridWidth = 0;
    m_gridHeight = 0;
    m_floatingAngle = 0;
    m_state = LevelStateIdle;

    m_levelScore = 0;
    m_levelProgressChange = 0;

    m_currentLevel = 0;
    m_hasMoves = 0;

    m_changing[0] = 0;
    m_changing[1] = 0;
    setGameArea(0,0, 65536, 65536);
    m_changingCounter = 0;
    m_illegalMoveCounter = 0;
    for (int f=0; f<4096; f++) {
        m_cosineTable[f] = (int)(cos((float)f/4096.0f * 3.14159f*2.0f) * 65536.0f);
    }
    resetGrid(width, height);


    // Reset sprays
    pengine->createSprayType(&m_scoreSpray, (TexFont<<16)+10, 0, 0,0,65535,0, 65536/8, 0,0,0, scoreParticleRenderFunction, this);
    pengine->createSprayType(&m_smokeSpray, (TexParticle<<16),0, 0,0, 65536/4,65536/4, 65536/16,65536/16, 65536/4,65536/8,0,0,0,65536,0,8000);
    pengine->createSprayType(&m_sparkleSpray, (TexParticle<<16)+2,0, 80000,3000, 32536,80536, 65536/20,65536/16, -9000,6000,0,0,0,65536,-32000,32000, 1);
    pengine->createSprayType(&m_fruitSpray, (TexPieces<<16),8, 164000,4000, 65536,65536, 65536/10,65536/10, -5500,4000,0,0,0,65536,-90000,180000, 0);
    pengine->createSprayType(&m_morphSpray, (TexParticle<<16)+3, 0,
                              0,2000, 65536/3,65536/8, 65536/2,0, -32000,0,0,0,0,65536,0,16000, 1);
}


CNpcLevel::~CNpcLevel()
{
    resetGrid(0,0);				// release
}


/**
 *
 * Custom particlerender function used for flying texts in the game.
 *
 */
void CNpcLevel::scoreParticleRenderFunction(void *data, SParticle *p)
{
    CNpcLevel *l = (CNpcLevel*)data;
    int x = p->x;// -  p->size/2;
    int y = p->y; // - p->size/2;
    char testr[20];

    int size = 120000 + 65536-p->lifeTime/2;
    size/=14;

    y-=size/2;

    if ((p->userData>>16)==0) {
        size/=2;
        int val = (p->userData&255);
        int type = ((p->userData>>8)&255);
        switch (type) {
        case 0:
        default:
            sprintf(testr, "%d of a kind", val);
            break;

        case 1:
            sprintf(testr, "flush of %d", val);
            break;

        case 2:
            sprintf(testr, "straight of %d", val);
            break;
        }
    } else {

        int j =((p->userData>>16)&255);
        if (j==50) {
            size/=2;
            strcpy(testr, "solve bonus");
        } else {

            if (j==51) {
                size/=2;
                strcpy(testr, "taptap");
            } else
                sprintf(testr, "x%d", j);
        }
    }
    char *text = testr;
    x-= strlen(testr)*size*3/5/2;

    char ch;
    // end fade
    int fade =255-(p->lifeTime>>6);
    if (fade<0) fade = 0;

    if (fade==0) {	// START FADE
        fade = (65536-p->lifeTime)/20;
        if (fade<0) fade = 0;
        //fade = (p->lifeTime>>7);
        //if (fade>255) fade = 255;
        fade>>=8;
    }
    while (*text!=0) {
        if (*text!=' ') {
            ch = *text-32;
            l->m_renderer->renderTile(x,y,size,size, 0, 0, (TexFont<<16) | ch | (fade<<24), 0);
        }
        x+=size*3/5;
        text++;

    }
}



/**
 *
 * Set new state for the level. Change required parameters accordingly
 *
 */
void CNpcLevel::setLevelState(LevelState state)
{
    m_state = state;

    switch (m_state) {
    case LevelStateBeginning:
        zeroMask(GRIDFLAG_CALCULATION_TEMP);
        m_renderer->effectNotify(NewLevelEffect, 0,0);
        m_startupCounter = 65536*5;
        break;

    case LevelStateGameOver:
        m_startupCounter = (m_gridWidth*m_gridHeight)*65536 ;
        break;

    case LevelStateCompleted:
        zeroMask(GRIDFLAG_CALCULATION_TEMP);
        m_startupCounter = 65536*5;
        break;

    default:
        break;
    }
}



/**
 *
 * Return a random block for this game
 *
 */
int CNpcLevel::randomBlock()
{
    int amount_of_numbers = 7;
    int rval =  (rand()&3)*13 + ((13-amount_of_numbers)+(rand()%amount_of_numbers));
    int empty_prob = -2+m_currentLevel * 3;
    if (empty_prob>38) empty_prob = 48;
    if ((rand() & 255) < empty_prob) rval = EMPTY_CARD;
    return rval;
}


/**
 *
 * Reset this level to a specific level index.
 * Reset required attributes
 *
 */
void CNpcLevel::createLevel(int levelIndex) {

    m_currentLevel = levelIndex;
    m_hint1 = 0;
    m_hint2 = 0;
    int tecount = 0;

    do {
        // "plant the seeds"
        for (int f = 0; f < m_gridWidth * m_gridHeight; f++) {
            m_grid[f].index = randomBlock();
            m_grid[f].flags = 0;
            m_grid[f].animationCount = (rand()&65535)*8;
            m_grid[f].destroying = -1;
            m_grid[f].dropping = -1;
            m_grid[f].wobble = 0;
            m_grid[f].wobbleinc = 0;
        }

        tecount++;
    } while (checkDestroyWholeLevel(0) != 0 || hasMovesLeft() != 1);

    m_hasMoves = 1;
    zeroMask(0xFFFFFFFF);
    m_changing[0] = 0;
    m_changing[1] = 0;
    m_levelScore = 0;
    m_levelProgressChange = 0;
    cancelSelection(0);
    cancelSelection(1);
    m_destroyingRound = 0;
    hasMovesLeft();         // update hint

    // We're done. Let the game begin
    setLevelState(LevelStateBeginning);
}



/**
 *
 * Recreate the grid with desired width and height
 *
 */
void CNpcLevel::resetGrid(int width, int height)
{
    if (m_grid) {
        delete [] m_grid;
        m_grid = 0;
    }

    m_hint1 = 0;
    m_hint2 = 0;
    m_gridWidth = width;
    m_gridHeight = height;

    // Was a release call, return
    if (m_gridHeight<1 || m_gridWidth<1) return;

    // Create
    m_grid = new SNpcGridItem[ m_gridWidth * m_gridHeight ];
    m_itemWidth = 65536 / m_gridWidth;
    m_itemHeight = 65536 / m_gridHeight;
    int w = m_itemWidth;
    int h = m_itemHeight;

    int x,y;

    // (Re)calculate coordinates
    for (int g=0; g<m_gridWidth; g++) {
        x = (g<<16)/m_gridWidth + (65536/2)/m_gridWidth;

        // This game (Match'em Poker) is a square based.
        // These customization flags are left in the source
        // for anyone who is interested trying them.
#ifdef SQUARE_BASED
        int yoff= (65536/2)/m_gridHeight;
#else
        int yoff = (g&1)*h/2;
#endif
        SNpcGridItem *i = m_grid + g;
        for (int f=0; f<m_gridHeight; f++) {
            i->gridxpos = g;
            i->gridypos = f;
            y = ((f)<<16)/m_gridHeight + yoff;
            i->lposx = x - (w>>1);
            i->lposy = y - (h>>1);
            i->genCount = 0;
            i->destroying = -1;
            i->dropping = -1;
            i->animationCount = 0;
            i->wobble = 0;
            i->wobbleinc = 0;
            i+=m_gridWidth;
        }
    }
}


/**
 *
 * Chectk matches at specified location in a specified mode (straight, same colour etc.)
 * This function is recursive: Calling itself to get the final resutls
 *
 */
int CNpcLevel::calculateSimilar(int index, int x, int y, int checkmode, int dir)
{
    switch (dir) {
    case 0: y--; break;
    case 1: y++;  break;
    case 2: x--; break;
    case 3: x++; break;
    }

    SNpcGridItem *i = getGridItemAt(x,y);

    if (!i) return 0;
    if (i->index==EMPTY_CARD) return 0;
    if (i->index==-1) return 0;
    if (i->dropping!=-1) return 0;
    if (i->destroying>0) return 0;

    int earth_index = i->index/13;
    int number_index = i->index-earth_index*13;

    switch (checkmode) {
    default:
    case 0:					// similar earth
        if ((i->flags & GRIDFLAG_SAME_EARTH) != 0) return 0;
        if ((earth_index != index/13)) return 0;
        i->flags |= GRIDFLAG_SAME_EARTH;
        break;

    case 1:					// similar number
        if ((i->flags & GRIDFLAG_SAME_NUMBER) != 0) return 0;
        if (number_index != (index-index/13*13)) return 0;
        i->flags |= GRIDFLAG_SAME_NUMBER;
        break;

    case 2:					// "normal" minus straight
        if ((i->flags & GRIDFLAG_MINUS_STRAIGHT) != 0) return 0;
        if ((dir&1)==0) {		// backwards
            if (number_index!=(index-index/13*13)-1) return 0;
        } else {
            if (number_index!=(index-index/13*13)+1) return 0;
        }
        i->flags |= GRIDFLAG_MINUS_STRAIGHT;
        break;

    case 3:					// "reverse" plus straight
        if ((i->flags & GRIDFLAG_PLUS_STRAIGHT) != 0) return 0;
        if ((dir&1)==0) {		// backwards
            if (number_index!=(index-index/13*13)+1) return 0;
        } else {
            if (number_index!=(index-index/13*13)-1) return 0;
        }
        i->flags |= GRIDFLAG_PLUS_STRAIGHT;
        break;
    }
    return 1+calculateSimilar(i->index, x,y, checkmode, dir);
}


/**
 *
 * Check if something would destroy at blockx,blocky.
 * Is true, and apply is nonzero, mark the area to be destroyed.
 *
 */
int CNpcLevel::checkDestroyAt(int blockx, int blocky, char apply)
{

    SNpcGridItem *i = getGridItemAt(blockx, blocky);
    int rval = 0;
    int sim_earth;
    int sim_number;
    int minus_straight;
    int plus_straight;

    if (i->index!=-1 && i->index!=EMPTY_CARD) {

        for (int dir = 0; dir<2; dir++) {
            zeroMask(GRIDFLAG_PLUS_STRAIGHT | GRIDFLAG_MINUS_STRAIGHT | GRIDFLAG_SAME_EARTH | GRIDFLAG_SAME_NUMBER | GRIDFLAG_CALCULATION_TEMP);

            minus_straight = calculateSimilar(i->index, blockx, blocky, 2, dir*2) +
                    calculateSimilar(i->index, blockx, blocky, 2, dir*2+1) + 1;
            if (minus_straight>3) {
                i->flags |= GRIDFLAG_MINUS_STRAIGHT;
                if (apply) applyDestroy(GRIDFLAG_MINUS_STRAIGHT);
                rval = 1;
            }

            plus_straight = calculateSimilar(i->index, blockx, blocky, 3, dir*2) +
                    calculateSimilar(i->index, blockx, blocky, 3, dir*2+1) + 1;
            if (plus_straight>3) {
                i->flags |= GRIDFLAG_PLUS_STRAIGHT;
                if (apply) applyDestroy(GRIDFLAG_PLUS_STRAIGHT);
                rval = 1;
            }

            // similar earth
            sim_earth = calculateSimilar(i->index, blockx, blocky, 0, dir*2) +
                    calculateSimilar(i->index, blockx, blocky, 0, dir*2+1) + 1;

            if (sim_earth>=4) {
                i->flags |= GRIDFLAG_SAME_EARTH;
                if (apply) applyDestroy(GRIDFLAG_SAME_EARTH);
                rval = 1;
            }

            sim_number = calculateSimilar(i->index, blockx, blocky, 1, dir*2) +
                    calculateSimilar(i->index, blockx, blocky, 1, dir*2+1) + 1;
            if (sim_number>2) {
                i->flags |= GRIDFLAG_SAME_NUMBER;
                if (apply) applyDestroy(GRIDFLAG_SAME_NUMBER);
                rval = 1;
            }
        }
    }
    return rval;
}


/**
 *
 * Check all the coordinates in a grid for destroying
 * If apply is nonzero, mark all areas to be destroyed.
 *
 */
int CNpcLevel::checkDestroyWholeLevel(char apply)
{
    int rval = 0;
    for (int f=0; f<m_gridHeight; f++)
        for (int g=0; g<m_gridWidth; g++)
            if (checkDestroyAt(g,f,apply) != 0) rval = 1;
    return rval;
}



/**
 *
 * Show hint by wobbling the hintblocks and spray a notification particle
 *
 */
void CNpcLevel::wobbleHint()
{
    if (m_hint1==0 || m_hint2==0) return;		// error
    m_hint1->wobbleinc = -65535/16;
    m_hint2->wobbleinc = -65535/15;

    if (m_currentLevel==0) {			// switch us
        int x= xtoGameArea((65536/m_gridWidth+m_hint1->lposx+m_hint2->lposx)/2);
        int y= ytoGameArea((65536/m_gridHeight+m_hint1->lposy+m_hint2->lposy)/2);
        int dx= xtoGameArea(65536/2)-x;
        int dy= ytoGameArea(65536/2)-y;
        m_particleEngine->spray(1, x,y,0, dx/4,dy/4,0, 51<<16, &m_scoreSpray);
    }
}


/**
 *
 * Try to switch two items with eachother.
 * If the switch would result a destroy, return one
 * otherwise, return zero.
 *
 */
int CNpcLevel::tryMoveWith(SNpcGridItem *i1, SNpcGridItem *i2)
{
    if (i1==0 || i2==0) return 0;

    int ctemp = i1->index;
    i1->index = i2->index;
    i2->index = ctemp;

    int rval = 0;
    if (checkDestroyAt(i1->gridxpos, i1->gridypos, 0) == 1 ||
            checkDestroyAt(i2->gridxpos, i2->gridypos, 0) == 1) {
        rval = 1;
    }

    ctemp = i1->index;				// change back
    i1->index = i2->index;
    i2->index = ctemp;
    //qdebug() << "trymovewith end";
    return rval;
}


/**
 *
 * Does the level has two neighbouring items that would cause
 * a destroy when switched with eachother.
 * Update the hint as well
 *
 */
int CNpcLevel::hasMovesLeft()
{
    m_hint1 = 0;
    m_hint2 = 0;

    // Seek through moves
    for (int y=m_gridHeight-1; y>1; y--) {
        SNpcGridItem *i1 = (m_grid+m_gridWidth*y);
        for (int x=0; x<m_gridWidth; x++) {

            // Simulate the moves..Remember Match'em Poker is square based
#ifdef SQUARE_BASED             // With Square grid
            if (tryMoveWith(i1,getGridItemAt(x,y+1)) == 1) {
                m_hint1 = i1;
                m_hint2 = getGridItemAt(x,y+1);
                return 1;
            }

            if (tryMoveWith(i1,getGridItemAt(x+1,y)) == 1) {
                m_hint1 = i1;
                m_hint2 = getGridItemAt(x+1,y);
                return 1;
            }
#else                           // With hexagon grid
            if (tryMoveWith(i1,getGridItemAt(x,y+1)) == 1) {
                m_hint1 = i1;
                m_hint2 = getGridItemAt(x,y+1);
                return 1;
            }

            if (tryMoveWith(i1,getGridItemAt(x+1,y-1+(x&1))) == 1) {
                m_hint1 = i1;
                m_hint2 = getGridItemAt(x+1,y-1+(x&1));
                return 1;
            }

            if (tryMoveWith(i1,getGridItemAt(x+1,y-1+(x&1)+1)) == 1) {
                m_hint1 = i1;
                m_hint2 = getGridItemAt(x+1,y-1+(x&1)+1);
                return 1;
            }
#endif
            i1++;
        }
    }
    return 0;
}


/**
 *
 * Destroy items mark with a specific
 *
 */
void CNpcLevel::applyDestroy(unsigned int flag)
{
    char wasFirst = 0;
    int width = m_itemWidth;
    int height = m_itemHeight;
    m_doingNothing = false;
    int blocks = 0;
    int mx = 0;
    int my = 0;

    SNpcGridItem *i = m_grid;
    SNpcGridItem *i_target = i+m_gridWidth*m_gridHeight;
    while (i!=i_target) {
        // Just destoy the ones that are not already destroying
        if (i->index!=-1 && (i->flags&flag) && (i->destroying==-1)) {
            // At least one of the line wasnt marked before, .. this was first found. mark for scores
            if (!(i->flags&GRIDFLAG_MARKED)) wasFirst = 1;

            mx += i->lposx;
            my += i->lposy;
            blocks++;
            i->flags |=GRIDFLAG_MARKED;

            // Start destroying. (destroying is >= 0)
            i->destroying = 0;
        }
        i++;
    }


    if (blocks<1) return;
    if (wasFirst==1) {

        // Determine how much  scores comes from this destroy
        int scoreAdd = (blocks*blocks*2) / (m_destroyingRound+1);

        int sind = 0;
        int scoreMul = 1;

        // Most scores from straights, then from earth, then from number
        switch (flag) {
        case GRIDFLAG_MINUS_STRAIGHT:
        case GRIDFLAG_PLUS_STRAIGHT:
            sind = 2;
            scoreMul = 11;
            break;

        case GRIDFLAG_SAME_EARTH:
            sind = 1;
            scoreMul = 3;
            break;


        case GRIDFLAG_SAME_NUMBER:
            sind = 0;
            scoreMul = 7;
            break;
        }
        if (scoreAdd<1) scoreAdd = 1;
        scoreAdd = (scoreAdd*scoreMul)/3;

        m_levelProgressChange += (scoreAdd);
        int x= xtoGameArea(mx/blocks+width/2);
        int y = ytoGameArea(my/blocks+height/2);
        int dx= xtoGameArea(65536/2)-x;
        int dy= ytoGameArea(65536/2)-y;

        // A notification particle telling about this destroy
        m_particleEngine->spray(1, x, y,0, dx,dy, 0, sind*256 + blocks, &m_scoreSpray);
        m_levelScore += scoreAdd;
    }
}


/**
 *
 * Try to switch m_changing[0] with i
 * return 1 if succesful
 *
 */
int CNpcLevel::tryApplyChange(SNpcGridItem *item)
{

    // cannot select another peace when other than in normal mode
    if (m_state != LevelStateNormal) {
        m_renderer->effectNotify(IllegalMoveEffect, 0, 0);
        cancelSelection(0);
        cancelSelection(1);
        return 0;						// cannot continue, level is processing itself.
    }


    char allowChange = 0;

    // For kids. Difficulty is legacy and always 1 in Match'em Poker
    if (m_difficulty==0) allowChange = 1;

    if (allowChange==0) {
        // Must be next  to changing[0] to be allowed
#ifdef SQUARE_BASED
        if ((abs(m_changing[0]->gridxpos - item->gridxpos) +
             abs(m_changing[0]->gridypos - item->gridypos)) <= 1) allowChange = 1;
#else
        int yofs = (m_changing[0]->gridxpos&1);
        int dx = m_changing[0]->gridxpos - item->gridxpos;
        int dy = m_changing[0]->gridypos - item->gridypos;
        if (dx==0 && abs(dy)<=1) allowChange = 1;
        dy += yofs;
        if (dx==1 || dx==-1) if (dy==0 || dy==1) allowChange = 1;
#endif

    }

    if (allowChange) {
        m_renderer->effectNotify(ChangingEffect, 0, 0);
        m_changing[1] = item;
        m_changingCounter = 65536;
        m_illegalMoveCounter = 0;

        // Changing is now enabled
        item->flags ^= GRIDFLAG_SELECTED;			// flip the first bit
    }
    else
    {
        m_renderer->effectNotify(IllegalMoveEffect, 0, 0);
        cancelSelection(0);
        return 0;
    }
    return 1;
}


/**
 *
 * Clear selection flag's and state
 *
 */
void CNpcLevel::cancelSelection(int index)
{
    if (m_changing[index]!=0) {
        m_changing[index]->posoffsetx = 0;
        m_changing[index]->posoffsety = 0;
        m_changing[index]->flags &= 0xFFFFFFFF^GRIDFLAG_SELECTED;
        m_changing[index] = 0;
    }

}


/*!
  Handles pointer events arriving from above.

  Descriptions for \a type:
    0 = down,
    1 = move and
    2 = up
*/
void CNpcLevel::handleMouseEvent(int type, int fixedX, int fixedY)
{
    if (m_state == LevelStateBeginning
            || m_state == LevelStateCompleted
            || m_state == LevelStateGameOver)
    {
        // Our state won't allow a pointer event
        return;
    }

    // we are currently changing: Do not accept the event
    if (m_changingCounter>0) return;

    fixedX = ((fixedX-m_areaX)<<12)/(m_areaWidth>>4);
    fixedY = ((fixedY-m_areaY)<<12)/(m_areaHeight>>4);
    // The click doesn't hit the game level
    if (fixedX<0 || fixedY<0 || fixedX>65535 || fixedY>65535) return;

    int gx = ((fixedX*m_gridWidth)>>16);
#ifdef SQUARE_BASED
    int gy = ((fixedY*m_gridHeight)>>16);
#else
    int gyoff = (65536/m_gridHeight/2) -(65536/m_gridHeight/2) * (gx&1);
    int gy = (((fixedY+gyoff)*m_gridHeight)>>16);
#endif
    if (gy<0) return;

    SNpcGridItem *i = getGridItemAt(gx, gy);

    switch (type) {
    case 1:
        if (m_changing[0]!=0) {
            if (m_difficulty>0) {
                float fx = (fixedX-m_changing[0]->lposx)-(65536/m_gridWidth/2);
                float fy = (fixedY-m_changing[0]->lposy)-(65536/m_gridHeight/2);

                float le = sqrtf(fx*fx+fy*fy);
                if (le>6000.0f) {
                    fx=fx/le*6000.0f;
                    fy=fy/le*6000.0f;
                }

                m_changing[0]->posoffsetx = fx;
                m_changing[0]->posoffsety = fy;

                gx = m_changing[0]->lposx + m_changing[0]->posoffsetx + (65536/m_gridWidth/2);
                gy = m_changing[0]->lposy + m_changing[0]->posoffsety + (65536/m_gridHeight/2);

                gx = ((gx*m_gridWidth)>>16);
#ifdef SQUARE_BASED
                gy = ((gy*m_gridHeight)>>16);
#else
                int gyoff = (65536/m_gridHeight/2) -(65536/m_gridHeight/2) * (gx&1);
                gy = (((gy+gyoff)*m_gridHeight)>>16);
                if (gy<0) return;
#endif

                i = getGridItemAt(gx,gy);

            } else {
                m_changing[0]->posoffsetx = (fixedX-m_changing[0]->lposx)-(65536/m_gridWidth/2);
                m_changing[0]->posoffsety = (fixedY-m_changing[0]->lposy)-(65536/m_gridHeight/2);
            }

            if (i!=m_changing[0]) {
                if (m_changing[1]!=i) {
                    m_changing[1] = i;
                }
                m_dragBegan = 1;

            }
        } else return;

        break;


    case 2:
        if (m_difficulty>0 && m_dragBegan && m_changing[1]) i = m_changing[1];

        if (m_changing[0]!=0 && m_changing[0]!=i && i->destroying==-1 && i->dropping==-1) {
            tryApplyChange(i);
        } else {
            if (m_dragBegan) {
                cancelSelection(0);
                cancelSelection(1);
            }
        }

        m_dragBegan = 0;
        break;

    case 0:
        m_dragBegan = 0;
        if (!i) return;

        if (i==m_changing[0]) {			// was already selected and now we are cancelling it.
            m_renderer->effectNotify(ClickEffect, 0, 0);
            cancelSelection(0);
        } else {
            if (i->destroying==-1 && i->dropping==-1) {

                if (m_changing[0]==0)
                {
                    // Click an empty card: Open it
                    if (i->index == EMPTY_CARD) {

                        m_renderer->effectNotify(ChangeCompletedEffect, 0, 0);
                        while (i->index == EMPTY_CARD) i->index = randomBlock();
                        m_levelProgressChange -= 20;

                        m_particleEngine->spray(1,
                                         xtoGameArea(i->lposx + m_itemWidth/2),
                                         ytoGameArea(i->lposy+m_itemHeight/2),
                                         200, 0, 0, 0, 0, &m_morphSpray);

                        m_particleEngine->spray(20,
                                         xtoGameArea(i->lposx + m_itemWidth/2),
                                         ytoGameArea(i->lposy+m_itemHeight/2),
                                         8000, 0, -16000, 64000, 0, &m_sparkleSpray);

                        checkDestroyAt(i->gridxpos, i->gridypos);

                    } else {
                        m_renderer->effectNotify(ClickEffect, 0, 0);
                        m_changing[0] = i;
                        i->flags ^= GRIDFLAG_SELECTED;			// flip the first bit
                    }
                }
                else {
                    tryApplyChange(i);
                }
            }
        }
        break;
    }
}



/**
 *
 * Return a griditem from a specific location
 *
 */
SNpcGridItem *CNpcLevel::getGridItemAt(int blockx, int blocky)
{
    if (blockx<0 || blocky<0 || blockx>=m_gridWidth || blocky>=m_gridHeight) return 0;
    return m_grid + blocky*m_gridWidth + blockx;
}



/**
 *
 * Zero specific flags from the whole level
 *
 */
void CNpcLevel::zeroMask(unsigned int maskToZero)
{
    for (int f = 0; f < m_gridWidth * m_gridHeight; f++) {
        m_grid [f].flags &= (0xFFFFFFFF^maskToZero);
    }
}


/**
 *
 * Run the level, called once / frame
 *
 */
void CNpcLevel::run(int fixedFrameTime16Bit)
{
    if (m_state == LevelStateBeginning)
        m_deckVisibility += ((65536 - m_deckVisibility)*(fixedFrameTime16Bit>>4))>>10;
    else
        m_deckVisibility -= ((m_deckVisibility)*(fixedFrameTime16Bit>>4))>>10;

    if (m_state == LevelStateIdle)
        return;

    // Limit frametime just to be sure it's not too large
    // For example an incoming phonecall could do it.
    if (fixedFrameTime16Bit>8000) fixedFrameTime16Bit = 8000;

    int f,g;
    m_floatingAngle += fixedFrameTime16Bit;
    int width = 65536 / m_gridWidth;
    int height = 65536 / m_gridHeight;

    // Seek though all of the griditems and run it's functionality.
    // Also check and mark if something is currently happening in the level
    m_doingNothing = true;

    int something_destroyed = 0;
    int something_dropped = 0;
    int something_destroying = 0;
    int something_dropping = 0;
    int something_created = 0;

    int ind = 0;
    SNpcGridItem *i = m_grid;
    SNpcGridItem *i_target = i + m_gridWidth*m_gridHeight;
    while (i!=i_target) {
        if (ind<m_gridWidth && i->index == -1) {
            int safeCount = 0;
            do {            // try a block as long as required
                i->index = randomBlock();
                safeCount++;
            } while (safeCount<1000 && checkDestroyAt(ind,0,0)!=0);

            something_created = 1;
            m_doingNothing = false;
        }

        if (i->destroying>=0) {
            something_destroying = 1;
            f = i->destroying;
            i->destroying+=((fixedFrameTime16Bit*DESTROYINGSPEED)>>8);           // destroying speed
            if (f<=(65536+65536/2) && i->destroying>(65536+65536/2)) {

                m_particleEngine->spray(10, xtoGameArea(i->lposx+width/2), ytoGameArea(i->lposy+height/2),8000,
                                 0,-10000, 16000, 0, &m_sparkleSpray);


                m_particleEngine->spray(6+(rand()&3), xtoGameArea(i->lposx+width/2), ytoGameArea(i->lposy+height/2),6000,
                                 0,-40000, 50000, 0, &m_fruitSpray);


            }

            if (i->destroying>=65536*2) {			// destroy, finally.
                if (i==m_changing[0] || i==m_changing[1]) {
                    cancelSelection(0);
                    cancelSelection(1);
                    m_changingCounter = 0;

                }

                i->index=-1;
                i->destroying = -1;
                i->dropping = -1;

                i->posoffsetx = 0;
                i->posoffsety = 0;
                i->flags = 0;
                i->animationCount = 0;
                something_destroyed = 1;
            }
        }
        i->animationCount += fixedFrameTime16Bit;
        i->animationCount&=(65536*16-1);
        g = ((i->wobble * fixedFrameTime16Bit)>>12);		// string effect
        i->wobbleinc-=g;
        g = ((i->wobbleinc * fixedFrameTime16Bit)>>13);		// slowdown
        i->wobbleinc -= g;
        g = ((i->wobbleinc * fixedFrameTime16Bit)>>13);		// add
        i->wobble+=g;


        if (m_state != LevelStateGameOver) {
            if (i->flags&GRIDFLAG_SELECTED) {
                i->genCount += (((8000-i->genCount) * fixedFrameTime16Bit) >> 14);
            } else {
                int g = (m_cosineTable[ ((m_floatingAngle>>4) + (ind<<10))&4095 ]>>6);
                i->genCount += (((g-i->genCount)*fixedFrameTime16Bit)>>13);

                // gameovereffect
                i->genCount += (((-i->genCount) * fixedFrameTime16Bit) >> 14);
            }
        }
        i++;
        ind++;
    }



    if (something_destroyed==1) {
        m_renderer->effectNotify(DestroyingEffect, m_destroyingRound,0);
        m_destroyingRound++;
    }


    int count = 0;
    i = m_grid;
    i_target = i + m_gridWidth*(m_gridHeight-1);
    while (i!=i_target) {
        if (i->dropping==-1 && i->index!=-1) {
            // destroying pieces will drop
            if ((i[m_gridWidth].index==-1 || i[m_gridWidth].dropping!=-1)) {
                i->dropping = 5000; // enable dropping.
                something_dropping = 1;
                i->posoffsety=0;
            }
        } else {
            something_dropping = 1;
            i->dropping+=fixedFrameTime16Bit*4;
            i->posoffsety+=((i->dropping*fixedFrameTime16Bit)>>16);
            if (i->posoffsety>=height) {

                if (i[m_gridWidth].index==-1) {
                    //i->posoffsety = 0;
                    i->posoffsety-=height;
                    i[m_gridWidth].animationCount = i->animationCount;
                    i[m_gridWidth].destroying = i->destroying;
                    i[m_gridWidth].flags = i->flags;
                    i[m_gridWidth].index = i->index;
                    i[m_gridWidth].posoffsetx = i->posoffsetx;
                    i[m_gridWidth].posoffsety = i->posoffsety;
                    i[m_gridWidth].dropping = i->dropping;
                    i[m_gridWidth].wobble = i->wobble;
                    i[m_gridWidth].wobbleinc = i->wobbleinc;
                    i->index = -1;
                    i->dropping = -1;
                    i->wobble = 0;
                    i->wobbleinc = 0;
                }
            }

            if (i[m_gridWidth].index!=-1 && i[m_gridWidth].dropping==-1) {
                // stop dropping...
                f = count/m_gridWidth;
                checkDestroyAt(count-f*m_gridWidth, f, 1);
                something_dropped = 1;
                i->dropping = -1;
                i->wobbleinc = 4000;
                i->posoffsety = 0;
            }

        }
        count++;
        i++;
    }

    // stop the last line
    i = m_grid + m_gridWidth*(m_gridHeight-1);
    i_target=i+m_gridWidth;
    while (i!=i_target) {
        if (i->dropping!=-1) {
            i->dropping = -1;
            i->posoffsety =0;
        }
        i++;
    }

    if (something_destroyed != 0 || something_dropped != 0 ||
            something_destroying != 0 || something_dropping != 0)
        m_doingNothing = false;



    switch (m_state) {
    case LevelStateBeginning:
    {
        m_startupCounter -= fixedFrameTime16Bit;

        if (m_startupCounter <= 0) {
            m_startupCounter = 0;
            setLevelState(LevelStateNormal);
        }

        int amount = m_gridWidth * m_gridHeight;
        SNpcGridItem *i = m_grid;

        for (int f = 0; f < m_gridWidth * m_gridHeight; f++) {
            int g = m_startupCounter - ((f<<16)*4/amount);

            if (g<0) {
                g = 0;
                if ((i->flags&GRIDFLAG_CALCULATION_TEMP) == 0) {
                    i->flags|=GRIDFLAG_CALCULATION_TEMP;
                    // do something when block is in place
                    m_renderer->effectNotify(BlockBeginFinishedEffect, 0,0);
                }
            } else i->wobble = -1000;

            g>>=4;
            if (g>1024) g = 1024;
            i->posoffsetx = DECK_POSITION_X-i->lposx -m_itemWidth/2;
            i->posoffsety = DECK_POSITION_Y-i->lposy - m_itemHeight/2;
            i->posoffsetx = (i->posoffsetx * g)>>10;
            i->posoffsety = (i->posoffsety * g)>>10;
            i->genCount = g*48;
            i++;
        }

        break;
    }
    case LevelStateCompleted:
    {
        m_startupCounter -= fixedFrameTime16Bit;

        if (m_startupCounter <= 0) {
            m_startupCounter = 0;
            setLevelState(LevelStateIdle);
        }

        int amount = m_gridWidth * m_gridHeight;
        SNpcGridItem *i = m_grid;

        for (int f = 0; f < m_gridWidth * m_gridHeight; f++) {
            int g = ((f<<16)*4/amount) - m_startupCounter;

            g>>=4;

            if (g>1024) g = 1024;

            if (g<=0)
                g = 0;
            else {
                if ((i->flags&GRIDFLAG_CALCULATION_TEMP) == 0) {
                    i->flags|=GRIDFLAG_CALCULATION_TEMP;
                    // do something when block is taken from its place
                    m_renderer->effectNotify(BlockVanishStartedEffect, 0, 0);
                }
            }

            i->posoffsetx = ((65536-i->lposx + m_itemWidth) * g)>>10;
            i->posoffsety = ((65536-i->lposy + m_itemHeight) * g)>>10;
            i->genCount = g*48;

            i++;
        }

        break;
    }
    case LevelStateGameOver:
    {
        int jj = ((m_gridWidth*m_gridHeight)<<16) - m_startupCounter/2;
        if (jj<65536) jj=65536;
        jj = (__int64)((jj * (__int64)fixedFrameTime16Bit) >>16);
        m_startupCounter -= jj;

        if (m_startupCounter <= -65536 * 32) {
            m_startupCounter = -65536 * 32;
            setLevelState(LevelStateIdle);

            // Wait and do nothing until the game changes its (level) state to
            // something else.
        }

        SNpcGridItem *i = m_grid;
        for (int f = 0; f < m_gridWidth * m_gridHeight; f++) {
            int g = (f<<16)-m_startupCounter;
            if (g<0) g = 0;
            if (g>65536*28) g = 65536*28;
            i->genCount=-(g>>5);
            i++;
        }

        break;
    }
    case LevelStateNormal:
    {
        if (something_created)
            checkDestroyWholeLevel();

        // check if the level have moves left and update the hint
        if (something_destroyed || something_dropped || something_created) {
            m_hint1 = 0;
            m_hint2 = 0;

            if (something_destroying==0 && something_dropping==0) {

                if (hasMovesLeft()) {

                } else {
                    // NO MOVES LEFT!!
                    // give scores
                    m_particleEngine->spray(1, xtoGameArea(65536/2),
                                     ytoGameArea(65536/4),
                                     0, 0, 8000, 0, (50<<16), &m_scoreSpray);

                    m_levelScore += 800;
                    m_levelProgressChange += 40;


                    // destroy randomly?
                    int dcount = m_gridHeight;
                    int safeCount = 0;
                    while (dcount>0 && safeCount<2000) {

                        SNpcGridItem *i = getGridItemAt((rand()&255)*m_gridWidth/256, (rand()&255)*m_gridHeight/256);
                        if (i->index!=-1 && ((i->flags&GRIDFLAG_MARKED) == 0) && i!=m_changing[0] && i!=m_changing[1]) {
                            i->flags |= GRIDFLAG_MARKED;
                            dcount--;
                        }
                        safeCount++;
                    }
                    applyDestroy(GRIDFLAG_MARKED);

                }
            }
        }

        // Combo
        if (m_destroyingRound > 3 && m_doingNothing)
        {
            m_particleEngine->spray(1, xtoGameArea(65536 / 2),
                                    ytoGameArea(65536 / 2),
                                    0, 0, 8000, 0,
                                    ((m_destroyingRound - 1) << 16),
                                    &m_scoreSpray);

            m_renderer->effectNotify(XBonusEffect, 0, 0);

            m_levelProgressChange += m_destroyingRound * 3;
            m_levelScore += (m_destroyingRound * 30);
            m_destroyingRound = 0;
        }

        if (m_changingCounter > 0) {
            m_changingCounter -= fixedFrameTime16Bit * 6;
            // do the change with m_changing[0] and [1]

            if (m_changingCounter <= 0) {
                int temp = m_changing[0]->index;
                m_changing[0]->index = m_changing[1]->index;
                m_changing[1]->index = temp;

                // reset the destroying rounds.
                m_destroyingRound = 0;

                int allow_move = 0;

                // check destroy.
                if (checkDestroyAt(m_changing[0]->gridxpos, m_changing[0]->gridypos)!=0) allow_move = 1;
                if (checkDestroyAt(m_changing[1]->gridxpos, m_changing[1]->gridypos)!=0) allow_move = 1;

#ifdef ALLOW_VOID_MOVES
                if (allow_move == 0) {
                    allow_move = 1;         // subtract some scores
                    m_levelProgressChange -= 15;
                }
#endif
                if (allow_move != 0) {
                    m_renderer->effectNotify(ChangeCompletedEffect, 0, 0);
                    m_changingCounter = 0;
                }
                else {
                    // Was an illegal move. Change back.
                    if (m_illegalMoveCounter == 0) {
                        m_changingCounter = 65536;
                        m_illegalMoveCounter++;
                        m_renderer->effectNotify(IllegalMoveEffect, 0, 0);
                    }
                    else {
                        // This was a "backwards" move.
                        m_changingCounter = 0; // Stay still.
                    }
                }

                if (m_changingCounter == 0) {
                    m_changing[0]->flags &= 0xFFFFFFFF^GRIDFLAG_SELECTED;
                    m_changing[1]->flags &= 0xFFFFFFFF^GRIDFLAG_SELECTED;
                    m_changing[0]->posoffsetx = 0;
                    m_changing[0]->posoffsety = 0;
                    m_changing[1]->posoffsetx = 0;
                    m_changing[1]->posoffsety = 0;
                    m_changing[0] = 0;
                    m_changing[1] = 0;
                }
            } else {
                int dx,dy;
                if (m_changing[0] && m_changing[1]) {
                    dx = (m_changing[1]->lposx - m_changing[0]->lposx);
                    dy = (m_changing[1]->lposy - m_changing[0]->lposy);


                    int i = 65536-m_changingCounter;

                    dx = ((dx*(i>>4))>>12);
                    dy = ((dy*(i>>4))>>12);

                    i >>=6;
                    i&=4095;
                    int ndx = (((dy) * (m_cosineTable[i]>>2)) >> 14);
                    int ndy = (((-dx) * (m_cosineTable[i]>>2)) >> 14);

                    m_changing[0]->posoffsetx = dx+ndx*3/2;
                    m_changing[0]->posoffsety = dy+ndy;
                    m_changing[1]->posoffsetx = -dx-ndx*3/2;
                    m_changing[1]->posoffsety = -dy-ndy;
                } else {
                    cancelSelection(0);
                    cancelSelection(1);
                    m_changingCounter = 0;
                }

            }

            break;		// dont continue.
        }
    }
        break;
    default:
        break;
    } // switch (m_state)
}



int CNpcLevel::xtoGameArea(const int sourcex) {
    return m_areaX + ((sourcex*(m_areaWidth>>6))>>10);
}

int CNpcLevel::ytoGameArea(const int sourcey) {
    return m_areaY + ((sourcey*(m_areaHeight>>6))>>10);
}


//const char NumLetters[] = { 28, 29, 30, 31, 32, 33, 34, 35, 26, 9, 16, 10, 0 };


/**
 *
 * Render single grid item
 *
 */
void CNpcLevel::renderTileCaller(SNpcGridItem *i, TileRenderer &renderer) {
    // change size from i's gencount
    int sizeAdd = TILE_SIZE_ADD + (i->genCount>>2);
    int fade = 0;

    int width = m_itemWidth;
    int height = m_itemHeight;

    int tex = TexPieces;
    int earth_index =i->index/13;
    int num_index = i->index - earth_index*13;
    num_index++;
    if (num_index>12) {
        num_index = 0;
    }

    if (i->index == JOKER_CARD) {
        earth_index = rand() & 3;
        num_index = 6+ (rand()&7);
    }

    if (i->destroying!=-1) {
        tex = TexPiecesSelected;
        //sizeAdd -= i->destroying/8;
    }

    if (i->flags&GRIDFLAG_MARKED) {			// marked
        tex = TexPiecesSelected;
    }


    if (i->flags&GRIDFLAG_SELECTED) {
        tex = TexPiecesSelected;
    }

    int wadd = ((sizeAdd*5)>>3) + m_itemWidth / 2;
    int hadd = (sizeAdd>>2);

    int rx = i->lposx + i->posoffsetx - (wadd>>1);
    int ry = i->lposy + i->posoffsety - (hadd>>1); //- (hadd>>1);			// align to bottom
    ry+=i->wobble;
    width += wadd;
    height += hadd;

    if (width <= 0 || height <= 0) {
        return;
    }

    if (m_state == LevelStateGameOver) {
        fade = -i->genCount / 200;
    }

    if (fade<0) fade = 0;
    if (fade>255) fade = 255;

    int rx2 = xtoGameArea(rx+width);
    int ry2 = ytoGameArea(ry+height);
    rx = xtoGameArea(rx);
    ry = ytoGameArea(ry);

    width = rx2-rx;
    height = ry2-ry;

    if (i->destroying!=-1) {
        fade = ((i->destroying>>1)-65536*4/5);
        if (fade<0) fade = 0; else fade*=10;
    }

    if (fade<255) {
        if (i->index != EMPTY_CARD) {
            // base
            renderer.renderTile(
                        rx,
                        ry,
                        width,
                        height,
                        0,0,
                        1 | (tex<<16) | (fade<<24), 0);


            // "earth"
            renderer.renderTile(
                        rx+(width>>2),
                        ry+(height>>4),
                        (width*4)>>3,
                        height>>1,
                        0,0,
                        (earth_index+2) | (tex<<16) | (fade<<24), 0);

            if (num_index==13) num_index = 0;
            //num
            renderer.renderTile(
                        rx+(width>>2),
                        ry+(height>>1),
                        (width*4)>>3,
                        (height>>1),
                        0,0,
                        (num_index+6) | (tex<<16) | (fade<<24), 0);


        } else {
            // Empty card
            renderer.renderTile(
                        rx,
                        ry,
                        width,
                        height,
                        0,0,
                        0 | (tex<<16) | (fade<<24), 0);
        }

    }


    // draw flare if the card is destroying
    if (i->destroying!=-1) {
        //fade = 255-(((i->destroying>>8) * (i->destroying>>8))>>10);
        fade = 255-(i->destroying>>9)*5/4;
        if (fade<0) fade=-fade*5;

        if (fade<0) fade = 0;
        int size = 4000 + (((i->destroying>>8) * (i->destroying>>8))>>4);

        if (fade<255)
            renderer.renderTile(
                        rx-size,
                        ry-size,
                        width+size*2,
                        height+size*2,
                        i->animationCount>>4,1,2 | (TexParticle<<16) | (fade<<24), 0);
    }
}


/*!
  Draws the level.
*/
void CNpcLevel::draw(TileRenderer &renderer)
{
    if (m_state == LevelStateIdle) {
        return;
    }

    SNpcGridItem *i = m_grid;
    SNpcGridItem *i_target = i+m_gridWidth * m_gridHeight;

    // First pass.
    while (i!=i_target) {
        if (i->index!=0xFFFFFFFF && //i->index!=8 &&
                !(i->flags&GRIDFLAG_HIDDEN) &&
                !(i->flags&GRIDFLAG_SELECTED) &&
                !(i->flags&GRIDFLAG_MARKED) &&
                (i->genCount<1000)) {
            renderTileCaller(i, renderer);
        }
        i++;
    }

    // Second pass.
    i = m_grid;
    while (i!=i_target) {
        if (i->index!=0xFFFFFFFF &&
                !(i->flags&GRIDFLAG_HIDDEN)) {

            if (i->flags&(GRIDFLAG_SELECTED|GRIDFLAG_MARKED) || i->genCount>=1000) {
                renderTileCaller(i, renderer);
            }
        }
        i++;
    }


    // Deck draw
    if (m_deckVisibility>1024) {
        for (int f=0; f<3; f++) {
            int ysize = m_itemHeight*3;
            int xsize = m_itemWidth*3;
            int amount = 256;
            //if (m_startupCounter<65536*4) amount = 0;
            int a = (m_startupCounter>>4) + (f*650);
            renderer.renderTile(DECK_POSITION_X -xsize/2 + ((((((m_cosineTable[ a& 4095 ]>>6)*(m_cosineTable[ (a*2+1500)&4095]>>8))>>8))*amount)>>8),
                                 DECK_POSITION_Y -ysize/2 + (((m_cosineTable[ (a+1200)& 4095 ]>>6)*amount)>>8),
                                 xsize,
                                 ysize,
                                 0, 0, 0|(TexPieces<<16) | ((255-(m_deckVisibility>>8))<<24), 0);
        }
    }
}



void CNpcLevel::saveToFile(FILE *file) {

    SNpcGridItem *i = m_grid;
    SNpcGridItem *i_target = m_grid + m_gridWidth * m_gridHeight;
    while (i!=i_target) {
        fwrite(&i->index, sizeof(int), 1, file);
        i++;
    }
}

void CNpcLevel::loadFromFile(FILE *file) {
    // just create something so flags etc are ok
    createLevel(0);
    SNpcGridItem *i = m_grid;
    SNpcGridItem *i_target = m_grid + m_gridWidth * m_gridHeight;
    while (i!=i_target) {
        fread(&i->index, sizeof(int), 1, file);
        i->posoffsetx = 0;
        i->posoffsety = 0;
        i++;
    }

    if (checkDestroyWholeLevel()!=0) {

    }

    hasMovesLeft();         // update hint
    m_destroyingRound = 0;
}


