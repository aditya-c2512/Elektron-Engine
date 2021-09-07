#include "ElektronEngine.h"

/*
* NOTES :
* 1. D3D12 uses a left-hand coordinate system. y-axis is up, x-axis is right, and z-axis is forward(into the screen).
* 2.
*/

int main()
{
	ElektronEngine engine;
    while (engine.isRun())
	{
		engine.broadcast();
	}
	return 0;
}