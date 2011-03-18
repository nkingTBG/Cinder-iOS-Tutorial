#include "cinder/app/AppCocoaTouch.h"
#include "cinder/audio/Output.h"
#include "cinder/audio/Io.h"
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
#include "Resources.h"

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
    virtual void    touchesBegan( TouchEvent event);
    virtual void    touchesMoved( TouchEvent event);
    virtual void    touchesEnded( TouchEvent event);
	virtual void	serveBall();
	
	virtual void	boundaries();
	virtual void	collisions(Vec2f, Vec2f);
	virtual void	aiPaddle();
	
	Vec2f pos, vel, acc;
	float wid, hei, rad;
	Vec2f paddleCenter, aiPaddleCenter, paddleLast, aiPaddleLast, paddleVel, aiPaddleVel;
	float paddleRadius;
	
	bool serve, colliding, userServe;
	float easingFactorX, easingFactorY;
	float maxSpeed, minSpeed;
	
	int userScore, aiScore;
	
	Texture texture1, texture2, texture3, paddleTexture, puckTexture;
	float goalWidth, goalHeight, goalBoxLeft, goalBoxRight;
	
	audio::SourceRef sound1, sound2;
};

void PongGame::setup()
{
	userServe = true;
    serve = false;
	
	acc = Vec2f(0,0);
	wid = getWindowWidth();
	hei = getWindowHeight();
	//rad = 50.0f;
	rad = 25.0f;
	paddleRadius = 50.0f;
	goalWidth = wid * 0.20f;
	goalHeight = 8;
	goalBoxLeft = goalWidth + goalHeight;
	goalBoxRight = wid - goalWidth - goalHeight;
	//paddleCenter = Vec2f(getWindowCenter().x, hei - paddleRadius - goalHeight);
	//aiPaddleCenter = Vec2f(wid/2, paddleRadius + goalHeight);
	
	//pos = Vec2f(aiPaddleCenter.x, aiPaddleCenter.y + paddleRadius + rad);
    pos = Vec2f(wid/2, hei * 0.8f);
    vel = Vec2f(0,0);
    paddleCenter = Vec2f(wid/2, hei - paddleRadius - goalHeight);
    aiPaddleCenter = Vec2f( wid/2, paddleRadius + goalHeight);
    
	easingFactorX = 0.05f;
    easingFactorY = 0.15f;
	maxSpeed = 100.0f;
	minSpeed = 3.0f;
	
	userScore = 0;
	aiScore = 0;
	
	texture1 = Texture( loadImage( loadResource( "brushed2.jpg" ) ) );
	texture2 = Texture( loadImage( loadResource( "brushed.jpg" ) ) );
	texture3 = Texture( loadImage( loadResource( "table_surface.jpg" ) ) );
	paddleTexture = Texture( loadImage(loadResource( "paddle_140.png" ) ) );
	puckTexture = Texture( loadImage(loadResource( "puck_122.png" ) ) );
	
	sound1 = audio::load( loadResource(RES_SOUND1) );
	sound2 = audio::load( loadResource(RES_SOUND2) );
   // serveBall();
}


void PongGame::update()
{	
	//if (serve) {
		
	//} else{
		vel += acc;
		float velSpeed = vel.lengthSquared();
		if(velSpeed > maxSpeed * maxSpeed){
			vel.safeNormalize();
			vel *= maxSpeed;
		} else if (velSpeed < minSpeed * minSpeed) {
			vel.safeNormalize();
			vel *= minSpeed;
		}
		pos += vel;
		aiPaddle();
		boundaries();
		
		collisions(aiPaddleCenter, aiPaddleVel);
		collisions(paddleCenter, paddleVel);
	//}
    
}

void PongGame::aiPaddle()
{	
	float engageThreshY = 1 - ( pos.y / (hei * 0.3f) );
    if(engageThreshY < 0 || vel.y > 0){
        engageThreshY = 0;   
    }
	aiPaddleLast = aiPaddleCenter;
	if(aiPaddleCenter.x < paddleRadius + rad){
		aiPaddleCenter.x = paddleRadius + rad;
	} else if (aiPaddleCenter.x > wid - paddleRadius - rad) {
		aiPaddleCenter.x = wid - paddleRadius - rad;
	}
    if(aiPaddleCenter.y >= hei * 0.3f){
        aiPaddleCenter.y = hei * 0.3f;
    } else if (aiPaddleCenter.y <= paddleRadius + goalHeight){
        aiPaddleCenter.y = paddleRadius + goalHeight;
    }
    aiPaddleCenter.x -= (aiPaddleCenter.x - pos.x) * easingFactorX;
    if(vel.y > 0 && pos.y > hei/2){
        aiPaddleCenter.y -= (aiPaddleCenter.y - paddleRadius - goalHeight) * easingFactorY;
    } else {
        aiPaddleCenter.y -= (aiPaddleCenter.y - pos.y) * easingFactorY * engageThreshY;
    }
	
    aiPaddleVel = aiPaddleCenter - aiPaddleLast;
   /* if(aiPaddleVel.y <= 0){
        aiPaddleVel.y = vel.y * -1;
    }*/
}

void PongGame::collisions(Vec2f paddle_, Vec2f paddleVel_)
{

	//distances for detecting a collision
	float minDistanceSQ	= (paddleRadius + rad) * (paddleRadius + rad);
	float distanceSQ	= (paddle_.x - pos.x)*(paddle_.x - pos.x) + (paddle_.y - pos.y)*(paddle_.y - pos.y);
	
	
	if( distanceSQ < minDistanceSQ && colliding == false ){
		//if the distance is less then the threshold, collision was detected
        colliding = true;
        
        
		Vec2f collisionDir = paddle_ - pos;
		collisionDir.normalize();
		pos = paddle_ - ( (paddleRadius + rad) * collisionDir );
		
		Vec2f netVel = paddleVel_ - vel;
        
		float totalForce = collisionDir.x * netVel.x + collisionDir.y * netVel.y;
		Vec2f collisionVector = collisionDir * totalForce;
        
        netVel -= collisionVector;
        vel = netVel + paddleVel;
        
		audio::Output::play(sound1);
	} else {
		colliding = false;
	}

}

void PongGame::boundaries(){
	if(pos.x < 0 + rad){
		vel.x *= -0.65f;
		pos.x = 0 + rad;
		audio::Output::play(sound1);
	} else if(pos.x > wid - rad){
		vel.x *= -0.65f;
		pos.x = wid - rad;
		audio::Output::play(sound1);
	}
	
	if(pos.y <= 0 + goalHeight + rad && (pos.x < goalBoxLeft || pos.x > goalBoxRight)){
		vel.y *= -0.65f;
		pos.y = 0 + rad + goalHeight;
		audio::Output::play(sound1);
	} else if(pos.y + rad >= hei - goalHeight && (pos.x < goalBoxLeft || pos.x > goalBoxRight)){
		vel.y *= -0.65f;
		pos.y = hei - goalHeight - rad;
		audio::Output::play(sound1);
	}
	
	
	if(pos.y < 0 - rad){
		userServe = true;
		userScore++;
		audio::Output::play(sound2);
		serveBall();
	} else if (pos.y > hei + rad){
		userServe = false;
		audio::Output::play(sound2);
		aiScore++;
		serveBall();
	}
}

void PongGame::serveBall(){
	if(userServe){
		pos = Vec2f(wid/2, hei * 0.8f);
		vel = Vec2f(0,0);
        paddleCenter = Vec2f(wid/2, hei - paddleRadius - goalHeight);
        aiPaddleCenter = Vec2f( wid/2, paddleRadius + goalHeight);
	} else {
		pos = Vec2f(wid/2, hei * 0.2f);
        vel = Vec2f(0,0);
        aiPaddleCenter = Vec2f( Rand::randFloat(paddleRadius, wid - paddleRadius), paddleRadius + goalHeight);
	}
	//serve = true;
	colliding = false;
}

void PongGame::resize( ResizeEvent event )
{
	//mCam.lookAt( Vec3f( 0, 0, -hei * 2.2f ), Vec3f::zero(), Vec3f(0,-1,0) );
	//mCam.setPerspective( 60, event.getAspectRatio(), 1, 2000 );
	setWindowSize(event.getWidth(), event.getHeight());
}

void PongGame::touchesBegan( TouchEvent event)
{   
    if(serve){
        for( vector<TouchEvent::Touch>::const_iterator t_iterator = event.getTouches().begin(); t_iterator != event.getTouches().end(); ++t_iterator ) {
            if(t_iterator->getY() > hei * 0.3 && t_iterator->getY() < hei * 0.7){
                serve = false;
            }
        }
    }
}

void PongGame::touchesMoved( TouchEvent event)
{
    for( vector<TouchEvent::Touch>::const_iterator t_iterator = event.getTouches().begin(); t_iterator != event.getTouches().end(); ++t_iterator ) {
        if(t_iterator->getY() > hei/2 ){
            paddleLast = paddleCenter;
            paddleCenter = t_iterator->getPos();
            paddleVel = paddleCenter - paddleLast;
            if (paddleCenter.x < paddleRadius) {
                paddleCenter.x = paddleRadius;
            } else if (paddleCenter.x > wid - paddleRadius) {
                paddleCenter.x = wid - paddleRadius;
            }
            if (paddleCenter.y < hei * 0.7f + paddleRadius) {
                paddleCenter.y = hei * 0.7f + paddleRadius;
            } else if (paddleCenter.y > hei - paddleRadius - goalHeight) {
                paddleCenter.y = hei - paddleRadius - goalHeight;
            }
            if(serve && userServe){
                Vec2f paddleDirection = paddleVel;
                paddleDirection.normalize();
                pos = paddleCenter + paddleDirection * (rad + paddleRadius);
            }
            if(t_iterator->getY() > hei * 0.3 && t_iterator->getY() < hei * 0.7){
                serve = false;
            }
        }
    }
}

void PongGame::touchesEnded( TouchEvent event)
{
    
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
	gl::draw( texture3, Rectf(0,0,wid, hei) );
	
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT2 );
	GLfloat light_RGB2[] = { 0.6f, 0.6f, 0.6f };
	glLightfv(GL_LIGHT2, GL_AMBIENT, light_RGB2);
	
	//draw paddles
	glEnable(GL_TEXTURE_2D);
	glEnable( GL_DEPTH_TEST );
	paddleTexture.bind();
	drawCube(Vec3f(paddleCenter.x, paddleCenter.y, 0), Vec3f(paddleRadius * 2, paddleRadius * 2, 0.1f) );
	drawCube(Vec3f(aiPaddleCenter.x, aiPaddleCenter.y, 0), Vec3f(paddleRadius * 2, paddleRadius * 2, 0.1f) );
	
	//draw puck
	puckTexture.bind();

	drawSphere( Vec3f(pos.x, pos.y, 0), rad, 32 );
	
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
	drawSolidRect(Rectf(0, hei, goalWidth, hei - goalHeight));
	drawSolidRect(Rectf(wid, hei, wid - goalWidth, hei - goalHeight));
	drawSolidCircle(Vec2f(goalWidth, hei), goalHeight, 24);
	drawSolidCircle(Vec2f(wid - goalWidth, hei), goalHeight, 24);
	
	//draw touch area
	color( Colorf(0.0f, 0.0f, 0.0f) );
	drawSolidRect( Rectf(wid, hei, 0, hei  ) );
	drawString( "Touch Area", Vec2f(50, hei), Colorf(0.3f, 0.3f, 0.3f), Font( "Gill Sans", 40 ) );
	
	if(serve){
		drawString( "Touch to Serve", Vec2f(wid * 0.25f, hei * 0.4f), Colorf(0, 0, 0), Font( "Gill Sans", 60 ) );
		ostringstream score;
		score << "Score is " << userScore << " - " << aiScore;
		drawString( score.str(), Vec2f(wid * 0.25f, hei * 0.3f), Colorf(0, 0, 0), Font( "Gill Sans", 60 ) );
		
	}
	
}

CINDER_APP_COCOA_TOUCH( PongGame, RendererGl )
