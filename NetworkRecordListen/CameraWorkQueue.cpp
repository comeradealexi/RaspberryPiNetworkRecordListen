#include "CameraWorkQueue.h"

CameraWorkQueue::CameraWorkQueue() 
{
	m_thread = std::thread([this]
	{
		while (m_finish == false)
		{
			bool bHasWork = false;
			Work workToDo;

			{//Scope for the mutex etc so we can do the work without keeping the mutex locked
				std::unique_lock<std::mutex> lck(m_mtx);

				m_cv.wait(lck, [this] { return m_finish || m_workQueue.size() > 0; });

				if (m_workQueue.size() > 0)
				{
					bHasWork = true;
					workToDo = std::move(m_workQueue.front());
					m_workQueue.pop();
				}
			}
			if (bHasWork)
			{
				workToDo();
			}
		}
	});
}

CameraWorkQueue::~CameraWorkQueue()
{
	m_finish = true;
	m_cv.notify_one();
	m_thread.join();
}

void CameraWorkQueue::AddWork(Work work)
{
	{//Unsure if this scope here is needed, I doubt it.
		std::unique_lock<std::mutex> lck(m_mtx);
		m_workQueue.push(work);
	}
	m_cv.notify_one();
}
