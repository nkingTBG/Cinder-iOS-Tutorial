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
	float width, height, rad;
	Vec2f paddleCenter, aiPaddleCenter, paddleLast, aiPaddleLast, paddleVel, aiPaddleVel;
    Vec2f targetAiPaddleCenter, targetPos;
	float paddleRadius;
	
	bool serve, colliding, userServe;
	float easingFactorX, easingFactorY, easingFactorServe;
	float maxSpeed, minSpeed;
	
	int userScore, aiScore;
	
	Texture texture1, texture2, texture3, paddleTexture, puckTexture;
	float goalWidth, goalHeight, goalBoxLeft, goalBoxRight;
	
	audio::SourceRef sound1, sound2;
    
    bool touchMoved, trapped, iPad;
};

void PongGame::setup()
{
	userServe = true;
    serve = true;
    touchMoved = false;
    trapped = false;
	
	acc = Vec2f(0,0);
	width = getWindowWidth();
	height = getWindowHeight();
    if(width == 768){
        iPad = true;   
    } else {
        iPad = false;
    }

	rad = 35.0f;
	paddleRadius = 55.0f;
	goalWidth = width * 0.20f;
	goalHeight = 8;
	goalBoxLeft = goalWidth + goalHeight;
	goalBoxRight = width - goalWidth - goalHeight;

    pos = Vec2f(width/2, height * 0.8f);
    vel = Vec2f(0,0);
    paddleCenter = Vec2f(width/2, height - paddleRadius - goalHeight);
    aiPaddleCenter = Vec2f( width/2, paddleRadius + goalHeight);
    if(userServe){
        targetAiPaddleCenter = Vec2f( width/2, paddleRadius + goalHeight);
        targetPos = Vec2f(width/2, height * 0.8f);
    } else {
        targetAiPaddleCenter = Vec2f( Rand::randFloat(goalBoxLeft + paddleRadius, goalBoxRight - paddleRadius), paddleRadius + goalHeight);
        targetPos = Vec2f(width/2, height * 0.2f);
    }

	easingFactorX = 0.10f;
    easingFactorY = 0.20f;
    easingFactorServe = 0.1f;
	maxSpeed = 50.0f;
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
	if (serve) {
		serveBall();
	} else{
        
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
		
        
        //paddleVel = paddleCenter - paddleLast;
    
		collisions(aiPaddleCenter, aiPaddleVel);
		collisions(paddleCenter, paddleVel);
	
        if(!touchMoved){
            paddleLast = paddleCenter;
            paddleVel = paddleCenter - paddleLast;   
        }
        
        touchMoved = false;
        
    }
}

void PongGame::aiPaddle()
{	
	float engageThreshY = 1 - ( pos.y / (height * 0.5f) );
    if(engageThreshY < 0 || vel.y > 0){
        engageThreshY = 0;   
    }

	aiPaddleLast = aiPaddleCenter;
	
    aiPaddleCenter.x -= (aiPaddleCenter.x - pos.x) * easingFactorX;
    if(vel.y > 0 && pos.y > height/2){
        aiPaddleCenter.y -= (aiPaddleCenter.y - paddleRadius - goalHeight) * easingFactorY;
    } else {
        aiPaddleCenter.y -= (aiPaddleCenter.y - pos.y) * easingFactorY * engageThreshY;
    }
    if(aiPaddleCenter.x < paddleRadius){
		aiPaddleCenter.x = paddleRadius;
	} else if (aiPaddleCenter.x > width - paddleRadius) {
		aiPaddleCenter.x = width - paddleRadius;
	}
    if(aiPaddleCenter.y >= height * 0.3f){
        aiPaddleCenter.y = height * 0.3f;
    } else if (aiPaddleCenter.y <= paddleRadius + goalHeight){
        aiPaddleCenter.y = paddleRadius + goalHeight;
    }
    
    if( ( abs(aiPaddleCenter.x - pos.x) <= (paddleRadius + rad) * 1.1f ) && ( pos.x + rad >= width || pos.x - rad <= 0)  && ( abs(aiPaddleCenter.y - pos.y) <= (paddleRadius + rad) * 1.1f ) ){
        trapped = true; 
        if(vel.y < 0){
            vel.y = 0;
        }
    }
    
    if(pos.x < width - rad * 5 && pos.x > rad * 5 || abs(aiPaddleCenter.y - pos.y) > (paddleRadius * 3.0f ) ){
        trapped = false;   
    }
    
    if(trapped){
        aiPaddleCenter.y -= (aiPaddleCenter.y - paddleRadius - goalHeight) * easingFactorY;
        aiPaddleCenter.x -= (aiPaddleCenter.x - width/2) * easingFactorX;
        vel.y += 0.15f;
    }
    
    

    aiPaddleVel = aiPaddleCenter - aiPaddleLast;

}

void PongGame::collisions(Vec2f paddle_, Vec2f paddleVel_)
{

	//distances for detecting a collision
	float minDistanceSQ	= (paddleRadius + rad) * (paddleRadius + rad);
	float distanceSQ	= (paddle_.x - pos.x)*(paddle_.x - pos.x) + (paddle_.y - pos.y)*(paddle_.y - pos.y);
	
	
	if( distanceSQ < minDistanceSQ && colliding == false ){
		//if the distance is less then the threshold, collision was detected
        colliding = true;
        
        if(paddleVel_.length() < 1){
            Vec2f collisionDir = paddle_ - pos;
            collisionDir.safeNormalize();
            pos = paddle_ - ( (paddleRadius + rad) * collisionDir );
            vel *= -0.6f;
        } else {
        
            Vec2f collisionDir = paddle_ - pos;
            collisionDir.normalize();
            pos = paddle_ - ( (paddleRadius + rad) * collisionDir );
            
            Vec2f netVel = paddleVel_ - vel;
            
            float totalForce = collisionDir.x * netVel.x + collisionDir.y * netVel.y;
            totalForce *= 0.6f;
            Vec2f collisionVector = collisionDir * totalForce;
            
            netVel -= collisionVector;
            vel = netVel + paddleVel_;
        }
        
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
	} else if(pos.x > width - rad){
		vel.x *= -0.65f;
		pos.x = width - rad;
		audio::Output::play(sound1);
	}
	
	if(pos.y <= 0 + goalHeight + rad && (pos.x < goalBoxLeft || pos.x > goalBoxRight)){
		vel.y *= -0.65f;
		pos.y = 0 + rad + goalHeight;
		audio::Output::play(sound1);
	} else if(pos.y + rad >= height - goalHeight && (pos.x < goalBoxLeft || pos.x > goalBoxRight)){
		vel.y *= -0.65f;
		pos.y = height - goalHeight - rad;
		audio::Output::play(sound1);
	}
	
	
	if(pos.y < 0 - rad){
		userServe = true;
		userScore++;
        pos = Vec2f(width + rad, height/2);
        targetAiPaddleCenter = Vec2f( width/2, paddleRadius + goalHeight);
		audio::Output::play(sound2);
		serveBall();
	} else if (pos.y > height + rad){
		userServe = false;
        pos = Vec2f(width + rad, height/2);
        targetAiPaddleCenter = Vec2f( Rand::randFloat(goalBoxLeft + paddleRadius, goalBoxRight - paddleRadius), paddleRadius + goalHeight);
		audio::Output::play(sound2);
		aiScore++;
		serveBall();
	}
}

void PongGame::serveBall(){
	if(userServe){
		vel = Vec2f(0,0);
        targetPos = Vec2f(width/2, height * 0.8f);
        Vec2f targetPaddleCenter = Vec2f(width/2, height - paddleRadius - goalHeight);
        pos -= (pos - targetPos) * easingFactorServe;
        paddleCenter -= (paddleCenter - targetPaddleCenter) * easingFactorServe;
        aiPaddleCenter -= (aiPaddleCenter - targetAiPaddleCenter) * easingFactorServe;
	} else {
        vel = Vec2f(0,0);
        targetPos = Vec2f(width/2, height * 0.2f);
        pos -= (pos - targetPos) * easingFactorServe;
        aiPaddleCenter -= (aiPaddleCenter - targetAiPaddleCenter) * easingFactorServe;
	}
	serve = true;
	colliding = false;
}

void PongGame::resize( ResizeEvent event )
{
	setWindowSize(event.getWidth(), event.getHeight());
}

void PongGame::touchesBegan( TouchEvent event)
{   
    Vec2f tempDist = pos - targetPos;
    if(serve && tempDist.lengthSquared() < 100 ){
        for( vector<TouchEvent::Touch>::const_iterator t_iterator = event.getTouches().begin(); t_iterator != event.getTouches().end(); ++t_iterator ) {
            if(t_iterator->getY() > height * 0.3 && t_iterator->getY() < height * 0.7){
                serve = false;
                vel = Vec2f(0,0);
            }
        }
    }
}

void PongGame::touchesMoved( TouchEvent event)
{   
    if(!serve){
        touchMoved = true;
        for( vector<TouchEvent::Touch>::const_iterator t_iterator = event.getTouches().begin(); t_iterator != event.getTouches().end(); ++t_iterator ) {
            if(t_iterator->getY() > height/2 ){
                paddleLast = paddleCenter;
                if(iPad){
                    paddleCenter = t_iterator->getPos();
                } else {
                    paddleCenter = Vec2f(t_iterator->getX(), t_iterator->getY()- rad);
                }
                
                paddleVel = paddleCenter - paddleLast;
                if (paddleCenter.x < paddleRadius) {
                    paddleCenter.x = paddleRadius;
                } else if (paddleCenter.x > width - paddleRadius) {
                    paddleCenter.x = width - paddleRadius;
                }
                if (paddleCenter.y < height * 0.7f + paddleRadius) {
                    paddleCenter.y = height * 0.7f + paddleRadius;
                } else if (paddleCenter.y > height - paddleRadius - goalHeight) {
                    paddleCenter.y = height - paddleRadius - goalHeight;
                }
                
                if(abs(paddleCenter.y - pos.y) < rad){
                    Vec2f tempDist = paddleCenter - pos;
                    if(tempDist.lengthSquared() <= (paddleRadius + rad) * (paddleRadius + rad)){
                        if (paddleCenter.x < paddleRadius + rad * 2) {
                            paddleCenter.x = paddleRadius + rad * 2;
                        } else if (paddleCenter.x > width - paddleRadius - rad * 2) {
                            paddleCenter.x = width - paddleRadius - rad * 2;
                        }
                    }
                }
            }
        }
    } 
}

void PongGame::touchesEnded( TouchEvent event)
{   
    paddleLast = paddleCenter;
    paddleVel = paddleCenter - paddleLast;
}

void PongGame::draw()
{
	clear();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_DEPTH_TEST );
	
	setMatricesWindowPersp(width, height);
	//setMatricesWindow(width, height);
	glEnable(GL_TEXTURE_2D);
	
	
	
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHT1 );
	GLfloat light_position[] = { 0, 0, 200.0f, 0.000001f };
	glLightfv( GL_LIGHT0, GL_POSITION, light_position );
	GLfloat light_RGB[] = { 0.2f, 0.2f, 0.2f };
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_RGB);
	
	//draw background
	gl::draw( texture3, Rectf(0,0,width, height) );
	
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
	drawSolidRect(Rectf(width, 0, width - goalWidth, goalHeight));
	drawSolidCircle(Vec2f(goalWidth,0), goalHeight, 24);
	drawSolidCircle(Vec2f(width - goalWidth,0), goalHeight, 24);
	//bottom
	drawSolidRect(Rectf(0, height, goalWidth, height - goalHeight));
	drawSolidRect(Rectf(width, height, width - goalWidth, height - goalHeight));
	drawSolidCircle(Vec2f(goalWidth, height), goalHeight, 24);
	drawSolidCircle(Vec2f(width - goalWidth, height), goalHeight, 24);
	
	//draw touch area
	color( Colorf(0.0f, 0.0f, 0.0f) );
	drawSolidRect( Rectf(width, height, 0, height  ) );
	drawString( "Touch Area", Vec2f(50, height), Colorf(0.3f, 0.3f, 0.3f), Font( "Arial", 40 ) );
	
	if(serve){
		drawString( "Touch to Serve", Vec2f(width * 0.25f, height * 0.4f), Colorf(0, 0, 0), Font( "Arial", 60 ) );
		ostringstream score;
		score << "Score is " << userScore << " - " << aiScore;
		drawString( score.str(), Vec2f(width * 0.25f, height * 0.3f), Colorf(0, 0, 0), Font( "Arial", 60 ) );
		
	}
	
}

CINDER_APP_COCOA_TOUCH( PongGame, RendererGl )
