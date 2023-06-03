#pragma once

#include "fbxtypes.h"
#include "fbxnode.h"

namespace fbx
{
	/// <summary>
	/// the object received a connection event with other object pointer
	/// </summary>
	enum class ConnectionEvent
	{
		PARENTED,	// the object is going to become a child under the given connectionObject
		ADD_CHILD	// the current object is going to become a parent for the given connectionObject
	};

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
}
