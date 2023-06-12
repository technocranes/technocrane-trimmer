#pragma once

#include <vector>
#include "fbxdocument.h"

namespace fbx
{
	// forward
	struct AnimationCurve;
	struct AnimationCurveNode;
	struct Model;
	struct NodeAttribute;

	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

	public:
		void Clear();

		// this is a scene based operations
		Model* FindModel(const char* name);

		bool Retrieve(FBXDocument* document);
		bool Store(FBXDocument* document);

	private:
		bool m_Verbose{ false };

		std::vector<AnimationCurve*>	m_curves;
		std::vector<AnimationCurveNode*> m_curveNodes;
		std::vector<Model*>				m_models;
		std::vector<NodeAttribute*>		m_nodeAttributes;
	};
}

