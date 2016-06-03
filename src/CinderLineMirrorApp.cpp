#include "cinder/app/AppNative.h"
#include "cinder/Rand.h"

#include "cinder/Capture.h"

#include "cinder/Camera.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"

#include "cinder/gl/Fbo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define LINE_COUNT 400
#define SEG_COUNT 500

class CinderLineMirrorApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    void drawFBO();
    void initVBOS();
    void keyDown( KeyEvent event );
    CaptureRef			mCapture;
    gl::TextureRef		mTexture;
    gl::VboMeshRef		mLineVBOs[LINE_COUNT];
//    Font mFont;
    CameraPersp		mCamera;
    
    //fbo shit
    gl::Fbo				mFbo;
    static const int	FBO_WIDTH = 1920*2, FBO_HEIGHT = 1080*2;
};

void CinderLineMirrorApp::setup()
{
//    getRenderer()->setFrameSize(1920, 1080);
    //init the capture
    try {
        mCapture = Capture::create( 640, 480 );
        mCapture->start();
    }
    catch( ... ) {
        console() << "Failed to initialize capture" << std::endl;
    }
    gl::Fbo::Format format;
    format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
    format.enableDepthBuffer(false);
    mFbo = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
//    mFont = Font( "Quicksand Book Regular", 12.0f );

    initVBOS();
    hideCursor();
    setFullScreen(true);
}

void CinderLineMirrorApp::initVBOS()
{
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticColorsRGB();
    layout.setDynamicPositions();
    for(int j = 0; j < LINE_COUNT; j++)
    {
        mLineVBOs[j] = gl::VboMesh::create( SEG_COUNT, SEG_COUNT, layout, GL_LINE_STRIP );
        vector<uint32_t> indices;
        vector<Color> colors;
        int colorIndex = j % 3;
        Color c;
        if(colorIndex==0)
            c =Color(1, 0, 0);
        else if(colorIndex==1)
            c =Color(0, 1, 0);
        else if(colorIndex==2)
            c =Color(0, 0, 1);
        
        for( int i = 0; i < SEG_COUNT; ++i )
        {
            indices.push_back( i );
            colors.push_back(c);
        }
        mLineVBOs[j]->bufferIndices( indices );
        mLineVBOs[j]->bufferColorsRGB( colors );
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
        int windW = FBO_WIDTH;//getWindowWidth();
        int windH = FBO_HEIGHT;//getWindowHeight();
        
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
//        mTexture = gl::Texture::create( mCapture->getSurface() );
    }
    
}


void CinderLineMirrorApp::drawFBO()
{
    //save (and eventually restore) the current fbo binding.
//    gl::SaveFramebufferBinding bindingSaver;
    
    // bind the framebuffer - now everything we draw will go there
//    mFbo.bindFramebuffer();
//        Color c =Color( Rand().nextFloat(255),Rand().nextFloat(1),Rand().nextFloat(1) );
//        gl::clear( c );
    gl::clear();
//    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    gl::setMatricesWindow( FBO_WIDTH, FBO_HEIGHT );
    gl::lineWidth(.4);
    
//  Vec2i v(0,getWindowHeight()*2);
    
    //    if( mTexture )
    {
        glPushMatrix();
//        gl::translate(v);
//        gl::scale(1,-1);
        //        gl::translate( Vec3f( getMousePos().x, getMousePos().y,0 ) );
        //        gl::draw( mTexture );
        for(int i = 0; i < LINE_COUNT; i++)
        {
            gl::draw( mLineVBOs[i] );
        }
        glPopMatrix();
    }
//    mFbo.unbindFramebuffer();
    //    gl::drawString( "Framerate: " + to_string(getAverageFps()), Vec2f( 10.0f, 10.0f ), Color::white(), mFont );

}
void CinderLineMirrorApp::draw()
{
    drawFBO();
//    gl::clear(Color(0,0,0 ));
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    
//    gl::draw( mFbo.getTexture(0), Rectf(0, 0, 2*getWindowWidth(), 2*getWindowHeight() ) );
}

void CinderLineMirrorApp::keyDown( KeyEvent event )
{
    if( event.getChar() == 'f' )
    {
        setFullScreen(!isFullScreen());
        if(isFullScreen())
            hideCursor();
        else
            showCursor();
    }
    else if(event.getChar() == cinder::app::KeyEvent::KEY_ESCAPE)
    {
        exit(0);
    }
}

CINDER_APP_NATIVE( CinderLineMirrorApp, RendererGl )
