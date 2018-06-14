#include "LANNetworkCheck.h"

#include <thread>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <RaspberryPiLog.h>

static const bool k_bDBGForceNoSuccessfulPings = false;
static constexpr char k_ipDoorLogName[] = "ipDoorLog.csv";
using namespace std::chrono;

#if !defined(DEBUG)
static_assert(k_bDBGForceNoSuccessfulPings == false, "Are you sure you meant to do a release build with this enabled?");
#endif

LANNetworkCheck::LANNetworkCheck(const std::initializer_list<std::pair<std::string, std::string>>& ipList)
{
	for (auto& ip : ipList)
	{
		IPAndTime ipT(ip.first, ip.second);
		m_ipList.push_back(ipT);
	}
}

LANNetworkCheck::~LANNetworkCheck()
{
	m_killThread = true;
	while (m_flagThreadRunning == true)
	{
		std::this_thread::sleep_for(milliseconds(10));
	}
}

void LANNetworkCheck::Update()
{
	if (m_flagThreadRunning == false && m_killThread == false)
	{
		std::thread thr([this]()
		{
			std::array<char, k_TmpBufferSize> buffer;
			while (m_killThread == false)
			{
				auto tpStart = system_clock::now();
				for (auto& ip : m_ipList)
				{
					static const std::string k_strCommandLine = "sudo ping -w 5 -W 5 ";
					std::string cmdLind = k_strCommandLine;
					cmdLind += ip.m_strName;

					FILE* pPipe = popen(cmdLind.c_str(), "r");
					cmdLind.clear();
					if (pPipe)
					{
						while (!feof(pPipe)) 
						{
							buffer = {}; //Clear the buffer for security?
							if (fgets(buffer.data(), k_TmpBufferSize, pPipe) != nullptr)
								cmdLind += buffer.data();

						}
						pclose(pPipe);

						std::string strSearch = "bytes from ";
						strSearch += ip.m_strName;

						if (cmdLind.find(strSearch) != std::string::npos && k_bDBGForceNoSuccessfulPings == false)
						{
							std::cout << ip.m_strName << " - successful ping\n";
							ip.m_lastSuccessfulPing = system_clock::now();
						}
					}
#if defined(DEBUG) && 0
					std::cout << cmdLind << "\n";
#endif
				}
				auto totalTime = duration_cast<seconds>(system_clock::now() - tpStart);

				if (totalTime.count() < k_PingFrequencySeconds)
					std::this_thread::sleep_for(seconds(k_PingFrequencySeconds - totalTime.count()));
			}
			m_flagThreadRunning = false;
		});
		thr.detach();
		m_flagThreadRunning = true;
	}
}

bool LANNetworkCheck::IsAnyValidIpConnected(seconds timeSeconds)
{
	const auto tpNow = system_clock::now();

	for (const auto& ip : m_ipList)
	{
		auto secondsSincePing = duration_cast<seconds>(tpNow - ip.m_lastSuccessfulPing);
		if (secondsSincePing < timeSeconds)
			return true;
	}
	return false;
}

bool fileExists(const char* kSzFileName)
{
	struct stat buf;
	if (stat(kSzFileName, &buf) != -1)
	{
		return true;
	}
	return false;
}

void LANNetworkCheck::LogLastIpTimes()
{
	bool bWriteheader = false;
	if (fileExists(k_ipDoorLogName) == false)
		bWriteheader = true;

	std::ofstream ipLog(k_ipDoorLogName, std::ios_base::out | std::ios_base::app);
	
	if (bWriteheader)
		ipLog << "Ip Address,Last Successful Ping,Current Time\n";
	

	for (const auto& ip : m_ipList)
	{
		ipLog << ip.m_strName << "(" << ip.m_strHumanName << ") " << "," << Log::ToString(ip.m_lastSuccessfulPing)<< "," << Log::ToString(system_clock::now()) << "\n";
	}
}
