
#include "Globals.h"

#include "SpawnPrepare.h"
#include "World.h"





class cSpawnPrepareCallback :
	public cChunkCoordCallback
{
public:
	cSpawnPrepareCallback(std::shared_ptr<cSpawnPrepare> a_SpawnPrepare) :
		m_SpawnPrepare(std::move(a_SpawnPrepare))
	{
	}
protected:

	std::shared_ptr<cSpawnPrepare> m_SpawnPrepare;

	virtual void Call(int a_ChunkX, int a_ChunkZ, bool a_IsSuccess) override
	{
		m_SpawnPrepare->PreparedChunkCallback(a_ChunkX, a_ChunkZ);
	}
};





cSpawnPrepare::cSpawnPrepare(cWorld & a_World, int a_PrepareDistance, std::function<void()> a_PreparationCompletedCallback):
	m_World(a_World),
	m_TotalChunks(a_PrepareDistance * a_PrepareDistance),
	m_NumPrepared(0),
	m_LastReportTime(std::chrono::steady_clock::now()),
	m_LastReportChunkCount(0),
	m_PreparationCompletedCallback(std::move(a_PreparationCompletedCallback))
{
}







void cSpawnPrepare::PrepareChunks(cWorld & a_World, int a_PrepareDistance, std::function<void()> a_PreparationCompletedCallback)
{
	auto PerformanceAnalysisObject = std::make_shared<cSpawnPrepare>(a_World, a_PrepareDistance, a_PreparationCompletedCallback);
	auto HalfPrepareDistance = (a_PrepareDistance - 1) / 2.f;
	auto NegativePrepareDistance = FloorC(-HalfPrepareDistance), PositivePrepareDistance = FloorC(HalfPrepareDistance);

	for (int ChunkX = NegativePrepareDistance; ChunkX <= PositivePrepareDistance; ++ChunkX)
	{
		for (int ChunkZ = NegativePrepareDistance; ChunkZ <= PositivePrepareDistance; ++ChunkZ)
		{
			a_World.PrepareChunk(ChunkX, ChunkZ, cpp14::make_unique<cSpawnPrepareCallback>(PerformanceAnalysisObject));
		}
	}
}





void cSpawnPrepare::PreparedChunkCallback(int a_ChunkX, int a_ChunkZ)
{
	m_NumPrepared += 1;
	if (m_NumPrepared == m_TotalChunks)
	{
		if (m_PreparationCompletedCallback)
		{
			m_PreparationCompletedCallback();
		}

		LOG("Preparing spawn (%s): completed!", m_World.GetName().c_str());
		return;
	}

	// Report progress every 1 second:
	auto Now = std::chrono::steady_clock::now();
	if (Now - m_LastReportTime > std::chrono::seconds(1))
	{
		float PercentDone = static_cast<float>(m_NumPrepared * 100) / m_TotalChunks;
		float ChunkSpeed = static_cast<float>((m_NumPrepared - m_LastReportChunkCount) * 1000) / std::chrono::duration_cast<std::chrono::milliseconds>(Now - m_LastReportTime).count();
		LOG("Preparing spawn (%s): %.02f%% (%d/%d; %.02f chunks / sec)",
			m_World.GetName().c_str(), PercentDone, m_NumPrepared, m_TotalChunks, ChunkSpeed
		);
		m_LastReportTime = Now;
		m_LastReportChunkCount = m_NumPrepared;
	}
}

