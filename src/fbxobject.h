#pragma once

#include "fbxtypes.h"
#include "fbxtime.h"

namespace fbx
{
	class FBXDocument;
	struct AnimationLayer;
	struct AnimationCurveNode;

	struct DataView
	{
		const u8* begin = nullptr;
		const u8* end = nullptr;
		bool is_binary = true;

		bool operator!=(const char* rhs) const { return !(*this == rhs); }
		bool operator==(const char* rhs) const;

		u64 toU64() const;
		int toInt() const;
		u32 toU32() const;
		double toDouble() const;
		float toFloat() const;

		template <int N>
		void toString(char(&out)[N]) const
		{
			char* cout = out;
			const u8* cin = begin;
			while (cin != end && cout - out < N - 1)
			{
				*cout = (char)*cin;
				++cin;
				++cout;
			}
			*cout = '\0';
		}
	};

	struct FBXNodeView
	{
			enum class Type
			{
				ROOT,
				GEOMETRY,
				MATERIAL,
				SHADER,
				MESH,
				TEXTURE,
				LIMB_NODE,
				NULL_NODE,
				CAMERA,
				LIGHT,
				NODE_ATTRIBUTE,
				CLUSTER,
				SKIN,
				CONSTRAINT,
				CONSTRAINT_POSITION,
				ANIMATION_STACK,
				ANIMATION_LAYER,
				ANIMATION_CURVE,
				ANIMATION_CURVE_NODE
			};
			/*
			FBXNodeView(const FBXDocument& _scene, const FBXNode& _element);

			virtual ~FBXNodeView() {}
			virtual Type getType() const = 0;

			const FBXDocument& getScene() const;
			FBXNodeView* resolveObjectLink(int idx) const;
			FBXNodeView* resolveObjectLink(Type type, const char* property, int idx) const;
			FBXNodeView* resolveObjectLinkReverse(Type type) const;
			FBXNodeView* getParents(int idx) const;

			bool isNode() const { return is_node; }


			template <typename T> T* resolveObjectLink(int idx) const
			{
				return static_cast<T*>(resolveObjectLink(T::s_type, nullptr, idx));
			}

			uint64_t id;
			char name[128];
			const FBXNode& element;
			const FBXNodeView* node_attribute;	// contains some specified class properties ontop of base class

			const void* eval_data;
			const void* render_data;

			//bool Selected;
			//PropertyString			Name;
			//PropertyBool			Selected;

			//PropertyList			mProperties;

			// retrive properties values and connections
			virtual bool Retrieve();
			/*
			void PropertyAdd(PropertyBase* pProperty)
			{
				mProperties.Add(pProperty);
			}
			*/
		protected:

			bool is_node;
			const FBXDocument& scene;
		
	};


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// AnimationStack

	struct AnimationStack : FBXNodeView
	{
		static const Type s_type = Type::ANIMATION_STACK;

		AnimationStack(const FBXDocument& _scene, const FBXNode& _element);

		virtual i64		getLoopStart() const = 0;
		virtual i64		getLoopStop() const = 0;

		virtual int getLayerCount() const = 0;
		virtual const AnimationLayer* getLayer(int index) const = 0;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// AnimationLayer

	struct AnimationLayer : FBXNodeView
	{
		static const Type s_type = Type::ANIMATION_LAYER;

		AnimationLayer(const FBXDocument& _scene, const FBXNode& _element);

		virtual const AnimationCurveNode* getCurveNode(int index) const = 0;
		virtual const AnimationCurveNode* getCurveNode(const FBXNodeView& bone, const char* property) const = 0;

		//PropertyInt		LayerID;	// rearranged order of layers, defined by users

		//PropertyBool	Solo;		//!< <b>Read Write Property:</b> If true, the layer is soloed. When you solo a layer, you mute other layers that are at the same level in the hierarchy, as well as the children of those layers. Cannot be applied to the BaseAnimation Layer.
		//PropertyBool	Mute;		//!< <b>Read Write Property:</b> If true, the layer is muted. A muted layer is not included in the result animation. Cannot be applied to the BaseAnimation Layer.
		//PropertyBool	Lock;		//!< <b>Read Write Property:</b> If true, the layer is locked. You cannot modify keyframes on a locked layer.

		//PropertyAnimatableDouble	Weight; //!< <b>Read Write Property:</b> The weight value of a layer determines how much it is present in the result animation. Takes a value from 0 (the layer is not present) to 100. The weighting of a parent layer is factored into the weighting of its child layers, if any. BaseAnimation Layer always has a Weight of 100. 

		//PropertyBaseEnum<FBLayerMode>			LayerMode;	//!< <b>Read Write Property:</b> Layer mode. By default, the layer is in kFBLayerModeAdditive mode. Cannot be applied to the BaseAnimation Layer.
		//PropertyBaseEnum<FBLayerRotationMode>	LayerRotationMode; //!< <b>Read Only Property:</b> Layer rotation mode. Cannot be applied to the BaseAnimation Layer.


		virtual int getSubLayerCount() const = 0;
		virtual const AnimationLayer* getSubLayer(int index) const = 0;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// AnimationCurve

	struct AnimationCurve : FBXNodeView
	{
		static const Type s_type = Type::ANIMATION_CURVE;

		AnimationCurve(const FBXDocument& _scene, const FBXNode& _element);

		virtual int getKeyCount() const = 0;
		virtual const i64* getKeyTime() const = 0;
		virtual const float* getKeyValue() const = 0;
		virtual const int* getKeyFlag() const = 0;

		virtual double Evaluate(const OFBTime& time) const = 0;
	};


	/////////////////////////////////////////////////////////////////////////////////////////////////////
// AnimationCurveNode

	struct AnimationCurveNode : FBXNodeView
	{
		static const Type s_type = Type::ANIMATION_CURVE_NODE;

		AnimationCurveNode(const FBXDocument& _scene, const FBXNode& _element);

		virtual OFBVector3 getNodeLocalTransform(double time) const = 0;
		virtual const FBXNodeView* GetOwner() const = 0;

		// return next anim node linked under property layers stack (in order how layers have been sorted)
		virtual AnimationCurveNode* GetNext() = 0;
		virtual const AnimationCurveNode* GetNext() const = 0;
		virtual void LinkNext(const AnimationCurveNode* pNext) = 0;

		virtual AnimationLayer* getLayer() const = 0;

		virtual int getCurveCount() const = 0;
		virtual const AnimationCurve* getCurve(int index) const = 0;

		virtual bool Evaluate(double* Data, const OFBTime pTime) const = 0;
	};


	struct TakeInfo
	{
		DataView name;
		DataView filename;
		double local_time_from;
		double local_time_to;
		double reference_time_from;
		double reference_time_to;
	};


	/// <summary>
	/// Functional class over the FBModel data structure in the scene
	/// </summary>
	struct FBXModel : FBXNodeView
	{
		//! a constructor
		FBXModel(const FBXDocument& _scene, const FBXNode& _element);

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

		FBXModel* Parent() const {
			return mParent;
		}

		FBXModel* Children() const {
			return mFirstChild;
		}

		FBXModel* GetNext() const {
			return mNext;
		}
		FBXModel* GetPrev() const {
			return mPrev;
		}

		void AddChild(FBXModel* pChild);

		const int GetAnimationNodeCount() const;
		const AnimationCurveNode* GetAnimationNode(int index) const;

		// find nodes by name or by typeId, connected to specified layer. with multilayer or multitake it could be more than one translation node

		const AnimationCurveNode* FindAnimationNode(const char* name, const AnimationLayer* pLayer) const;
		// fast way to look for location translation, rotation, visibility, etc.
		const AnimationCurveNode* FindAnimationNodeByType(const int typeId, const	AnimationLayer* pLayer) const;

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

		std::vector<AnimationCurveNode*>		mAnimationNodes;

	protected:

		bool evalLocal(OFBMatrix* result, const OFBVector3& translation, const OFBVector3& rotation, const OFBVector3& scaling) const;

		//
		OFBMatrix			mGlobalCache;
		OFBMatrix			mLocalCache;
		OFBTime				mCacheTime;

		//
		FBXModel* mParent;
		//std::vector<Model*>						mChildren;


		FBXModel* mFirstChild;

		// sibling children under a parent
		FBXModel* mNext;
		FBXModel* mPrev;
	};

}

