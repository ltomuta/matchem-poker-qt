#pragma once

#include <math.h>

#if defined(_WIN32) && !defined(__SYMBIAN32__)
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#endif


#ifndef M_PI
#define M_PI 3.1415926535897931f
#endif

GLuint loadTexture(const char *filename);

struct MemoryChunk
{
	const void *ptr;
	unsigned int size;
#if defined(_WIN32) && !defined(__SYMBIAN32__)
	HANDLE fd;
	HANDLE mapping;
#else
	unsigned int fd;
#endif
};

const void *lockFile(const char *name, MemoryChunk *chunk);
void unlockFile(MemoryChunk *chunk);

#if 1

/** from powervr sdk */
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                       0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                       0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                      0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                      0x8C03

struct PVR_Texture_Header
{
unsigned int dwHeaderSize;            /*!< size of the structure */
unsigned int dwHeight;                /*!< height of surface to be created */
unsigned int dwWidth;                /*!< width of input surface */
unsigned int dwMipMapCount;            /*!< number of mip-map levels requested */
unsigned int dwpfFlags;                /*!< pixel format flags */
unsigned int dwTextureDataSize;        /*!< Total size in bytes */
unsigned int dwBitCount;            /*!< number of bits per pixel  */
unsigned int dwRBitMask;            /*!< mask for red bit */
unsigned int dwGBitMask;            /*!< mask for green bits */
unsigned int dwBBitMask;            /*!< mask for blue bits */
unsigned int dwAlphaBitMask;        /*!< mask for alpha channel */
unsigned int dwPVR;                    /*!< magic number identifying pvr file */
unsigned int dwNumSurfs;            /*!< the number of surfaces present in the pvr */
} ;


// from PVRTexLibGlobals.h
const unsigned int PVRTEX_PIXELTYPE = 0xff;
enum 
{
	// OpenGL version of pixel types
		OGL_RGBA_4444= 0x10,
		OGL_RGBA_5551,
		OGL_RGBA_8888,
		OGL_RGB_565,
		OGL_RGB_555,
		OGL_RGB_888,
		OGL_I_8,
		OGL_AI_88,
		OGL_PVRTC2,
		OGL_PVRTC4,
};

void pvrTexImage2D(const PVR_Texture_Header *pvr);

#endif

