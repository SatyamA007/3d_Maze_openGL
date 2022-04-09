
#ifndef _READTEXTURE_H
#define _READTEXTURE_H


int readTiffImage(char *name, GLuint *textureID);

int writetiff(char *filename, char *description,
      int x, int y, int width, int height, int compression);
#endif //_READTEXTURE_H