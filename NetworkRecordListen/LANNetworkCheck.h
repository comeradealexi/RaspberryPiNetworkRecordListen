#pragma once

#include <string>
#include <initializer_list>
#include <vector>
#include <atomic>
#include <chrono>

struct IPAndTime
{
	IPAndTime(const std::string& str, const std::string& str2) : m_strName(str), m_strHumanName(str2){ }
	std::string m_strName;
	std::string m_strHumanName;
	std::chrono::system_clock::time_point m_lastSuccessfulPing = {};
};

class LANNetworkCheck
{
public:
	static constexpr size_t k_PingFrequencySeconds = 60;
	static constexpr size_t k_TmpBufferSize = 128;

	LANNetworkCheck(const std::initializer_list<std::pair<std::string,std::string>>& ipList);
	~LANNetworkCheck();

	void Update();
	bool IsAnyValidIpConnected(std::chrono::seconds timeSeconds);
	void Kill() { m_killThread = true; }
	void LogLastIpTimes();
	const std::vector<IPAndTime>& GetIpList() const { return m_ipList; }

private:
	std::vector<IPAndTime> m_ipList;
	std::atomic_bool m_flagThreadRunning{ false };
	std::atomic_bool m_killThread{ false };
};

