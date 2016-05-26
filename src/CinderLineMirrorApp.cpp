#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderLineMirrorApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void CinderLineMirrorApp::setup()
{
}

void CinderLineMirrorApp::mouseDown( MouseEvent event )
{
}

void CinderLineMirrorApp::update()
{
}

void CinderLineMirrorApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( CinderLineMirrorApp, RendererGl )
