
#include "fbxobject.h"

using namespace fbx;

void FBXObject::Retreive(const FBXDocument& _document, const FBXNode& _element)
{
	// try to find object name

	if (_element.getPropertiesCount() > 1)
	{
		const std::string name = _element.getProperties().at(1).to_string(true, true);
#if defined(_WIN32)
		strcpy_s(m_Name, sizeof(char)*128, name.c_str());
#else
		strcpy(m_Name, name.c_str());
#endif
	}
	else
	{
		m_Name[0] = '\0';
	}

	OnRetreive(_document, _element);
}

void FBXObject::Store(FBXDocument& _document, FBXNode& _element)
{
	OnStore(_document, _element);
}
