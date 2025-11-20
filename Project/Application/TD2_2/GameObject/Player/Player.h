#pragma once
#include "../GameObject.h"

class Player : public GameObject {
public:
   Player() = default;
   ~Player() = default;
   void Initialize();
   void Update() override;
   void Draw(ICamera* camera) override;


};