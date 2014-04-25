/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

#include "tilerenderer.h"
#include <qglobal.h> // For Q_UNUSED macro


/*!
  \class TileRenderer
  \brief
*/


/*!
  Constructor.
*/
TileRenderer::TileRenderer()
    : m_game(0)
{
}


/*!
  Destructor.
*/
TileRenderer::~TileRenderer()
{
}


/*!
  This should be called after game is created.
*/
void TileRenderer::setGame(ITileGame *game)
{
    m_game = game;
}


/*!
  For rendering a tile.
*/
void TileRenderer::renderTile(int fixedX,
                              int fixedY,
                              int fixedWidth,
                              int fixedHeight,
                              int angle,
                              int mode,
                              unsigned int tileIndex,
                              unsigned int arg)
{
    Q_UNUSED(fixedX);
    Q_UNUSED(fixedY);
    Q_UNUSED(fixedWidth);
    Q_UNUSED(fixedHeight);
    Q_UNUSED(angle);
    Q_UNUSED(mode);
    Q_UNUSED(tileIndex);
    Q_UNUSED(arg);
}


/*!
  For rendering the background.
*/
void TileRenderer::renderBackground(unsigned int index)
{
    Q_UNUSED(index);
}


/*!
  For rendering the foreground.
*/
void TileRenderer::renderForeground(unsigned int index)
{
    Q_UNUSED(index);
}


/*!
*/
void TileRenderer::effectNotify(TileGameEffect effect,
                                unsigned int arg1,
                                unsigned int arg2)
{
    Q_UNUSED(effect);
    Q_UNUSED(arg1);
    Q_UNUSED(arg2);
}


/*!
*/
int TileRenderer::run(int fixedFrameTime16Bit)
{
    Q_UNUSED(fixedFrameTime16Bit);
    return 0;
}
