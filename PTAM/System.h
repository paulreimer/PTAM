// -*- c++ -*-
// Copyright 2008 Isis Innovation Limited
//
// System.h
//
// Defines the System class
//
// This stores the main functional classes of the system, like the
// mapmaker, map, tracker etc, and spawns the working threads.
//
#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <cvd/image.h>
#include <cvd/rgb.h>
#include <cvd/byte.h>

class ATANCamera;
class Map;
class MapMaker;
class Tracker;
class ARDriver;
class MapViewer;

class testApp;

class System
{
	friend class testApp;
public:
	void setup();
	void update();
	void draw();
	void drawFrameFullscreen();
	
protected:
	CVD::Image<CVD::Rgb<CVD::byte> > mimFrameRGB;
	CVD::Image<CVD::byte> mimFrameBW;
	Tracker *mpTracker; 

	ATANCamera *mpCamera;

	CVD::ImageRef videoSize;

private:
  Map *mpMap; 
  MapMaker *mpMapMaker; 
  ARDriver *mpARDriver;
  MapViewer *mpMapViewer;
  
  bool mbDone;
};



#endif
