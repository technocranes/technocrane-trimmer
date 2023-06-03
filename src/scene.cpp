
#include "scene.h"
#include "animationCurve.h"
#include "animationCurveNode.h"
#include "camera.h"

using namespace fbx;

Scene::Scene()
{}

Scene::~Scene()
{
	Clear();
}



void Scene::Clear()
{
	for (auto curve : m_curves)
	{
		delete curve;
	}
	m_curves.clear();

	for (auto curveNode : m_curveNodes)
	{
		delete curveNode;
	}
	m_curveNodes.clear();

	for (auto model : m_models)
	{
		delete model;
	}
	m_models.clear();
}

Model* Scene::FindModel(const char* name)
{
	for (auto model : m_models)
	{
		if (model->HasName() && strcmp(name, model->GetName()) == 0)
		{
			return model;
		}
	}

	return nullptr;
}

bool Scene::Store(FBXDocument* document)
{
	for (auto& iter : document->m_objectMap)
	{
		if (iter.first == 0)
			continue;

		if (iter.second.object == nullptr)
			continue;

		iter.second.object->Store(*document, *iter.second.element);
	}
	return true;
}

bool Scene::Retrive(FBXDocument* document)
{
	// prepare every supported type of objects

	for (auto& iter : document->m_objectMap)
	{
		if (iter.first == 0)
			continue;

		const char* nodeId{ iter.second.element->getNamePtr() };

		if (strcmp(nodeId, "Geometry") == 0)
		{
			// TODO: parse geometry
		}
		else if (strcmp(nodeId, AnimationCurve::GetClassName()) == 0)
		{
			printf("animation curve found!\n");
			AnimationCurve* newCurve = AnimationCurve::Create();
			newCurve->Retreive(*document, *iter.second.element);
			m_curves.push_back(newCurve);
			iter.second.object = newCurve;
		}
		else if (strcmp(nodeId, AnimationCurveNode::GetClassName()) == 0)
		{
			AnimationCurveNode* curveNode = AnimationCurveNode::Create();
			curveNode->Retreive(*document, *iter.second.element);
			m_curveNodes.push_back(curveNode);
			iter.second.object = curveNode;
		}
		else if (strcmp(nodeId, Model::GetClassName()) == 0)
		{
			// get model subclass
			const std::string sub_class = iter.second.element->getProperties().at(2).to_string(true);

			if (strcmp(sub_class.c_str(), "Mesh") == 0)
			{
				// TODO: model with mesh
			}
			else if (strcmp(sub_class.c_str(), Camera::GetSubClassName()) == 0)
			{
				// TODO: camera model
				printf("camera found!\n");

				Camera* camera = Camera::Create();
				camera->Retreive(*document, *iter.second.element);
				iter.second.object = camera;

				m_models.push_back(camera);
			}
		}
	}

	// parse connections and assign properties

	for (auto& conn : document->m_connections)
	{
		FBXObject* parent = document->m_objectMap[conn.to].object;
		FBXObject* child = document->m_objectMap[conn.from].object;

		if (!parent || !child)
			continue;

		switch (child->GetType())
		{
		case FBXObject::Type::ANIMATION_CURVE:

			if (parent->IsNode() && parent->GetType() == FBXObject::Type::ANIMATION_CURVE_NODE)
			{
				parent->OnDataConnectionNotify(fbx::ConnectionEvent::ADD_CHILD, child, &conn);
				child->OnDataConnectionNotify(fbx::ConnectionEvent::PARENTED, parent, &conn);
			}

			break;
		case FBXObject::Type::ANIMATION_CURVE_NODE:

			if (parent->IsNode())
			{
				child->OnDataConnectionNotify(fbx::ConnectionEvent::PARENTED, parent, &conn);
				parent->OnDataConnectionNotify(fbx::ConnectionEvent::ADD_CHILD, child, &conn);
			}

			break;
		}
	}
	return true;
}