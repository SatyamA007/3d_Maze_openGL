
#include "readTextures.h"

/*Using Sam Leffler's libtiff library */
TIFFRGBAImage img;
uint32 *raster;


size_t npixels;
int imgwidth, imgheight, imgcomponents;

int hasABGR = 0;
char *imgfilename  = NULL;



int readTiffImage(char *name, GLuint *textureID){
  TIFF *tif;
  char emsg[1024];


  glBindTexture(GL_TEXTURE_2D, *textureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


  tif = TIFFOpen(name, "r");
  if (tif == NULL){
    fprintf(stderr, "tif == NULL\n");
    exit(1);
  }
  if (TIFFRGBAImageBegin(&img, tif, 0,emsg)){
    npixels = img.width*img.height;
    raster = (uint32 *)_TIFFmalloc(npixels*sizeof(uint32));
    if (raster != NULL){
      if (TIFFRGBAImageGet(&img, raster, img.width, img.height) == 0){
      TIFFError(name, emsg);
      exit(1);
      }
    }
    TIFFRGBAImageEnd(&img);
    fprintf(stderr, "Read image %s (%d x %d)\n", name, img.width, img.height);
  }
  else {
    TIFFError(name, emsg);
    exit(1);
  }
  imgwidth = img.width;
  imgheight = img.height;

  // code based upon
  // http://www.opengl.org/developers/code/mjktips/libtiff/showtiff.c
  
  /* If cannot directly display ABGR format, we need to reverse the component
     ordering in each pixel.  :-(  */
  if (!hasABGR) {
    int i;

    for (i = 0; i < npixels; i++) {
      register unsigned char *cp = (unsigned char *) &raster[i];
      int t;

      t = cp[3];
      cp[3] = cp[0];
      cp[0] = t;
      t = cp[2];
      cp[2] = cp[1];
      cp[1] = t;
    }
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               imgwidth, imgheight,0, GL_RGBA, GL_UNSIGNED_BYTE,raster);

}



int writetiff(char *filename, char *description,
      int x, int y, int width, int height, int compression){
  TIFF *file;
  GLubyte *image, *p;
  int i;

  file = TIFFOpen(filename, "w");
  if (file == NULL) {
    return 1;
  }
  image = (GLubyte *) malloc(width * height * sizeof(GLubyte) * 3);

  /* OpenGL's default 4 byte pack alignment would leave extra bytes at the
     end of each image row so that each full row contained a number of bytes
     divisible by 4.  Ie, an RGB row with 3 pixels and 8-bit componets would
     be laid out like "RGBRGBRGBxxx" where the last three "xxx" bytes exist
     just to pad the row out to 12 bytes (12 is divisible by 4). To make sure
     the rows are packed as tight as possible (no row padding), set the pack
     alignment to 1. */
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
  TIFFSetField(file, TIFFTAG_IMAGEWIDTH, (uint32) width);
  TIFFSetField(file, TIFFTAG_IMAGELENGTH, (uint32) height);
  TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(file, TIFFTAG_COMPRESSION, compression);
  TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 3);
  TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, 1);
  TIFFSetField(file, TIFFTAG_IMAGEDESCRIPTION, description);
  p = image;
  for (i = height - 1; i >= 0; i--) {
    if (TIFFWriteScanline(file, p, i, 0) < 0) {
      free(image);
      TIFFClose(file);
      return 1;
    }
    p += width * sizeof(GLubyte) * 3;
  }
  TIFFClose(file);
  return 0;
}
