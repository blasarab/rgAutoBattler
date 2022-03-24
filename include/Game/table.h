//
// Created by ivan on 14.12.20..
//

#ifndef LABYRINTH_HUNT_TABLE_H
#define LABYRINTH_HUNT_TABLE_H

#include <vector>
#include <map>
#include <ctime>
#include "Knight.h"

using namespace std;

#define TABLESIZE 8

enum {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
};

class Table {
private:
    static int InBounds(int x, int y){
        if(x < 0 || x >= TABLESIZE) return false;
        if(y < 0 || y >= TABLESIZE) return false;
        return true;
    }

    static int CoordToIndex( int x, int y ){
        return y * TABLESIZE + x;
    }



public:
    vector<char> Grid;
    Knight *knight;
    int Treasure;
    vector<int> Traps;


    Table(){
        Grid.resize(TABLESIZE*TABLESIZE);
        knight = new Knight();
    };

    void RandomiseLabyrinth(){
        ResetGrid();
        ResetKnight();
        srand(glfwGetTime());
        Visit(1,1);
        generateTraps();
    }
    void ResetGrid(){
        for (int i=0; i<TABLESIZE*TABLESIZE; ++i)
            Grid[i] = '#';
    }

    void ResetKnight(){
        knight->knightPos = {1,1};
        knight->Degrees = 180.f;
        knight->knightIndex = 9;
        knight->HP = 1;
    }

    void Visit(int x, int y){
        Grid[CoordToIndex(x,y)] = ' ';

        int dirs[4] = { NORTH, EAST, SOUTH, WEST};

        for(int & dir : dirs){
            int r = rand() & 3;
            int temp = dirs[r];
            dirs[r] = dir;
            dir = temp;
        }

        for(int dir : dirs){
            int dx = 0, dy = 0;
            switch(dir){
                case NORTH: dy = -1; break;
                case SOUTH: dy = 1; break;
                case EAST: dx = 1; break;
                case WEST: dx = -1; break;
            }
            int x2 = x + (dx<<1);
            int y2 = y + (dy<<1);

            if(InBounds(x2, y2) && Grid[CoordToIndex(x2,y2)] == '#'){
                Grid[CoordToIndex(x2-dx, y2-dy)] = ' ';
                Visit(x2, y2);
            }
        }
    }

    void generateTreasure(){
        srand(glfwGetTime());
        while(true){
            int ind = rand() % (TABLESIZE*TABLESIZE - 1);
            if(Grid[ind] == ' ' && ind != knight->knightIndex){
                Treasure = ind;
                break;
            }
        }
    }

    void generateTraps(){
        srand(glfwGetTime());
        int i = 4;
        Traps.resize(4);
        while(i--){
            int ind = rand() % (TABLESIZE*TABLESIZE - 1);
            if(Grid[ind] == ' ' && ind != knight->knightIndex){
                Traps[i] = ind;
                Grid[ind] = 't';
            }
            else
                i++;
        }
    }

    void move(glm::vec2 dir){
        if(dir.x == 0 && dir.y == 1)
            knight->Degrees = 270.0f;
        else if(dir.x == 0 && dir.y == -1)
            knight->Degrees = 90.0f;
        else if(dir.x == 1 && dir.y == 0)
            knight->Degrees = 180.0f;
        else if(dir.x == -1 && dir.y == 0)
            knight->Degrees = 0.0f;

        if(!Wall(knight->knightPos + dir) &&
            InBounds(knight->knightPos.x + dir.x, knight->knightPos.y + dir.y)) {
            knight->knightPos += dir;
            knight->knightIndex = CoordToIndex(knight->knightPos.x, knight->knightPos.y);
        }
    }

    bool found(){
        if(knight->knightIndex == Treasure)
            return true;
        return false;
    }

    bool nagazio(int dtime){
        if(dtime%2 == 0 && (Traps[0] == knight->knightIndex || Traps[1] == knight->knightIndex
           || Traps[2] == knight->knightIndex || Traps[3] == knight->knightIndex))
            return true;
        return false;
    }

    bool Wall(glm::vec2 coord){
        if(Grid[CoordToIndex(coord.x, coord.y)] == '#')
            return true;
        else
            return false;
    }

    bool mrtav(){
        if(knight->HP == 0){
            return true;
        }
        return false;
    }

};

#endif //LABYRINTH_HUNT_TABLE_H
