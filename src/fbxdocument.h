#ifndef FBXDOCUMENT_H
#define FBXDOCUMENT_H

#include "fbxnode.h"
#include "fbxobject.h"
#include <unordered_map>

namespace fbx {

	class Scene;

	struct ObjectPair
	{
		const FBXNode* element; // this is a raw file data element
		FBXObject* object;		// this is unpacked functional element
	};

	/// <summary>
	/// a class that holds unpacked fbx nodes and connections
	///  the class is used as a source data to retrive or store fbx file
	///  by using FbxImporter, FbxExporter
	/// </summary>
	class FBXDocument
	{
	public:
		FBXDocument();
		~FBXDocument();

	public:
		// this is a raw document operations

		// create some basic document nodes
		void CreateHeader();
		void CreateGlobalSettings();
		void CreateDocuments();
		void CreateReferences();
		void CreateDefinitions();

		void UpdateHeader();
		void UpdateGlobalSettings(fbx::i64 startTime, fbx::i64 stopTime, double fps);
		void UpdateDefinitions();
		void UpdateAnimationTakeTime(fbx::i64 startTime, fbx::i64 stopTime);


		FBXNode* FindNode(const char *name, const FBXNode *parent) const;

		FBXNode		&GetRoot() const { return (FBXNode&) m_root; }
		FBXNode		*GetRootPtr() const { return (FBXNode*) &m_root; }

		std::uint32_t GetVersion() const;
		void Print();

		int64_t GenerateUid() { return ++last_uid; }

		bool ParseConnections();
		bool ParseObjects();


	protected:
		std::uint32_t version;
		int64_t last_uid;

		FBXNode			m_root;

		std::unordered_map<i64, ObjectPair> m_objectMap;
		std::vector<Connection> m_connections;
	
		void PopulateObjectMap(FBXNode& root);
		void PopulateConnections(const FBXNode& root);

		friend class Scene;
	};

} // namespace fbx

#endif // FBXDOCUMENT_H
