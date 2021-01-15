//
// Created by ivan on 14.12.20..
//

#ifndef AUTO_BATTLER_UNIT_H
#define AUTO_BATTLER_UNIT_H


class Unit {
public:
    int UnitID;
    int Type;			// 1 - warrior | 2 - mage | 3 - assassino
    int PositionIndex;
    //int AttackRange;
    int AttackDamage;
    int CurrentHP;
    int MaxHP;
    int CurrentEnergy;
    int MaxEnergy;
    bool InCombat;
    bool Alive;
    float AttackSpeed;


    Unit(int id, int u, int index){
        this->UnitID = id;
        this->Type = u;
        this->PositionIndex = index;
        if(u == 1){
            //this->AttackRange = 1;
            this->AttackDamage = 10;
            this->CurrentHP = 100;
            this->MaxHP = 100;
            this->CurrentEnergy = 0;
            this->MaxEnergy = 0;
            this->Alive = true;
            this->InCombat = false;
            this->AttackSpeed = 1.0;
        }
        else if(u == 2){
            //this->AttackRange = 3;
            this->AttackDamage = 100;
            this->CurrentHP = 50;
            this->MaxHP = 50;
            this->CurrentEnergy = 0;
            this->MaxEnergy = 100;
            this->Alive = true;
            this->InCombat = false;
            this->AttackSpeed = 0.0;
        }
        else if(u == 3){
            //this->AttackRange = 1;
            this->AttackDamage = 15;
            this->CurrentHP = 30;
            this->MaxHP = 30;
            this->CurrentEnergy = 0;
            this->MaxEnergy = 0;
            this->Alive = true;
            this->InCombat = false;
            this->AttackSpeed = 1.0;
        }
        else{ // ovo je dummy za udaranje , za testiranje
            //this->AttackRange = 1;
            this->AttackDamage = 0;
            this->CurrentHP = 300;
            this->MaxHP = 300;
            this->CurrentEnergy = 0;
            this->MaxEnergy = 0;
            this->Alive = true;
            this->InCombat = false;
            this->AttackSpeed = 1.0;
        }
    }

    /*void move(vec2 dir){
        //TODO: OVDE SAM STAO POMERI GA ZA TAJ DIR I UPDEJTUJ SVE MAPE I SRANJA
    }
*/
};


#endif //AUTO_BATTLER_UNIT_H
