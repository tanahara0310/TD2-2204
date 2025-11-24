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

Vector2 GameUtils::Normalize(const Vector2& v) {
   Vector2 result = {};
   float length = std::sqrt(v.x * v.x + v.y * v.y);
   if (length > 0.0f) {
	  result.x = v.x / length;
	  result.y = v.y / length;
	  return result;
   }

   return { 0.0f, 0.0f };
}
