
#ifndef _FBXEXPORTER__H_
#define _FBXEXPORTER__H_

#include <fstream>
#include <stdint.h>

namespace fbx
{

	//////////////////////
	// some forward declaration
	class FBXDocument;
	class FBXNode;
	class FBXProperty;

#define EXPORTER_BUFFER_SIZE	204800 // 200 kb	 // 1 << 16

	///////////////////////////////////////////////////////////////////////////////////
	// Exporter class

	class Exporter;

	//////////////////////////////////////////////////////////////
	// ExporterNode
	struct ExporterNode
	{
	public:
		// a constructor
		ExporterNode(Exporter *pExporter, FBXNode *pNode, std::ostream* stream, int version)
			: mExporter(pExporter)
			, mNode(pNode)
			, m_Stream(stream)
			, m_Version(version)
		{}

		virtual uint32_t Write(uint32_t start_offset, bool is_last) {
			return 0;
		}
		virtual uint32_t WriteChildren(uint32_t start_offset, bool is_last) {
			return 0;
		}

	protected:
		Exporter *mExporter;
		FBXNode	*mNode;
		std::ostream* m_Stream{ nullptr };
		int m_Version{ 0 };
	};

	//////////////////////////////////////////////////////////////
	// ExporterNodeBinary
	struct ExporterNodeBinary : public ExporterNode
	{
	public:
		//! a constructor
		ExporterNodeBinary(Exporter *pExporter, FBXNode *pNode, std::ostream* stream, int version)
			: ExporterNode(pExporter, pNode, stream, version)
		{}

		//
		virtual uint32_t Write(uint32_t start_offset, bool is_last) override;
		virtual uint32_t WriteChildren(uint32_t start_offset, bool is_last) override;

	protected:

		static uint32_t getBytes(FBXNode *pNode, bool is_last);
		static uint32_t getBytesChildren(FBXNode *pNode, bool is_last);
		static uint32_t getBytesProperties(FBXNode *pNode);
	};

	//////////////////////////////////////////////////////////////////
	// ExporterNodeAscii
	struct ExporterNodeAscii : public ExporterNode
	{
	public:
		// a constructor
		ExporterNodeAscii(Exporter *pExporter, FBXNode *pNode, std::ostream* stream, int version)
			: ExporterNode(pExporter, pNode, stream, version)
		{}

		//
		virtual uint32_t Write(uint32_t start_offset, bool is_last) override;
		virtual uint32_t WriteChildren(uint32_t start_offset, bool is_last) override;
	};

	struct ExporterProperty
	{

	};

	//////////////////////////////////////////////////////////////////////////////////
	// Exporter
	class Exporter
	{
	public:
		//! a constructor
		Exporter();
		//
		Exporter(bool isBinary);

		//! a destructor
		~Exporter();

		//
		bool Initialize(const char *filename, bool isBinary);

		//
		bool Export(const FBXDocument &document);

		bool Destroy();
		/*
		std::ofstream &GetStream()
		{
			return mFile;
		}
		*/
		const char* GetStreamBuffer() const { return m_StreamBuffer; }

	protected:

		char			*m_StreamBuffer;
		std::ofstream	m_File;

		std::string		m_Filename;
		bool			m_IsBinary;

		bool WriteBinary(const FBXDocument &document, std::ostream* stream);
		bool WriteASCII(const FBXDocument &document, std::ostream& stream);
	};


};

#endif