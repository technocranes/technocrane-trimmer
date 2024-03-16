#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include "cgidata.h"
#include "fbxtypes.h"

class CGIConvert
{
public:
	
	bool IsEmpty() const { return m_Packets.empty(); }
	int GetNumberOfPackets() const { return static_cast<int>(m_Packets.size()); }

	const CGIDataCartesian& GetPacket(const int index) { return m_Packets[index]; }

	bool LoadPackets(uint8_t* buffer, size_t size, const float frame_rate)
	{
		if (IsAscii(buffer, size))
		{
			return LoadAscii(buffer, size, frame_rate, m_Packets);
		}

		return LoadBinary(buffer, size, m_Packets);
	}

	void SetFOV(float w, float h)
	{
		m_fovAnimation = true;
		m_chipWidth = w;
		m_chipHeight = h;
	}

	bool IsCalibratedCGI() const
	{
		if (m_Packets.empty())
		{
			printf("calibratedCGI: not CGI data loaded.\n");
			return false;
		}
		if (m_Packets.front().zoom < 0) return true;
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

	// transform between TD-Coordinate system and MAYA/MB
	bool                          m_trafoToMayaCoordinateSystem{ true };

	// transform from technofolly meters to animation metric system (default: cm, hence 100.0 is used)
	float                         m_metricScalingFactorTD2FBX{ 100.0f };

	std::vector<CGIDataCartesian> m_Packets;

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
	bool LoadAscii(uint8_t* buffer, size_t size, float frame_rate, std::vector<CGIDataCartesian>& packets)
	{
		char line[1024]{ 0 };
		char start_letter = '-';
		int  frame_number = -1;

		int packet_number = 0;
		int temp = 0;

		membuf mem_buf(reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(buffer + size));
		std::istream in(&mem_buf);

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

			packets.emplace_back(data);

		} while (!in.eof());

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
	bool LoadBinary(uint8_t* buffer, size_t size, std::vector<CGIDataCartesian>& packets)
	{
		//membuf mem_buf(reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(buffer + size));
		//std::istream in(&mem_buf);
		//in.ignore(std::numeric_limits<std::streamsize>::max());

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

		packets.resize(number_of_packets);

		memcpy(packets.data(), buffer, size);

		/*
		for (size_t i = 0; i < number_of_packets; ++i) {
			in.read(reinterpret_cast<char*>(&packets[i]), sizeof(CGIDataCartesian));
			packets[i].packetNumber = i;
		}
		*/
		return true;
	}
};
