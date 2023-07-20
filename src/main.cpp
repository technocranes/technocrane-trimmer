#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "cgidata.h"
#include "fbxdocument.h"
#include "fbximporter.h"
#include "fbxexporter.h"
#include "scene.h"
#include "model.h"
#include "animationCurve.h"
#include "animationCurveNode.h"
#include "nodeAttribute.h"
#include "fbxtime.h"
#include "fbxutil.h"
#include "cgiConvert.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include "emscripten_browser_file.h"
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


/// <summary>
/// Print to console information about raw *.cgi stream, like start / stop timecodes
/// </summary>
/// <param name="buffer"></param>
/// <param name="size"></param>
/// <param name="frameRate"></param>
/// <returns></returns>
EXTERN int PrintCGIInfo(uint8_t* buffer, size_t size, double frameRate)
{
	CGIConvert cgiConvert;
	cgiConvert.LoadPackets(buffer, size, static_cast<float>(frameRate));

	printf("Loaded packets - %d\n", cgiConvert.GetNumberOfPackets());
	if (!cgiConvert.IsEmpty())
	{
		const CGIDataCartesian& firstPacket = cgiConvert.GetPacket(0);
		const CGIDataCartesian& lastPacket = cgiConvert.GetPacket(cgiConvert.GetNumberOfPackets() - 1);

		printf("Start TimeCode %d:%d:%d:%d\n", firstPacket.timeCode.hours, firstPacket.timeCode.minutes, firstPacket.timeCode.seconds,
			firstPacket.timeCode.frames);
		printf("End TimeCode %d:%d:%d:%d\n", lastPacket.timeCode.hours, lastPacket.timeCode.minutes, lastPacket.timeCode.seconds,
			lastPacket.timeCode.frames);
	}
	return 0;
}

bool PrepareCameraAnimation(fbx::Scene& scene, CGIConvert& cgiConvert, double startTime, double endTime, double fps)
{
	auto node = scene.FindModel("TDCamera");
	if (node == nullptr)
		return false;

	fbx::AnimationCurveNode* translationNode = node->FindAnimationNodeByName("T");
	fbx::AnimationCurveNode* rotationNode = node->FindAnimationNodeByName("R");

	if (node->GetNodeAttribute() == nullptr)
	{
		printf("node attribute is empty\n");
		return false;
	}

	fbx::AnimationCurveNode* fieldOfViewNode = node->GetNodeAttribute()->FindAnimationNodeByName("FieldOfView");
	fbx::AnimationCurveNode* focusDistanceNode = node->GetNodeAttribute()->FindAnimationNodeByName("FocusDistance");

	fbx::AnimationCurveNode* zoomNode = node->FindAnimationNodeByName("Zoom");
	fbx::AnimationCurveNode* focusNode = node->FindAnimationNodeByName("Focus");
	fbx::AnimationCurveNode* irisNode = node->FindAnimationNodeByName("Iris");
	fbx::AnimationCurveNode* trackPosNode = node->FindAnimationNodeByName("TrackPos");
	fbx::AnimationCurveNode* packetNumberNode = node->FindAnimationNodeByName("PacketNumber");

	fbx::AnimationCurveNode* tcHourNode = node->FindAnimationNodeByName("TCHour");
	fbx::AnimationCurveNode* tcMinuteNode = node->FindAnimationNodeByName("TCMinute");
	fbx::AnimationCurveNode* tcSecondNode = node->FindAnimationNodeByName("TCSecond");
	fbx::AnimationCurveNode* tcFrameNode = node->FindAnimationNodeByName("TCFrame");
	fbx::AnimationCurveNode* tcRateNode = node->FindAnimationNodeByName("TCRate");
	// TODO: TCSubframe

	if (translationNode == nullptr || rotationNode == nullptr
		|| !fieldOfViewNode || !focusDistanceNode || !zoomNode || !focusNode
		|| !tcHourNode || !tcMinuteNode || !tcSecondNode || !tcFrameNode)
	{
		printf("not all template animation nodes are found!\n");
		return false;
	}
		
	auto posX = translationNode->GetCurve(0);
	auto posY = translationNode->GetCurve(1);
	auto posZ = translationNode->GetCurve(2);

	auto rotX = rotationNode->GetCurve(0);
	auto rotY = rotationNode->GetCurve(1);
	auto rotZ = rotationNode->GetCurve(2);

	auto fieldOfViewCurve = fieldOfViewNode->GetCurve(0);
	auto focusDistanceCurve = focusDistanceNode->GetCurve(0);

	auto focusCurve = focusNode->GetCurve(0);
	auto zoomCurve = zoomNode->GetCurve(0);
	auto irisCurve = irisNode->GetCurve(0);

	auto trackPosCurve = trackPosNode->GetCurve(0);
	auto packetNumberCurve = packetNumberNode->GetCurve(0);

	auto tcHourCurve = tcHourNode->GetCurve(0);
	auto tcMinuteCurve = tcMinuteNode->GetCurve(0);
	auto tcSecondCurve = tcSecondNode->GetCurve(0);
	auto tcFrameCurve = tcFrameNode->GetCurve(0);
	auto tcRateCurve = tcRateNode->GetCurve(0);
	
	const int keyCount = cgiConvert.GetNumberOfPackets();
	const bool hasTrimRegion = (endTime > 0.0);
	int realKeyCount = (hasTrimRegion) ? 0 : keyCount;

	if (hasTrimRegion)
	{
		for (int i = 0; i < keyCount; ++i)
		{
			const auto& packet = cgiConvert.GetPacket(i);
			fbx::OFBTime time(packet.timeCode.hours, packet.timeCode.minutes, packet.timeCode.seconds, packet.timeCode.frames, 0, fbx::OFBTimeMode::eCustom, fps);

			// TRIM OPERATION
			const double timeSec = time.GetSecondDouble();
			if (timeSec < startTime)
				continue;
			else if (timeSec > endTime)
				break;

			realKeyCount += 1;
		}
	}

	if (realKeyCount <= 0)
		return false;

	posX->SetKeyCount(realKeyCount);
	posY->SetKeyCount(realKeyCount);
	posZ->SetKeyCount(realKeyCount);

	rotX->SetKeyCount(realKeyCount);
	rotY->SetKeyCount(realKeyCount);
	rotZ->SetKeyCount(realKeyCount);

	// camera attribute processed values
	if (cgiConvert.IsCalibratedCGI())
	{
		fieldOfViewCurve->SetKeyCount(realKeyCount);
		focusDistanceCurve->SetKeyCount(realKeyCount);
	}

	// raw values
	zoomCurve->SetKeyCount(realKeyCount);
	focusCurve->SetKeyCount(realKeyCount);
	irisCurve->SetKeyCount(realKeyCount);
	trackPosCurve->SetKeyCount(realKeyCount);
	packetNumberCurve->SetKeyCount(realKeyCount);
	packetNumberCurve->SetKeyConstFlags();

	tcHourCurve->SetKeyCount(realKeyCount);
	tcHourCurve->SetKeyConstFlags();
	tcMinuteCurve->SetKeyCount(realKeyCount);
	tcMinuteCurve->SetKeyConstFlags();
	tcSecondCurve->SetKeyCount(realKeyCount);
	tcSecondCurve->SetKeyConstFlags();
	tcFrameCurve->SetKeyCount(realKeyCount);
	tcFrameCurve->SetKeyConstFlags();
	tcRateCurve->SetKeyCount(1);
	tcRateCurve->SetKeyConstFlags();
	tcRateCurve->SetKey(0, fbx::OFBTime(0), static_cast<float>(fps));

	realKeyCount = 0;
	for (int i = 0; i < keyCount; ++i)
	{
		const auto& packet = cgiConvert.GetPacket(i);
		fbx::OFBTime time(packet.timeCode.hours, packet.timeCode.minutes, packet.timeCode.seconds, packet.timeCode.frames, 0, fbx::OFBTimeMode::eCustom, fps);

		// TRIM OPERATION
		const double timeSec = time.GetSecondDouble();
		if (hasTrimRegion)
		{
			if (timeSec < startTime)
				continue;
			else if (timeSec > endTime)
				break;
		}

		fbx::FVector4 posXYZ, rotXYZ;
		cgiConvert.ConvertToFBX(packet, posXYZ, rotXYZ);

		posX->SetKey(realKeyCount, time, posXYZ.x);
		posY->SetKey(realKeyCount, time, posXYZ.y);
		posZ->SetKey(realKeyCount, time, posXYZ.z);

		rotX->SetKey(realKeyCount, time, rotXYZ.x);
		rotY->SetKey(realKeyCount, time, rotXYZ.y);
		rotZ->SetKey(realKeyCount, time, rotXYZ.z);

		// processed camera node attribute values
		if (cgiConvert.IsCalibratedCGI())
		{
			fieldOfViewCurve->SetKey(realKeyCount, time, cgiConvert.ConvertFocalLength(packet));
			focusDistanceCurve->SetKey(realKeyCount, time, cgiConvert.ConvertFocusDistance(packet));
		}

		// raw values
		zoomCurve->SetKey(realKeyCount, time, packet.zoom);
		focusCurve->SetKey(realKeyCount, time, packet.focus);
		irisCurve->SetKey(realKeyCount, time, packet.iris);
		trackPosCurve->SetKey(realKeyCount, time, packet.spare.trackPos);
		packetNumberCurve->SetKey(realKeyCount, time, static_cast<float>(packet.packetNumber));

		tcHourCurve->SetKey(realKeyCount, time, static_cast<float>(packet.timeCode.hours));
		tcMinuteCurve->SetKey(realKeyCount, time, static_cast<float>(packet.timeCode.minutes));
		tcSecondCurve->SetKey(realKeyCount, time, static_cast<float>(packet.timeCode.seconds));
		tcFrameCurve->SetKey(realKeyCount, time, static_cast<float>(packet.timeCode.frames));

		realKeyCount += 1;
	}
	return true;
}

/**
 * Load CGI, trim it and save into fbx.
 * 
 * \param buffer - cgi data
 * \param size size of cgi data
 * \return status of the operation
 */
EXTERN int TrimAndExportToFBX(uint8_t* buffer, size_t size, double frameRate, double startTimeSec, double endTimeSec, int isBinary) 
{
	CGIConvert cgiConvert;
	if (!cgiConvert.LoadPackets(buffer, size, static_cast<float>(frameRate))
		|| cgiConvert.IsEmpty())
	{
		printf("Faled to load cgi stream packets or stream has no packets!\n");
		return -1;
	}

	//
	// write into fbx

	fbx::FBXDocument doc;
	fbx::Importer lImporter;

#ifdef __EMSCRIPTEN__
	constexpr const char* templateFilename{ "assets/tdcamera2.fbx" };
#else
	constexpr const char* templateFilename{ "C:\\work\\technocrane\\tdcamera.fbx" };
#endif
	printf("import a template file - %s\n", templateFilename);
	bool lSuccess = lImporter.Initialize(templateFilename);

	if (lSuccess)
	{
		std::cout << "Import" << std::endl;
		lImporter.Import(doc);

		std::cout << "Parse" << std::endl;
		// prepare scene data
		doc.ParseConnections();
		doc.ParseObjects();

		// modify keyframes
		
		std::cout << "Scene retrieve" << std::endl;
		fbx::Scene scene;
		scene.Retrieve(&doc);

		std::cout << "Prepare camera animation" << std::endl;
		if (!PrepareCameraAnimation(scene, cgiConvert, startTimeSec, endTimeSec, frameRate))
		{
			std::cout << "Failed to prepare camera animation" << std::endl;
			return -1;
		}

		std::cout << "Scene store" << std::endl;
		scene.Store(&doc);

		// modify doc global information
		const CGIDataCartesian& firstPacket = cgiConvert.GetPacket(0);
		const CGIDataCartesian& lastPacket = cgiConvert.GetPacket(cgiConvert.GetNumberOfPackets() - 1);
		
		fbx::OFBTime startTime(firstPacket.timeCode.hours, firstPacket.timeCode.minutes, firstPacket.timeCode.seconds, 0, 0, fbx::OFBTimeMode::eCustom, frameRate);
		fbx::OFBTime stopTime(lastPacket.timeCode.hours, lastPacket.timeCode.minutes, lastPacket.timeCode.seconds + 1, 0, 0, fbx::OFBTimeMode::eCustom, frameRate);

		if (startTimeSec > 0.0) startTime.SetSecondDouble(startTimeSec);
		if (endTimeSec > 0.0) stopTime.SetSecondDouble(endTimeSec);

		doc.UpdateHeader();
		doc.UpdateGlobalSettings(startTime.Get(), stopTime.Get(), frameRate);
		doc.UpdateDefinitions();
		doc.UpdateAnimationTakeTime(startTime.Get(), stopTime.Get());

		// save to file
		
#ifdef __EMSCRIPTEN__
		fbx::Exporter	lExporter;

		std::cout << "Writing TDCamera.fbx" << std::endl;
		
		lExporter.Initialize("", false);
		lExporter.Export(doc);

		const std::string filename{ "TDCamera.fbx" };
		const std::string mime_type{ "application/text/plain" };

		printf("Ready to download!\n");
		emscripten_browser_file::download(filename, mime_type, lExporter.GetStreamBuffer());
#else
		fbx::Exporter	lExporter;

		constexpr const char* outputFilename{ "c:\\work\\technocrane\\test-cgi.fbx" };
		std::cout << "Writing " << outputFilename << std::endl;

		lExporter.Initialize(outputFilename, isBinary > 0);
		lExporter.Export(doc);
#endif
	}
	
	return 1;
}


/**
 * main entry point.
 *  Arguments <filename to read> <frameRate> <startTime> <endTime>
 *
 * \return
 */
int main(int argc, char* argv[]) {
	printf("Welcome To A Technocrane Trimmer!\n");

#ifndef __EMSCRIPTEN__

	if (argc < 5)
	{
		printf("Wrong number of arguments, please provide\n");
		printf(" <filename to read> <frameRate> <startTime> <endTime>\n");
		return -1;
	}

	const char* fname{ argv[1] };
	std::ifstream fstream(fname, std::ios::binary | std::ios::ate);
	std::streamsize size = fstream.tellg();

	if (size <= 0)
	{
		printf("Failed to read the file!\n");
		return -1;
	}

	fstream.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(size+1, 0);
	if (fstream.read((char*)buffer.data(), size))
	{
		double frameRate, startTime, endTime;
		sscanf_s(argv[2], "%lf", &frameRate);
		sscanf_s(argv[3], "%lf", &startTime);
		sscanf_s(argv[4], "%lf", &endTime);

		TrimAndExportToFBX(buffer.data(), size, frameRate, startTime, endTime, false);
	}
#endif
	return 0;
}