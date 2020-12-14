//
// Created by ivan on 14.12.20..
//

#ifndef PROJECT_BASE_GAME_H
#define PROJECT_BASE_GAME_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Game {
public:
    unsigned int Width;
    unsigned int Height;


    // constructor/destructor
    Game(unsigned int, unsigned int);
    ~Game();
    // initialise game state and load all shaders/textures/models
    void Init();
    // game loop
    void ProcessInput(float deltatime);
    void Update(float deltatime);
    void Render();
    // reset
    void resetStage();

};


#endif //PROJECT_BASE_GAME_H
