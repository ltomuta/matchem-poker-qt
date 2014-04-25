/**
 * Copyright (c) 2011-2014 Microsoft Mobile.
 */

#include "tilegameinterfaces.h"
#include "ParticleEngine.h"
#include "tilerenderer.h"


/*!
  \class ITileGame
  \brief
*/


/*!
  Constructor
 */
ITileGame::ITileGame(TileRenderer *renderer, int x, int y, int width, int height)
    : m_particleEngine(0),
      m_renderer(renderer),
      m_areaX(0),
      m_areaY(0),
      m_areaWidth(0),
      m_areaHeight(0),
      m_state(NotSetState),
      m_logoState(0),
      m_hudState(0)
{
    m_particleEngine = new ParticleEngine(renderer);
    setGameArea(x, y, width, height);
}


/*!
  Destructor.
*/
ITileGame::~ITileGame()
{
    delete m_particleEngine;
}


/*!
  Runs the game.
*/
int ITileGame::run(int fixedFrameTime16Bit)
{
    int hudStateTarget = 0;
    int logoStateTarget = 0;

    switch (m_state) {
    case NotSetState:
        if (load() == 1) {
            setGameState(PausedState);
        }
        else {
            setGameState(MenuState);
        }

        break;
    case MenuState:
        logoStateTarget = 65535;
        break;
    case RunningState:
        hudStateTarget = 65535;
        break;
    default:
        break;
    }

    m_logoState += (((logoStateTarget - m_logoState) * fixedFrameTime16Bit) >> 13);
    m_hudState += (((hudStateTarget - m_hudState) * fixedFrameTime16Bit) >> 14);

    int result = gameRun(fixedFrameTime16Bit);
    m_particleEngine->run(fixedFrameTime16Bit);

    return result;
}




/*!
  Draws the game.
*/
int ITileGame::draw()
{
    m_renderer->renderBackground(0);
    int rval = gameDraw();
    m_renderer->renderForeground(0);
    return rval;
}


/*!
  Changes the game state to \a state. Sends notifications accordingly.
*/
void ITileGame::setGameState(TileGameState state)
{
    switch (state) {
    case MenuState:
        m_renderer->effectNotify(MenuEffect, 0, 0);
        break;
    case GameOverState:
        m_renderer->effectNotify(GameOverEffect, 0, 0);
        break;
    }

    m_state = state;
    gameStateChanged();	// Notify
}


/*!
  Sets the game area according to \a x, \a y, \a width and \a height.
*/
void ITileGame::setGameArea(int x, int y, int width, int height)
{
    m_areaX = x;
    m_areaY = y;
    m_areaWidth = width;
    m_areaHeight = height;
}


/*!
  Handles the mouse event received from the framework by propagating it
  to the derived class instance. Implement the appropriate actions in the
  derived class.
*/
void ITileGame::handleMouseEvent(MouseEvent eventType, int fixedX, int fixedY)
{
    handleGameMouseEvent(eventType, fixedX, fixedY);
}
