#include "cinder/app/AppCocoaTouch.h"
#include "cinder/app/App.h"
#include "cinder/app/Renderer.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace gl;

class PongGame : public AppCocoaTouch {
public:
	virtual void	setup();
	virtual void	resize( ResizeEvent event );
	virtual void	update();
	virtual void	draw();
	virtual void	mouseDown( MouseEvent event );
	virtual void	mouseDrag( MouseEvent event );
	
	Vec2f pos, vel, acc;
	float wid, hei, rad;
	Vec2f paddleCenter;
	float paddleWidth, paddleHeight;
};

void PongGame::setup()
{
	pos = Vec2f(0,0);
	vel = Vec2f( Rand::randFloat(2),Rand::randFloat(2) );
	acc = Vec2f(0,0);
	wid = getWindowWidth();
	hei = getWindowHeight();
	rad = 25.0f;
	paddleHeight = 10.0f;
	paddleCenter = Vec2f(getWindowCenter().x,getWindowHeight() - paddleHeight * 2);
	paddleWidth = 80.0f;
}

void PongGame::resize( ResizeEvent event )
{
	//mCam.lookAt( Vec3f( 0, 0, -hei * 2.2f ), Vec3f::zero(), Vec3f(0,-1,0) );
	//mCam.setPerspective( 60, event.getAspectRatio(), 1, 2000 );
}

void PongGame::mouseDown( MouseEvent event )
{
	paddleCenter.x = event.getPos().x;
}

void PongGame::mouseDrag( MouseEvent event )
{
	paddleCenter.x = event.getPos().x;
}

void PongGame::update()
{
	vel += acc;
	pos += vel;
	
	if(pos.x < 0 + rad){
		vel.x *= -1.0f;
		pos.x = 0 + rad;
	} else if(pos.x > wid - rad){
		vel.x *= -1.0f;
		pos.x = wid - rad;
	}
	
	if(pos.y < 0 + rad){
		vel.y *= -1.0f;
		pos.y = 0 + rad;
	} else if(pos.y > hei - rad){
		vel.y *= -1.0f;
		pos.y = hei - rad;
	}
}

void PongGame::draw()
{
	clear();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	setMatricesWindow( getWindowSize() );
	color( ColorAf(0,0,0,0.85f) );
	drawSolidRect( Rectf( 0, 0, wid, hei) );
	color(Colorf(1,1,1));
	
	
	string line = "test";
	
	drawStringCentered( line, getWindowCenter(), Colorf(1,1,1), Font( "Gill Sans", 60 ) );
	cout << getWindowCenter();
	//gl::drawStringCentered("Hello World", Vec2f(0, hei));
	color(Colorf(1,0,0));
	drawSolidCircle(pos, rad ,32);
	
	color(Colorf(1,1,1));
	
	drawSolidRect( Rectf(paddleCenter.x - paddleWidth, paddleCenter.y - paddleHeight, paddleCenter.x + paddleWidth, paddleCenter.y + paddleHeight ) );
	
	
}

CINDER_APP_COCOA_TOUCH( PongGame, RendererGl )
