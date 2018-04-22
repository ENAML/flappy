
#include "common.hpp"

#include "State.hpp"
#include "Renderer.hpp"
#include "IO.hpp"
#include "gui.h"

/**
 * TODO:
 * move this someplace better
 * see: https://stackoverflow.com/a/402010
 */
bool circleRectCollision(Vec2f circlePos, float circleRadius, Vec2f rectPos, Vec2f rectSize)
{
    // circleDistance.x = abs(circle.x - rect.x);
    // circleDistance.y = abs(circle.y - rect.y);
    Vec2f circleDist;
    circleDist.x = abs(circlePos.x - rectPos.x);
    circleDist.y = abs(circlePos.y - rectPos.y);

    // if (circleDistance.x > (rect.width/2 + circle.r)) { return false; }
    // if (circleDistance.y > (rect.height/2 + circle.r)) { return false; }
    if (circleDist.x > (rectSize.width/2 + circleRadius)) { return false; }
    if (circleDist.y > (rectSize.height/2 + circleRadius)) { return false; }

    // if (circleDistance.x <= (rect.width/2)) { return true; } 
    // if (circleDistance.y <= (rect.height/2)) { return true; }
    if (circleDist.x <= (rectSize.width/2)) { return true; } 
    if (circleDist.y <= (rectSize.height/2)) { return true; }

    // cornerDistance_sq = (circleDistance.x - rect.width/2)^2 +
    //                      (circleDistance.y - rect.height/2)^2;
    auto xC = (circleDist.x - rectSize.width/2);
    auto yC = (circleDist.y - rectSize.height/2);

    auto cornerDistance_sq = (xC*xC) + (yC*yC);

    // return (cornerDistance_sq <= (circle.r^2));
    return (cornerDistance_sq <= (circleRadius * circleRadius));
}


/**
 * wrapper object for game
 */
class App
{
    public:
    State state;
    Renderer renderer; 

    App()
    {
        printlog(0, "Creating App");
    }

    virtual ~App()
    {}
};

/**
 * Global app variable
 */
App game;


void game_update()
{
    //---------------------------------------------------------------------------
    // update time 
    //---------------------------------------------------------------------------
    game.state.tick += 1;
    game.state.frameTime = GetFrameTime();
    // printlog(1, "%f", game.state.frameTime);
    game.state.fps = (int)(1.0f / game.state.frameTime);
    // printlog(0, "fps: %d | frameTime %f", game.state.fps, game.state.frameTime);
    
    //---------------------------------------------------------------------------
    // handle input 
    //---------------------------------------------------------------------------
    {
        game.state.mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE);
        game.state.mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

        game.state.mousePos = GetMousePosition();
        
        if (game.state.mousePressed)
        {
            game.state.mousePressedPos = GetMousePosition();
            // printlog(0, "[%i] mouse pressed", game.state.tick);
        }

        if (game.state.mouseDown)
        {
            game.state.mouseDragPos = GetMousePosition();
        }

        // toggle debug menu(s)
        if (IsKeyPressed(KEY_G))
        {
            game.state.guiVisible = game.state.guiVisible ? false : true;
        }
    }
    // println("mousePos{x: %f, y: %f}", getMouseX, game.state.mousePos.y);


    //---------------------------------------------------------------------------
    // Update
    //---------------------------------------------------------------------------
    if (
        game.state.canUpdate &&
        (
            game.state.ticksPerUpdate == 1 ||
            game.state.tick % game.state.ticksPerUpdate == 0
        )
    ) {
        switch (game.state.gameState.running)        
        {
            case RunningT::Running:
                {
                    auto& state = game.state;
                    auto& gameState = state.gameState;

                    // update xOffset
                    gameState.xOffset += speed * DELTA_TIME;

                    // trigger jump
                    if (state.mousePressed)
                    {
                        gameState.birdVY = jumpForce;
                    }
                    // add gravity to bird vel
                    gameState.birdVY += gravity * DELTA_TIME;

                    // update bird pos
                    gameState.birdY += gameState.birdVY * DELTA_TIME;
                    // clamp bird pos to floor/ceil
                    gameState.birdY = Math::clamp(
                        gameState.birdY,
                        birdSize,
                        floorY - birdSize
                    );

                    // update pipes
                    for (auto& pipe : gameState.pipes)
                    {
                        // pipe needs new position
                        if (pipe.x - gameState.xOffset + pipeWidth <= 0.0)
                        {
                            // get pipe with highest x pos
                            auto maxPipe = Vec2f();
                            for (auto& pipe : gameState.pipes)
                                if (pipe.x > maxPipe.x)
                                    maxPipe = pipe;
                            
                            // set pipe pos based on max x pos
                            pipe.x = maxPipe.x + rng::range(200, 300);
                            pipe.y = rng::range(
                                50 + halfGap,
                                floorY - 50 - halfGap
                            );
                            // printlog(1, "update pipe! <x: %f, y: %f>", pipe.x, pipe.y);
                        }
                    }

                    /**
                     * TODO: handle collisions
                     * - pipes
                     * - floor
                     */
                    bool hitPipe = false;
                    for (auto& pipe : gameState.pipes)
                    {
                        auto birdPos = Vec2f(birdX, gameState.birdY);
                        auto birdRadius = birdSize;

                        // ~rectPos=(x -. xOffset, 0.),
                        // ~rectW=pipeWidth,
                        // ~rectH=y -. halfGap,
                        auto xPos = pipe.x - gameState.xOffset;

                        auto topPipePos = Vec2f(xPos, 0);
                        auto topPipeSize = Vec2f(pipeWidth, pipe.y - halfGap);

                        auto btmPipePos = Vec2f(xPos, pipe.y + halfGap);
                        auto btmPipeSize = Vec2f(pipeWidth, floorY);

                        // bool topPipeCollides = circleRectCollision(birdPos, birdRadius, topPipePos, topPipeSize);
                        // if (topPipeCollides)
                        // {
                        //     // printlog(1, "TOP PIPE COLLIDES");
                        //     hitPipe = true;
                        //     break;
                        // }

                        bool btmPipeCollides = circleRectCollision(birdPos, birdRadius, btmPipePos, btmPipeSize);
                        if (btmPipeCollides)
                        {
                        printlog(1, "bird pos <x: %f, y: %f>", birdPos.x, birdPos.y);
                        printlog(1, "btm pos <x: %f, y: %f>", btmPipePos.x, btmPipePos.y);

                            // printlog(1, "BTM PIPE COLLIDES");
                            hitPipe = true;
                            break;
                        }
                    }

                    bool hitFloor = gameState.birdY >= floorY - birdSize;                   
                    // if (hitFloor)
                    //     printlog(1, "hit floor :(");
                    
                    // update score
                    for (auto& pipe : gameState.pipes)
                    {
                        // ((x, _)) => birdX +. xOffset <= x && birdX +. xOffset +. speed *. deltaTime > x,
                        bool scored = (
                            birdX + gameState.xOffset <= pipe.x &&
                            birdX + gameState.xOffset + speed * DELTA_TIME > pipe.x
                        );
                        if (scored)
                        {
                            gameState.score += 1;
                            printlog(0, "score!: %d", gameState.score);
                        }
                    }

                }
                break;
            case RunningT::Dead:
                // TODO
                break;
            case RunningT::Restart:
                // TODO
                break;
        }
    }


    //---------------------------------------------------------------------------
    // Draw
    //---------------------------------------------------------------------------
    game.renderer.render(
        &game.state
    );

    // if (game.state.tick % 200 == 1)
    // {
    //     println("[DEBUG : %5i] ", game.state.tick);
    // }

    //---------------------------------------------------------------------------
    // finish game loop iteration
    //---------------------------------------------------------------------------
    game.state.tick++;
}



/**
 * -----------------------------------------------------------------------------
 * /////////////////////////// <<  MAIN  >> ////////////////////////////////////
 * -----------------------------------------------------------------------------
 */
int main(int argc, char **argv)
{
 	//------------------------------------------------------------------------------ 
	// TEST / DEBUG
	//------------------------------------------------------------------------------ 
    

 	//------------------------------------------------------------------------------ 
	// Initialization
	//------------------------------------------------------------------------------ 

    // loadConfig("resources/test.json");

    SetConfigFlags(
        // FLAG_SHOW_LOGO |
        FLAG_VSYNC_HINT |
        FLAG_MSAA_4X_HINT |
        // FLAG_FULLSCREEN_MODE |
        // FLAG_WINDOW_RESIZABLE |
        0
    );
	InitWindow(
        game.state.screenWidth * game.renderer.platformRenderScale,
        game.state.screenHeight * game.renderer.platformRenderScale,
        (void *)"FLAPPY"
    );

    game.renderer.init();

    /**
     * init game
     */
    // TODO

    //------------------------------------------------------------------------------ 
    // BEGIN Main game loop
    //------------------------------------------------------------------------------    
    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop(game_update, 0, 1);
    #else
        SetTargetFPS(FPS);

        while (!WindowShouldClose()) // Detect window close button or ESC key
        {
            game_update();
        }
    #endif

	//------------------------------------------------------------------------------   
	// De-Initialization
	//------------------------------------------------------------------------------    

    // Close window and OpenGL context
	CloseWindow();        

	return 0;
}