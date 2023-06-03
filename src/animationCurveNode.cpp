
#include "animationCurveNode.h"
#include "animationCurve.h"
#include "fbxutil.h"
#include "fbxdocument.h"
#include "model.h"

namespace fbx
{
	struct AnimationCurveNodeImpl : AnimationCurveNode
	{
		AnimationCurveNodeImpl()
			: AnimationCurveNode()
		{
		}


		const FBXObject* GetOwner() const override
		{
			return m_Owner;
		}

		AnimationCurveNode* GetNext() override
		{
			return m_Next;
		}

		const AnimationCurveNode* GetNext() const override
		{
			return m_Next;
		}

		void LinkNext(const AnimationCurveNode* pNext) override
		{
			m_Next = (AnimationCurveNode*)pNext;
		}

		OFBVector3 getNodeLocalTransform(double time) const override
		{
			u64 fbx_time = secondsToFbxTime(time);

			auto getCoord = [](const Curve& curve, i64 fbx_time) {
				if (!curve.curve) return 0.0;
				return curve.curve->Evaluate(fbx_time);
			};

			return { getCoord(m_Curves[0], fbx_time), getCoord(m_Curves[1], fbx_time), getCoord(m_Curves[2], fbx_time) };
		}

		AnimationLayer* getLayer() const override
		{
			return m_Layer;
		}

		int getCurveCount() const override
		{
			return m_NumberOfCurves;
		}
		const AnimationCurve* getCurve(int index) const override
		{
			return m_Curves[index].curve;
		}
		bool AttachCurve(const AnimationCurve* pCurve, const Connection* connection)
		{
			bool lSuccess = false;
			if (m_NumberOfCurves < 3)
			{
				m_Curves[m_NumberOfCurves].curve = pCurve;
				m_Curves[m_NumberOfCurves].connection = connection;
				m_NumberOfCurves += 1;

				lSuccess = true;
			}
			return lSuccess;
		}

		bool Evaluate(double* Data, const OFBTime pTime) const override
		{
			double* pData = Data;
			for (int i = 0; i < m_NumberOfCurves; ++i)
			{
				*pData = m_Curves[i].curve->Evaluate(pTime);
				pData += 1;
			}
			return true;
		}

		void OnDataConnectionNotify(fbx::ConnectionEvent connnectionEvent, FBXObject* connectionObject, const Connection* connection) override
		{
			switch (connnectionEvent)
			{
			case fbx::ConnectionEvent::PARENTED:
				// we expect to be attached to a model
				if (connectionObject->IsNode())
				{
					Model* model = reinterpret_cast<Model*>(connectionObject);
					m_Owner = model;
					m_BoneLinkProperty = connection->property;

					const std::string propertyId = connection->property->to_string(true);

					if (strcmp(propertyId.c_str(), ANIMATIONNODE_TYPENAME_TRANSLATION) == 0)
						m_Mode = ANIMATIONNODE_TYPE_TRANSLATION;
					else if (strcmp(propertyId.c_str(), ANIMATIONNODE_TYPENAME_ROTATION) == 0)
						m_Mode = ANIMATIONNODE_TYPE_ROTATION;
					else if (strcmp(propertyId.c_str(), ANIMATIONNODE_TYPENAME_SCALING) == 0)
						m_Mode = ANIMATIONNODE_TYPE_SCALING;
					else if (strcmp(propertyId.c_str(), ANIMATIONNODE_TYPENAME_VISIBILITY) == 0)
						m_Mode = ANIMATIONNODE_TYPE_VISIBILITY;
					else if (strcmp(propertyId.c_str(), ANIMATIONNODE_TYPENAME_FIELDOFVIEW) == 0)
						m_Mode = ANIMATIONNODE_TYPE_FIELD_OF_VIEW;
				}
				break;
			case fbx::ConnectionEvent::ADD_CHILD:
				// we expect to have animation curve to be added as a child
				if (connectionObject->GetType() == FBXObject::Type::ANIMATION_CURVE)
				{
					AttachCurve(reinterpret_cast<AnimationCurve*>(connectionObject), connection);
				}
				
				break;
			}
		}

		Type GetType() const override { return Type::ANIMATION_CURVE_NODE; }

		void OnRetreive(const FBXDocument& _document, const FBXNode& _element) override
		{

		}

	protected:

		AnimationLayer* m_Layer{ nullptr };
		AnimationCurveNode* m_Next{ nullptr };	// linked anim nodes under owner for different layers

		FBXObject* m_Owner{ nullptr };
		const FBXProperty* m_BoneLinkProperty;

		AnimationNodeType m_Mode{ ANIMATIONNODE_TYPE_CUSTOM };

	protected:

		struct Curve
		{
			const AnimationCurve* curve = nullptr;
			const Connection* connection = nullptr;
		};

		int m_NumberOfCurves = 0;
		Curve m_Curves[3];
	};

	AnimationCurveNode::AnimationCurveNode()
		: FBXObject()
	{}

	AnimationCurveNode* AnimationCurveNode::Create()
	{
		return new AnimationCurveNodeImpl();
	}

}
