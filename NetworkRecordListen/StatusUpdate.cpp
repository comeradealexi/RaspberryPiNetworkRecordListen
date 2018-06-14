#include <RaspberryPiShared.h>
#include "StatusUpdate.h"
#include <thread>

StatusUpdate::StatusUpdate(std::chrono::minutes updateInterval)
	: m_updateInterval(updateInterval)
	, m_lastTime(std::chrono::system_clock::now())
{

}

StatusUpdate::~StatusUpdate()
{
}

void StatusUpdate::Update(bool bValidIPConnected)
{
	if (bValidIPConnected)
	{
		m_lastTime = std::chrono::system_clock::now();
		return;
	}

	if (m_bIsWorking == false)
	{
		auto timeSpan = std::chrono::system_clock::now() - m_lastTime;

		if (timeSpan > m_updateInterval)
		{
			Log::Get() << "Begining Status Update Send";
			m_bIsWorking = true;
			std::thread thrd([this]
			{
				std::string strPictureName = "StatusPicture";
				strPictureName += std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
				strPictureName += ".jpg";

				std::string systemCall = "raspistill -rot 180 -w 1280 -h 720 -o ";
				systemCall += strPictureName;
				Log::Get() << "Taking Status Picture" << systemCall;
				int iRet = std::system(systemCall.c_str());
				Log::Get() << "Returned code: " << iRet;
				if (iRet >= 0)
				{
					std::stringstream ss;
					ss << "echo \"";
					ss << "StatusUpdate: " << Log::ToString(std::chrono::system_clock::now());
					ss << Log::Return_current_time_and_date();
					ss << "\" ";
					ss << "| mutt -a \"" << strPictureName << "\" ";
					ss << "-s \"" << "Raspberry Pi Status Update" << "\"" << " -- houghton1411@aol.com";
					Log::Get() << "Running following command line: ";
					Log::Get() << ss.str();
					iRet = std::system(ss.str().c_str());
					Log::Get() << "Returned code: " << iRet;
				}
				m_bIsWorking = false;
			});
			thrd.detach();

			m_lastTime = std::chrono::system_clock::now();
		}
	}
}

void StatusUpdate::WaitToFinish()
{
	while (m_bIsWorking)
	{
		Log::Get() << "Waiting for Status Update to Finish...";
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
