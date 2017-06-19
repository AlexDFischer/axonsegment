#include "project.h"

int nLinkCapacity(unsigned long intensity1, unsigned long intensity2)
{
  long intensityDiff = (signed) (intensity1 - intensity2);
  int result = (int) round(75.0 * exp(-1.0 * intensityDiff * intensityDiff / (.5 * MAX_INTENSITY * MAX_INTENSITY)));
  return result;
}

/** cost of assigning voxel to background */
int sourceCapacity(unsigned long intensity)
{
	/** 0 means definitely background, 1 means definitely foreground */
	double score = (intensity + 1.0) / (MAX_INTENSITY + 2.0);
	int result = (int) (-150.0 * log(1 - score));
  return result;
}

/** cost of assigning voxel to foreground */
int sinkCapacity(unsigned long intensity)
{
	/** 0 means definitely background, 1 means definitely foreground */
  intensity = min(2 * intensity, (MAX_INTENSITY + intensity) / 2);
	double score = (double) (intensity + 1.0) / (MAX_INTENSITY + 2.0);
	int result = (int) (-30.0 * log(score));
  return result;
}

VolumeGraph *buildGraphFromRaw(char *fileName)
{

  int len = strlen(fileName);
  char fileNameExt[len + 5];
  strcpy(fileNameExt, fileName);
  strcpy(fileNameExt + len, ".txt");
  FILE *f = fopen(fileNameExt, "r");
  if (f == NULL)
  {
    fprintf(stderr, "%s: unable to open file %s: %s\n", programName, fileNameExt, strerror(errno));
    return NULL;
  }
  if (fscanf(f, "%dx%dx%d\n", &width, &height, &depth) != 3)
  {
    fprintf(stderr, "%s: invalid first line of %s\n", programName, fileNameExt);
    return NULL;
  }
  if (fscanf(f, "%d\n", &bytesPerPixel) != 1)
  {
    fprintf(stderr, "%s: invalid second line of %s\n", programName, fileNameExt);
    return NULL;
  }
  int scaleX, scaleY, scaleZ;
  if (fscanf(f, "scale: %d:%d:%d", &scaleX, &scaleY, &scaleZ) != 3)
  {
    fprintf(stderr, "%s: invalid third line of %s\n", programName, fileNameExt);
    return NULL;
  }
  if (fclose(f))
  {
    fprintf(stderr, "%s: unable to close file %s: %s\n", programName, fileNameExt, strerror(errno));
    return NULL;
  }
  printf("dealing with a %dx%dx%d volume with %d bytes per pixel\n", width, height, depth, bytesPerPixel);
  long dimensions[] = {width, height, depth};
  VolumeGraph *g = new VolumeGraph(dimensions);
  // only maintain 2 slices in memory at one time
  unsigned char *temp, *slice0 = (unsigned char *) malloc(width * height * bytesPerPixel), *slice1 = (unsigned char *) malloc(width * height * bytesPerPixel);
  strcpy(fileNameExt + len, ".raw");
  f = fopen(fileNameExt, "r");
  int x = 0, y = 0, z = 0; // z is z-value of slice 0
  unsigned long intensity1, intensity2;
  fread(slice1, bytesPerPixel, width * height, f);
  if (ferror(f))
  {
    fprintf(stderr, "%s: error reading from file %s: %s\n", programName, fileNameExt, strerror(ferror(f)));
    return NULL;
  }
  unsigned long maxIntensity = 0, maxIX, maxIY, maxIZ;
  unsigned long minIntensity = INT_MAX, minIX, minIY, minIZ;
  for (z = 0; z < depth; z++)
  {
    temp = slice0;
    slice0 = slice1;
    slice1 = temp;
    if (z < depth - 1)
    {
      fread(slice1, bytesPerPixel, width * height, f);
      if (ferror(f))
      {
        fprintf(stderr, "%s: error reading from file %s: %s\n", programName, fileNameExt, strerror(ferror(f)));
        return NULL;
      }
    }
    for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
      {
        switch (bytesPerPixel)
        {
          case 1:
            intensity1 = ((uint8_t  *) slice0)[y * width + x];
            break;
          case 2:
            intensity1 = ((uint16_t *) slice0)[y * width + x];
            break;
          case 4:
            intensity1 = ((uint32_t *) slice0)[y * width + x];
            break;
          case 8:
            intensity1 = ((uint64_t *) slice0)[y * width + x];
            break;
        }
        if (intensity1 > maxIntensity)
        {
          maxIX = x;
          maxIY = y;
          maxIZ = z;
          maxIntensity = intensity1;
        }
        if (intensity1 < minIntensity)
        {
          minIX = x;
          minIY = y;
          minIZ = z;
          minIntensity = intensity1;
        }
        g->add_terminal_weights(z * width * height + y * width + x, sourceCapacity(intensity1), sinkCapacity(intensity1));
        int capacity;
        if (x < width - 1)
        {
          switch (bytesPerPixel)
          {
            case 1:
              intensity2 = ((uint8_t  *) slice0)[y * width + x + 1];
              break;
            case 2:
              intensity2 = ((uint16_t *) slice0)[y * width + x + 1];
              break;
            case 4:
              intensity2 = ((uint32_t *) slice0)[y * width + x + 1];
              break;
            case 8:
              intensity2 = ((uint64_t *) slice0)[y * width + x + 1];
              break;
          }
          capacity = nLinkCapacity(intensity1, intensity2);
          g->add_edge(z * width * height + y * width + x, z * width * height + y * width + x + 1, capacity, capacity);
        }
        if (y < height - 1)
        {
          switch (bytesPerPixel)
          {
            case 1:
              intensity2 = ((uint8_t  *) slice0)[(y + 1) * width + x];
              break;
            case 2:
              intensity2 = ((uint16_t *) slice0)[(y + 1) * width + x];
              break;
            case 4:
              intensity2 = ((uint32_t *) slice0)[(y + 1) * width + x];
              break;
            case 8:
              intensity2 = ((uint64_t *) slice0)[(y + 1) * width + x];
              break;
          }
          capacity = nLinkCapacity(intensity1, intensity2);
          g->add_edge(z * width * height + y * width + x, z * width * height + (y + 1) * width + x, capacity, capacity);
        }
        if (z < depth - 1)
        {
          switch (bytesPerPixel)
          {
            case 1:
              intensity2 = ((uint8_t  *) slice1)[y * width + x];
              break;
            case 2:
              intensity2 = ((uint16_t *) slice1)[y * width + x];
              break;
            case 4:
              intensity2 = ((uint32_t *) slice1)[y * width + x];
              break;
            case 8:
              intensity2 = ((uint64_t *) slice1)[y * width + x];
              break;
          }
          capacity = nLinkCapacity(intensity1, intensity2);
          g->add_edge(z * width * height + y * width + x, (z + 1) * width * height + y * width + x, capacity, capacity);
        }
      }
    }
    cout << "\rfinished slice " << z + 1 << " out of " << depth << flush;
  }
  cout << endl;
  fclose(f);
  free(slice0);
  free(slice1);
  return g;
}
