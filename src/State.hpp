/**
 * -----------------------------------------------------------------------------
 * State.hpp
 * -----------------------------------------------------------------------------
 */
#pragma once

#include <vector>

#include "common.hpp"

using namespace std;


/**
 * SEE: https://github.com/bsansouci/reprocessing-example/tree/livestream-flappybird 
 */

// constants
const float jumpForce = -500.;
const float speed = 175.;
const float pipeWidth = 50.;
const float halfGap = 70.; 
const float birdSize = 15.; // 20.; // bird radius
const float gravity = 1400.;
const float birdX = 50.;
const float defaultBirdY = 50.;
// const float pipeHeight = 350.; // TODO: UNUSED; REMOVE
const float floorY = 500.;


enum class RunningT {
    Running,
    Dead,
    Restart,
};



class GameState
{
public:
    RunningT running = RunningT::Running;
    int score = 0;
    float birdY = defaultBirdY;
    float birdVY = 0.;
    float xOffset = 0;
    float birdRotation = 0; // handled / updated by renderer only
    vector<Vec2f> pipes = {
        Vec2f(0), 
        Vec2f(0), 
        Vec2f(0), 
        Vec2f(0), 
    };

    // constructor
    GameState()
    {
        printlog(0, "creating GameState");

        // generate pipes
        int pipeX = 500;
        int incr = 200;
        for (auto& pipe : this->pipes)
        {
            pipe.x = pipeX;
            pipe.y = 150;

            pipeX += incr;
            // printlog(0, "pipe<%f, %f>", pipe.x, pipe.y);
        }
    }

    // destructor
    virtual ~GameState()
    {
        printlog(0, "destroying GameState");
    }
};

class InputState
{
public:
    bool mousePressed = false;
    bool mouseDown = false;
    Vec2f mousePressedPos;
    Vec2f mouseDragPos;
    Vec2f mousePos;
    bool toggleGui = false;
};


class State
{
public:
    // config / debug
    bool canUpdate = true;

    // default is 1; the higher the number,
    // the slower the game (for debugging mostly)
    int ticksPerUpdate = 1; // 2;
    
    // timing-related stuff
    int tick = 0;
    float frameTime = 0;
    int fps = 0;

    // screen
    int screenWidth = SCREEN_W;
    int screenHeight = SCREEN_H;

    InputState inputState;
    GameState gameState = GameState();

    // constructor
    State()
    {
        printlog(0, "creating State");
    }

    // destructor
    virtual ~State()
    {
        printlog(0, "destroying State");
    }
};