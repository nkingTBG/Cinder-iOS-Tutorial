#include "cinder/app/AppCocoaTouch.h"
#include "cinder/app/App.h"
#include "cinder/app/Renderer.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"
#include "cinder/gl/Fbo.h"
#include <iostream>
#include <sstream>

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
	
	int userScore, aiScore;
	
	Texture texture1, texture2, texture3, paddleTexture, puckTexture;
	float goalWidth, goalHeight, goalBoxLeft, goalBoxRight;
};

void PongGame::setup()
{
	userServe = false;
	serveBall();
	acc = Vec2f(0,0);
	wid = getWindowWidth();
	hei = getWindowHeight();
	//rad = 50.0f;
	rad = 25.0f;
	touchArea = Vec2f(getWindowWidth(), 100);
	paddleRadius = 70;
	goalWidth = wid * 0.25f;
	goalHeight = 8;
	goalBoxLeft = goalWidth + goalHeight;
	goalBoxRight = wid - goalWidth - goalHeight;
	paddleCenter = Vec2f(getWindowCenter().x, hei - touchArea.y - paddleRadius - goalHeight);
	aiPaddleCenter = Vec2f(wid/2, paddleRadius + goalHeight);
	
	pos = Vec2f(aiPaddleCenter.x, aiPaddleCenter.y + paddleRadius + rad);
	easingFactor = 0.05f;
	engageThresh = 0.03f;
	maxSpeed = 60.0f;
	minSpeed = 3.0f;
	
	userScore = 0;
	aiScore = 0;
	
	texture1 = Texture( loadImage( loadResource( "brushed2.jpg" ) ) );
	texture2 = Texture( loadImage( loadResource( "brushed.jpg" ) ) );
	texture3 = Texture( loadImage( loadResource( "table_surface.jpg" ) ) );
	paddleTexture = Texture( loadImage(loadResource( "paddle_140.png" ) ) );
	puckTexture = Texture( loadImage(loadResource( "puck_122.png" ) ) );
	
	
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

	
	float minDistanceSQ	= (paddleRadius + rad) * (paddleRadius + rad);
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
		vel.x *= -0.65f;
		pos.x = 0 + rad;
	} else if(pos.x > wid - rad){
		vel.x *= -0.65f;
		pos.x = wid - rad;
	}
	
	if(pos.y - rad <= 0 + goalHeight && (pos.x < goalBoxLeft || pos.x > goalBoxRight)){
		vel.y *= -0.65f;
		pos.y = 0 + rad;
	} else if(pos.y + rad >= hei - touchArea.y - goalHeight && (pos.x < goalBoxLeft || pos.x > goalBoxRight)){
		vel.y *= -0.65f;
		pos.y = hei - touchArea.y - goalHeight - rad;
	}
	
	
	if(pos.y < 0 - rad){
		userServe = true;
		userScore++;
		serveBall();
	} else if (pos.y > hei -touchArea.y + rad){
		userServe = false;
		aiScore++;
		serveBall();
	}
}

void PongGame::serveBall(){
	if(userServe){
		pos = Vec2f(paddleCenter.x, paddleCenter.y - paddleRadius - rad);
		vel = Vec2f( Rand::randFloat(-10, 10), -10 );
	} else {
		pos = Vec2f(aiPaddleCenter.x, aiPaddleCenter.y + paddleRadius + rad);
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
	glEnable( GL_DEPTH_TEST );
	
	setMatricesWindowPersp(wid, hei);
	//setMatricesWindow(wid, hei);
	glEnable(GL_TEXTURE_2D);
	
	
	
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHT1 );
	GLfloat light_position[] = { 0, 0, 200.0f, 0.000001f };
	glLightfv( GL_LIGHT0, GL_POSITION, light_position );
	GLfloat light_RGB[] = { 0.2f, 0.2f, 0.2f };
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_RGB);
	
	//draw background
	gl::draw( texture3, getWindowBounds() );
	
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT2 );
	GLfloat light_RGB2[] = { 0.6f, 0.6f, 0.6f };
	glLightfv(GL_LIGHT2, GL_AMBIENT, light_RGB2);
	
	//draw paddles
	glEnable(GL_TEXTURE_2D);
	glEnable( GL_DEPTH_TEST );
	paddleTexture.bind();
	drawCube(Vec3f(paddleCenter.x, paddleCenter.y, 0), Vec3f(140, 140, 0.1f) );
	drawCube(Vec3f(aiPaddleCenter.x, aiPaddleCenter.y, 0), Vec3f(140, 140, 0.1f) );
	
	//draw puck
	puckTexture.bind();
	drawCube( Vec3f(pos.x, pos.y, 0), Vec3f(rad * 2,rad * 2, 0.1f) );
	
	glDisable(GL_LIGHT2);
	glDisable( GL_LIGHTING );
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	
	
	//draw goal posts
	//top
	color(Colorf(0.0f, 0.0f, 0.0f));
	drawSolidRect(Rectf(0, 0, goalWidth, goalHeight));
	drawSolidRect(Rectf(wid, 0, wid - goalWidth, goalHeight));
	drawSolidCircle(Vec2f(goalWidth,0), goalHeight, 24);
	drawSolidCircle(Vec2f(wid - goalWidth,0), goalHeight, 24);
	//bottom
	drawSolidRect(Rectf(0, hei - touchArea.y, goalWidth, hei - touchArea.y - goalHeight));
	drawSolidRect(Rectf(wid, hei - touchArea.y, wid - goalWidth, hei - touchArea.y - goalHeight));
	drawSolidCircle(Vec2f(goalWidth, hei - touchArea.y), goalHeight, 24);
	drawSolidCircle(Vec2f(wid - goalWidth, hei - touchArea.y), goalHeight, 24);
	
	//draw touch area
	color( Colorf(0.0f, 0.0f, 0.0f) );
	drawSolidRect( Rectf(wid, hei, 0, hei - touchArea.y  ) );
	drawString( "Touch Area", Vec2f(50, hei - touchArea.y), Colorf(0.3f, 0.3f, 0.3f), Font( "Gill Sans", 40 ) );
	
	if(serve){
		drawString( "Touch to Serve", Vec2f(wid * 0.25f, hei * 0.4f), Colorf(0, 0, 0), Font( "Gill Sans", 60 ) );
		ostringstream score;
		score << "Score is " << userScore << " - " << aiScore;
		drawString( score.str(), Vec2f(wid * 0.25f, hei * 0.3f), Colorf(0, 0, 0), Font( "Gill Sans", 60 ) );
		
	}
	
}

CINDER_APP_COCOA_TOUCH( PongGame, RendererGl )
