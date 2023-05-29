#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "cgidata.h"
#include "fbxdocument.h"
#include "fbximporter.h"
#include "fbxexporter.h"
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif


#ifdef __cplusplus
    #ifdef __EMSCRIPTEN__
        #define EXTERN extern "C" EMSCRIPTEN_KEEPALIVE
    #else
        #define EXTERN extern "C"
    #endif
#else
    #ifdef __EMSCRIPTEN__
        #define EXTERN EMSCRIPTEN_KEEPALIVE
    #else
        #define EXTERN
    #endif
#endif


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

	return (c == EOF);
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

	membuf mem_buf( reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(buffer + size));
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

bool LoadPackets(uint8_t* buffer, size_t size, const float frame_rate,
	std::vector<CGIDataCartesian>& packets) 
{
	if (IsAscii(buffer, size))
	{
		return LoadAscii(buffer, size, frame_rate, packets);
	}

	return LoadBinary(buffer, size, packets);
}

/**
 * Load CGI, trim it and save into fbx.
 * 
 * \param buffer
 * \param size
 * \return 
 */
EXTERN int load_file(uint8_t* buffer, size_t size) {
	/// Load a file - this function is called from javascript when the file upload is activated
	std::cout << "load_file triggered, buffer " << &buffer << " size " << size << std::endl;

	// do whatever you need with the file contents
	std::vector<CGIDataCartesian> packets;
	LoadPackets(buffer, size, 30.f, packets);

	printf("Loaded packets - %zu\n", packets.size());
	if (!packets.empty())
	{
		const CGIDataCartesian& firstPacket = packets[0];
		const CGIDataCartesian& lastPacket = packets[packets.size() - 1];

		printf("Start TimeCode %d:%d:%d:%d\n", firstPacket.timeCode.hours, firstPacket.timeCode.minutes, firstPacket.timeCode.seconds,
			firstPacket.timeCode.frames);
		printf("End TimeCode %d:%d:%d:%d\n", lastPacket.timeCode.hours, lastPacket.timeCode.minutes, lastPacket.timeCode.seconds,
			lastPacket.timeCode.frames);
	}
	

	// write into fbx

	fbx::FBXDocument doc;

	//doc.createHeader();
	//doc.createGlobalSettings();
	//doc.createDocuments();
	//doc.createReferences();
	//doc.createDefinitions();

	fbx::Importer lImporter;
	bool lSuccess = lImporter.Initialize("C:\\work\\technocrane\\tdcamera.fbx");

	if (lSuccess)
	{
		lImporter.Import(doc);

		// modify keyframes
		
		doc.ParseObjects();
		doc.ParseConnections();

		//

		fbx::Exporter	lExporter;

		std::cout << "Writing test.fbx" << std::endl;
		lExporter.Initialize("c:\\work\\technocrane\\test-cgi.fbx", false);

		lExporter.Export(doc);
	}
	/*
	fbx::Exporter	lExporter;

	std::cout << "Writing test.fbx" << std::endl;
	lExporter.Initialize("c:\\work\\technocrane\\test-cgi.fbx", false);

	lExporter.Export(doc);
	*/
	return 1;
}


/**
 * main entry point.
 *
 * \return
 */
int main(int argc, char* argv[]) {
	printf("Welcome To A Technocrane Trimmer!\n");

	if (argc > 1)
	{
		const char* fname{ argv[1] };
		std::ifstream fstream(fname, std::ios::binary | std::ios::ate);
		std::streamsize size = fstream.tellg();

		if (size <= 0)
		{
			printf("Failed to read the file!\n");
			return -1;
		}

		fstream.seekg(0, std::ios::beg);

		std::vector<uint8_t> buffer(size);
		if (fstream.read((char*)buffer.data(), size))
		{
			/* worked! */
			load_file(buffer.data(), buffer.size());
		}
	}

	return 0;
}