// Copyright 2008 Isis Innovation Limited
#include "System.h"
#include "OpenGL.h"
#include <gvars3/instances.h>
#include <stdlib.h>
#include "ATANCamera.h"
#include "MapMaker.h"
#include "Tracker.h"
#include "ARDriver.h"
#include "MapViewer.h"

using namespace CVD;
using namespace std;
using namespace GVars3;

void
System::setup()
{
	mimFrameBW.resize(videoSize);
	mimFrameRGB.resize(videoSize);
  // First, check if the camera is calibrated.
  // If not, we need to run the calibration widget.
  Vector<NUMTRACKERCAMPARAMETERS> vTest;
  
  vTest = GV3::get<Vector<NUMTRACKERCAMPARAMETERS> >("Camera.Parameters", ATANCamera::mvDefaultParams, HIDDEN);
  mpCamera = new ATANCamera("Camera");
  Vector<2> v2;
  if(v2==v2) ;
  if(vTest == ATANCamera::mvDefaultParams)
    {
      cout << endl;
      cout << "! Camera.Parameters is not set, need to run the CameraCalibrator tool" << endl;
      cout << "  and/or put the Camera.Parameters= line into the appropriate .cfg file." << endl;
      exit(1);
    }
  
  mpMap = new Map;
  mpMapMaker = new MapMaker(*mpMap, *mpCamera);
  mpTracker = new Tracker(videoSize, *mpCamera, *mpMap, *mpMapMaker);
  mpARDriver = new ARDriver(*mpCamera, videoSize);
  mpMapViewer = new MapViewer(*mpMap);
  
  mbDone = false;
};

void System::update()
{
	try {
		// We use two versions of each video frame:
		// One black and white (for processing by the tracker etc)
		// and one RGB, for drawing.
		
		static bool bFirstFrame = true;
		if(bFirstFrame)
		{
			mpARDriver->Init();
			bFirstFrame = false;
		}
		
		if(!mpMap->IsGood())
			mpARDriver->Reset();
		
		mpTracker->TrackFrame(mimFrameBW, false);		
	}
	catch(CVD::Exceptions::All e)
	{
		cout << endl;
		cout << "!! Failed to run system; got exception. " << endl;
		cout << "   Exception was: " << endl;
		cout << e.what << endl;
	}
}
