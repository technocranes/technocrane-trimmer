#pragma once

#include "fbxnode.h"
#include "fbxobject.h"

namespace fbx
{
	// forward
	struct AnimationCurveNodeImpl;

	/// <summary>
	/// Functional class over the FBModel data structure in the scene
	/// </summary>
	struct Model : FBXObject
	{
	protected:
		//! a constructor
		Model(int64_t id);

	public:

		static const char* GetClassName() { return "Model"; }

		//PropertyBaseEnum<OFBRotationOrder>		RotationOrder;

		// Limits
		//PropertyBool							RotationActive;
		//PropertyVector3							PreRotation;
		//PropertyVector3							PostRotation;

		//PropertyVector3							RotationOffset;
		//PropertyVector3							RotationPivot;

		//PropertyVector3							ScalingOffset;
		//PropertyVector3							ScalingPivot;

		// Animatable
		//PropertyAnimatableDouble			Visibility;
		//PropertyBool						VisibilityInheritance;
		//PropertyAnimatableVector3			Translation;
		//PropertyAnimatableVector3			Rotation;
		//PropertyAnimatableVector3			Scaling;

		//PropertyVector3						GeometricTranslation;
		//PropertyVector3						GeometricRotation;
		//PropertyVector3						GeometricScaling;


		//PropertyBool							QuaternionInterpolation;

		//PropertyBool						Show;
		//PropertyBool						Pickable;
		//PropertyBool						Transformable;

		//PropertyBool							CastsShadows;		// geometry will produce shadows
		//PropertyBool							ReceiveShadows;		// geometry will receive shadows

		//PropertyBool						PrimaryVisibility;	// could cast shadow, but not display it's geometry

		OFBMatrix getGlobalTransform() const;

		Model* Parent() const {
			return m_Parent;
		}

		Model* Children() const {
			return m_FirstChild;
		}

		Model* GetNext() const {
			return m_Next;
		}
		Model* GetPrev() const {
			return m_Prev;
		}

		void AddChild(Model* pChild);

		int GetAnimationNodeCount() const;
		AnimationCurveNode* GetAnimationNode(int index) const;

		// find nodes by name or by typeId, connected to specified layer. with multilayer or multitake it could be more than one translation node

		const AnimationCurveNode* FindAnimationNode(const char* name, const AnimationLayer* pLayer) const;
		// fast way to look for location translation, rotation, visibility, etc.
		const AnimationCurveNode* FindAnimationNodeByType(const int typeId, const	AnimationLayer* pLayer) const;

		AnimationCurveNode* FindAnimationNodeByName(const char* name) const;

		//
		//void GetMatrix(OFBMatrix& pMatrix, ModelTransformationType pWhat = eModelTransformation, bool pGlobalInfo = true, const OFBTime* pTime = nullptr) const;
		//void GetVector(OFBVector3& pVector, ModelTransformationType pWhat = eModelTranslation, bool pGlobalInfo = true, const OFBTime* pTime = nullptr) const;
		void GetRotation(OFBVector4& pQuat, const OFBTime* pTime = nullptr) const;

		/** If the model is visible.
		*	Note. this query will consider self Visibility property, plus parent node/set Visibility.
		*   The visibility of a model is affected by 4 parameters:
		*   1. The model's own visibility
		*   2. The model's parent's visibility (if any)
		*   3. The visibility of the set to which model belongs (if any)
		*   4. VisibilityInheritance of the model
		*	\param pEvaluateInfo	evaluate info,
		*	\return true if visible for the given evaluate info.
		*/
		bool IsVisible(const OFBTime* pTime = nullptr);

		virtual bool HasCustomDisplay() const { return false; }
		//virtual void CustomModelDisplay(OFBRenderConveyer* pConveyer) const;

		virtual void OnDataConnectionNotify(fbx::ConnectionEvent, FBXObject* connectionObject, const Connection* connection) override;


	protected:

		bool evalLocal(OFBMatrix* result, const OFBVector3& translation, const OFBVector3& rotation, const OFBVector3& scaling) const;

		//
		OFBMatrix			m_GlobalCache = OFBMatrix::Identity;
		OFBMatrix			m_LocalCache = OFBMatrix::Identity;
		OFBTime				m_CacheTime = OFBTime::Zero;

		//
		Model* m_Parent{ nullptr };
		//std::vector<Model*>						mChildren;


		Model* m_FirstChild{ nullptr };

		// sibling children under a parent
		Model* m_Next{ nullptr };
		Model* m_Prev{ nullptr };

		std::vector<AnimationCurveNode*>		m_AnimationNodes;
	};
}

