#pragma once
#include "../GameObject.h"

class Player : public GameObject {
public:
   Player() = default;
   ~Player() = default;
   //void Initialize(Model* model, TextureManager::LoadedTexture texture);
   void Update() override;
   void Draw(ICamera* camera) override;
};