#include "VideoRecorder.h"
#include <RaspberryPiLog.h>
#include <unistd.h>
#include <array>
#include <iostream>

static constexpr bool k_dbg_dont_run_command_line = false;
static constexpr bool k_dbg_dont_delete_files = false;

VideoRecorder::VideoRecorder(std::chrono::minutes minutesToDeleteRecordings, std::chrono::minutes minutesMaxRecordTime) : m_minutesToDeleteRecordings(minutesToDeleteRecordings), m_minutesMaxRecordTime(minutesMaxRecordTime)
{
	std::array<char, 256> charBuffer;
	if (getcwd(charBuffer.data(), charBuffer.size()) != nullptr)
		m_CWD = charBuffer.data();

	static bool bOnce = false;
	if (bOnce == false)
	{
		std::atexit([]()
		{
			system("pkill raspivid");
			system("sudo pkill raspivid");
		});
		bOnce = true;
	}
}


VideoRecorder::~VideoRecorder()
{

}

void VideoRecorder::DeleteFilesIfTimeClose()
{
	if (m_bIsRecording == false)
	{
		auto timeNow = std::chrono::system_clock::now();
		for (const auto& f : m_recordList)
		{
			auto timeDiff = timeNow - f.second;

			if (std::chrono::duration_cast<std::chrono::minutes>(timeDiff) > m_minutesToDeleteRecordings)
			{
				if (k_dbg_dont_delete_files == false)
				{
					Log::Get() << "Deleting File Because Valid Ip Was Found";
					Log::Get() << f.first;
					std::remove(f.first.c_str());
				}
			}
		}
		m_recordList.clear();
	}
	else
	{
		Log::Get() << "ERROR: Should Not Be Calling Delete Files While Recording!\n";
	}
}

void VideoRecorder::Update(bool bValidIPConnected)
{
	if (bValidIPConnected)
	{
		if (m_bIsRecording)
		{
			Log::Get() << "Stopping recording because a valid IP connected.";
			StopRecording();
		}

		DeleteFilesIfTimeClose();
	}
}

void VideoRecorder::StopRecording()
{
	Log::Get() << "Stopping Recording";
	system("pkill raspivid");
	system("sudo pkill raspivid");
	m_bIsRecording = false;
}

void VideoRecorder::DoRecord()
{
	if (m_bIsRecording == false)
	{
		Log::Get() << "Starting Record...";
		std::string strFileName = m_CWD;
		strFileName += "/DoorMonitor";
		auto tp = std::chrono::system_clock::now();
		strFileName += std::to_string(std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count());
		strFileName += k_FileExtension;
		m_recordList.push_back({ strFileName, tp });
		Log::Get() << "File name is:";
		Log::Get() << strFileName;

		static constexpr const char* k_cmdLine = "raspivid -w 1280 -h 720 -rot 180 -fps 15 -t 0 -o ";
		std::string strCmdLine = k_cmdLine;
		strCmdLine += strFileName;
		strCmdLine += " &"; //Async

		Log::Get() << "Running command line:" << strCmdLine;

#ifdef DEBUG
		std::cout << "Running Record Command Line: " << strCmdLine << std::endl;
#endif
		if (k_dbg_dont_run_command_line == false)
		{
			auto systemRet = std::system(strCmdLine.c_str());
			Log::Get() << "System call returned code:" << systemRet;
		}

		m_bIsRecording = true;
	}
	else
	{
		Log::Get() << "Already recording";
	}
}