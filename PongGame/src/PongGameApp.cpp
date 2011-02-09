#include "cinder/app/AppCocoaTouch.h"
#include "cinder/app/Renderer.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;

class PongGame : public AppCocoaTouch {
public:
	virtual void	setup();
	virtual void	resize( ResizeEvent event );
	virtual void	update();
	virtual void	draw();
	virtual void	mouseDown( MouseEvent event );
	
	CameraPersp	mCam;
	Vec2f pos, vel, acc;
	float wid, hei, rad;
};

void PongGame::setup()
{
	pos = Vec2f(0,0);
	vel = Vec2f( Rand::randFloat(6),Rand::randFloat(6) );
	acc = Vec2f(0,0);
	wid = getWindowWidth() * 0.5f;
	hei = getWindowHeight() * 0.5f;
	rad = 25.0f;
}

void PongGame::resize( ResizeEvent event )
{
	mCam.lookAt( Vec3f( 0, 0, -hei * 2.2f ), Vec3f::zero(), Vec3f(0,-1,0) );
	mCam.setPerspective( 60, event.getAspectRatio(), 1, 2000 );
}

void PongGame::mouseDown( MouseEvent event )
{
}

void PongGame::update()
{
	vel += acc;
	pos += vel;
	
	if(pos.x < -wid + rad){
		vel.x *= -1.0f;
		pos.x = -wid + rad;
	} else if(pos.x > wid - rad){
		vel.x *= -1.0f;
		pos.x = wid - rad;
	}
	
	if(pos.y < -hei + rad){
		vel.y *= -1.0f;
		pos.y = -hei + rad;
	} else if(pos.y > hei - rad){
		vel.y *= -1.0f;
		pos.y = hei - rad;
	}
}

void PongGame::draw()
{
	gl::clear( Color( 1.0f, 0.5f, 0.15f ) );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	gl::setMatrices( mCam );
	gl::color( ColorAf(0,0,0,0.85f) );
	gl::drawSolidRect( Rectf(-wid, -hei, wid, hei) );
	gl::color(Colorf(1,1,1));
	gl::drawStringCentered("Hello World", Vec2f(0, hei));
	gl::drawSolidCircle(pos, rad ,32);
	
	
}

CINDER_APP_COCOA_TOUCH( PongGame, RendererGl )
