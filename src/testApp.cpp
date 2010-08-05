#include "testApp.h"

#include "MapMaker.h"
#include "Tracker.h"
#include "TrackerData.h"
#include "MapViewer.h"

#include <cvd/gl_helpers.h>
#include <cvd/image_convert.h>

#include <gvars3/instances.h> 

using namespace TooN;

#define _USE_LIVE_VIDEO
#define VIDEO_WIDTH		640
#define VIDEO_HEIGHT	480
#define VIDEO_SIZE		VIDEO_WIDTH,VIDEO_HEIGHT

GLfloat lightOnePosition[] = {40.0, 40, 100.0, 0.0};
GLfloat lightOneColor[] = {0.99, 0.99, 0.99, 1.0};

GLfloat lightTwoPosition[] = {-40.0, 40, 100.0, 0.0};
GLfloat lightTwoColor[] = {0.99, 0.99, 0.99, 1.0};

/// add a translation specified from the first three coordinates of a vector
/// @param v the translation vector
/// @ingroup gGL
inline void glTranslate(const Vector<> & v)
{
	glTranslated(v[0], v[1], v[2]);
}

/// multiply a TooN 3x3 matrix onto the current matrix stack. The GL matrix
/// last column and row are set to 0 with the lower right element to 1.
/// The matrix is also transposed to account for GL's column major format.
/// @param m the transformation matrix
/// @ingroup gGL
inline void glMultMatrix(const Matrix<3,3> &m)
{
	GLdouble glm[16];
	glm[0] = m[0][0]; glm[1] = m[1][0]; glm[2] = m[2][0]; glm[3] = 0;
	glm[4] = m[0][1]; glm[5] = m[1][1]; glm[6] = m[2][1]; glm[7] = 0;
	glm[8] = m[0][2]; glm[9] = m[1][2]; glm[10] = m[2][2]; glm[11] = 0;
	glm[12] = 0; glm[13] = 0; glm[14] = 0; glm[15] = 1;
	glMultMatrixd(glm);
}

inline void glMultMatrix(const SO3<> &so3)
{
	glMultMatrix(so3.get_matrix());
}

inline void glMultMatrix(SE3<> se3)
{
	glTranslate(se3.get_translation());
	glMultMatrix(se3.get_rotation());
}


//--------------------------------------------------------------
void
testApp::setup()
{
	bDrawMap	= false;
	bDrawAR		= false;
	bDrawFASTCorners = true;
	
	ofBackground(0, 0, 0);
	ofSetBackgroundAuto(true);
	//ofDisableSetupScreen();
	
	font.loadFont("fonts/Calluna.ttf", 12);

	CVD::ImageRef videoSize(VIDEO_SIZE);
	ptam.videoSize = videoSize;

#ifdef _USE_LIVE_VIDEO
	vidGrabber.setVerbose(true);
	vidGrabber.initGrabber(VIDEO_SIZE);
	cout << "Set up vidgrabber of size (" << videoSize.x << "," << videoSize.y << ")" << endl;
#else
	vidPlayer.loadMovie("fingers.mov");
	vidPlayer.play();
#endif
	
	GVars3::GUI.LoadFile(ofToDataPath("camera.cfg"));
	GVars3::GUI.StartParserThread();
	
	ptam.setup();
	
	//some model / light stuff
    glEnable (GL_DEPTH_TEST);
    glShadeModel (GL_SMOOTH);
	
    /* initialize lighting */
    glLightfv (GL_LIGHT0, GL_POSITION, lightOnePosition);
    glLightfv (GL_LIGHT0, GL_DIFFUSE, lightOneColor);
    glEnable (GL_LIGHT0);
    glLightfv (GL_LIGHT1, GL_POSITION, lightTwoPosition);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, lightTwoColor);
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);
    glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
    glEnable (GL_COLOR_MATERIAL);
	
    //load the squirrel model - the 3ds and the texture file need to be in the same folder
    model.loadModel("squirrel/NewSquirrel.3ds", 20);
	
    //you can create as many rotations as you want
    //choose which axis you want it to effect
    //you can update these rotations later on
    model.setRotation(0, 90, 1, 0, 0);
    model.setRotation(1, 270, 0, 0, 1);
    model.setScale(0.9, 0.9, 0.9);
    model.setPosition(ofGetWidth()/2, ofGetHeight()/2, 0);	
}

//--------------------------------------------------------------
void
testApp::exit()
{
	GVars3::GUI.StopParserThread();
}

//--------------------------------------------------------------
void
testApp::update()
{
    bool bNewFrame = false;
	
#ifdef _USE_LIVE_VIDEO
	vidGrabber.grabFrame();
	bNewFrame = vidGrabber.isFrameNew();
#else
	vidPlayer.idleMovie();
	bNewFrame = vidPlayer.isFrameNew();
#endif
	
	if (bNewFrame)
	{
		GetAndFillFrameBWandRGB(ptam.mimFrameBW, ptam.mimFrameRGB);
		ptam.update();
	}
	
	model.setRotation(1, 270 + ofGetElapsedTimef() * 60, 0, 0, 1);
}

//--------------------------------------------------------------
void
testApp::draw()
{
	float w = ofGetWidth();
	float h = ofGetHeight();

	Tracker& tracker = *ptam.mpTracker;

	ofPoint scale;
	glPushMatrix();
	{
		glTranslatef(0., 0., -1.);
#ifdef _USE_LIVE_VIDEO
		vidGrabber.draw(0,0,w,h);
		scale.set(w/vidGrabber.getWidth(),
				  h/vidGrabber.getHeight());
#else
		vidPlayer.draw(0,0,w,h);
		scale.set(w/vidPlayer.getWidth(),
				  h/vidPlayer.getHeight());
#endif
	}
	glPopMatrix();

	glPushMatrix();
	{
		glScalef(scale.x, scale.y, 1.);

		if(tracker.mnInitialStage==Tracker::TRAIL_TRACKING_STARTED)
		{
			glPointSize(5);
			glLineWidth(2);
			glEnable(GL_POINT_SMOOTH);
			glEnable(GL_LINE_SMOOTH);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glBegin(GL_LINES);
			
			list<Trail>& trails = tracker.mlTrails;
			Level& level = tracker.mCurrentKF.aLevels[0];
			for(list<Trail>::iterator i = trails.begin(); i!=trails.end();)
			{
				list<Trail>::iterator next = i; next++;
				Trail &trail = *i;
				CVD::ImageRef irStart = trail.irCurrentPos;
				CVD::ImageRef irEnd = irStart;

				bool bFound = trail.mPatch.FindPatch(irEnd, level.im, 10, level.vCorners);

				if(!bFound)
					glColor3f(0,1,1); // Failed trails flash purple before dying.
				else
					glColor3f(1,1,0);
				glVertex(trail.irInitialPos);
				if(bFound)
					glColor3f(1,0,0);
				glVertex(trail.irCurrentPos);
				
				i = next;
			}
			glEnd();
		}
		else if (bDrawMap)
			ptam.mpMapViewer->DrawMap(tracker.GetCurrentPose());
	//	else if(bDrawAR)
	//		ptam.mpARDriver->Render(ptam.mimFrameRGB, tracker.GetCurrentPose());
		else if (tracker.mMap.IsGood() && tracker.mnLostFrames < 3)
		{
			vector<TrackerData*>& tdata = tracker.vIterationSet;
			vector<TrackerData*>::reverse_iterator it;
			for(it = tdata.rbegin(); it!= tdata.rend(); it++)
			{
				if(! (*it)->bFound)
					continue;
	//			glColor(gavLevelColors[(*it)->nSearchLevel]);
	//			glVertex((*it)->v2Image);
				glColor3d(gavLevelColors[(*it)->nSearchLevel][0],
						  gavLevelColors[(*it)->nSearchLevel][1],
						  gavLevelColors[(*it)->nSearchLevel][2]);

				ofCircle((*it)->v2Image[0],
						 (*it)->v2Image[1],
						 3);
			}
		}
		else {
			Level& level = tracker.mCurrentKF.aLevels[0];
			if(bDrawFASTCorners)
			{
				ofSetColor(0xff00ff);
				for(unsigned int i=0; i<level.vCorners.size(); i++)
					ofCircle(level.vCorners[i][0],
							 level.vCorners[i][1],
							 1);
			}

			if(tracker.mMap.IsGood())
				tracker.RenderGrid();
		}
	}
	glPopMatrix();

	string caption;
	if(bDrawMap)
		caption = ptam.mpMapViewer->GetMessageForUser();
	else
		caption = tracker.GetMessageForUser();

	ofSetColor(0xffffff);
	font.drawString(caption, 4, ofGetHeight() - 8);
	
	//lets tumble the world with the mouse
	glPushMatrix();
	{
		SE3<> cam = tracker.GetCurrentPose();

		// Set up 3D projection
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
//		glMultMatrix(ptam.mpCamera->MakeUFBLinearFrustumMatrix(0.005, 100));
		glMultMatrix(cam);
		
//		LookAt(i, cam.inverse().get_translation(), 0.02 );
		
//		glLoadIdentity();
//		glMultMatrix(ase3WorldFromEye[i]);

		ofSetColor(0xffffff);
		model.draw();
	}
	glPopMatrix();
}

//--------------------------------------------------------------
void
testApp::keyPressed(int key)
{
	switch (key)
	{
		case ' ':
			ptam.mpTracker->mbUserPressedSpacebar = true;
			break;
		case 'r':
			ptam.mpTracker->Reset();
			break;
		case 's':
#ifdef _USE_LIVE_VIDEO
			vidGrabber.videoSettings();
#else
			vidPlayer.videoSettings();
#endif
			break;
	}
}

//--------------------------------------------------------------
void
testApp::keyReleased(int key)
{}

//--------------------------------------------------------------
void
testApp::mouseMoved(int x, int y)
{}

//--------------------------------------------------------------
void
testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void
testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void
testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void
testApp::windowResized(int w, int h)
{}

//--------------------------------------------------------------
void
testApp::GetAndFillFrameBWandRGB(CVD::Image<CVD::byte> &imBW,
								 CVD::Image<CVD::Rgb<CVD::byte> > &imRGB)
{
	unsigned char*	pixels	= NULL;
	unsigned int	size	= 0;

#ifdef _USE_LIVE_VIDEO
	pixels	= vidGrabber.getPixels();
	size	= vidGrabber.width*vidGrabber.height*3;
#else
	pixels	= vidPlayer.getPixels();
	size	= vidPlayer.width*vidPlayer.height*3;
#endif

	memcpy(imRGB.data(), pixels, size);
	CVD::convert_image(imRGB, imBW);
}
