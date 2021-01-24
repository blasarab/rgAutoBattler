//
// Created by blasarab on 24.1.21..
//

#ifndef LABYRINTH_HUNT_KNIGHT_H
#define LABYRINTH_HUNT_KNIGHT_H

class Knight{

private:




public:
    glm::vec2 knightPos;
    int knightIndex;
    int HP;
    float Degrees;

    Knight(): knightPos({1,1}), knightIndex(9), HP(100), Degrees(180.0f)
    {}


};





#endif //LABYRINTH_HUNT_KNIGHT_H
