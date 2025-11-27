#include "GameUtils.h"
#include "Utility/FrameRate/FrameRateController.h"

namespace {
EngineSystem* sEngine = nullptr;
}

void GameUtils::Initialize(EngineSystem* engine) {
   if (sEngine) return;
   sEngine = engine;
   InputSource::Initialize(sEngine);
}

float GameUtils::GetDeltaTime() {
   if (sEngine) {
	  auto frameRateController = sEngine->GetComponent<FrameRateController>();
	  if (frameRateController) {
		 return frameRateController->GetDeltaTime();
	  }
   }
   return 0.0f;
}

float GameUtils::RandomFloat(float min, float max) {
   auto& randomGen = RandomGenerator::GetInstance();
   return randomGen.GetFloat(min, max);
}
