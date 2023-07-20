#pragma once

#include "fbxtypes.h"
#include "fbxtime.h"
#include "fbxconnection.h"

namespace fbx
{
	class FBXDocument;
	struct AnimationLayer;
	struct AnimationCurveNode;
	struct AnimationCurve;
	struct NodeAttribute;

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

	/// <summary>
	/// base unpacked data type of fbx node, the object represents a scene element
	/// </summary>
	struct FBXObject
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

			virtual Type GetType() const = 0;

			/// <summary>
			/// a constructor with unique id
			/// </summary>
			/// <param name="id">unique id</param>
			FBXObject(int64_t id)
				: m_Id(id)
			{}

			virtual ~FBXObject() {}

			bool IsNode() const { return m_isNode; }
			const char* GetName() const { return m_Name; }
			bool HasName() const { return m_Name[0] != 0; }
			
			/// <summary>
			/// raw FBXNode to be converted into a more logical and functional FBXObject based class 
			/// </summary>
			/// <param name="_document">fbx document with raw nodes and connections</param>
			/// <param name="_element">source node to be converted into the object</param>
			void Retreive(const FBXDocument& _document, const FBXNode& _element);
			
			/// <summary>
			/// make a raw fbx node from the given FBXObject based class
			/// </summary>
			/// <param name="_document">fbx document with raw nodes and connections</param>
			/// <param name="_element">raw element to preprare data from the given object</param>
			void Store(FBXDocument& _document, FBXNode& _element);

			/// <summary>
			/// callback when given object is connected or being used as a parent for the new connection
			/// </summary>
			/// <param name="">type of connection event</param>
			/// <param name="connectionObject">another object that is taking part in the connection</param>
			/// <param name="connection">new connection data</param>
			virtual void OnDataConnectionNotify(fbx::ConnectionEvent, FBXObject* connectionObject, const Connection* connection) {}

			const NodeAttribute* GetNodeAttribute() const { return m_NodeAttribute; }

		protected:

			bool m_isNode{ true };
			int64_t m_Id{ 0 };
			char m_Name[128]{ 0 };

			const NodeAttribute* m_NodeAttribute{ nullptr };	// contains some specified class properties ontop of base class

			virtual void OnRetreive(const FBXDocument& _document, const FBXNode& _element) = 0;
			virtual void OnStore(FBXDocument& _document, FBXNode& _element) {}
	};


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// AnimationStack

	struct AnimationStack : FBXObject
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

	struct AnimationLayer : FBXObject
	{
		static const Type s_type = Type::ANIMATION_LAYER;

		AnimationLayer(const FBXDocument& _scene, const FBXNode& _element);

		virtual const AnimationCurveNode* getCurveNode(int index) const = 0;
		virtual const AnimationCurveNode* getCurveNode(const FBXObject& bone, const char* property) const = 0;

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

	

	struct TakeInfo
	{
		DataView name;
		DataView filename;
		double local_time_from;
		double local_time_to;
		double reference_time_from;
		double reference_time_to;
	};


}

