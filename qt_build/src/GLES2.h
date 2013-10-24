/*
 *  GLES2Renderer.h
 *  seven
 *
 *  Created by jarnoh on 15.4.2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "utility.h"
#include "TileInterfaces.h"


typedef unsigned int TDWORD;

class GLES2Renderer: public ITileRenderer 
	{
	public:
		GLES2Renderer();
		virtual ~GLES2Renderer();
		
		void renderTile( int fixedx, int fixedy, 
						int fixedWidth, int fixedHeight, 
						int angle,
						int mode,
						unsigned int tileIndex, unsigned int arg );
		
		void renderBackground( unsigned int index );
		void renderForeground( unsigned int index ) ;
		void effectNotify( eTILEGAME_EFFECT effect, unsigned int arg1, unsigned int arg2 );
		
		int run( int fixedFrameTime16Bit );
	protected:
		void play(int id);		
		int previousMode;

		GLuint pixelShader;
		GLuint vertexShader;
		GLuint program;
		GLuint vbo;

        GLuint angleAlphaLocation;
        GLuint sizeTScaleLocation;
        GLuint tposPositionLocation;
        
	};

