#pragma once
#include "EngineSystem.h"
#include "KeyConfig.h"
#include "MathCore.h"

class GameUtils {
public:
   static void Initialize(EngineSystem* engine);

   static float GetDeltaTime();

   static float RandomFloat(float min, float max);
private:
};
