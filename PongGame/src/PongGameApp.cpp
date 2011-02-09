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
	
	virtual void	boundaries();
	virtual void	collisions();
	
	Vec2f pos, vel, acc;
	float wid, hei, rad;
	Vec2f paddleCenter;
	
	Vec2f touchArea;
	bool start, colliding;
	float paddleLast;
};

void PongGame::collisions(){
	
	float minDistanceSQ	= 200*200 + rad * rad;
	float distanceSQ		= (paddleCenter.x - pos.x)*(paddleCenter.x - pos.x) + (paddleCenter.y - pos.y)*(paddleCenter.y - pos.y);
	
	
	if( distanceSQ <= minDistanceSQ && colliding == false ){
		colliding = true;
		Vec2f collisionDir = Vec2f(paddleCenter - vel);
		
		if(abs(collisionDir.x / collisionDir.y) > 4){
			collisionDir.y = abs(collisionDir.x) / -4;
		}
		
		collisionDir.normalize();
		
		float paddleSpeed = paddleCenter.x - paddleLast;
		if(paddleSpeed > 14){
			paddleSpeed = 14;
		}
		
		Vec2f paddleVel;
		
		if(paddleSpeed < 1){
			paddleVel = Vec2f( paddleSpeed , vel.y * -0.7f - 3);
		} else {
			paddleVel = Vec2f( paddleSpeed , -10 );
		}
		
		

		
		
		Vec2f collisionVel = Vec2f(paddleVel - vel);
		float totalForce = collisionDir.x * collisionVel.x + collisionDir.y * collisionVel.y;
		Vec2f velocityChange = collisionDir * totalForce;
		collisionVel -= velocityChange;
		vel = collisionVel + paddleVel;
		paddleVel += velocityChange; 
	} else {
		colliding = false;
	}

}

void PongGame::boundaries(){
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
	} else if (pos.y > hei -touchArea.y + rad){
		startUp();
	}
}

void PongGame::startUp(){
	pos = Vec2f(0,0);
	vel = Vec2f( Rand::randFloat(10), 10 );
	start = true;
	colliding = false;
}

void PongGame::setup()
{
	startUp();
	acc = Vec2f(0,0);
	wid = getWindowWidth();
	hei = getWindowHeight();
	rad = 25.0f;
	touchArea = Vec2f(getWindowWidth(), 100);
	paddleCenter = Vec2f(getWindowCenter().x, hei + 50);
}

void PongGame::resize( ResizeEvent event )
{
	//mCam.lookAt( Vec3f( 0, 0, -hei * 2.2f ), Vec3f::zero(), Vec3f(0,-1,0) );
	//mCam.setPerspective( 60, event.getAspectRatio(), 1, 2000 );
	setWindowSize(event.getWidth(), event.getHeight());
}

void PongGame::mouseDown( MouseEvent event )
{
	if(event.getY() > hei - touchArea.y){
		paddleLast = paddleCenter.x;
		paddleCenter.x = event.getPos().x;
	}
	if(start){
		start = false;
	}
}

void PongGame::mouseDrag( MouseEvent event )
{
	if(event.getY() > hei - touchArea.y){
		paddleLast = paddleCenter.x;
		paddleCenter.x = event.getPos().x;
	}
}

void PongGame::update()
{	
	if (start) {
			
	} else{
		vel += acc;
		pos += vel;
		boundaries();
		collisions();
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
		drawSolidCircle( paddleCenter, 200, 64);
		
		//draw touch area
		color( Colorf(0.2f, 0.2f, 0.5f) );
		drawSolidRect( Rectf(wid, hei, 0, hei - touchArea.y  ) );
		drawString( "Touch Area", Vec2f(50, hei - touchArea.y), Colorf(0.3f, 0.3f, 0.65f), Font( "Gill Sans", 40 ) );
	}
}

CINDER_APP_COCOA_TOUCH( PongGame, RendererGl )
