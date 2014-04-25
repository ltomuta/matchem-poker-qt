/**
 * Copyright (c) 2011-2014 Microsoft Mobile.
 */

#include "tilenpc.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For strlen()

#include "tilerenderer.h"


#define INFOSCREEN_LINECOUNT 17
const char *info_screen_lines[INFOSCREEN_LINECOUNT] = {
    "Match'em Poker v1.2",
    "A Nokia example",
    "ported from iOS",
    "using Qt GameEnabler",
    "projects.developer.",
    "nokia.com/",
    "qtgameenabler",
    "",
    "Simply switch cards",
    "with eachother to",
    "create poker hands.",
    "",
    "Audio samples from",
    "FREESOUND.ORG used.",
    "Thanks to:",
    "milton, mattwasser,",
    "Christianjinnyzoe"
};


/*


 !"#$% ´()*+,-./
0123456789:;<=>?
@ABCDEFGHIJKLMNO
PQRSTUVWXYZ[\]^_
`abcdefghijklmno
pqrstuvwxyz{|}~

*/
const char *happy_strings[] = {
    "Playing cards were most likely invented in China in 1120 AD and first introduced to Europe in the 1300s. ",
    "Due to French influence, Spades represent Royalty, Diamonds represent Merchants, Clubs represent Peasants, and Hearts represent the Clergy.",
    "A fifth suit was added in 1937 but never caught on because people had to buy all new decks.",
    "When Columbus landed on U.S. shores in 1492, his men plucked wide leaves from trees, marked them with images, and played cards. ",
    "There are 1,326 possible 2-card combinations a player can start with in Texas Hold’em.",
    "There is a 1 in 16 chance of being dealt pocket pairs.",
    "There is a 1 in 8.3 chance of flopping a set after being dealt pocket pairs."
    "Poker is…a fascinating, wonderful, intricate adventure on the high seas of human nature.",
    "40 to 50 million Americans regularly play poker. That’s more than one in five… ",
    "Each year more than 70 million decks of cards are sold in the U.S.",
    "Make a Waldorf salad, with apples, celery, walnuts, and dressing.",
    "\"Patience, and shuffle the cards.\" -Cervantes, Don Quixote.",
    "The next best thing to playing and winning is playing and losing. The main thing is to play.",
    "I must complain the cards are ill shuffled till I have a good hand.",
    "POKER, n. A game said to be played with cards for some purpose to this lexicographer unknown.",
    "If one is able and strong, then one should disguise oneself in order to appear inept and weak.",
    "Perception is reality.",
    "Clubs -Alexander, the Great.",
    "Spades - King David.",
    "It’s not enough to succeed. Others must fail.",
    "Diamonds - Julius Caesar.",
    "There are two \"one-eyed\" Jacks in a deck of cards. The Jack of Spades and the Jack of Hearts.",
    "A \"Dead man's hand\" is Aces and Eights.",
    "Hearts - Charlemagne.",
    "No one knows the true origins of poker.",
    "I'm getting tired of poker now...",
    "Don't play if you need to win.",
    "Sound effects are all from FreeSound.org from following tags: milton, mattwasser, and Christianjinnyzoe.",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "...",
    "Warning, End of the list.. ",
};




// Score roll
const int rollWidth = 10000;
const int rollSpacing = 6500;
const int scoreRollStart = 33000;
const int scoreRollXPos[] = { 6500,6500 + rollSpacing, scoreRollStart,scoreRollStart+rollSpacing,scoreRollStart+rollSpacing*2,scoreRollStart+rollSpacing*3,scoreRollStart+rollSpacing*4 };


// Textures used by the game
GameTexture gameTextures[]=
{

    { TexLogo, "logo",1,1},
    { TexPieces, "playing_cards", 4,5},
    { TexPiecesSelected, "playing_cards_selected", 4, 5},
    { TexFontScore, "",10,1},
    { TexGradient, ""},
    { TexMeter, "timer_numbers",4,5},
    { TexMeterBase, "timer_frames",1,2},
    { TexParticle, "particles",2,2},
    { TexFont, "font", 16,6},
    { TexBackground, "menu_bg", 2,2,},
    { TexExtra1, "dice_pause_play", 2, 1},
    { TexEndOfList }
};


// Create a main instance of the game
ITileGame *createTileGame( TileRenderer *renderer, int x, int y, int width, int height  ) {
    return new CTileNpc( renderer, x,y,width,height );
}


CTileNpc::CTileNpc( TileRenderer *renderer, int x,int y, int width, int height  ) : ITileGame( renderer,x,y,width,height )
{
    m_level = new CNpcLevel(renderer, m_particleEngine, TILE_GRIDWIDTH, TILE_GRIDHEIGHT);

    m_infoScreenDisplayCounter = -1;
    m_effectAngle = 0;
    m_bgIndex1 = 1;
    m_bgIndex2 = 0;
    m_fadingBgCounter = -1;
    m_menuCounter = -1;
    m_menuModeCounter = -1;

    m_timeSinceLastScore = 0;
    m_gameIsOn = 0;
    m_timer = 0;
    m_gameOverCounter = -1;
    m_pauseCounter = -1;
    m_levelCompletedPoem = "NOT_SET";
    m_timeTimerEffect = 65535;
    m_completedTextAngle = 0;
    m_completedTextCounter = -65536*21;

    m_eventCounter = 0;
    m_bgAngle = 0;
    m_logoWobble = 0;
    m_logoWobbleInc = 0;

    m_currentLevel = 0;
    m_blockTimerEffect = 0;
    m_targetTimer= 0;


    m_levelCompletedCounter = 0;
    m_highScore = 0;
    m_timer = 0;
    m_score = 0;
    m_displayScore =0;
    m_difficulty = 1;			// adults, 0 for kids ?
    m_level->setGameArea(x-0000,y-8000,width+0000,height+6000 );
    //m_level->setGameArea( x,y, width, height );

    memset( m_scoreRollPos, 0, sizeof(int) *7  );
    memset( m_scoreRollTargetPos, 0, sizeof(int) *7  );
    memset( m_scoreRollExTargetPos, 0, sizeof(int) *7  );
    memset( m_scoreRollInc, 0, sizeof(int) *7  );
}


CTileNpc::~CTileNpc()
{
    save();
    if (m_level) {
        delete m_level;
        m_level = 0;
    };
    if (m_particleEngine) {
        delete m_particleEngine;
        m_particleEngine = 0;
    };
}


/**
 *
 * Notification arriving when the game state has been changed
 *
 */
void CTileNpc::gameStateChanged()
{
    m_infoScreenDisplayCounter = -1;
    m_pauseCounter = -1;
    switch (m_state) {
    case PausedState:
        m_completedTextCounter = -65536*21;
        m_pauseCounter = 0;
        break;
    case GameOverState:
        m_completedTextCounter = -65536*21;
        m_level->setLevelState( LevelStateGameOver );
        break;
    case RunningState:
        if (m_gameIsOn==0) {
            nextLevel(0);
        }
        break;

    case MenuState:
        changeBg(0);
        m_timeTimerEffect = 65536;
        m_logoWobble = 65535;
        m_menuCounter = 0;
        m_menuModeCounter = 0;
        m_completedTextCounter = -65536*21;
        break;

    case ShowingInfoScreenState:
        m_infoScreenDisplayCounter = 0;
        break;

    };
}



/**
 *
 * Set the game's state to gameover
 *
 */
void CTileNpc::gameOver()
{
    m_gameOverCounter = 0;
    m_gameIsOn = 0;
    setGameState( GameOverState );
}


/**
 *
 * Set the game's state to level completed
 *
 */
void CTileNpc::levelCompleted()
{
    m_levelCompletedCounter = 0;
    m_completedTextCounter = -65536*20;
    m_levelCompletedPoem = happy_strings[ m_currentLevel % 44 ];
    m_renderer->effectNotify( LevelCompletedEffect, 0,0 );
    m_level->setLevelState( LevelStateCompleted );
    m_score += m_currentLevel*100;
    m_particleEngine->spray(
                20,
                65536/2, 20000, 1000,
                0,-50000, 65536,
                0, &m_level->m_fruitSpray );

}


/**
 *
 * Mark the game ended. Called when outside event wan't to end the game
 *
 */
void CTileNpc::endGame()
{
    gameOver();
}


/**
 *
 * Change the background to another one. Update will manage the fading automatically
 *
 */
void CTileNpc::changeBg( int newBg )
{
    if (newBg==-1) {
        newBg = ((m_currentLevel+1)&1);
    }
    m_bgIndex2 = newBg;
    if (m_bgIndex1==m_bgIndex2) return;		// no need
    m_fadingBgCounter = 0;		// START fading
}


/**
 *
 * Update the game
 *
 */
int CTileNpc::gameRun( int fixedFrameTime )
{
    if (fixedFrameTime>8000) fixedFrameTime = 8000;

    // Run the infoscreen if visible
    if (m_infoScreenDisplayCounter>=0)
        m_infoScreenDisplayCounter+=fixedFrameTime;


    // Run the background
    m_effectAngle+=fixedFrameTime;
    if (m_fadingBgCounter>=0) {
        m_fadingBgCounter+=fixedFrameTime;
        if (m_fadingBgCounter>65535) {
            m_bgIndex1 = m_bgIndex2;
            m_bgIndex2 = 0;
            m_fadingBgCounter = -1;
        };
    };


    // Display/Hide menu animation according to gameState (MenuState)
    if (m_state==MenuState)
    {
        m_menuCounter+=fixedFrameTime;
        m_menuModeCounter+=fixedFrameTime;
    }
    else
    {
        if (m_menuCounter>=0) {
            if (m_menuCounter>65536*3) m_menuCounter = 65536*3;
            m_menuCounter-=fixedFrameTime;
        }

        if (m_menuModeCounter>=0) {
            if (m_menuModeCounter>65536*2) m_menuModeCounter = 65536*2;
            m_menuModeCounter-=fixedFrameTime*2;
        }
    }


    // Display/Hide level completed animatino according game's current state
    if ( m_gameIsOn && (m_level->getState()==LevelStateIdle || m_level->getState()==LevelStateCompleted) )
    {
        m_levelCompletedCounter+=fixedFrameTime;
    }
    else
    {
        if (m_levelCompletedCounter>65536*3) m_levelCompletedCounter = 65536*3;
        if (m_levelCompletedCounter>-1) m_levelCompletedCounter-=fixedFrameTime*2;
    }

    // General event counter, occurs 4 times in a second
    m_eventCounter+=fixedFrameTime*4;
    while ( m_eventCounter>65536)
    {
        // If menu is active, randomly squirt particles around
        if (m_state==MenuState)
        {
            if ((rand()&255) < 128 ) {
                int dx = ((rand() & 511) - 256)<<6;
                int dy = ((rand() & 255) - 512)<<7;


                m_particleEngine->spray(
                            (7+(rand()&3))*5,
                            65536/2+dx, 11000, (rand()&511)*12,
                            dx,dy, (rand()&65535)/1,
                            0, 	&m_level->m_fruitSpray );


            };
            m_eventCounter = 0;
        };


        // If the game is running
        if (m_state==RunningState) {

            // Fasten up the game so a single level won't last forever
            m_timeTimerEffect-=20;

            // If we are in idlestate ( level is completed, squirt sparkle particles around as a "fireworks"-effect
            if (m_level->getState() == LevelStateIdle )
            {
                for (int j=0; j<3; j++)
                    if ((rand()&255)<100)
                        m_particleEngine->spray( 10, 2*(rand()&65535), 20000+(rand()&255)*40, 5000, 0,0, 32000, 0, &m_level->m_sparkleSpray );
            }

            // Enable hint display if enough time has been passed and our state allows it.
            int timeLimit = 4+m_currentLevel*4;
            if (timeLimit>20) timeLimit= 20;
            if (m_level->getState() == LevelStateNormal && m_timeSinceLastScore>timeLimit*65536)
            {
                if ((rand()&255)<32)
                    m_level->wobbleHint();
            }
        }

        // An event has been procecced, decrease counter
        m_eventCounter-=65536;


        // Target the scores to be displayed to actual scores collected.
        int d=(m_score - m_displayScore);
        d>>=1;
        if (abs(d)<1) {
            if (m_displayScore>m_score) d=-1;
            else if (m_displayScore<m_score) d = 1;
        }
        m_displayScore += d;
    }

    // Rotate the background according our game's current speed.
    int i = (m_timeTimerEffect>>9);
    if (i<1096) i = 1096;
    m_bgAngle+=(i*fixedFrameTime)>>12;

    // Logo's wobble effect
    int g = ((m_logoWobble * fixedFrameTime)>>13);		// string effect
    m_logoWobbleInc-=g;
    g = ((m_logoWobbleInc * fixedFrameTime)>>14);		// slowdown
    m_logoWobbleInc -= g;
    g = ((m_logoWobbleInc * fixedFrameTime)>>13);		// add
    m_logoWobble+=g;


    // Run the gameovercounter .. Increased if active according passed time
    if (m_gameOverCounter>=0) m_gameOverCounter+=fixedFrameTime;
    // Run the pcounter .. Increased if active according passed time
    if (m_pauseCounter>=0) m_pauseCounter+=fixedFrameTime;

    // Increase an angle attribute used for showing the levelcompleted-animation
    m_completedTextAngle += fixedFrameTime;

    // Call level's run if required
    if (m_state==GameOverState || m_state==RunningState)
        m_level->run( fixedFrameTime );


    if (m_state == RunningState) {
        if (m_level->m_hasMoves==0) {
            // this should not happend
        }

        // Check if level have collected scores that are not yet added into our counter.
        // Take them and reset an attribute calculating time passed since last scoreadd.
        int scoreAdd = m_level->takeLevelScore();
        if (scoreAdd>0) {
            m_timeSinceLastScore = 0;
        };
        m_score+=scoreAdd;

        // Update the high-score
        if (m_score>m_highScore) m_highScore = m_score;

        // Check if level have process have been going forward
        m_targetTimer += m_level->takeLevelProgressChange() * m_blockTimerEffect;
        m_timer = m_targetTimer;

        // Run a counter for displaying the "poem" after each level.
        if (m_completedTextCounter>=-65536*20)
        {
            // Wait for level to clear itself up first
            if (m_level->getState() != LevelStateCompleted)
            {
                int tlen = strlen(m_levelCompletedPoem);
                m_completedTextCounter += fixedFrameTime * 28;
                if (m_completedTextCounter>800*65536) m_completedTextCounter = -65536*21;		// end and stop
            }
        }


        switch (m_level->getState()) {
        case LevelStateNormal:
            m_timeSinceLastScore += fixedFrameTime;
            if (m_timeSinceLastScore>m_waitBeforeTimerStartsTime)
            {
                m_targetTimer += (int)( (float)fixedFrameTime * (float)m_timeTimerEffect / 65536.0f );
            }

            // If nothing is changing in the level currently, check if the game is over or level completed.
            if (m_level->doingNothing() == true) {

                if (m_targetTimer >=(65536<<8)) {
                    levelCompleted();
                }
                if (m_targetTimer<=0) gameOver();

            }
            break;

        case LevelStateIdle:
            break;

        default:
            break;
        }
    }


    // Run the scroller-screen on top of the game,
    // Set a targetvalue for each of them
    int v = ((m_timer>>8)*100) >> 16;
    if (v>99) v = 99;
    // First two are level progress
    m_scoreRollTargetPos[1] = (v%10) << 16;
    m_scoreRollTargetPos[0] = ((v/10)%10) <<16;
    // And the rest are for displaying scores
    m_scoreRollTargetPos[6] = (m_displayScore%10) << 16;
    m_scoreRollTargetPos[5] = ((m_displayScore/10)%10) <<16;
    m_scoreRollTargetPos[4] = ((m_displayScore/100)%10) <<16;
    m_scoreRollTargetPos[3] = ((m_displayScore/1000)%10) <<16;
    m_scoreRollTargetPos[2] = ((m_displayScore/10000)%10) <<16;

    bool rollChanged = false;

    // Rotate the rolls according new targets
    for (int sr = 0; sr<7; sr++)
    {
        if (m_scoreRollTargetPos[sr]!=m_scoreRollExTargetPos[sr]) rollChanged = true;
        m_scoreRollExTargetPos[sr] = m_scoreRollTargetPos[sr];

        int delta = m_scoreRollTargetPos[sr] - m_scoreRollPos[sr];
        int negDelta = (m_scoreRollTargetPos[sr]-65536*10) - m_scoreRollPos[sr];
        int plusDelta = (m_scoreRollTargetPos[sr]+65536*10) - m_scoreRollPos[sr];

        if (abs(delta)>abs(negDelta)) delta = negDelta;
        if (abs(delta)>abs(plusDelta)) delta = plusDelta;
        m_scoreRollInc[sr] += ((((delta)>>2)*fixedFrameTime ) >> 12);
        m_scoreRollInc[sr] -= (((m_scoreRollInc[sr]>>3) * fixedFrameTime) >> 10 );
        m_scoreRollPos[sr] += m_scoreRollInc[sr];
        if (m_scoreRollPos[sr]<0) m_scoreRollPos[sr] += 65536*10;
        if (m_scoreRollPos[sr]>65536*10) m_scoreRollPos[sr] -= 65536*10;
    }


    // There used to be a soundeffect when a roll position changed,
    // This event used to be used for that purpose.
    // Currently, the sound is disabled though
    if (rollChanged)
    {
        m_renderer->effectNotify( ScoreChangedEffect, 0,0 );
    }
    return 1;
};



#define BGTILE_OFF 40000


/**
 *
 * Render the game
 *
 */
int CTileNpc::gameDraw( ) {

    // Render the background actually here.
    if (m_fadingBgCounter==-1)
        m_renderer->renderTile( 0,0, 65536, 100000, 0, 0, (TexBackground<<16) + m_bgIndex1, 0 );
    else {
        int a = (m_fadingBgCounter>>8);
        m_renderer->renderTile( 0,0, 65536, 100000, 0, 0, (TexBackground<<16) + m_bgIndex1 | (a<<24), 0 );
        m_renderer->renderTile(  0,0, 65536, 100000, 0, 0, (TexBackground<<16) + m_bgIndex2 | ((255-a)<<24), 0 );
    };

    m_renderer->renderTile( -BGTILE_OFF,-BGTILE_OFF, 65536+BGTILE_OFF*2, 65536+BGTILE_OFF*2, ((m_bgAngle>>2)&65535), 1, ((TexBackground<<16) + 2) | (200<<24), 0 );



    // Render the level if our state allows us to
    if (m_state!=MenuState && m_state!=PausedState)
    {
        m_level->draw( *m_renderer );
    }



    // Render the infoscreen if counter indicating it's visibility says so
    if (m_infoScreenDisplayCounter>=0)
    {
        int f;
        for (int l=0; l<INFOSCREEN_LINECOUNT; l++)
            if (info_screen_lines[l][0]!=0)
            {
                f = 65536*2 + ((l)*32000) - m_infoScreenDisplayCounter * 2;
                if (f<0) f = 0;
                if (f>65535) f= 65535;
                writeEffectText( 3000+l * 5500, info_screen_lines[l], 5000, f );
            }
    }


    // Render the menu according a counter indicating it's visibility
    if (m_menuCounter>=0)
    {
        int fade = 65536*2-(m_menuCounter);
        fade/=2;
        if (fade<0) fade = 0;
        writeEffectText( 65536 - 4500, "START", 14000, fade );

        char testr[64];
        sprintf(testr, "HI %d", m_highScore);
        writeText(0,0, testr, fade>>8, 5000,3500);

#if !defined(__APPLE__) && !defined(Q_WS_HARMATTAN)
        // There was no soft exit button in old iPhone, legacy from here.
        // Also, no exit button is required in Harmattan.
        writeText(42000, 0, "EXIT", fade >> 8, 8000, 5500);
#endif
    }

    // Draw the "info"-button
    if (m_menuModeCounter>0)
    {
        int fade = 65536-(m_menuModeCounter);
        if (fade<0) fade = 0;
        writeEffectText( 65536*5/4+4000, "INFO", 8000, fade );
    }


    if (m_state!=MenuState)
    {
        // Write "level completed" - notification
        if (m_levelCompletedCounter>0) {
            int fade = 65536-(m_levelCompletedCounter);
            //fade/=2;
            if (fade<0) fade = 0;

            writeEffectText( 19000+fade, "LEVEL", 7000, fade );
            writeEffectText( 24000+fade, "COMPLETED", 9500, fade );
        };


        // Write "game paused"-screen, with resume and end - buttons
        if (m_pauseCounter>0) {
            int fade = 65536-(m_pauseCounter*2);
            if (fade<0) fade = 0;
            writeEffectText( 65536/2-4000-fade, "GAME", 9500, fade );
            writeEffectText( 65536/2+4000+fade, "PAUSED", 9500, fade );

            writeEffectText( 65536*4/3-4000-fade, "RESUME", 5500, fade );
            writeEffectText( 65536*4/3+4000+fade, "END", 5500, fade );
        };



        // Write "game over" screen with all the information about the game.
        if (m_gameOverCounter>0 && m_infoScreenDisplayCounter<0)
        {
            int fade = m_gameOverCounter-40000*2;
            if (fade>65535) fade = 65535;
            if (fade>0) {
                writeEffectText( 12000-4000-(65535-fade)/2, "TOO BAD", 5000, 65535-fade );
                fade = m_gameOverCounter-40000*3;
                if (fade>0) {
                    if (fade>65535) fade = 65535;
                    writeEffectText( 12000+4000+(65535-fade)/2, "GAME OVER", 9000, 65535-fade );

                    fade = m_gameOverCounter-40000*4;
                    if (fade>0) {
                        if (fade>65535) fade = 65535;
                        writeEffectText( 40000, "YOUR SCORE", 5000, 65535-fade );

                        fade = m_gameOverCounter-40000*5;
                        if (fade>0) {
                            if (fade>65535) fade = 65535;
                            char testr[12];
                            sprintf(testr, "%d", m_score );
                            writeEffectText( 45000, testr, 9000, 65535-fade );

                            fade = m_gameOverCounter-40000*6;
                            if (fade>0) {
                                if (fade>65535) fade = 65535;
                                writeEffectText( 60000, "AT LEVEL", 5000, 65535-fade );

                                fade = m_gameOverCounter-40000*7;
                                if (fade>0) {
                                    if (fade>65535) fade = 65535;
                                    sprintf(testr, "%d", m_currentLevel+1 );
                                    writeEffectText( 65000, testr, 9000, 65535-fade );
                                }
                            }
                        }
                    }
                }
            }
        }
    }



    // Writes the level-completed-poem.
    // Function self-checks if it should be visible or not.
    writeLevelCompletedString();


    // Draw the ingame - HUD accoring an attribute indicating it's visibility
    if (m_hudState>0)
    {
        int x = -3000;
        int w = 65536+7000;
        int h = 16000;
        int y = -(65536-m_hudState)/3 - 2000;

        m_renderer->renderTile(2000,y,15000,h,0,0, (TexMeterBase<<16) | 1, 0);
        m_renderer->renderTile(29000,y,35000,h,0,0, (TexMeterBase<<16) | 1, 0);


        // Render the roll-backgrounds
        for (int f=0; f<7; f++)
            m_renderer->renderTile(scoreRollXPos[f] - rollWidth/2,
                                   y+h/2-(h*7/32),
                                   rollWidth,
                                   h*14/32,
                                   0,0, (TexMeter<<16), 0);

        // Render the rolls
        for (int yroll = -2; yroll<=1; yroll++)
        {
            for (int f=0; f<7; f++)
            {
                int num = ((m_scoreRollPos[f] >> 16) - 1 - yroll ) % 10;
                if (num<0) num  = 10+num;

                int yofs = ((m_scoreRollPos[f]&65535)>>4) + (yroll*4096);

                //m_level->getCosineTable()[ yofs&4096
                int ypos = sin( (float)yofs / 4096.0f * 3.14159f/4.0f )*(float)h/3.3f;

                //int numheight = (65536<<10) / (abs(ypos)+4096);
                int numheight = h/2 - abs(ypos)*13/8;


                //numheight/=3;
                m_renderer->renderTile(scoreRollXPos[f] - rollWidth/2,
                                       y+h/2-(numheight/2) + ypos,
                                       rollWidth,
                                       numheight,
                                       0,0, (TexMeter<<16) | (1+num), 0 );
            }
        }


        // Overlay for the bar
        m_renderer->renderTile(x,y,w,h,0,0, (TexMeterBase<<16) | 0, 0);

        // Render pausebutton
        if (m_state == PausedState)
            m_renderer->renderTile(x+21000,y-65536+m_hudState,h*2/3,h*2/3*m_areaHeight/m_areaWidth,0,0, (TexExtra1<<16) | 1, 0);
        else
            m_renderer->renderTile(x+21000,y-65536+m_hudState,h*2/3,h*2/3*m_areaHeight/m_areaWidth,0,0, (TexExtra1<<16) | 0, 0);

        // Render the current-level with two separate numbers.

        // 1
        m_renderer->renderTile( 14000,6000-65536+m_hudState,14000,14000*m_areaWidth/m_areaHeight,0, 0, (TexParticle<<16)+0,0 );
        int n = ((m_currentLevel+1)/10);
        if (n==0) n = 10;
        m_renderer->renderTile( 14200,6200-65536+m_hudState,13000,13000*m_areaWidth/m_areaHeight,0, 0, (TexMeter<<16)+n,0 );

        // adn 2
        m_renderer->renderTile( 20000,9000-65536+m_hudState,14000,14000*m_areaWidth/m_areaHeight,0, 0, (TexParticle<<16)+0,0 );
        n = ((m_currentLevel+1)%10);
        if (n==0) n = 10;
        m_renderer->renderTile( 20200,9200-65536+m_hudState,13000,13000*m_areaWidth/m_areaHeight,0, 0, (TexMeter<<16)+n,0 );
    }

    // Render the game logo according an attribute indicating it's visibility
    if (m_logoState>0)
    {
        int mx = 65536/2;
        int my = (-60536 + m_logoState*2);
        int w = ((((65535 - m_logoWobble*2)>>2) * 1200)>>8);
        int h = ((((65535 + m_logoWobble*2)>>2) * 1200)>>8);
        int fade = 255-(m_logoState>>4);
        if (fade<0) fade = 0;
        m_renderer->renderTile( mx-w/2,my-h,w,h, 0,0, (TexLogo<<16) | (fade<<24), 0 );
    }

    // Render the particles
    m_particleEngine->draw();

    return 1;
}


/**
 *
 * Incoming click from the framework
 *
 */
void CTileNpc::handleGameMouseEvent(MouseEvent eventType, int fixedX, int fixedY)
{

    if (m_state == ShowingInfoScreenState) {
        if (eventType == ButtonDown) {
            setGameState(MenuState);
        }

        return;
    }

    if (eventType == ButtonDown)
    {
        switch (m_state)
        {
        case GameOverState:
            if (m_level->getState() == LevelStateIdle)
            {
                if (m_gameOverCounter<65536*6)
                    m_gameOverCounter = 65536*6;
                else
                    setGameState(MenuState);
            }
            return;

        case PausedState:
            m_particleEngine->spray( 20, fixedX, fixedY, 5000, 0,0, 32000, 0, &m_level->m_sparkleSpray );

            if (fixedY > 83000) {
                if (fixedY > 90000) {
                    gameOver();
                }
                else {
                    setGameState(RunningState);
                }
            }

            return;

        case MenuState:
            m_particleEngine->spray(20, fixedX, fixedY, 5000, 0, 0, 32000, 0,
                                    &m_level->m_sparkleSpray);

            if (fixedX > 42000 && fixedY < 10000) {
#ifndef Q_WS_HARMATTAN
                // No exit button required in Harmattan
                m_renderer->effectNotify(ExitEffect, 0, 0);
#endif
            }
            else {
                if (fixedY > 84000) {
                    // Display the info screen
                    setGameState(ShowingInfoScreenState);
                }
                else {
                    // Start the game
                    if (fixedY < 60000) {
                        m_logoWobbleInc += 16000;
                    }
                    else {
                        setGameState( RunningState );
                        m_logoWobble = -20000;
                    }
                }
            }

            return;

        default:
            // Level completed
            if (m_level->getState() == LevelStateIdle) {
                m_particleEngine->spray( 50, fixedX,fixedY, 5000, 0,0, 76000, 0, &m_level->m_sparkleSpray );
                nextLevel();
            }
            else {
                if (fixedY < 12000) {
                    if (fixedX > 21000 && fixedX < 28000) {
                        setGameState( PausedState );
                    }
                    else {
                        return;
                    }
                }
            }
            break;
        }
    } // if (eventType == ButtonDown)


    // Send mouse event's to the level if not yet used and it exists
    if (m_gameIsOn
            && m_state == RunningState
            && m_level->getState() != LevelStateIdle)
    {
        switch (eventType) {
        case ButtonDown: m_level->handleMouseEvent(0, fixedX, fixedY); break;
        case MouseDrag: m_level->handleMouseEvent(1, fixedX, fixedY); break;
        case ButtonUp: m_level->handleMouseEvent(2, fixedX, fixedY); break;
        }
    }
}


/**
 *
 * Create new level and set game's states for playing.
 *
 */
void CTileNpc::nextLevel( int restartAt )
{

    if (restartAt>=0) {
        m_currentLevel = restartAt;
        m_score = 0;
        m_displayScore =0;
        m_gameIsOn = 1;				// mark game ongoing
        m_gameOverCounter = -1;
    } else {
        m_currentLevel++;
        m_score+=m_currentLevel*m_currentLevel*10;
    }

    m_timeSinceLastScore = 0;
    m_completedTextCounter = -21*65536;
    m_targetTimer = 65536*100;


    float temp = 5.0f / ((float)m_currentLevel+6.0f);
    m_timeTimerEffect = -(int)( 240000.0f*(1.0f-temp));
    m_timeTimerEffect -= m_currentLevel*13000;						// linear component
    m_blockTimerEffect = (65536*22) / (m_currentLevel+60);			// 30


    //m_waitBeforeTimerStartsTime = 65536 - m_currentLevel*5000;
    m_waitBeforeTimerStartsTime = 65536*15 / (3+m_currentLevel);
    if (m_waitBeforeTimerStartsTime<0) m_waitBeforeTimerStartsTime = 0;

    m_level->createLevel( m_currentLevel );

    changeBg();
}


/**
 *
 * Legacy from original ITileGame-interface
 *
 */
void CTileNpc::setIntAttribyte( TileGameAttribute att, int set )
{
    switch (att) {
    case AttributeDifficulty:
        m_difficulty = set;
        m_level->m_difficulty = set;
        break;
    }
}


/**
 *
 * Legacy from original ITileGame-interface
 *
 */
int CTileNpc::getIntAttribute( TileGameAttribute att, unsigned int arg ) {
    switch (att) {
    default:
        return 0;
    case AttributeDifficulty:
        return m_difficulty;
        break;
    case AttributeHighScore:
        return m_highScore;
        break;
    case AttributeScore:
        return m_score;
    case TimeLimitCurrent:
        return m_timer;
    case TimeLimitMax:
        return 65536 * 256;
    case AttributeCurrentLevel:
        return m_currentLevel;

    };
};


#ifdef __APPLE__
#define SAVE_FILE "../Documents/fcstate.bin"
#else
#define SAVE_FILE "fcstate.bin"
#endif


/**
 *
 * Save the game's current state into a file
 *
 */
int CTileNpc::save()
{
    FILE *file = fopen(SAVE_FILE, "wb" );
    if (!file) return 0;
    fwrite( &m_difficulty, 1,11*sizeof(int), file );
    if (m_gameIsOn==1) m_level->saveToFile( file );
    fclose(file);
    return 1;
}


/**
 *
 * Load the game's current state from a file
 *
 */
int CTileNpc::load()
{

    FILE *file = fopen(SAVE_FILE, "rb" );
    if (!file) return 0;
    int rval = 0;
    fread( &m_difficulty, 1,11*sizeof(int), file );
    if (m_gameIsOn) {
        m_level->loadFromFile( file );
        m_level->m_difficulty=m_difficulty;
        changeBg();
        rval = 1;
    }
    fclose(file);
    return rval;
}


/*!
  From ITileGame.
*/
void CTileNpc::pause()
{
}


/*!
  From ITileGame.
*/
void CTileNpc::resume()
{
}


/**
 *
 * Writes a wobbling text to desired position
 *
 */
void CTileNpc::writeEffectText( int y, const char *text, int charSize, int fade )
{
    char ch;
    int totcount =0;
    int *cosin = m_level->getCosineTable();
    int yadd;
    int xadd;
    int charSpace = charSize*5/8;
    int x= 65536/2 - strlen(text)*charSpace/2;
    while (*text!=0)
    {
        if (*text!=' ')
        {
            if (ch>='a' && ch<='z') ch = ch-'a'+'A';        // to high-case
            ch = *text -32;

            yadd = 	((cosin[(totcount*850 + m_effectAngle/70)&4095]>>8)*fade)/600;
            yadd += (cosin[(1000+m_effectAngle/10+totcount*600)&4095]>>8);
            xadd = (cosin[(m_effectAngle/30+totcount*800)&4095]>>8);

            m_renderer->renderTile( x+xadd,y+yadd,
                                    charSize,charSize, fade*2, 0, (TexFont<<16) | ch | ((fade>>8)<<24), 0 );

        }
        totcount++;
        x+=charSpace;
        text++;
    }
}




/**
 *
 * Write a text into specified position of the screen
 *
 */
void CTileNpc::writeText( int x,int y, const char *text, int fade, int charSize, int charSpace ) {
    char ch;
    while (*text!=0) {
        if (*text!=' ') {
            ch = *text-32;
            m_renderer->renderTile( x,y,charSize,charSize, 0, 0, (TexFont<<16) | ch | (fade<<24), 0 );
        }
        x+=charSpace;
        text++;
    };
};



/**
 *
 * Write a "level completed string" with a custom effect
 *
 */
void CTileNpc::writeLevelCompletedString()
{

    int x = 0;
    int y = 38000;
    const char *text = m_levelCompletedPoem;
    int charSize = 6000;
    int charSpace = charSize*5/8;
    int fade = 0;

    int *cosineTable = m_level->getCosineTable();
    int totCount = 0;
    int f,g;
    char testr[128];		// Word, line.
    char cur_line[256];

    cur_line[0] = 0;		// Zero the line
    int sizeinc;
    char ch;
    while (1)
    {

        // Seek the next word to be added in our current line (cur_line)
        while ((*text==' ' || *text==10 || *text==13) && *text!=0) *text++;
        f=0;
        while (*text!=' ' && *text!=13 && *text!=10 && *text!=0 && f<255)
        {
            testr[f] = *text;
            *text++;
            f++;
        };
        testr[f]=0;


        // Draw the current line (cur_line)
        if ( f==0 || (strlen(testr)+strlen(cur_line))*charSpace > 62000 )
        {
            f=0;
            int j = strlen(cur_line);
            x = 65536/2-j*charSpace/2;
            x-=charSpace/4;
            while (cur_line[f]!=0)
            {
                ch = cur_line[f];
                if (ch>='a' && ch<='z') ch = ch-'a'+'A';        // to high-case
                ch-=32;
                g= ((totCount<<16) - m_completedTextCounter);
                //if (g<0) g = 0;
                if (g<-128*65536) g = g+128*65536; else	if (g<0) g= 0;

                sizeinc = -abs(g)/256;
                fade = (g>>11);

                if (fade>255) return;
                fade = abs(fade);

                if (ch!=255 && fade<255)
                {
                    m_renderer->renderTile(
                                x-sizeinc/2+(cosineTable[(1000+m_completedTextAngle/20+f*600)&4095]>>10),
                                g/64+y-sizeinc/2+(cosineTable[(m_completedTextAngle/30+f*800)&4095]>>9),
                                charSize+sizeinc,
                                charSize+sizeinc,
                                -((sizeinc*8)&65535),
                                0,
                                (TexFont<<16) | ch | (fade<<24),
                                0 );
                }
                totCount++;
                x+=charSpace;
                f++;
            }

            // line drawn
            cur_line[0] = 0;		// clear it.
            x=0;
            y+=charSpace*4/3;


            cur_line[0] = 0;
        } else strcat( cur_line, " ");

        if (testr[0]==0) break;
        strcat( cur_line, testr );
    }
}
