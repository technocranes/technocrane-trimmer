
#include "animationCurve.h"
#include "fbxdocument.h"

using namespace fbx;

struct AnimationCurveImpl : AnimationCurve
{
	/// <summary>
	/// a constructor
	/// </summary>
	AnimationCurveImpl(int64_t id)
		: AnimationCurve(id)
	{
	}

	double Evaluate(const OFBTime& time) const override
	{
		if (m_LastEvalTime.Get() == time.Get())
		{
			return m_LastEvalValue;
		}
		else
		{
			((OFBTime*)&m_LastEvalTime)->Set(time.Get());

			size_t count = m_Values.size();
			float result = 0.0f;

			if (count > 0)
			{
				i64 fbx_time(time.Get());

				if (fbx_time < m_Times[0]) fbx_time = m_Times[0];
				if (fbx_time > m_Times[count - 1]) fbx_time = m_Times[count - 1];

				for (size_t i = 1; i < count; ++i)
				{
					if (m_Times[i] >= fbx_time)
					{
						float t = float(double(fbx_time - m_Times[i - 1]) / double(m_Times[i] - m_Times[i - 1]));
						result = m_Values[i - 1] * (1 - t) + m_Values[i] * t;
						break;
					}
				}
			}
			return result;
		}

	}

	void OnDataConnectionNotify(fbx::ConnectionEvent connnectionEvent, FBXObject* connectionObject, const Connection* connection) override
	{
		if (connnectionEvent == fbx::ConnectionEvent::PARENTED)
		{
			AnimationCurveNode* curveNode = reinterpret_cast<AnimationCurveNode*>(connectionObject);
			m_Owner = curveNode;
		}
	}

	int getKeyCount() const override { return (int)m_Times.size(); }
	const i64* getKeyTime() const override { return &m_Times[0]; }
	const float* getKeyValue() const override { return &m_Values[0]; }
	const int* getKeyFlag() const override { return &m_Flags[0]; }

	void SetKeyCount(const int count) override
	{
		m_Times.resize(count);
		m_Values.resize(count);
		//m_Flags.resize(count);
		SetKeyLinearFlags();
	}

	void SetKey(int index, const OFBTime& time, const float value, const int flags) override
	{
		m_Times[index] = time.Get();
		m_Values[index] = value;
		//m_Flags[index] = flags;
	}

	void SetKeyFlags(const std::vector<int32_t>&& flags)
	{
		m_Flags = flags;
	}

	/// <summary>
	/// ;KeyAttrFlags: Linear 260
	/// </summary>
	void SetKeyLinearFlags() override
	{
		m_Flags = std::vector<int32_t>(1, 260);
	}

	/// <summary>
	/// ;KeyAttrFlags: Constant|ConstantStandard	2
	/// </summary>
	void SetKeyConstFlags() override
	{
		m_Flags = std::vector<int32_t>(1, 2);
	}

	std::vector<i64>		m_Times;
	std::vector<float>		m_Values;
	std::vector<int32_t>	m_Flags;

	// cache
	OFBTime		m_LastEvalTime = OFBTime::MinusInfinity;
	float		m_LastEvalValue = 0.0f;

	Type GetType() const override { return Type::ANIMATION_CURVE; }


	void OnRetreive(const FBXDocument& _document, const FBXNode& _element) override
	{
		const FBXNode* times = _document.FindNode("KeyTime", &_element);
		const FBXNode* values = _document.FindNode("KeyValueFloat", &_element);
		const FBXNode* flags = _document.FindNode("KeyAttrFlags", &_element);

		if (times && times->getPropertiesCount() > 0)
		{
			const FBXProperty& prop = times->getProperties().at(0);
			if (prop.GetType() == FBXProperty::ARRAY_LONG)
			{
				m_Times.resize(prop.GetCount());
				prop.GetData(m_Times.data());
			}
			else printf("Invalid animation curve, times property!\n");
		}

		if (values && values->getPropertiesCount() > 0)
		{
			const FBXProperty& prop = values->getProperties().at(0);
			if (prop.GetType() == FBXProperty::ARRAY_FLOAT)
			{
				m_Values.resize(prop.GetCount());
				prop.GetData(m_Values.data());
			}
			else printf("Invalid animation curve, values property!\n");
		}

		if (flags && flags->getPropertiesCount() > 0)
		{
			const FBXProperty& prop = flags->getProperties().at(0);
			if (m_Values.size() == prop.GetCount())
			{
				m_Flags.resize(prop.GetCount());
				prop.GetData(m_Flags.data());
			}
			else if (1 == prop.GetCount() && prop.GetType() == FBXProperty::INTEGER)
			{
				int value = 0;
				prop.GetData(&value);
				m_Flags.resize(1, value);
			}
			else
			{
				printf("Invalid animation curve, flags property!\n");
			}
		}

		if (m_Times.size() != m_Values.size())
		{
			printf("Invalid animation curve\n");
		}
	}

	void OnStore(FBXDocument& document, FBXNode& element) override
	{
		element.Clear();

		element.addProperty(m_Id);
		element.addProperty("AnimCurve::");
		element.addProperty("");

		element.addPropertyNode("Default", 0.0f);
		element.addPropertyNode("KeyVer", 4009);
		element.addPropertyNode("KeyTime", m_Times);
		element.addPropertyNode("KeyValueFloat", m_Values);

		// ; KeyAttrFlags: Cubic | TangeantAuto 264
		// ;KeyAttrFlags: Linear 260
		// ;KeyAttrFlags: Constant|ConstantStandard	2
		element.addPropertyNode("KeyAttrFlags", m_Flags);
		
		// ;KeyAttrDataFloat: RightAuto:0, NextLeftAuto:0
		element.addPropertyNode("KeyAttrDataFloat", std::vector<int32_t>({ 0, 0, 218434821, 0 }));
		element.addPropertyNode("KeyAttrRefCount", std::vector<int32_t>(1, static_cast<int32_t>(m_Values.size())));
	}

private:
	AnimationCurveNode* m_Owner{ nullptr };
};

AnimationCurve::AnimationCurve(int64_t id)
	: FBXObject(id)
{}


AnimationCurve* AnimationCurve::Create(int64_t id)
{
	AnimationCurveImpl* curve = new AnimationCurveImpl(id);
	return curve;
}
