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

class FBXDocument
{
public:
    FBXDocument();
	~FBXDocument();

public:
	// this is a raw document operations

	// create some basic document nodes
    void createHeader();
	void createGlobalSettings();
	void createDocuments();
	void createReferences();
	void createDefinitions();

	void UpdateHeader();
	void UpdateGlobalSettings(fbx::i64 startTime, fbx::i64 stopTime, double fps);
	void UpdateDefinitions();
	void UpdateAnimationTakeTime(fbx::i64 startTime, fbx::i64 stopTime);


	FBXNode* FindNode(const char *name, const FBXNode *parent) const;

	FBXNode		&getRoot() const
	{
		return (FBXNode&) m_root;
	}
	FBXNode		*getRootPtr() const
	{
		return (FBXNode*) &m_root;
	}

    //std::vector<FBXNode> nodes;

    std::uint32_t getVersion() const;
    void print();

	int64_t generate_uid() { return ++last_uid; }

	bool ParseConnections();
	bool ParseObjects();

public:
	

protected:
    std::uint32_t version;
	int64_t last_uid;

	FBXNode			m_root;

	std::unordered_map<i64, ObjectPair> m_objectMap;
	std::vector<Connection> m_connections;
	friend class Scene;

	void PopulateObjectMap(FBXNode& root);
	void PopulateConnections(const FBXNode& root);
};

} // namespace fbx

#endif // FBXDOCUMENT_H
