#include "Public/Timer.h"

using namespace std::chrono;

void VTimer::Start()
{
	m_StartTime = high_resolution_clock::now();
	m_EndTime = STDTimePoint{};
	m_LastDuration = 0;
	m_HasStarted = true;
}

//Returns the time elapsed in Ms because WTF MSVC. Might be a good idea to add in option to allow user to choose ms/s
long long int VTimer::GetTimeElapsed() const
{
	if (!m_HasStarted)
	{
		return 0;
	}
	else
	{
		high_resolution_clock::duration TimeElapsed = high_resolution_clock::now() - m_StartTime;
		return duration_cast<milliseconds>(TimeElapsed).count();
	}
}

void VTimer::Stop()
{
	m_HasStarted = false; 
	//By default the duration is in nanoseconds, so we have to do this. Come on WTF
	high_resolution_clock::duration TimeElapsed = high_resolution_clock::now() - m_StartTime;
	m_LastDuration = duration_cast<milliseconds>(TimeElapsed).count();
	m_EndTime = high_resolution_clock::now();
}
