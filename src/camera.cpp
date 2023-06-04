
#include "camera.h"
#include "fbxobject.h"

using namespace fbx;

struct CameraImpl : Camera
{
	CameraImpl(int64_t id)
		: Camera(id)
	{
	}

	Type GetType() const override { return Type::CAMERA; }

	void OnRetreive(const FBXDocument& _document, const FBXNode& _element) override
	{

	}
};

Camera::Camera(int64_t id)
	: Model(id)
{}

Camera* Camera::Create(int64_t id)
{
	CameraImpl* newCamera = new CameraImpl(id);
	return newCamera;
}
