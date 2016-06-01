#include "cinder/app/AppNative.h"
#include "cinder/Rand.h"

#include "cinder/Capture.h"

#include "cinder/Camera.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define LINE_COUNT 300
#define SEG_COUNT 500

class CinderLineMirrorApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    void initVBOS();
    
    CaptureRef			mCapture;
    gl::TextureRef		mTexture;
    gl::VboMeshRef		mLineVBOs[LINE_COUNT];
    
    CameraPersp		mCamera;
};

void CinderLineMirrorApp::setup()
{
    //init the capture
    try {
        mCapture = Capture::create( 640, 480 );
        mCapture->start();
    }
    catch( ... ) {
        console() << "Failed to initialize capture" << std::endl;
    }
    initVBOS();
}

void CinderLineMirrorApp::initVBOS()
{
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
//    layout.setStaticColorsRGB();
    layout.setDynamicPositions();
    for(int j = 0; j < LINE_COUNT; j++)
    {
        mLineVBOs[j] = gl::VboMesh::create( SEG_COUNT, SEG_COUNT, layout, GL_LINE_STRIP );
        vector<uint32_t> indices;
        vector<Color> colors;

        for( int i = 0; i < SEG_COUNT; ++i )
        {
            indices.push_back( i );
    //        colors.push_back(Color(Rand().nextFloat(1),Rand().nextFloat(1),Rand().nextFloat(1)));
        }
        mLineVBOs[j]->bufferIndices( indices );
        //    mLineVBOs[0]->bufferColorsRGB( colors );
    }
    
}
void CinderLineMirrorApp::mouseDown( MouseEvent event )
{
}

void CinderLineMirrorApp::update()
{
//    gl::setMatrices( mCamera );
    if( mCapture && mCapture->checkNewFrame() ) {
        //update verts
        Surface8u mySurface = mCapture->getSurface();
        
        int mySurface_height = mySurface.getHeight();
        int mySurface_width = mySurface.getHeight();
        
        u_char* pix = mySurface.getData();
        
        float t = getElapsedSeconds();
        int windW = getWindowWidth();
        int windH = getWindowHeight();
        
        float lineMaxAmp = 6.f;
        float maxLineFreq =2.7;
        
        u_int8_t colorOffset[3] = {mySurface.getRedOffset(),
                                   mySurface.getGreenOffset(),
                                   mySurface.getBlueOffset()};
        
        for(int j = 0; j < LINE_COUNT; j++)
        {
            //which color - red green or blue, this line represents
            u_int8_t colorIndex = colorOffset[j % 3];
            
            //value fed into the sine function
            float tVal = 0.f;
            // dynmaically generate our new positions based on a simple sine wave
            gl::VboMesh::VertexIter iter = mLineVBOs[j]->mapVertexBuffer();
            for( int i = 0; i < SEG_COUNT; ++i )
            {

                //normalized position for this vert
                Vec2f norm(i*1.f/SEG_COUNT,j*1.f/LINE_COUNT);
                //the index of the (start of the) pixel
                int pixIndex = (int)((int)(norm.y*mySurface_height+1) *mySurface_width*4 - norm.x*mySurface_width*4);
                
                //normalized pixel brightness
                float brightnessNorm = pix[pixIndex+colorIndex]/255.f;
                tVal += brightnessNorm*maxLineFreq;
                iter.setPosition( Vec3f(i*windW*1.f/(SEG_COUNT-1),
                                        j*windH*1.f/(LINE_COUNT-1)+lineMaxAmp*sin(t+tVal),
                                        0) );
                ++iter;
            }
        }
        mTexture = gl::Texture::create( mCapture->getSurface() );
    }
}

void CinderLineMirrorApp::draw()
{
//    Color c =Color( Rand().nextFloat(255),Rand().nextFloat(1),Rand().nextFloat(1) );
//    gl::clear( c );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    gl::lineWidth(.4);
//    if( mTexture )
    {
        glPushMatrix();
        
//        gl::translate( Vec3f( getMousePos().x, getMousePos().y,0 ) );
//        gl::draw( mTexture );
        for(int i = 0; i < LINE_COUNT; i++)
        {
            int colorIndex = i % 3;
            if(colorIndex==0) gl::color(1, 0, 0);
            if(colorIndex==1) gl::color(0, 1, 0);
            if(colorIndex==2) gl::color(0, 0, 1);
            gl::draw( mLineVBOs[i] );
        }
        glPopMatrix();
    }
}

CINDER_APP_NATIVE( CinderLineMirrorApp, RendererGl )
