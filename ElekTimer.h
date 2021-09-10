#pragma once
class ElekTimer
{
public :
	ElekTimer();

	float Time() const; //Gives Time in seconds
	float DeltaTime() const; //Gives Delta Time in seconds

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	double SecondsPerCount;
	double elekDeltaTime;
	__int64 BaseTime;
	__int64 PausedTime;
	__int64 StopTime;
	__int64 PrevTime;
	__int64 CurrTime;
	bool Stopped;
};