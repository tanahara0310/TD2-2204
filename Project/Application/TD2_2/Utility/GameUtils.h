#pragma once
#include "EngineSystem.h"
#include "KeyConfig.h"
#include "MathCore.h"

class GameUtils {
public:
   static void Initialize(EngineSystem* engine);

   static float GetDeltaTime();

   static Vector2 Normalize(const Vector2& v);
private:
};
