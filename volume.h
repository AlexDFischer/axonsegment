#include "project.h"
#include "tiffio.h"

struct Volume
{
  int width, height, depth, pixelFormat, bytesPerPixel;
  unsigned char *data;
};

typedef struct Volume Volume;

int getIndex(Volume *volume, int x, int y, int z);

unsigned long getIntensity(Volume *volume, int x, int y, int z);

unsigned long getIntensity(Volume *volume, int index);

void setIntensity(Volume *volume, int x, int y, int z, unsigned long intensity);

void setIntensity(Volume *volume, int index, unsigned long intensity);

void mallocVolume(Volume *volume);

unsigned long maxIntensity(Volume *volume);

unsigned long minIntensity(Volume *volume);

void printVolume(Volume *volume);

int readRaw(Volume *volume, char *fileName);

int writeRaw(Volume *volume, char *fileName);

int writeTiff(Volume *volume, char *dirName);
