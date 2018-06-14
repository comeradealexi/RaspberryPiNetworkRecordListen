#pragma once
#include <chrono>
#include <string>
#include <vector>

class VideoRecorder
{
public:
	static constexpr const char* k_FileExtension = ".h264";
	
	VideoRecorder(std::chrono::minutes minutesToDeleteRecordings, std::chrono::minutes minutesMaxRecordTime);
	~VideoRecorder();

	void Update(bool bValidIPConnected);
	void DoRecord();
	bool IsRecording() const { return m_bIsRecording; }
private:
	void StopRecording();
	void DeleteFilesIfTimeClose();

private:
	bool m_bIsRecording { false };
	std::string m_CWD;
	std::vector<std::pair<std::string, std::chrono::system_clock::time_point>> m_recordList;
	std::chrono::minutes m_minutesToDeleteRecordings;
	std::chrono::minutes m_minutesMaxRecordTime;
};

