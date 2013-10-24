/**
 * Copyright  2011 Nokia Corporation.
 *
 */


#ifndef __ParticleEngine__
#define __ParticleEngine__

#include "tilegameinterfaces.h"

#define MAX_PARTICLES 512



struct SParticleSprayType;

struct SParticle {
    int x,y, size, angle;
    int dx, dy, sizeinc, angleinc;
    int lifeTime;

    unsigned int userData;
    unsigned int tileIndex;

    SParticleSprayType *spray;
};



typedef void (*PARTICLERENDERFUNCTION_TYPE) ( void *data, SParticle *p );


struct SParticleSprayType {
    int renderType;

    int gravity;
    int fraction;

    int lifeTime;
    int lifeTimeRandom;

    int size;
    int sizeRandom;

    int sizeInc;
    int sizeIncRandom;

    int angle;
    int angleRandom;

    int angleInc;
    int angleIncRandom;

    int firstBlock;
    int blockCount;

    PARTICLERENDERFUNCTION_TYPE renderFunction;
    void *dataToRenderFunction;
};






class ParticleEngine {
public:
    ParticleEngine( TileRenderer *renderer );
    virtual ~ParticleEngine();

    void createSprayType( SParticleSprayType *target,
                          int firstBlock,
                          int blockCount,
                          int gravity,
                          int fraction,
                          int lifeTime,
                          int lifeTimeRandom,
                          int size,
                          int sizeRandom,
                          int sizeinc,
                          int sizeincRandom,
                          PARTICLERENDERFUNCTION_TYPE renderFunction = 0,
                          void *dataToRenderFunction = 0,
                          int angle = 0,
                          int angleRandom = 0,
                          int angleinc= 0,
                          int angleincRandom=0,
                          int type = 0);

    void spray( int count,
                int x, int y, int posrandom,
                int dx, int dy, int dirrandom,
                unsigned int userData,
                SParticleSprayType *spray );

    void run( int fixedFrameTime16Bit );
    void draw();


protected:
    TileRenderer *m_renderer;
    SParticle m_particles[MAX_PARTICLES];
    int m_cp;
};


#endif
