
#include "camera.h"
#include "fbxobject.h"

using namespace fbx;

struct CameraImpl : Camera
{
	CameraImpl()
		: Camera()
	{
	}

	Type GetType() const override { return Type::CAMERA; }

	void OnRetreive(const FBXDocument& _document, const FBXNode& _element) override
	{

	}
};

Camera::Camera()
	: Model()
{}

Camera* Camera::Create()
{
	CameraImpl* newCamera = new CameraImpl();
	return newCamera;
}
