#include "fbxdocument.h"
#include "fbxutil.h"

#include <ctime>

using std::string;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::uint32_t;
using std::uint8_t;

namespace fbx {

	FBXDocument::FBXDocument()
	{
		version = 7700;
		last_uid = 999999;
	}

	FBXDocument::~FBXDocument()
	{
		m_objectMap.clear();
	}


	void FBXDocument::createHeader()
	{
		FBXNode headerExtension("FBXHeaderExtension");

		headerExtension.addPropertyNode("FBXHeaderVersion", (int32_t)FBX_HEADER_VERSION);
		headerExtension.addPropertyNode("FBXVersion", (int32_t)getVersion());
		headerExtension.addPropertyNode("EncryptionType", (int32_t)0);

		{
			FBXNode creationTimeStamp("CreationTimeStamp");
			creationTimeStamp.addPropertyNode("Version", (int32_t)1000);
			creationTimeStamp.addPropertyNode("Year", (int32_t)2017);
			creationTimeStamp.addPropertyNode("Month", (int32_t)5);
			creationTimeStamp.addPropertyNode("Day", (int32_t)2);
			creationTimeStamp.addPropertyNode("Hour", (int32_t)14);
			creationTimeStamp.addPropertyNode("Minute", (int32_t)11);
			creationTimeStamp.addPropertyNode("Second", (int32_t)46);
			creationTimeStamp.addPropertyNode("Millisecond", (int32_t)917);
			headerExtension.addChild(creationTimeStamp);
		}
		const char* creator = "FBX <Neill3d> Exporter (Beta)";
		headerExtension.addPropertyNode("Creator", creator);
		{
			FBXNode sceneInfo("SceneInfo");
			sceneInfo.addProperty(std::vector<uint8_t>({ 'G','l','o','b','a','l','I','n','f','o',0,1,'S','c','e','n','e','I','n','f','o' }), 'S');
			sceneInfo.addProperty("UserData");
			sceneInfo.addPropertyNode("Type", "UserData");
			sceneInfo.addPropertyNode("Version", (int32_t)FBX_SCENEINFO_VERSION);
			{
				FBXNode metadata("MetaData");
				metadata.addPropertyNode("Version", (int32_t)FBX_SCENEINFO_VERSION);
				metadata.addPropertyNode("Title", "");
				metadata.addPropertyNode("Subject", "");
				metadata.addPropertyNode("Author", "");
				metadata.addPropertyNode("Keywords", "");
				metadata.addPropertyNode("Revision", "");
				metadata.addPropertyNode("Comment", "");
				sceneInfo.addChild(metadata);
			}
			{
				FBXNode properties("Properties70");

				{
					FBXNode p("P");
					p.addProperty("DocumentUrl");
					p.addProperty("KString");
					p.addProperty("Url");
					p.addProperty("");
					p.addProperty("/foobar.fbx");
					properties.addChild(p);
				}
				{
					FBXNode p("P");
					p.addProperty("SrcDocumentUrl");
					p.addProperty("KString");
					p.addProperty("Url");
					p.addProperty("");
					p.addProperty("/foobar.fbx");
					properties.addChild(p);
				}
				{
					FBXNode p("P");
					p.addProperty("Original");
					p.addProperty("Compound");
					p.addProperty("");
					p.addProperty("");
					properties.addChild(p);
				}
				{
					FBXNode p("P");
					p.addProperty("Original|ApplicationVendor");
					p.addProperty("KString");
					p.addProperty("");
					p.addProperty("");
					p.addProperty("Technocrane");
					properties.addChild(p);
				}
				{
					FBXNode p("P");
					p.addProperty("Original|ApplicationName");
					p.addProperty("KString");
					p.addProperty("");
					p.addProperty("");
					p.addProperty("Technocrane Trimmer");
					properties.addChild(p);
				}
				{
					FBXNode p("P");
					p.addProperty("Original|ApplicationVersion");
					p.addProperty("KString");
					p.addProperty("");
					p.addProperty("");
					p.addProperty("0.1");
					properties.addChild(p);
				}
				{
					FBXNode p("P");
					p.addProperty("Original|DateTime_GMT");
					p.addProperty("DateTime");
					p.addProperty("");
					p.addProperty("");
					p.addProperty("01/01/1970 00:00:00.000");
					properties.addChild(p);
				}
				{
					FBXNode p("P");
					p.addProperty("Original|FileName");
					p.addProperty("KString");
					p.addProperty("");
					p.addProperty("");
					p.addProperty("/test.fbx");
					properties.addChild(p);
				}
				{
					FBXNode p("P");
					p.addProperty("LastSaved");
					p.addProperty("Compound");
					p.addProperty("");
					p.addProperty("");
					properties.addChild(p);
				}
				{
					FBXNode p("P");
					p.addProperty("LastSaved|ApplicationVendor");
					p.addProperty("KString");
					p.addProperty("");
					p.addProperty("");
					p.addProperty("Technocrane");
					properties.addChild(p);
				}
				{
					FBXNode p("P");
					p.addProperty("LastSaved|ApplicationName");
					p.addProperty("KString");
					p.addProperty("");
					p.addProperty("");
					p.addProperty("Technocrane Trimmer");
					properties.addChild(p);
				}
				{
					FBXNode p("P");
					p.addProperty("LastSaved|DateTime_GMT");
					p.addProperty("DateTime");
					p.addProperty("");
					p.addProperty("");
					p.addProperty("01/01/1970 00:00:00.000");
					properties.addChild(p);
				}

				sceneInfo.addChild(properties);
			}
			headerExtension.addChild(sceneInfo);

		}
		m_root.addChild(headerExtension);

		// FINISH with the header extension

		//
		// fileId

		const char* GENERIC_CTIME = "1970-01-01 10:00:00:000";
		std::string GENERIC_FILEID("\x28\xb3\x2a\xeb\xb6\x24\xcc\xc2\xbf\xc8\xb0\x2a\xa9\x2b\xfc\xf1");

		std::vector<uint8_t> raw(GENERIC_FILEID.size());
		for (size_t i = 0; i < GENERIC_FILEID.size(); ++i) {
			raw[i] = uint8_t(GENERIC_FILEID[i]);
		}

		m_root.addPropertyNode("FileId", raw, 'R');
		//root.addPropertyNode("FileId", std::vector<uint8_t> ({'F', 'o', 'o', 'B', 'a', 'r'}), 'R');

		//
		// creationTime is replaced by dymmy value currently...
		/*
		char buffer[256] = { 0 };

		time_t     now = time(0);
		struct tm  tstruct;
		localtime_s(&tstruct, &now);
		// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
		// for more information about date/time format
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S:306", &tstruct);
		*/

		m_root.addPropertyNode("CreationTime", GENERIC_CTIME);
		m_root.addPropertyNode("Creator", creator);
	}


	void FBXDocument::createGlobalSettings()
	{
		FBXNode global_settings("GlobalSettings");
		global_settings.addPropertyNode("Version", 1000);

		FBXNode properties("Properties70");

		properties.addP70int("UpAxis", 2);
		properties.addP70int("UpAxisSign", 1);
		properties.addP70int("FrontAxis", 2);
		properties.addP70int("FrontAxisSign", 1);
		properties.addP70int("CoordAxis", 0);
		properties.addP70int("CoordAxisSign", 1);
		properties.addP70int("OriginUpAxis", -1);
		properties.addP70int("OriginUpAxisSign", 1);
		properties.addP70double("UnitScaleFactor", 1.0);
		properties.addP70double("OriginalUnitScaleFactor", 1.0);
		properties.addP70color("AmbientColor", 0.4, 0.4, 0.4);
		properties.addP70string("DefaultCamera", "Producer Perspective");
		properties.addP70enum("TimeMode", 6);
		properties.addP70enum("TimeProtocol", 0);
		properties.addP70enum("SnapOnFrameMode", 0);
		properties.addP70time("TimeSpanStart", 0);
		properties.addP70time("TimeSpanStop", 230930790000);
		properties.addP70double("CustomFrameRate", -1.0);
		properties.addP70Compound("TimeMarker", "Compound", "", "");
		properties.addP70int("CurrentTimeMarker", -1);

		global_settings.addChild(properties);

		m_root.addChild(global_settings);
	}

	void FBXDocument::createDocuments()
	{
		FBXNode docs("Documents");
		docs.addPropertyNode("Count", int32_t(1));

		FBXNode doc("Document");

		// generate uid
		int64_t uid = generate_uid();
		doc.addProperty(uid);
		doc.addProperty("");
		doc.addProperty("Scene");
		FBXNode p("Properties70");
		p.addP70Compound("SourceObject", "object", "", "");
		p.addP70string("ActiveAnimStackName", "");
		doc.addChild(p);

		// UID for root node in scene heirarchy.
		// always set to 0 in the case of a single document.
		// not sure what happens if more than one document exists,
		// but that won't matter to us as we're exporting a single scene.
		doc.addPropertyNode("RootNode", int64_t(0));

		docs.addChild(doc);
		m_root.addChild(docs);
	}

	void FBXDocument::createReferences()
	{
		// empty for now
		FBXNode n("References");
		m_root.addChild(n);
	}

	void FBXDocument::createDefinitions()
	{
		// basically this is just bookkeeping:
		// determining how many of each type of object there are
		// and specifying the base properties to use when otherwise unspecified.

		// we need to count the objects
		int32_t count;
		int32_t total_count = 0;

		// and store them
		std::vector<FBXNode> object_nodes;
		FBXNode n, pt, p;

		// GlobalSettings
		// this seems to always be here in Maya exports
		n = FBXNode("ObjectType", FBXProperty("GlobalSettings"));
		count = 1;
		n.addPropertyNode("Count", count);
		object_nodes.push_back(n);
		total_count += count;

		// now write it all
		FBXNode defs("Definitions");
		defs.addPropertyNode("Version", int32_t(100));
		defs.addPropertyNode("Count", int32_t(total_count));
		for (auto& n : object_nodes) { defs.addChild(n); }

		m_root.addChild(defs);
	}

	std::uint32_t FBXDocument::getVersion() const
	{
		return version;
	}

	void FBXDocument::print()
	{
		cout << "{\n";
		cout << "  \"version\": " << getVersion() << ",\n";
		cout << "  \"children\": [\n";
		bool hasPrev = false;

		m_root.print("  ");

		cout << "\n  ]\n}" << endl;
	}

	FBXNode* FBXDocument::FindNode(const char* name, const FBXNode* pParent) const
	{
		const FBXNode* lParent = (nullptr == pParent) ? &m_root : pParent;
		FBXNode* lResult = nullptr;

		auto& children = lParent->getChildren();

		for (auto iter = begin(children); iter != end(children); ++iter)
		{
			lResult = FindNode(name, &(*iter));

			if (nullptr != lResult)
				break;
		}

		if (nullptr == lResult)
		{
			if (0 == strcmp(lParent->getNamePtr(), name))
			{
				lResult = (FBXNode*)lParent;
			}
		}

		return lResult;
	}

	void FBXDocument::PopulateObjectMap(const FBXNode& node)
	{
		if (node.getPropertiesCount() > 0 && node.getProperties().at(0).GetType() == FBXProperty::LONG)
		{
			const u64 id{ static_cast<u64>(node.getProperties().at(0).AsLong()) };

			if (m_objectMap.find(id) != end(m_objectMap))
			{
				printf("already exist!\n");
			}

			m_objectMap[id] = { &node, nullptr };
		}

		for (auto& child : node.getChildren())
		{
			PopulateObjectMap(child);
		}
	}

	void FBXDocument::PopulateConnections(const FBXNode& node)
	{
		if (node.getPropertiesCount() == 0 || node.getProperties().at(0).GetType() != FBXProperty::STRING)
		{
			printf("Invalid connection\n");
			return;
		}

		const FBXProperty& prop = node.getProperties().at(0);

		const std::string prop_value = prop.to_string(true);
		if (strcmp(prop_value.c_str(), "OO") == 0)
		{
			if (node.getPropertiesCount() >= 3
				&& node.getProperties().at(1).GetType() == FBXProperty::LONG
				&& node.getProperties().at(2).GetType() == FBXProperty::LONG)
			{
				Connection c = { Connection::OBJECT_OBJECT };
				node.getProperties().at(1).GetData(&c.from);
				node.getProperties().at(2).GetData(&c.to);
				m_connections.push_back(std::move(c));
			}
			else
			{
				printf("Invalid OO connection");
			}
		}
		else if (strcmp(prop_value.c_str(), "OP") == 0)
		{
			if (node.getPropertiesCount() >= 4
				&& node.getProperties().at(1).GetType() == FBXProperty::LONG
				&& node.getProperties().at(2).GetType() == FBXProperty::LONG)
			{
				Connection c = { Connection::OBJECT_PROPERTY };
				node.getProperties().at(1).GetData(&c.from);
				node.getProperties().at(2).GetData(&c.to);
				c.property = &node.getProperties().at(3);
				m_connections.push_back(std::move(c));
			}
			else
			{
				printf("Invalid OP connection");
			}
		}
		else if (strcmp(prop_value.c_str(), "PP") == 0)
		{
			if (node.getPropertiesCount() >= 5
				&& node.getProperties().at(1).GetType() == FBXProperty::LONG)
			{
				Connection c = { Connection::PROPERTY_PROPERTY };
				c.from = node.getProperties().at(1).AsLong();
				c.srcProperty = &node.getProperties().at(2);
				c.to = node.getProperties().at(3).AsLong();
				c.property = &node.getProperties().at(4);

				m_connections.push_back(std::move(c));
			}
			else
			{
				printf("Invalid PP connection");
			}
		}
		else
		{
			printf("Not supported connection");
		}

		for (auto& child : node.getChildren())
		{
			PopulateConnections(child);
		}
	}

	bool FBXDocument::ParseConnections()
	{
		constexpr const char* CONNECTIONS_NAME{ "Connections" };
		m_connections.clear();

		if (const FBXNode* connections = FindNode(CONNECTIONS_NAME, &m_root))
		{
			for (auto& child : connections->getChildren())
			{
				PopulateConnections(child);
			}
			return true;
		}
		return false;
	}


	bool FBXDocument::ParseObjects()
	{
		constexpr const char* OBJECTS_NAME{ "Objects" };

		FBXNode* objs = FindNode(OBJECTS_NAME, &m_root);
		if (!objs) return true;

		// all objects map

		m_objectMap.clear();
		m_objectMap[0] = { &m_root, nullptr };

		for (auto& child : objs->getChildren())
		{
			PopulateObjectMap(child);
		}

		return true;
	}

} // namespace fbx
