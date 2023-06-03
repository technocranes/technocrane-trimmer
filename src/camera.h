#pragma once

#include "fbxdocument.h"
#include "fbxnode.h"
#include "model.h"

namespace fbx
{
	struct Camera : public Model
	{
	protected:
		Camera();

	public:

		static const Type s_type = Type::CAMERA;
		static const char* GetSubClassName() { return "Camera"; }

		static Camera* Create();

		
	};
}

