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
	virtual void	serveBall();
	
	virtual void	boundaries();
	virtual void	collisions(Vec2f, float, bool);
	virtual void	aiPaddle();
	
	Vec2f pos, vel, acc;
	float wid, hei, rad;
	Vec2f paddleCenter, aiPaddleCenter;
	float paddleRadius;
	
	Vec2f touchArea;
	bool serve, colliding, userServe;
	float paddleLast, aiPaddleLast;
	float easingFactor, engageThresh;
	float maxSpeed, minSpeed;
};

void PongGame::setup()
{
	userServe = false;
	serveBall();
	acc = Vec2f(0,0);
	wid = getWindowWidth();
	hei = getWindowHeight();
	rad = 25.0f;
	touchArea = Vec2f(getWindowWidth(), 100);
	paddleCenter = Vec2f(getWindowCenter().x, hei + 50);
	aiPaddleCenter = Vec2f(0, -150);
	paddleRadius = 200;
	easingFactor = 0.15f;
	engageThresh = 0.3f;
	maxSpeed = 25.0f;
	minSpeed = 1.0f;
}

void PongGame::update()
{	
	if (serve) {
		
	} else{
		vel += acc;
		float velSpeed = vel.lengthSquared();
		if(velSpeed > maxSpeed * maxSpeed){
			vel.normalize();
			vel *= maxSpeed;
		} else if (velSpeed < minSpeed * minSpeed) {
			vel.normalize();
			vel *= minSpeed;
		}
		pos += vel;
		aiPaddle();
		boundaries();
		
		collisions(aiPaddleCenter, aiPaddleLast, true);
		collisions(paddleCenter, paddleLast, false);
	}
}

void PongGame::aiPaddle()
{	
	engageThresh = 1 - ( pos.y / (hei - 100) );
	aiPaddleLast = aiPaddleCenter.x;
	aiPaddleCenter.x -= (aiPaddleCenter.x - pos.x) * easingFactor * engageThresh;
}

void PongGame::collisions(Vec2f paddle, float paddleLast_, bool is_ai)
{
	int mult;
	if(is_ai){
		mult = 1;	
	} else {
		mult = -1;
	}

	
	float minDistanceSQ	= 200*200 + rad * rad;
	float distanceSQ	= (paddle.x - pos.x)*(paddle.x - pos.x) + (paddle.y - pos.y)*(paddle.y - pos.y);
	
	
	if( distanceSQ <= minDistanceSQ && colliding == false ){
		colliding = true;
		Vec2f collisionDir = Vec2f(paddle - vel);
		
		if(abs(collisionDir.x / collisionDir.y) > 4){
			collisionDir.y = abs(collisionDir.x) / -4;
		}
		
		collisionDir.normalize();
		
		float paddleSpeed = paddle.x - paddleLast_;
		if(paddleSpeed > 14){
			paddleSpeed = 14;
		}
		
		Vec2f paddleVel;
		
		if(paddleSpeed < 1){
			paddleVel = Vec2f( paddleSpeed , vel.y * 0.7f * mult + 3 * mult);
		} else {
			paddleVel = Vec2f( paddleSpeed , 10 * mult );
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
	
	if(pos.y < 0 - rad){
		userServe = true;
		serveBall();
	} else if (pos.y > hei -touchArea.y + rad){
		userServe = false;
		serveBall();
	}
}

void PongGame::serveBall(){
	if(userServe){
		pos = Vec2f(paddleCenter.x, paddleCenter.y - paddleRadius);
		vel = Vec2f( Rand::randFloat(-10, 10), -10 );
	} else {
		pos = Vec2f(aiPaddleCenter.x, aiPaddleCenter.y + paddleRadius);
		vel = Vec2f( Rand::randFloat(-10, 10), 10 );
	}
	serve = true;
	colliding = false;
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
		if(serve && userServe){
			pos.x = paddleCenter.x;
		}
	} else if(serve){
		serve = false;
	}
}

void PongGame::mouseDrag( MouseEvent event )
{
	if(event.getY() > hei - touchArea.y){
		paddleLast = paddleCenter.x;
		paddleCenter.x = event.getPos().x;
		if(serve && userServe){
			pos.x = paddleCenter.x;
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
	
	
	//draw ball
	color(Colorf(1,0,0));
	drawSolidCircle(pos, rad ,32);
	
	//draw paddle
	color(Colorf(1,1,1));
	drawSolidCircle( paddleCenter, paddleRadius, 64);
	
	//draw opponent paddle
	drawSolidCircle( aiPaddleCenter, paddleRadius, 64);
	
	//draw touch area
	color( Colorf(0.2f, 0.2f, 0.5f) );
	drawSolidRect( Rectf(wid, hei, 0, hei - touchArea.y  ) );
	drawString( "Touch Area", Vec2f(50, hei - touchArea.y), Colorf(0.3f, 0.3f, 0.65f), Font( "Gill Sans", 40 ) );
	
	if(serve){
		
		drawString( "Touch to Serve", Vec2f(wid * 0.25f, hei * 0.35f), Colorf(1, 1, 1), Font( "Gill Sans", 60 ) );
		
	}
	
}

CINDER_APP_COCOA_TOUCH( PongGame, RendererGl )
