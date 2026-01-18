#pragma once

#include <chrono>


/*
* A simple wrapper class for a timer that can be used to test performace, right now it ONLY support recording one duration
* Starting the timer without storing the last duration first will result the older record being overwritten
* This could be done using the WIN32 API, which might be better for windows apps, which is what we are writing right now
*/



using STDTimePoint = std::chrono::steady_clock::time_point;
using namespace std::chrono;

class VTimer
{
public:
	VTimer() = default;
	void Start();
	double GetTimeElapsed() const;
	double GetLastDuration() const
	{
		return m_LastDuration;
	}

	void Stop();
private:
	//Store the time point 
	STDTimePoint m_StartTime;
	STDTimePoint m_EndTime;
	double m_LastDuration = 0.0;
	bool m_HasStarted = false;
};