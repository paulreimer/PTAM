#pragma once

#include "ofxThread.h"
#include "System.h"

class PTAMthread
: public ofxThread
{
public:
	void setup();
	void update();
	void threadedFunction();
	
protected:
	System system;
};
