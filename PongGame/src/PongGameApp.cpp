#include "cinder/app/AppCocoaTouch.h"
#include "cinder/app/App.h"
#include "cinder/app/Renderer.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"
#include "cinder/gl/Fbo.h"

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
	virtual void	startUp();
	
	Vec2f pos, vel, acc;
	float wid, hei, rad;
	Vec2f paddleCenter;
	float paddleWidth, paddleHeight;
	Vec2f touchArea;
	bool start;
};

void PongGame::startUp(){
	pos = Vec2f(0,0);
	vel = Vec2f( Rand::randFloat(10), 10 );
	start = true;
}

void PongGame::setup()
{
	startUp();
	acc = Vec2f(0,0);
	wid = getWindowWidth();
	hei = getWindowHeight();
	rad = 25.0f;
	touchArea = Vec2f(getWindowWidth(), 100);
	paddleHeight = 10.0f;
	paddleCenter = Vec2f(getWindowCenter().x, getWindowHeight() - paddleHeight * 2 - touchArea.y);
	paddleWidth = 80.0f;
}

void PongGame::resize( ResizeEvent event )
{
	//mCam.lookAt( Vec3f( 0, 0, -hei * 2.2f ), Vec3f::zero(), Vec3f(0,-1,0) );
	//mCam.setPerspective( 60, event.getAspectRatio(), 1, 2000 );
}

void PongGame::mouseDown( MouseEvent event )
{
	if(event.getY() > hei - touchArea.y){
		paddleCenter.x = event.getPos().x;
	}
	if(start){
		start = false;
	}
}

void PongGame::mouseDrag( MouseEvent event )
{
	if(event.getY() > hei - touchArea.y){
		paddleCenter.x = event.getPos().x;
	}
}

void PongGame::update()
{	
	if (start) {
			
	} else{
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
		} else if (pos.y > hei - rad - touchArea.y - paddleHeight * 3 && abs(pos.x - paddleCenter.x) < paddleWidth * 0.9f) {
			vel.y *= -1.0f;
			pos.y = hei - rad - touchArea.y - paddleHeight * 3;
		} else if (pos.y > hei -touchArea.y + rad){
			startUp();
		}
		
		if(paddleCenter.x < paddleWidth){
			paddleCenter.x = paddleWidth;
		} else if (paddleCenter.x > wid - paddleWidth) {
			paddleCenter.x = wid - paddleWidth;
		}
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
	
	if(start){
		
		drawString( "Touch to Start", Vec2f(wid * 0.25f, hei * 0.35f), Colorf(1, 1, 1), Font( "Gill Sans", 60 ) );
		
	} else {
	
		//draw ball
		color(Colorf(1,0,0));
		drawSolidCircle(pos, rad ,32);
		
		//draw paddle
		color(Colorf(1,1,1));
		drawSolidRect( Rectf(paddleCenter.x - paddleWidth, paddleCenter.y - paddleHeight, paddleCenter.x + paddleWidth, paddleCenter.y + paddleHeight ) );
		
		//draw touch area
		color( Colorf(0.2f, 0.2f, 0.5f) );
		drawSolidRect( Rectf(wid, hei, 0, hei - touchArea.y  ) );
		drawString( "Touch Area", Vec2f(50, hei - touchArea.y), Colorf(0.3f, 0.3f, 0.65f), Font( "Gill Sans", 40 ) );
	}
}

CINDER_APP_COCOA_TOUCH( PongGame, RendererGl )
