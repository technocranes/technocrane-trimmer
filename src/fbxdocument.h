#ifndef FBXDOCUMENT_H
#define FBXDOCUMENT_H

#include "fbxnode.h"
#include "fbxobject.h"
#include <unordered_map>

namespace fbx {

	struct Connection
	{
		enum Type
		{
			OBJECT_OBJECT,
			OBJECT_PROPERTY,
			PROPERTY_PROPERTY
		};

		Type type;
		u64 from;
		u64 to;
		// TODO: is that safe enough!
		const FBXProperty* srcProperty;
		const FBXProperty* property;
	};

	struct ObjectPair
	{
		const FBXNode* element; // this is a raw file data element
		FBXNodeView* object;		// this is unpacked functional element
	};

class FBXDocument
{
public:
    FBXDocument();
    
	// create some basic document nodes
    void createHeader();
	void createGlobalSettings();
	void createDocuments();
	void createReferences();
	void createDefinitions();

	FBXNode		*findNode(const char *name, const FBXNode *parent);

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

	bool ParseObjects();
	bool ParseConnections();

protected:
    std::uint32_t version;
	int64_t last_uid;

	FBXNode			m_root;

	std::unordered_map<uint64_t, ObjectPair> m_objectMap;
	std::vector<Connection> m_connections;

	void PopulateObjectMap(const FBXNode& root);
	void PopulateConnections(const FBXNode& root);
};

} // namespace fbx

#endif // FBXDOCUMENT_H
