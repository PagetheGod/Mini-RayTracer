#pragma once

#include <chrono>


/*
* A simple wrapper class for a timer that can be used to test performace, right now it ONLY support recording one duration
* Starting the timer without storing the last duration first will result the older record being overwritten
* This could be done using the WIN32 API, which might be better for windows apps, which is what we are writing right now
*/



using STDTimePoint = std::chrono::high_resolution_clock::time_point;


class VTimer
{
public:
	VTimer() = default;
	void Start();
	long long int GetTimeElapsed() const;
	long long int GetLastDuration() const
	{
		return m_LastDuration;
	}

	void Stop();
private:
	//Store the time point 
	STDTimePoint m_StartTime;
	STDTimePoint m_EndTime;
	long long int m_LastDuration = 0;
	bool m_HasStarted = false;
};