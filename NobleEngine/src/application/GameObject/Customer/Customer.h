#pragma once
#include<GameObject/HumanModel/HumanModel.h>
class Customer :
    public HumanModel
{
public:
    Customer();
    ~Customer();
    void Load()override;
    void UpdateAnimation()override;
};

