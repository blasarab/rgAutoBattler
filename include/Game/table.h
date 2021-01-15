//
// Created by ivan on 14.12.20..
//

#ifndef AUTO_BATTLER_TABLE_H
#define AUTO_BATTLER_TABLE_H

#include <cstdlib>
#include <vector>
#include <map>
#include <ctime>
#include "unit.h"
#include "AStar.hpp"

using namespace std;

#define TABLESIZE 8

struct vec2{
    int x, y;
    bool operator == (const vec2& coordinates_);
};

bool vec2::operator == (const vec2& coordinates_){
    return (x == coordinates_.x && y == coordinates_.y);
}

vec2 operator + (const vec2& left_, const vec2& right_){
    return{ left_.x + right_.x, left_.y + right_.y };
}

int coordToIndex(int x, int y){
    return x * TABLESIZE + y;
}

int coordToIndex(vec2 coords){
    return coords.x * TABLESIZE + coords.y;
}

vec2 indexToCoords(int index){
    vec2 rez;
    rez.x = index/8;
    rez.y = index%8;
    return rez;
}


class Table {
private:
    AStar::Generator generator;


public:
    vector<vector<int>> gameTable;
    map<int, Unit*> team1Units;		// mapa unita po njihovim indexima	0-4 team1
    map<int, Unit*> team2Units;		//									5-9 team2

    Table(){
        for(int i=0; i<8; i++){
            vector<int> a(8);			// 0-niko 1-team1warrior 2-team1mage 3-team1assassino
            gameTable.push_back(a);		    //        4-team2warrior 5-team2mage 6-team2assassino
        }
        generator.setWorldSize({8, 8});
        generator.setHeuristic(AStar::Heuristic::manhattan);
        generator.setDiagonalMovement(false);
    };

    void randomiseSelf(){
        if(!team1Units.empty()){
            for(int i=0; i<5; i++){
                vec2 pos = indexToCoords(team1Units[i]->PositionIndex);
                generator.removeCollision({pos.x, pos.y});
            }
            team1Units.clear();
        }
        srand(time(nullptr));
        for(int i=0; i<5; i++){
            int tipUnita = rand()%3 + 1;; // 1-warrior 2-mage 3-assassino
            int randomX = rand()%4;
            int randomY = rand()%8;
            if(gameTable[randomX][randomY]!=0)
                i--;
            else{
                gameTable[randomX][randomY] = i;
                team1Units[i] = new Unit(i, tipUnita, coordToIndex(randomX, randomY));
                generator.addCollision({randomX, randomY});
            }
        }
    }

    void randomiseEnemy(){
        if(!team2Units.empty()){
            for(int i=5; i<10; i++){
                vec2 pos = indexToCoords(team2Units[i]->PositionIndex);
                generator.removeCollision({pos.x, pos.y});
            }
            team2Units.clear();
        }
        srand(time(nullptr));
        for(int i=0; i<5; i++){
            int tipUnita = rand()%3 + 1;
            int randomX = rand()%4 + 4;
            int randomY = rand()%8;
            if(gameTable[randomX][randomY]!=0)
                i--;
            else{
                gameTable[randomX][randomY] = i+5;
                team2Units[i+8] = new Unit(5+i, tipUnita, coordToIndex(randomX, randomY));
                generator.addCollision({randomX, randomY});
            }
        }
    }

    void clearTable(){
        for(int i=0; i<8; i++)
            for(int j=0; j<8; j++)
                gameTable[i][j] = 0;

        team1Units.erase(team1Units.begin(), team1Units.end());
        team2Units.erase(team2Units.begin(), team2Units.end());
        generator.clearCollisions();
    }

    int manhattanDistance(vec2 source, vec2 target){
        return 10*(abs(source.x - target.x) + abs(source.y - target.y));
    }

    vec2 findClosestEnemy(int source, int i){
        vec2 sourceCoords = indexToCoords(source);
        int currentDistance;
        int currentBestEnemy;
        int maxDistance = -1;
        if(i<5){
            for( i=0; i<5; i++){
                if(team2Units[i+5]->Alive){
                    currentDistance = manhattanDistance(sourceCoords, indexToCoords(team2Units[i+5]->PositionIndex));
                    if(currentDistance < maxDistance){
                        maxDistance = currentDistance;
                        currentBestEnemy = i;
                    }
                }
            }
            return indexToCoords(team2Units[currentBestEnemy+5]->PositionIndex);
        }else {
            for(i=0; i<5; i++){
                if(team1Units[i]->Alive){
                    currentDistance = manhattanDistance(sourceCoords, indexToCoords(team1Units[i]->PositionIndex));
                    if(currentDistance < maxDistance){
                        maxDistance = currentDistance;
                        currentBestEnemy = i;
                    }
                }
            }
            return indexToCoords(team1Units[currentBestEnemy]->PositionIndex);
        }

    }

};

#endif //AUTO_BATTLER_TABLE_H
