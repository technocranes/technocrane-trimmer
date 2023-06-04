
#include "fbximporter.h"
#include "fbxdocument.h"
#include "fbxutil.h"

using namespace fbx;

///////////////////////////////////////////////////////////////////////////
//

bool checkMagic(Reader &reader)
{
	std::string magic("Kaydara FBX Binary  ");
	for (char c : magic) {
		if (reader.readUint8() != c) return false;
	}
	if (reader.readUint8() != 0x00) return false;
	if (reader.readUint8() != 0x1A) return false;
	if (reader.readUint8() != 0x00) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////////
//

Importer::Importer()
{
	mStreamBuffer = nullptr;
}

Importer::~Importer()
{
	Destroy();
}

bool Importer::Initialize(const char *filename)
{
	bool lSuccess = true;
	
	// buffer
	if (nullptr == mStreamBuffer)
		mStreamBuffer = new char[IMPORTER_BUFFER_SIZE];

	mFile.rdbuf()->pubsetbuf(mStreamBuffer, IMPORTER_BUFFER_SIZE);

	mFile.open(filename, std::ios::in | std::ios::binary);
	
	if (false == mFile.is_open())
	{
		int err = errno;
		printf("error - %d\n", err);
		lSuccess = false;
	}
	return lSuccess;
}

bool Importer::Import(FBXDocument &document)
{
	Reader reader(&mFile);
	
	
	mFile >> std::noskipws;
	if (!checkMagic(reader))
	{
		printf("error while import a document - %s\n", "Not a FBX file");
		return false;
	}

	uint32_t version = reader.readUint32();
	uint32_t maxVersion = 7700; // we need to support 7700

	if (version > maxVersion)
	{
		printf("error while import a document - %s\n", "Unsupported FBX version");
		return false;
	}

	uint64_t start_offset = 27; // magic: 21+2, version: 4
	FBXNode &root = document.getRoot();

	while (true)
	{
		FBXNode node;
		start_offset += node.read(mFile, start_offset, version);
		if (node.isNull()) break;
		root.addChild(std::move(node));
	};
	

	return true;
}

bool Importer::Destroy()
{
	if (mFile.is_open())
		mFile.close();

	if (nullptr != mStreamBuffer)
	{
		delete[] mStreamBuffer;
		mStreamBuffer = nullptr;
	}

	return true;
}

