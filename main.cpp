using namespace std;

#include <ctime>
#include "project.h"
#include "volume.h"
#include "graphUtils.h"
#include "RegionPushRelabel.h"

char *programName;
int width, height, depth, bytesPerPixel;

int main(int argc, char *argv[])
{
	clock_t startClock, buildGraphClock, maxFlowClock, writeSegmentationClock;
	startClock = clock();
	programName = argv[0];
	if (argc < 4)
	{
		cout << "useage:" << endl;
		cout << "    " << programName << " -[t/r]i input -[t/r]o output" << endl;
	}
	VolumeGraph *g;
	/*
	if (strcmp(argv[1], "-ri") == 0)
	{
		if ((g = buildGraphFromRaw(argv[2])) == NULL)
		{
			exit(1);
		}
	} else if (strcmp(argv[1], "-ti") == 0)
	{
		cout << programName << ": tiff input not yet supported" << endl;
	} else
	{
		cout << programName << ": invalid first argument " << argv[1] << endl;
		exit(0);
	}
	buildGraphClock = clock();
	*/
	// read volume from input file
	Volume *volume;
	volume = (Volume *) malloc(sizeof(Volume));
	if (strcmp(argv[1], "-ri") == 0)
	{
		if (readRaw(volume, argv[2]))
		{
			exit(1);
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
	width = volume->width;
	height = volume->height;
	depth = volume->depth;
  long dimensions[] = {volume->width, volume->height, volume->depth};
	g = new VolumeGraph(dimensions);
	for (int z = 0; z < depth; z++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int id1 = getIndex(volume, x, y, z), id2;
				unsigned long intensity1 = getIntensity(volume, id1);
				g->add_terminal_weights(id1, sourceCapacity(intensity1), sinkCapacity(intensity1));
				if (x < width - 1)
				{
					id2 = id1 + 1;
					int capacity = nLinkCapacity(intensity1, getIntensity(volume, id2));
					g->add_edge(id1, id2, capacity, capacity);
				}
				if (y < height - 1)
				{
					id2 = width + id1;
					int capacity = nLinkCapacity(intensity1, getIntensity(volume, id2));
					g->add_edge(id1, id2, capacity, capacity);
				}
				if (z < depth - 1)
				{
					id2 = width * height + id1;
					int capacity = nLinkCapacity(intensity1, getIntensity(volume, id2));
					g->add_edge(id1, id2, capacity, capacity);
				}
			}
		}
	}
	free(volume->data);
	buildGraphClock = clock();
	cout << "finished building graph: took " << (buildGraphClock - startClock) / (double) CLOCKS_PER_SEC << " seconds" << endl;
  g->compute_maxflow();
	cout << "Flow = " << g->get_flow() << endl;
	maxFlowClock = clock();
	cout << "solving maxFlow took " << (maxFlowClock - buildGraphClock) / (double) CLOCKS_PER_SEC << " seconds" << endl;
	/*
	Volume *volume = (Volume *) malloc(sizeof(Volume));
	volume->width = width;
	volume->height = height;
	volume->depth = depth;
	*/
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
	writeSegmentationClock = clock();
	cout << "numFG: " << numFG << "; numBG: " << numBG << endl;
	cout << "writing segmentation took " << (writeSegmentationClock - maxFlowClock) / (double) CLOCKS_PER_SEC << " seconds" << endl;
  delete g;
	free(volume->data);
	free(volume);
}
