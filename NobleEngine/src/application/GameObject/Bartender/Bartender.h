#pragma once
#include<GameObject/HumanModel/HumanModel.h>

class Bartender :
    public HumanModel
{
public:
    Bartender();
    ~Bartender();
    void Load()override;
private:
    void UpdateAnimation()override;

};

