/**
 * Copyright (c) 2012 Nokia Corporation.
 */

#ifndef TILERENDERER_H
#define TILERENDERER_H

#include "tilegameinterfaces.h"

class TileRenderer
{
public:
    TileRenderer();
    virtual ~TileRenderer();

public:
    void setGame(ITileGame *game);
    virtual void renderTile(int fixedX,
                            int fixedY,
                            int fixedWidth,
                            int fixedHeight,
                            int angle,
                            int mode,
                            unsigned int tileIndex,
                            unsigned int arg);
    virtual void renderBackground(unsigned int index);
    virtual void renderForeground(unsigned int index);
    virtual void effectNotify(TileGameEffect effect,
                              unsigned int arg1,
                              unsigned int arg2);
    virtual int run(int fixedFrameTime16Bit);

protected: // Data
    ITileGame *m_game; // Not owned
};

#endif // TILERENDERER_H
