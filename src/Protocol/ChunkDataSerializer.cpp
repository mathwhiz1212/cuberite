
// ChunkDataSerializer.cpp

// Implements the cChunkDataSerializer class representing the object that can:
//  - serialize chunk data to different protocol versions
//  - cache such serialized data for multiple clients

#include "Globals.h"
#include "ChunkDataSerializer.h"
#include "zlib/zlib.h"
#include "ByteBuffer.h"




cChunkDataSerializer::cChunkDataSerializer(
	const cChunkDef::BlockTypes   & a_BlockTypes,
	const cChunkDef::BlockNibbles & a_BlockMetas,
	const cChunkDef::BlockNibbles & a_BlockLight,
	const cChunkDef::BlockNibbles & a_BlockSkyLight,
	const unsigned char *           a_BiomeData
) :
	m_BlockTypes(a_BlockTypes),
	m_BlockMetas(a_BlockMetas),
	m_BlockLight(a_BlockLight),
	m_BlockSkyLight(a_BlockSkyLight),
	m_BiomeData(a_BiomeData)
{
}




const AString & cChunkDataSerializer::Serialize(int a_Version, int a_ChunkX, int a_ChunkZ)
{
	Serializations::const_iterator itr = m_Serializations.find(a_Version);
	if (itr != m_Serializations.end())
	{
		return itr->second;
	}
	
	AString data;
	switch (a_Version)
	{
		case RELEASE_1_2_5: Serialize29(data); break;
		case RELEASE_1_3_2: Serialize39(data); break;
		case RELEASE_1_8_0: Serialize80(data, a_ChunkX, a_ChunkZ); break;
		// TODO: Other protocol versions may serialize the data differently; implement here
		
		default:
		{
			LOGERROR("cChunkDataSerializer::Serialize(): Unknown version: %d", a_Version);
			ASSERT(!"Unknown chunk data serialization version");
			break;
		}
	}
	m_Serializations[a_Version] = data;
	return m_Serializations[a_Version];
}





void cChunkDataSerializer::Serialize29(AString & a_Data)
{
	// TODO: Do not copy data and then compress it; rather, compress partial blocks of data (zlib *can* stream)

	const int BiomeDataSize    = cChunkDef::Width * cChunkDef::Width;
	const int MetadataOffset   = sizeof(m_BlockTypes);
	const int BlockLightOffset = MetadataOffset   + sizeof(m_BlockMetas);
	const int SkyLightOffset   = BlockLightOffset + sizeof(m_BlockLight);
	const int BiomeOffset      = SkyLightOffset   + sizeof(m_BlockSkyLight);
	const int DataSize         = BiomeOffset      + BiomeDataSize;
	
	// Temporary buffer for the composed data:
	char AllData [DataSize];

	memcpy(AllData, m_BlockTypes, sizeof(m_BlockTypes));
	memcpy(AllData + MetadataOffset,   m_BlockMetas,    sizeof(m_BlockMetas));
	memcpy(AllData + BlockLightOffset, m_BlockLight,    sizeof(m_BlockLight));
	memcpy(AllData + SkyLightOffset,   m_BlockSkyLight, sizeof(m_BlockSkyLight));
	memcpy(AllData + BiomeOffset,      m_BiomeData,     BiomeDataSize);

	// Compress the data:
	// In order not to use allocation, use a fixed-size buffer, with the size
	// that uses the same calculation as compressBound():
	const uLongf CompressedMaxSize = DataSize + (DataSize >> 12) + (DataSize >> 14) + (DataSize >> 25) + 16;
	char CompressedBlockData[CompressedMaxSize];

	uLongf CompressedSize = compressBound(DataSize);
	
	// Run-time check that our compile-time guess about CompressedMaxSize was enough:
	ASSERT(CompressedSize <= CompressedMaxSize);
	
	compress2((Bytef*)CompressedBlockData, &CompressedSize, (const Bytef*)AllData, sizeof(AllData), Z_DEFAULT_COMPRESSION);

	// Now put all those data into a_Data:
	
	// "Ground-up continuous", or rather, "biome data present" flag:
	a_Data.push_back('\x01');
	
	// Two bitmaps; we're aways sending the full chunk with no additional data, so the bitmaps are 0xffff and 0, respectively
	// Also, no endian flipping is needed because of the const values
	unsigned short BitMap1 = 0xffff;
	unsigned short BitMap2 = 0;
	a_Data.append((const char *)&BitMap1, sizeof(short));
	a_Data.append((const char *)&BitMap2, sizeof(short));
	
	UInt32 CompressedSizeBE = htonl((UInt32)CompressedSize);
	a_Data.append((const char *)&CompressedSizeBE, sizeof(CompressedSizeBE));
	
	Int32 UnusedInt32 = 0;
	a_Data.append((const char *)&UnusedInt32,      sizeof(UnusedInt32));
	
	a_Data.append(CompressedBlockData, CompressedSize);
}





void cChunkDataSerializer::Serialize39(AString & a_Data)
{
	// TODO: Do not copy data and then compress it; rather, compress partial blocks of data (zlib *can* stream)

	const int BiomeDataSize    = cChunkDef::Width * cChunkDef::Width;
	const int MetadataOffset   = sizeof(m_BlockTypes);
	const int BlockLightOffset = MetadataOffset   + sizeof(m_BlockMetas);
	const int SkyLightOffset   = BlockLightOffset + sizeof(m_BlockLight);
	const int BiomeOffset      = SkyLightOffset   + sizeof(m_BlockSkyLight);
	const int DataSize         = BiomeOffset      + BiomeDataSize;
	
	// Temporary buffer for the composed data:
	char AllData [DataSize];

	memcpy(AllData, m_BlockTypes, sizeof(m_BlockTypes));
	memcpy(AllData + MetadataOffset,   m_BlockMetas,    sizeof(m_BlockMetas));
	memcpy(AllData + BlockLightOffset, m_BlockLight,    sizeof(m_BlockLight));
	memcpy(AllData + SkyLightOffset,   m_BlockSkyLight, sizeof(m_BlockSkyLight));
	memcpy(AllData + BiomeOffset,      m_BiomeData,     BiomeDataSize);

	// Compress the data:
	// In order not to use allocation, use a fixed-size buffer, with the size
	// that uses the same calculation as compressBound():
	const uLongf CompressedMaxSize = DataSize + (DataSize >> 12) + (DataSize >> 14) + (DataSize >> 25) + 16;
	char CompressedBlockData[CompressedMaxSize];

	uLongf CompressedSize = compressBound(DataSize);
	
	// Run-time check that our compile-time guess about CompressedMaxSize was enough:
	ASSERT(CompressedSize <= CompressedMaxSize);
	
	compress2((Bytef*)CompressedBlockData, &CompressedSize, (const Bytef*)AllData, sizeof(AllData), Z_DEFAULT_COMPRESSION);

	// Now put all those data into a_Data:
	
	// "Ground-up continuous", or rather, "biome data present" flag:
	a_Data.push_back('\x01');
	
	// Two bitmaps; we're aways sending the full chunk with no additional data, so the bitmaps are 0xffff and 0, respectively
	// Also, no endian flipping is needed because of the const values
	unsigned short BitMap1 = 0xffff;
	unsigned short BitMap2 = 0;
	a_Data.append((const char *)&BitMap1, sizeof(short));
	a_Data.append((const char *)&BitMap2, sizeof(short));
	
	UInt32 CompressedSizeBE = htonl((UInt32)CompressedSize);
	a_Data.append((const char *)&CompressedSizeBE, sizeof(CompressedSizeBE));
	
	// Unlike 29, 39 doesn't have the "unused" int
	
	a_Data.append(CompressedBlockData, CompressedSize);
}





void cChunkDataSerializer::Serialize80(AString & a_Data, int a_ChunkX, int a_ChunkZ)
{
	// TODO: Do not copy data and then compress it; rather, compress partial blocks of data (zlib *can* stream)

	// Blocktypes converter (1.8 included the meta into the blocktype):
	unsigned char Blocks[cChunkDef::NumBlocks * 2];
	size_t LastOffset = 0;
	for (size_t Index = 0; Index < cChunkDef::NumBlocks; Index++)
	{
		BLOCKTYPE BlockType = m_BlockTypes[Index] & 0xFF;
		NIBBLETYPE BlockMeta = m_BlockMetas[Index / 2] >> ((Index & 1) * 4) & 0x0f;
		Blocks[LastOffset] = (BlockType << 4) | ((unsigned char)BlockMeta);
		Blocks[LastOffset + 1] = (unsigned char)BlockType >> 4;
		LastOffset += 2;
	}

	const int BiomeDataSize    = cChunkDef::Width * cChunkDef::Width;
	const int BlockLightOffset = sizeof(Blocks);
	const int SkyLightOffset   = BlockLightOffset + sizeof(m_BlockLight);
	const int BiomeOffset      = SkyLightOffset   + sizeof(m_BlockSkyLight);
	const int DataSize         = BiomeOffset      + BiomeDataSize;

	// Temporary buffer for the composed data:
	char AllData [DataSize];
	memcpy(AllData, Blocks, sizeof(Blocks));
	memcpy(AllData + BlockLightOffset, m_BlockLight,    sizeof(m_BlockLight));
	memcpy(AllData + SkyLightOffset,   m_BlockSkyLight, sizeof(m_BlockSkyLight));
	memcpy(AllData + BiomeOffset,      m_BiomeData,     BiomeDataSize);

	cByteBuffer Packet(512 KiB);
	Packet.WriteVarInt(0x21);  // Packet id (Chunk Data packet)
	Packet.WriteBEInt(a_ChunkX);
	Packet.WriteBEInt(a_ChunkZ);
	Packet.WriteBool(true);  // "Ground-up continuous", or rather, "biome data present" flag
	Packet.WriteBEShort(0xffff);  // We're aways sending the full chunk with no additional data, so the bitmap is 0xffff
	Packet.WriteVarInt(DataSize);  // Chunk size
	Packet.WriteBuf(AllData, DataSize);  // Chunk data

	AString PacketData;
	Packet.ReadAll(PacketData);
	Packet.CommitRead();

	cByteBuffer NumberBuffer(20);
	if (PacketData.size() >= 256)
	{
		AString PostData;
		NumberBuffer.WriteVarInt(PacketData.size());
		NumberBuffer.ReadAll(PostData);
		NumberBuffer.CommitRead();

		// Compress the data:
		const uLongf CompressedMaxSize = 200000;
		char CompressedData[CompressedMaxSize];

		uLongf CompressedSize = compressBound(PacketData.size());
		// Run-time check that our compile-time guess about CompressedMaxSize was enough:
		ASSERT(CompressedSize <= CompressedMaxSize);
		compress2((Bytef*)CompressedData, &CompressedSize, (const Bytef*)PacketData.data(), PacketData.size(), Z_DEFAULT_COMPRESSION);

		NumberBuffer.WriteVarInt(CompressedSize + PostData.size());
		NumberBuffer.WriteVarInt(PacketData.size());
		NumberBuffer.ReadAll(PostData);
		NumberBuffer.CommitRead();

		a_Data.clear();
		a_Data.resize(PostData.size() + CompressedSize);
		a_Data.append(PostData.data(), PostData.size());
		a_Data.append(CompressedData, CompressedSize);
	}
	else
	{
		AString PostData;
		NumberBuffer.WriteVarInt(Packet.GetUsedSpace() + 1);
		NumberBuffer.WriteVarInt(0);
		NumberBuffer.ReadAll(PostData);
		NumberBuffer.CommitRead();

		a_Data.clear();
		a_Data.resize(PostData.size() + PacketData.size());
		a_Data.append(PostData.data(), PostData.size());
		a_Data.append(PacketData.data(), PacketData.size());
	}
}




