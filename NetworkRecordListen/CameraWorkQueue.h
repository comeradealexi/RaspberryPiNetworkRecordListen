#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

class CameraWorkQueue
{
	using Work = std::function<void()>;
public:
	CameraWorkQueue();
	~CameraWorkQueue();
	void AddWork(Work work);

private:
	std::thread m_thread;
	std::queue<Work> m_workQueue;
	std::condition_variable m_cv;
	std::mutex m_mtx;
	std::atomic_bool m_finish = { false };
};