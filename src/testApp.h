#pragma once

#include "ofMain.h"

#include "ofx3DModelLoader.h"

#ifdef USE_WEB_SOCKETS
#include "ThreadedWebSocketServer.h"
#include "WebSocketHandler.h"
#endif

#include <cvd/image.h>
#include <cvd/byte.h>
#include <TooN/TooN.h>

//#include "PTAMthread.h"
#include "System.h"
typedef System PTAMthread;

#define _USE_LIVE_VIDEO

class testApp
: public ofBaseApp
{
public:
	void setup();
	void exit();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);

	void GetAndFillFrameBWandRGB(CVD::Image<CVD::byte> &imBW,
								 CVD::Image<CVD::Rgb<CVD::byte> > &imRGB);
	
//	void SetupVideoOrtho(double w, double h);
//	void SetupVideoRasterPosAndZoom(double w, double h);
	
#ifdef _USE_LIVE_VIDEO
	ofVideoGrabber 		vidGrabber;
#else
	ofVideoPlayer 		vidPlayer;
#endif
	
	PTAMthread ptam;
	CVD::ImageRef videoSize;
	
	ofTrueTypeFont font;
	
	bool bDrawMap;
	bool bDrawAR;
	bool bDrawFASTCorners;
	
	ofx3DModelLoader model;
};
