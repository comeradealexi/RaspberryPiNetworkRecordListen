#pragma once
#include <chrono>
#include <atomic>

class StatusUpdate
{
public:
	StatusUpdate(std::chrono::minutes updateInterval);

	~StatusUpdate();
	void Update(bool bValidIPConnected);
	void WaitToFinish();

private:
	std::atomic_bool m_bIsWorking = {false};
	std::chrono::minutes m_updateInterval;
	std::chrono::system_clock::time_point m_lastTime;
};

