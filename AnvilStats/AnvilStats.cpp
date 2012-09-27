
// AnvilStats.cpp

// Implements the main app entrypoint

#include "Globals.h"
#include "Statistics.h"
#include "Processor.h"





int main(int argc, char * argv[])
{
	if (argc < 2)
	{
		LOG("Usage: %s <method number> [<world folder>]", argv[0]);
		LOG("\nNo method number present, aborting.");
		return -1;
	}
	
	AString WorldFolder;
	if (argc > 2)
	{
		WorldFolder = argv[2];
	}
	else
	{
		WorldFolder = "." + cFile::PathSeparator;
	}
	
	cCallbackFactory * Factory = NULL;
	switch (atol(argv[1]))
	{
		case 0: Factory = new cStatisticsFactory; break;
		default:
		{
			LOG("Unknown method \"%s\", aborting.", argv[1]);
			return -2;
		}
	}
	cProcessor Processor;
	Processor.ProcessWorld(WorldFolder, *Factory);
	
	delete Factory;
	
	LOG("Done");
}




