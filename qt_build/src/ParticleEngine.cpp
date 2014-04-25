/**
 * Copyright (c) 2011-2014 Microsoft Mobile.
 */

#include "ParticleEngine.h"

#include <math.h>
#include <stdlib.h>

#include "tilerenderer.h"


/*!
  \class ParticleEngine
  \brief
*/


/*!
  Constructor.
*/
ParticleEngine::ParticleEngine(TileRenderer *renderer)
    : m_renderer(renderer),
      m_cp(0)
{
    for (int f = 0; f < MAX_PARTICLES; ++f) {
        m_particles[f].lifeTime = 0; // Dead
    }
}


/*!
  Destructor.
*/
ParticleEngine::~ParticleEngine()
{
}


/**
 *
 * Helper for initializing a spray.
 *
 */
void ParticleEngine::createSprayType(SParticleSprayType *target,
                                       int firstBlock,
                                       int blockCount,
                                       int gravity,
                                       int fraction,
                                       int lifeTime,
                                       int lifeTimeRandom,
                                       int size,
                                       int sizeRandom,
                                       int sizeInc,
                                       int sizeIncRandom,
                                       PARTICLERENDERFUNCTION_TYPE renderFunction,
                                       void *dataToRenderFunction,
                                       int angle,
                                       int angleRandom,
                                       int angleInc,
                                       int angleIncRandom,
                                       int type) {

    target->renderType = type;
    target->gravity = gravity;
    target->firstBlock = firstBlock;
    target->blockCount = blockCount;
    target->fraction = fraction;
    target->lifeTime = lifeTime;
    target->lifeTimeRandom = lifeTimeRandom;
    target->size = size;
    target->sizeRandom = sizeRandom;
    target->sizeInc = sizeInc;
    target->sizeIncRandom = sizeIncRandom;
    target->angle = angle;
    target->angleRandom = angleRandom;
    target->angleInc = angleInc;
    target->angleIncRandom = angleIncRandom;

    if (renderFunction) {
        target->renderFunction = renderFunction;
        target->dataToRenderFunction = dataToRenderFunction;
    } else {
        target->renderFunction = 0;
        target->renderFunction = 0;

    }
}


/**
 *
 * Spray some particles
 *
 */
void ParticleEngine::spray(int count,
                             int x, int y, int posrandom,
                             int dx, int dy, int dirrandom,
                             unsigned int userData,
                             SParticleSprayType *spray)
{

    int l;
    int nx,ny;
    while (count) {
        SParticle *p = m_particles+m_cp;;
        m_cp++;
        if (m_cp>=MAX_PARTICLES) m_cp = 0;

        p->spray = spray;
        nx = (rand()&511) - 256;
        ny = (rand()&511) - 256;

        l = (int)(sqrtf((float)(nx*nx+ ny*ny)));

        if (l==0) l = 1;
        // random vactor
        int v = (rand()&255);
        nx = nx*v / l;
        ny = ny*v / l;

        p->userData = userData;

        p->x = ((nx * posrandom)>>8) + x;
        p->y = ((ny * posrandom)>>8) + y;
        p->dx = ((nx * dirrandom)>>8) + dx;
        p->dy = ((ny * dirrandom)>>8) + dy;


        p->lifeTime = spray->lifeTime + (((rand()&255)*spray->lifeTimeRandom)>>8);
        p->tileIndex = spray->firstBlock + ((((rand()&511) * spray->blockCount)+256)>>9);

        p->size = spray->size + (((rand()&255)*spray->sizeRandom)>>8);
        p->sizeinc = spray->sizeInc + ((((rand()&511)-256)*spray->sizeIncRandom)>>8);


        p->angle = spray->angle + (((rand()&255)*spray->angleRandom)>>8);
        p->angleinc = spray->angleInc + ((((rand()&511)-256)*spray->angleIncRandom)>>8);

        count--;
    }
}



/**
 *
 * Run the particles
 *
 */
void ParticleEngine::run(int fixedFrameTime16Bit)
{
    SParticle *p = m_particles;
    SParticle *p_target = p + MAX_PARTICLES;
    int tx,ty, tsize;

    while (p!=p_target) {
        if (p->lifeTime>0) {
            p->lifeTime -= fixedFrameTime16Bit;
            if (p->lifeTime>0) {
                // gravitaatio, slowdown, jne..t‰nne.
                if (p->spray) {
                    tx = ((((p->dx * p->spray->fraction)>>16) * fixedFrameTime16Bit)>>16);
                    ty = ((((p->dy * p->spray->fraction)>>16) * fixedFrameTime16Bit)>>16);
                    p->dx -= tx;
                    p->dy -= ty;

                    p->dy += ((p->spray->gravity*fixedFrameTime16Bit)>>16);
                }


                tx = ((p->dx * fixedFrameTime16Bit)>>16);
                ty = ((p->dy * fixedFrameTime16Bit)>>16);
                tsize = ((p->sizeinc * fixedFrameTime16Bit)>>16);


                p->x += tx;
                p->y += ty;
                p->size += tsize;

                p->angle += ((p->angleinc * fixedFrameTime16Bit)>>16);

                if (p->size<=0) p->lifeTime = 0;		// die if zero sized

            }
        }
        p++;
    }
}



/**
 *
 * Draw the particles
 *
 */
void ParticleEngine::draw()
{
    // draw from latest, to least latest
    SParticle *p = m_particles;
    int count = m_cp -1;
    int a;

    while (1) {
        if (count<0) count = MAX_PARTICLES-1;
        if (count==m_cp) break;
        p = m_particles + count;

        if (p->lifeTime>0) {
            if (p->spray->renderFunction) {
                (p->spray->renderFunction)(p->spray->dataToRenderFunction, p);
            } else {
                a = (p->lifeTime>>6);
                if (a>255) a = 255;
                a = 255-a;

                m_renderer->renderTile(p->x - p->size/2, p->y - p->size/2,
                                        p->size, p->size,
                                        p->angle,p->spray->renderType,
                                        p->tileIndex | (a<<24),
                                        p->userData);
            }
        }
        count--;
    }
}
