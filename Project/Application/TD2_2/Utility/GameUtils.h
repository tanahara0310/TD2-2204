#pragma once
#include "EngineSystem.h"
#include "KeyConfig.h"
#include "MathCore.h"

class GameUtils {
public:
   static void Initialize(EngineSystem* engine);

   static float GetDeltaTime();

   static float RandomFloat(float min, float max);
   static float PerlinNoise1D(float x);
   static float Gradient(int hash);
   static float Lerp(float a, float b, float t);
   static float ParlineNoise2D(float x, float y);
private:
   static void InitPerm();
   static float Fade(float t);
   static float Grad(int hash, float x, float y);
   static int perm[512];
   static bool isPermInitialized;
};
