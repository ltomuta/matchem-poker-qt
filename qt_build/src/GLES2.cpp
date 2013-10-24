/*
 * GLES2Renderer.cpp
 * @author jarnoh
 *
 */

#include "GLES2.h"
#include "TileInterfaces.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "utility.h"

#if defined(_WIN32) && !defined(__SYMBIAN32__)
#include <windows.h>
#include <mmsystem.h>
#endif

extern float matrixProjection[16];
float matrixProjection[16]=
{
    2/65536.0f, 0, 0, 0,
    //0, -2/(640.0f/360.0f*65536.0f), 0, 0,
    0, -2/(1.5f*65536.0f), 0, 0,
    0, 0, 1, 0,
    -1, 1, 0, 1,
};




const int vboStride = 2 * sizeof(GLfloat);
const GLfloat vboVertices[] = {
	-1, -1,
	1, -1,
	1, 1,
	-1, 1,
};


const char *glslPixel =
"uniform sampler2D texture;"
"varying mediump vec2 uv;"
"varying mediump float alpha;"
"void main(void)"
"{"
"gl_FragColor = texture2D(texture,uv)*vec4(1.0,1.0,1.0,alpha);"
"}";


const char *glslVertex =
"uniform mat4 m_projection;"
"uniform vec4 size_tscale;"
"uniform vec4 tpos_position;"
"uniform vec2 angle_alpha;"
"attribute vec4 vert;"
"varying vec2 uv;"
"varying float alpha;"
"void main(void) { "
"float a=angle_alpha.x*(3.1415*2.0/65536.0); "
"vec2 rot = vec2(vert.x*cos(a) + vert.y*sin(a), -vert.x*sin(a) + vert.y*cos(a)); "
"gl_Position = m_projection*vec4(tpos_position.zw+rot.xy*size_tscale.xy,0.0,1.0);"
"uv = (vert.xy+vec2(1.0,1.0))*vec2(0.5,0.5) * size_tscale.zw + tpos_position.xy; alpha = angle_alpha.y; }";


GameTexture *bind(TextureID id)
{
	static TextureID prevId=(TextureID)-1;
	static GameTexture *t;

	if(id==prevId) return t;

	prevId=(TextureID)-1;
	t=&gameTextures[0];
	while(1)
	{
		if(t->id==TexEndOfList) return NULL;
		if(t->id==id)
		{
			glBindTexture(GL_TEXTURE_2D, t->glId);
			prevId=id;
			return t;
		}
		t++;
	}
	return NULL;
}

void load(TextureID id)
{
	GameTexture *t=&gameTextures[0];
	while(1)
	{
		if(t->id==TexEndOfList) return;
		if(t->id==id)
		{
			glBindTexture(GL_TEXTURE_2D, t->id);
			char s[256];
			sprintf(s, "%s.pvr", t->name);
			t->glId=loadTexture(s);

			if(!t->tilesX) t->tilesX=1;
			if(!t->tilesY) t->tilesY=1;

//	glUniform4f(glGetUniformLocation(program, "tscale"), 1.0f/gt->tilesX, 1.0f/gt->tilesY, 0, 0);
//	glUniform4f(glGetUniformLocation(program, "tpos"), (t%gt->tilesX)/(float)gt->tilesX, (t/gt->tilesX%gt->tilesY)/(float)gt->tilesY, 0, 0);
			for(int i=0;i<16;i++) t->matrix[i]=0;
			t->matrix[0]=1.0f/t->tilesX;
			t->matrix[4*1+1]=1.0f/t->tilesY;
			t->matrix[4*2+2]=1.0f;
			t->matrix[4*3+3]=1.0f;

			//t->matrix[4*3+0]=(id%t->tilesX)/(float)t->tilesX;
			//t->matrix[4*3+1]=(id/t->tilesX%t->tilesY)/(float)t->tilesY;

			return;
		}
		t++;
	}
}

GLES2Renderer::GLES2Renderer()
{
	previousMode=-1;

	for(int i=0;i<TexEndOfList;i++) load((TextureID)i);

    pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(pixelShader, 1, (const char**)&glslPixel, NULL);
    //glShaderSource(pixelShader, 1, (const char**)&pszFragShader, NULL);//glslPixel
    glCompileShader(pixelShader);
    
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char**)&glslVertex, NULL);
    glCompileShader(vertexShader);
    
    program = glCreateProgram();
    
    // Attach the fragment and vertex shaders to it
    glAttachShader(program, pixelShader);
    glAttachShader(program, vertexShader);
    
    glBindAttribLocation(program, 0, "vert");
    //glBindAttribLocation(program, 1, "uv");
    
    glLinkProgram(program);
    glUseProgram(program);
    
    glUniform1i(glGetUniformLocation(program, "texture"), 0);
    glUniformMatrix4fv(glGetUniformLocation(program, "m_projection"), 1, GL_FALSE, matrixProjection);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glBufferData(GL_ARRAY_BUFFER, 4 * vboStride, vboVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vboStride, 0);
    
    GLint bLinked;
    glGetShaderiv(pixelShader, GL_COMPILE_STATUS, &bLinked);
    
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &bLinked);
    
    glGetProgramiv(program, GL_LINK_STATUS, &bLinked);
    
    
    angleAlphaLocation = glGetUniformLocation(program, "angle_alpha");
    sizeTScaleLocation = glGetUniformLocation(program, "size_tscale");
    tposPositionLocation = glGetUniformLocation(program, "tpos_position");
}

GLES2Renderer::~GLES2Renderer()
{
}


void GLES2Renderer::renderBackground( unsigned int index )
{
    glClearColor(0,0,0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable( GL_DEPTH_TEST );
    glEnable(GL_BLEND);
}

void GLES2Renderer::effectNotify( eTILEGAME_EFFECT effect, unsigned int arg1, unsigned int arg2 )
{
//	printf("effectNotify %x\n", effect);
	switch(effect)
	{
	default:
		break;
	}
}


void GLES2Renderer::renderTile( int x, int y, 
							   int w, int h, 
							   int angle,
							   int mode,
							   unsigned int tileIndex, unsigned int arg )
{
	TextureID texture=(TextureID)((tileIndex>>16)&0xff);
	//if(texture!=TexBackground) return;
	GameTexture *gt=bind(texture);
	if(!gt) return;

	if(mode!=previousMode)
	{
		if(mode==0)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		previousMode=mode;
	}

	w>>=1;
	h>>=1;

    int t=(tileIndex&65535);
    
    glUniform2f(angleAlphaLocation, angle,1-((tileIndex>>24)&0xff)/255.0f);
    glUniform4f(sizeTScaleLocation, w, h, 1.0f/gt->tilesX, 1.0f/gt->tilesY);
    glUniform4f(tposPositionLocation,
                (t%gt->tilesX)/(float)gt->tilesX, (t/gt->tilesX%gt->tilesY)/(float)gt->tilesY,
                x+w, y+h );
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


void GLES2Renderer::renderForeground( unsigned int index )
{
	return;
}


int GLES2Renderer::run( int fixedFrameTime16Bit )
{
	return 1;
}

