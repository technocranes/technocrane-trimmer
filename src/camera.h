#pragma once

#include "fbxdocument.h"
#include "fbxnode.h"
#include "model.h"

namespace fbx
{
	struct Camera : public Model
	{
	protected:
		Camera(int64_t id);

	public:

		static const Type s_type = Type::CAMERA;
		static const char* GetSubClassName() { return "Camera"; }

		static Camera* Create(int64_t id);

		
	};
}

