#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include "cgidata.h"
#include "fbxtypes.h"

/// <summary>
/// A wrapper class around input bytes array to view it as array of desired structured data
/// </summary>
template<typename T>
class ConstArrayView
{
public:
	ConstArrayView()
	{}

	ConstArrayView(uint8_t* buffer, size_t size)
	{
		m_Array = reinterpret_cast<T*>(buffer);
		m_Count = size / sizeof(T);
	}

	ConstArrayView(T* buffer, size_t count)
	{
		m_Array = buffer;
		m_Count = count;
	}

	const T& At(const size_t index) const
	{
		return m_Array[index];
	}

	const T& First() const
	{
		return m_Array[0];
	}

	const T& Last() const
	{
		return m_Array[m_Count - 1];
	}

	const size_t Count() const
	{
		return m_Count;
	}

	bool IsEmpty() const { return m_Count == 0; }

private:
	T* m_Array{ nullptr };
	size_t	m_Count{ 0 };
};

/// <summary>
/// extract some structured data information CGIDataCartesian from the input bytes
///  Additional helper methods for a data processing
/// Packets are auto sorted by the timecode
///  The class has pointers to a provided input data, 
/// you have to keep that data in memory while operating with the class
/// </summary>
class CGIConvert
{
public:
	
	bool IsEmpty() const { return m_PacketsView.IsEmpty(); }

	/// <summary>
	/// returns a total number of imported packets
	/// </summary>
	int GetNumberOfPackets() const { return static_cast<int>(m_PacketsView.Count()); }

	/// <summary>
	/// returns a packet sorted by timecode value
	/// </summary>
	const CGIDataCartesian& GetPacket(const int index) 
	{ 
		const size_t lookUpIndex = m_SortedPackets[index];
		return m_PacketsView.At(lookUpIndex); 
	}

	/// <summary>
	/// main entry method, process the input buffer
	/// </summary>
	bool LoadPackets(uint8_t* buffer, size_t size, const float frame_rate)
	{
		if (IsAscii(buffer, size))
		{
			return LoadAscii(buffer, size, frame_rate);// , m_Packets);
		}

		return LoadBinary(buffer, size); // , m_Packets);
	}

	void SetFOV(float w, float h)
	{
		m_fovAnimation = true;
		m_chipWidth = w;
		m_chipHeight = h;
	}

	bool IsCalibratedCGI() const
	{
		if (m_PacketsView.IsEmpty())
		{
			printf("calibratedCGI: not CGI data loaded.\n");
			return false;
		}
		if (m_PacketsView.First().zoom < 0) return true;
		return false;
	}

	float ConvertFocalLength(const CGIDataCartesian& cgiData) const
	{
		return -1.0f * cgiData.zoom; // focal length in mm is fine ....
	}
	void ConvertFocalLengthInv(float focalLength, CGIDataCartesian& cgiData) const
	{
		cgiData.zoom = -1.0f * focalLength; // focal length in mm is fine ...
	}

	std::pair<float, float> ConvertFOV(const CGIDataCartesian& cgiData) const
	{
		const double focalLength = static_cast<double>(ConvertFocalLength(cgiData));
		//const double pi=3.14159265358979323846;
		const double fovHorizontal = 2.0 * 180.0 * atan(0.5 * this->m_chipWidth / focalLength) / pi; // tan (fovh/2) = chipsize/2 / focal length
		const double fovVertical = 2.0 * 180.0 * atan(0.5 * this->m_chipHeight / focalLength) / pi;
		return std::pair<float, float>(static_cast<float>(fovHorizontal), static_cast<float>(fovVertical));
	}

	float ConvertFocusDistance(const CGIDataCartesian& cgiData) const
	{
		constexpr const float FOCUS_DISTANCE_INFINITY = 1000000.f; // infifity is 1km away...  

		if (fabs(cgiData.focus) < 0.0001f) return FOCUS_DISTANCE_INFINITY;
		return -1.0f * m_metricScalingFactorTD2FBX / cgiData.focus; // 

	}

	// --------------------------------------------------------------------------------------
	// FBX-Data in MAYA system: TX(X*100),TY(Z*100),TZ(-Y*100),
	//                          RX(roll),RY(-pan),RZ(tilt): start position (0,0,0): optical axis along -z-axis
	//                          Coordinate units are in cm. Rotation order ZXY
	//
	//               TD system: TX(X),TY(Y),TZ(Z)
	//                          RX(roll),RY(-tilt),RZ(-pan), start position (0,0,0): optical axis along x-axis
	//                          Coordinate units are in m.
	//          
	// MAYA_X=-TD_Y, MAYA_Y=TD_Z, MAYA_Z=-TD_X
	void ConvertToFBX(const CGIDataCartesian& data,
		fbx::FVector4& pos,
		fbx::FVector4& rot) const
	{

		if (m_trafoToMayaCoordinateSystem)
		{
			const float meterToCm = m_metricScalingFactorTD2FBX;
			pos = { -1.0f * data.y * meterToCm, data.z * meterToCm, -1.0f * data.x * meterToCm };
			
			fbx::FVector4 zxyRot = { data.tilt, -1.0f * data.pan, -1.0f * data.roll };
			rot = zxyRot;

			// in td: rot um y global (-pan), dann um x dynamic, dann roll  
			// in fbx: rot um x global (tilt), dann um y global (-pan), dann roll
		}
		else
		{
			pos = { data.x, data.y, data.z };
			rot = { data.roll, -1.0f * data.tilt, -1.0f * data.pan };
		}
	}

private:

	// 
	bool                          m_fovAnimation{ true };
	double                        m_chipWidth{ 640.0 };
	double                        m_chipHeight{ 480.0 };

	/// transform between TD-Coordinate system and MAYA/MB
	bool                          m_trafoToMayaCoordinateSystem{ true };

	/// transform from technofolly meters to animation metric system (default: cm, hence 100.0 is used)
	float                         m_metricScalingFactorTD2FBX{ 100.0f };

	ConstArrayView<CGIDataCartesian>	m_PacketsView;

	/// indices of packets data sorted by timestamp
	std::vector<size_t> m_SortedPackets;

	/// store in memory in case we unpack packets from ascii
	std::vector<CGIDataCartesian> m_UnpackedPackets;

	void CalculateSortedPacketIndices()
	{
		if (m_PacketsView.IsEmpty())
			return;

		m_SortedPackets.resize(m_PacketsView.Count());
		for (size_t i = 0; i < m_PacketsView.Count(); ++i)
			m_SortedPackets[i] = i;

		std::sort(begin(m_SortedPackets), end(m_SortedPackets), [&](const size_t a, const size_t b)
			{
				const CGIDataCartesian& packetA = m_PacketsView.At(a);
				const CGIDataCartesian& packetB = m_PacketsView.At(b);
				return packetA.timeCode < packetB.timeCode;
			});
	}

	/**
	 * check if file data is in ASCII or binary format.
	 *
	 * \param buffer
	 * \param size
	 * \return true if file is in ASCII
	 */
	bool IsAscii(uint8_t* buffer, size_t size)
	{
		uint8_t* readPtr = buffer;
		size_t pos = 0;

		int c;
		while ((c = *readPtr) != EOF && c <= 127 && pos < size)
		{
			++pos;
			++readPtr;
		}

		return (c == EOF || c == 0);
	}

	struct membuf : std::streambuf {
		membuf(char* begin, char* end) {
			this->setg(begin, begin, end);
		}
	};

	/**
	 * extract CGIData packets from a data buffer and put them into given packets array.
	 *
	 * \param buffer
	 * \param frame_rate
	 * \param size
	 * \param packets
	 * \return true if operation was successfull
	 */
	bool LoadAscii(uint8_t* buffer, size_t size, float frame_rate)
	{
		char line[1024]{ 0 };
		char start_letter = '-';
		int  frame_number = -1;

		int packet_number = 0;
		int temp = 0;

		membuf mem_buf(reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(buffer + size));
		std::istream in(&mem_buf);

		m_UnpackedPackets.clear();

		do {
			in.getline(line, 1024, '\n');
			if (in.eof()) break;

			CGIDataCartesian data;
			memset(&data, 0, sizeof(CGIDataCartesian));
			data.syncVal = TDDE_SYNC_VAL;
			data.checkSum = sizeof(CGIDataCartesian);

#ifdef _MSC_VER
			int parse = sscanf_s(line,
				"%c%d.00,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%f",
				&start_letter, 1, &frame_number,
				&data.x, &data.y, &data.z,
				&data.pan, &data.tilt, &data.roll,
				&data.zoom, &data.focus, &data.iris,
				&temp, &data.spare.trackPos);
#else
			int parse = sscanf(line,
				"%c%d.00,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%f",
				&start_letter, &frame_number,
				&data.x, &data.y, &data.z,
				&data.pan, &data.tilt, &data.roll,
				&data.zoom, &data.focus, &data.iris,
				&temp, &data.spare.trackPos);
#endif
			/*
			if (m_verbose>1){
				char buffer[1024];
				sprintf(buffer,
						"%c%d.00,%.4f,%.4f,%.4f,%.3f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
						startLetter,frameNumber,
						data.x,   data.y, data.z,
						data.pan, data.tilt, data.roll,
						data.zoom, data.focus, data.iris);
				log() << buffer;
			}
			*/
			if (parse != 13) {
				printf("loadCGI ERROR: Parse error for line ...\n");
				return false;
			}

			data.packetNumber = packet_number;
			packet_number += 1;

			CGIDataCartesianVersion1* ptr = reinterpret_cast<CGIDataCartesianVersion1*>(&data);
			ptr->frameNumber = frame_number;
			ptr->time = static_cast<float>(frame_number / frame_rate);

			m_UnpackedPackets.emplace_back(data);

		} while (!in.eof());

		m_PacketsView = ConstArrayView<CGIDataCartesian>(m_UnpackedPackets.data(), m_UnpackedPackets.size());
		CalculateSortedPacketIndices();

		return true;
	}

	/**
	 * read packets from binary source and put them into a given packets array.
	 *
	 * \param buffer
	 * \param size
	 * \param packets
	 * \return true if opeartion was successfull
	 */
	bool LoadBinary(uint8_t* buffer, size_t size)
	{
		if (size <= 0)
		{
			printf("File size is empty\n");
			return false;
		}

		const size_t number_of_packets = size / sizeof(CGIDataCartesian);

		if (number_of_packets == 0 || size % sizeof(CGIDataCartesian) != 0)
		{
			printf("Wrong file content\n");
			return false;
		}

		m_PacketsView = ConstArrayView<CGIDataCartesian>(buffer, size);
		CalculateSortedPacketIndices();

		return true;
	}
};
