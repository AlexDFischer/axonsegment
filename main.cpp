using namespace std;

#include "project.h"
#include "volume.h"
#include "RegionPushRelabel.h"

char *programName;

int nLinkCapacity(long intensityDiff, unsigned long minIntensity, unsigned long maxIntensity)
{
  return (int) round(25 * exp(-1.0 * intensityDiff * intensityDiff / (2.0 * (maxIntensity - minIntensity) * (maxIntensity - minIntensity))));
}

int sourceCapacity(long intensity, unsigned long minIntensity, unsigned long maxIntensity)
{
	/** 0 means definitely background, 1 means definitely foreground */
	double score = (double) (intensity - minIntensity + 1) / (maxIntensity - minIntensity + 2);
	return (int) (-100.0 * log(1 - score));
}

int sinkCapacity(long intensity, unsigned long minIntensity, unsigned long maxIntensity)
{
	/** 0 means definitely background, 1 means definitely foreground */
	double score = (double) (intensity - minIntensity + 1) / (maxIntensity - minIntensity + 2);
	return (int) (-100.0 * log(score));
}

int main(int argc, char *argv[])
{
	programName = argv[0];
	if (argc < 4)
	{
		cout << "useage:" << endl;
		cout << "    " << programName << " -[t/r]i input -[t/r]o output" << endl;
	}
	// read volume from input file
	Volume *volume;
	volume = (Volume *) malloc(sizeof(Volume));
	if (strcmp(argv[1], "-ri") == 0)
	{
		if (readRaw(volume, argv[2]))
		{
			return 1;
		}
	} else if (strcmp(argv[1], "-ti") == 0)
	{
		cout << programName << ": tiff input not yet supported" << endl;
	} else
	{
		cout << programName << ": invalid first argument " << argv[1] << endl;
		exit(0);
	}
	// construct graph
	unsigned long minI = minIntensity(volume);
	unsigned long maxI = maxIntensity(volume);
	cout << "minimum intensity: " << minI << "; maximum intensity: " << maxI << endl;
  long dimensions[] = {volume->width, volume->height, volume->depth};
	VolumeGraph *g = new VolumeGraph(dimensions);
	for (int z = 0; z < volume->depth; z++)
	{
		for (int y = 0; y < volume->height; y++)
		{
			for (int x = 0; x < volume->width; x++)
			{
				int id1 = getIndex(volume, x, y, z), id2;
				unsigned long intensity1 = getIntensity(volume, id1);
				g->add_terminal_weights(id1, sourceCapacity(intensity1, minI, maxI), sinkCapacity(intensity1, minI, maxI));
				if (x < volume->width - 1)
				{
					id2 = id1 + 1;
					int capacity = nLinkCapacity(intensity1 - getIntensity(volume, id2), minI, maxI);
					g->add_edge(id1, id2, capacity, capacity);
				}
			}
		}
	}
	cout << "finished building graph" << endl;
  g->compute_maxflow();
	cout << "Flow = " << g->get_flow() << endl;
	free(volume->data);
	volume->bytesPerPixel = 1;
	mallocVolume(volume);
	int numFG = 0, numBG = 0;
	for (int i = 0; i < volume->width * volume->height * volume->depth; i++)
	{

		if (g->get_segment(i) == 0)
		{
			numFG++;
			setIntensity(volume, i, 255l);
		} else
		{
			setIntensity(volume, i, 0l);
			numBG++;
		}
	}
	cout << "numFG: " << numFG << "; numBG: " << numBG << endl;
	if (strcmp(argv[3], "-ro") == 0)
	{
		writeRaw(volume, argv[4]);
	} else if (strcmp(argv[3], "-to") == 0)
	{
		writeTiff(volume, argv[4]);
	} else
	{
		cout << programName << ": invalid third argument: " << argv[3] << endl;
	}
  delete g;
	free(volume->data);
	free(volume);
}
