#include "PTAMthread.h"

//--------------------------------------------------------------
void PTAMthread::setup()
{
	startThread(true, false); // blocking, non-verbose
}

//--------------------------------------------------------------
void PTAMthread::update()
{
	if (lock())
	{
		system.update();
		unlock();
	}
}

//--------------------------------------------------------------
void PTAMthread::threadedFunction()
{}
