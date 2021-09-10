#include "ElekTimer.h"
#include <Windows.h>

ElekTimer::ElekTimer() : SecondsPerCount(0.0), elekDeltaTime(-1.0), BaseTime(0), PausedTime(0), PrevTime(0), CurrTime(0), Stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	SecondsPerCount = 1.0 / (double)countsPerSec;
}

float ElekTimer::Time() const
{
	if (Stopped)
	{
		return (float)(((StopTime - PausedTime) - BaseTime) * SecondsPerCount);
	}
	else
	{
		return (float)(((CurrTime - PausedTime) - BaseTime) * SecondsPerCount);
	}
}

float ElekTimer::DeltaTime() const
{
	return (float)elekDeltaTime;
}

void ElekTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	BaseTime = currTime;
	PrevTime = currTime;
	StopTime = 0;
	Stopped = false;
}

void ElekTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
	if (Stopped)
	{
		PausedTime += (startTime - StopTime);
		PrevTime = startTime;
		StopTime = 0;
		Stopped = false;
	}
}

void ElekTimer::Stop()
{
	if (!Stopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		StopTime = currTime;
		Stopped = true;
	}
}

void ElekTimer::Tick()
{
	if (Stopped)
	{
		elekDeltaTime = 0.0;
		return;
	}
	// Get the time this frame.
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	CurrTime = currTime;
	elekDeltaTime = (CurrTime - PrevTime) * SecondsPerCount;
	// Prepare for next frame.
	PrevTime = CurrTime;
	if (elekDeltaTime < 0.0)
	{
		elekDeltaTime = 0.0;
	}
}
