#include "EmailNotifier.h"
#include <fstream>
#include <ctime>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>
#include <future>

static constexpr std::chrono::minutes k_timeToSendEmailAfterMinutes = std::chrono::minutes(2);
static constexpr char k_emailFileName[] = "PiSSMTPEmail.txt";
static constexpr char k_emailSendTo[] = "houghton1411@aol.com";

EmailNotifier::EmailNotifier()
{
}


EmailNotifier::~EmailNotifier()
{
}

//echo "This is the message body" | mutt - a "still.jpg" - s "subject of message" --houghton1411@aol.com

void EmailNotifier::SendEmail(RaspberryPi::VideoSurveillance::DoorStatus ds, bool bTakePicture)
{
	static std::chrono::system_clock::time_point StillPictureTimer;


	if ((std::chrono::system_clock::now() - StillPictureTimer) > std::chrono::seconds(30) && bTakePicture)
	{
		StillPictureTimer = std::chrono::system_clock::now();
		//Start by taking a picture and emailing it.
		std::string strPictureName = "StillPicture";
		strPictureName += std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
		strPictureName += ".jpg";


		std::string systemCall = "raspistill -rot 180 -w 1280 -h 720 -o ";
		systemCall += strPictureName;
		Log::Get() << "Taking Picture" << systemCall;
		int iRet = std::system(systemCall.c_str());
		Log::Get() << "Returned code: " << iRet;
		if (iRet >= 0)
		{
			std::stringstream ss;
			ss << "echo \"";
			ss << "Door status is now: " << RaspberryPi::VideoSurveillance::DoorStatusToString(ds);
			ss << Log::Return_current_time_and_date();
			ss << "\" ";
			ss << "| mutt -a \"" << strPictureName << "\" ";
			ss << "-s \"" << "Raspberry Pi Door Triggered" << "\"" << " -- houghton1411@aol.com";
			Log::Get() << "Running following command line: ";
			Log::Get() << ss.str();
			iRet = std::system(ss.str().c_str());
			Log::Get() << "Returned code: " << iRet;
		}
	}
	else
	{
		if (bTakePicture)
			Log::Get() << "Not sending still picture as already sent one in last 30 seconds.";
		else
			Log::Get() << "Cannot send picture as video is recording";
	}

}
#if 0
void EmailNotifier::Update(DoorStatusList& dsList, const LANNetworkCheck& lanNetwork, bool bValidIpCurrently)
{
	if (bValidIpCurrently)
	{
		if (dsList.size())
		{
			dsList.clear();
		}
	}
	else
	{
		bool bAllOkayOrSent = true;
		for (auto& ds : dsList)
		{
			if (ds.m_bVerifiedOkay == false && ds.m_bEmailSent == false)
			{
				bAllOkayOrSent = false;
		
				if (std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - ds.m_time) > k_timeToSendEmailAfterMinutes)
				{
					Log::Get() << "Sending an email at ";
					Log::Get() << Log::ToString(std::chrono::system_clock::now());

					//send email
					std::stringstream strEmail;
					strEmail << "Subject: PiCameraProject ";
					strEmail << "\n";
					strEmail << "This Door Status:\n";
					strEmail << DoorStatusToString(ds.m_status);
					strEmail << "Time of door event:\n";
					strEmail << Log::ToString(ds.m_time) << "\n";
					strEmail << "Time Now:\n";
					strEmail << Log::ToString(std::chrono::system_clock::now()) << "\n";
					strEmail << "Ip Info (last successful pings):\n";



					for (const auto& ip : lanNetwork.GetIpList())
					{
						strEmail << ip.m_strName << " : ";
						strEmail << Log::ToString(ip.m_lastSuccessfulPing);
						strEmail << "\n";
					}


					std::ofstream outFile(k_emailFileName);
					outFile << strEmail.str();
					outFile.close();

					std::string strSendEmail = "ssmtp ";
					strSendEmail += k_emailSendTo;
					strSendEmail += " < ";
					strSendEmail += k_emailFileName;

					Log::Get() << "Email to be sent is: ";
					Log::Get() << "----START EMAIL----";
					Log::Get() << strEmail.str();
					Log::Get() << "----END EMAIL----";
					Log::Get() << "Command line to be run is : ";
					Log::Get() << strSendEmail;

					auto futureAsync = std::async([&]() -> int
					{
						return std::system(strSendEmail.c_str());
					});

					const int iTimeout = 20;
					int iGotValue;
					if (futureAsync.wait_for(std::chrono::seconds(iTimeout)) == std::future_status::ready)
					{
						iGotValue = futureAsync.get();
					}
					else
					{
						iGotValue = -1;
						system("pkill ssmtp");
						system("sudo pkill ssmtp");
						Log::Get() << "Timeout occured, took more than " << iTimeout << " seconds to complete";
					}

					Log::Get() << "Email system call returned code: " << iGotValue;

					ds.m_bEmailSent = true;
				}
				else
				{

				}
			}
		}

		if (bAllOkayOrSent)
			dsList.clear();
	}
}
#endif