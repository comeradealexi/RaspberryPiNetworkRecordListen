#include "Network.h"
#include <thread>
#include <RaspberryPiShared.h>
#include <stdio.h>
#include "LANNetworkCheck.h"
#include "VideoRecorder.h"
#include <chrono>
#include "EmailNotifier.h"
#include "StatusUpdate.h"

using namespace std::chrono;

void TestHasher()
{
	const char* cTest = "Alex";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = "Alex1";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = "Alex2";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = "Alex";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = "A";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = "ADASDASD ASD ASDAS ASD DA S";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = "123 1232 321 12 ";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = " ";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = "1 ";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = " 1";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = "1";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest)));
	cTest = "2";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = "3";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest)));
	cTest = "4";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = "  ";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
	cTest = "    ";
	printf("%s = %u\n", cTest, RaspberryPi::VideoSurveillance::CreatePacketHash(cTest, strlen(cTest) ));
}

int main()
{
	Log::Get() << "Starting Network Record Listen";

	//On the RPI, at shutdown the time is saved so on boot, the clocks briefly appear to be the same as what is was when shutdown but if connected to the internet the time will abruptly update!
	Log::Get() << "Sleeping for 5 minutes to give time for clocks to synchronise...";
	std::this_thread::sleep_for(minutes(5)); 
	Log::Get() << "Sleep complete.";
	
	static constexpr const std::chrono::minutes k_statusUpdateInterval = duration_cast<minutes>(hours(6));
	static constexpr const std::chrono::minutes k_MaxRecordTime = minutes(30);
	static constexpr const std::chrono::minutes k_MaxDeleteTimeout = minutes(10);
	static_assert(k_MaxDeleteTimeout > seconds(LANNetworkCheck::k_PingFrequencySeconds), "Ping frequency must be lower than the delete timeout");

	NetworkListen listen;
	LANNetworkCheck lanNetworkCheck({ {"192.168.0.30","Alex"}, {"192.168.0.34","Rebecca"} });
	VideoRecorder videoRecorder(k_MaxDeleteTimeout, k_MaxRecordTime);
	EmailNotifier emailNotifier;
	StatusUpdate statusUpdate(k_statusUpdateInterval);
	while (1)
	{
		DataPacket dp;
		if (listen.GetData(dp))
		{
			if (RaspberryPi::VideoSurveillance::VerifyPacket((const char*)dp.m_pData, dp.m_uiSize))
			{
				const char* pData = RaspberryPi::VideoSurveillance::OffsetByHeader((const char*)dp.m_pData);
				uint32_t dataSize = RaspberryPi::VideoSurveillance::OffsetByHeader(dp.m_uiSize);

				if (dataSize == sizeof(uint32_t))
				{
					RaspberryPi::VideoSurveillance::DoorStatus ds = *((RaspberryPi::VideoSurveillance::DoorStatus*)pData);
					Log::Get() << "Received a new door status update of: " << DoorStatusToString(ds);
					if (ds == RaspberryPi::VideoSurveillance::DoorStatus::Open && lanNetworkCheck.IsAnyValidIpConnected(duration_cast<seconds>(k_MaxDeleteTimeout)) == false)
					{
						statusUpdate.WaitToFinish();
						emailNotifier.SendEmail(ds, videoRecorder.IsRecording() == false);
						videoRecorder.DoRecord();
					}
					else
					{
						if (ds == RaspberryPi::VideoSurveillance::DoorStatus::Open)
							Log::Get() << "Not doing record because a valid ip is connected.";
					}
				}
				else
				{
					Log::Get() << "Packet of data receieved has an unexpected size: " << dp.m_uiSize;
				}
			}
			else
			{
				Log::Get() << "Received a bad packet of data." << "The size was: " << dp.m_uiSize;
			}
		}
		lanNetworkCheck.Update();
		bool bValidIp = lanNetworkCheck.IsAnyValidIpConnected(duration_cast<seconds>(k_MaxDeleteTimeout));
		videoRecorder.Update(bValidIp);
		
		//Don't send status update if the camera is in use.
		if (videoRecorder.IsRecording() == false)
			statusUpdate.Update(bValidIp);

		std::this_thread::sleep_for(milliseconds(400));
	}

	return 0;
}