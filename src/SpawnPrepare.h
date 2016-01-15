
#pragma once

#include <atomic>

class cWorld;



/** Generates and lights the spawn area of the world. Runs as a separate thread. */
class cSpawnPrepare
{

public:
	cSpawnPrepare(cWorld & a_World, int a_PrepareDistance, std::function<void()> a_PreparationCompletedCallback);

	static void PrepareChunks(cWorld & a_World, int a_PrepareDistance, std::function<void()> a_PreparationCompletedCallback = {});

protected:
	cWorld & m_World;
	const int m_TotalChunks;

	/** Total number of chunks already finished preparing. Preparation finishes when this number reaches m_MaxIdx. */
	int m_NumPrepared;

	/** The timestamp of the last progress report emitted. */
	std::chrono::steady_clock::time_point m_LastReportTime;

	/** Number of chunks prepared when the last progress report was emitted. */
	int m_LastReportChunkCount;

	std::function<void()> m_PreparationCompletedCallback;

	void PreparedChunkCallback(int a_ChunkX, int a_ChunkZ);

	friend class cSpawnPrepareCallback;

};

