#pragma once

#include <vector>
#include "fbxdocument.h"

namespace fbx
{
	// forward
	struct AnimationCurve;
	struct AnimationCurveNode;
	struct Model;


	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

	public:
		void Clear();

		// this is a scene based operations
		Model* FindModel(const char* name);

		bool Retrive(FBXDocument* document);
		bool Store(FBXDocument* document);

	private:


		std::vector<AnimationCurve*> m_curves;
		std::vector<AnimationCurveNode*> m_curveNodes;
		std::vector<Model*> m_models;

	};
}

