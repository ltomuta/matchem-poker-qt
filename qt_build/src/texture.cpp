#include "texture.h"

#if defined(_WIN32) 
#include <crtdbg.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#include <string.h>
#include <assert.h>

#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _ASSERT
#define _ASSERT(x)
#endif

#define MAX(x,y) ((x)>(y) ? (x) : (y))


const void *lockFile(const char *filename, MemoryChunk *chunk)
{
	_ASSERT(filename);
	_ASSERT(chunk);

	memset(chunk, 0, sizeof(MemoryChunk));

#if defined(_WIN32) && !defined(__SYMBIAN32__)
	wchar_t wideFileName[MAX_PATH];
	mbstowcs(wideFileName, filename, ARRAYSIZE(wideFileName));

	HANDLE fd=CreateFile(wideFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fd==INVALID_HANDLE_VALUE) return 0;

	chunk->fd=fd;

	LARGE_INTEGER size;
	GetFileSizeEx(fd, &size);

	chunk->size=size.LowPart;

	chunk->mapping=CreateFileMapping(fd, NULL, PAGE_READONLY, size.HighPart, size.LowPart, NULL);
	chunk->ptr=MapViewOfFile(chunk->mapping, FILE_MAP_READ, 0, 0, 0);
	return chunk->ptr;
#else
//#error TODO
	struct stat s;
	
	int fd=open(filename, O_RDONLY);
	if(fd==-1) return NULL;
	
	if(fstat(fd, &s)!=0 || s.st_size==0) 
	{
		close(fd);
		return NULL;
	}
	
	void *ptr=mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
	
	if(ptr!=(void*)-1)
	{
		chunk->ptr=ptr;
		chunk->fd=fd;
		chunk->size=s.st_size;
		return chunk->ptr;
	}
	else
	{
		ptr=malloc(s.st_size);
		if(read(fd, ptr, s.st_size)!=-1)
		{
			chunk->ptr=ptr;
			chunk->fd=fd;
			chunk->size=s.st_size;
			return chunk->ptr;
		}
	}
#endif
	return NULL;
}

void unlockFile(MemoryChunk *chunk)
{
	_ASSERT(chunk);

#if defined(_WIN32) && !defined(__SYMBIAN32__)
	CloseHandle(chunk->mapping);
	CloseHandle(chunk->fd);
#else
//#error TODO
	munmap((void*)chunk->ptr, chunk->size);
	close(chunk->fd);
#endif
	
	memset(chunk, 0, sizeof(MemoryChunk));
}

void pvrTexImage2D(const PVR_Texture_Header *pvr)
{
	const unsigned char *tex=((const unsigned char *)pvr)+pvr->dwHeaderSize;

	GLuint textureFormat;
	const bool alpha=(pvr->dwAlphaBitMask!=0);

	for(unsigned int level=0;level<=pvr->dwMipMapCount;level++)
	{
		GLsizei size;
		GLsizei width=pvr->dwWidth>>level;
		GLsizei height=pvr->dwHeight>>level;

		switch(pvr->dwpfFlags & PVRTEX_PIXELTYPE)
		{
		case OGL_PVRTC2:
			textureFormat = alpha ? GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
			size = ( MAX(width, 16) * MAX(height, 8) * 2 + 7) / 8;
			glCompressedTexImage2D(GL_TEXTURE_2D, level, textureFormat, width, height, 0, size, tex);
			break;
		case OGL_PVRTC4:
			textureFormat = alpha ? GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
			size = ( MAX(width, 8) * MAX(height, 8) * 4 + 7) / 8;
			glCompressedTexImage2D(GL_TEXTURE_2D, level, textureFormat, width, height, 0, size, tex);
			break;
				
		case OGL_RGBA_4444:
				size=width*height*2;
			glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, tex);
			break;
				
			case OGL_RGBA_8888:
				size=width*height*4;
				glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
				break;
				
		default:
				
			break;
		}

		tex += size;
	}
}

GLuint loadTexture(const PVR_Texture_Header *pvr)
{
	GLuint texName;
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	pvrTexImage2D(pvr);

	return texName;
}


GLuint loadTexture(const char *filename)
{
    printf("loading '%s'\n", filename);
    
	MemoryChunk m;
	const void *ptr=lockFile(filename, &m);

	if(!ptr) return 0;

	GLuint texName;
	unsigned int off=0;

	while(off<m.size)
	{
		PVR_Texture_Header *pvr=(PVR_Texture_Header*)((const char*)m.ptr+off);
		GLuint tn=loadTexture(pvr);
		if(off==0) texName=tn;
		off+=pvr->dwHeaderSize;
		off+=pvr->dwTextureDataSize;
	}

	unlockFile(&m);
	return texName;
}

