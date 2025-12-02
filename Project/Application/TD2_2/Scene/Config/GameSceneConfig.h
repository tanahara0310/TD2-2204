#pragma once
#include "MathCore.h"

namespace GameSceneConfig {
const Vector2 kFrameSize = { 2.0f, 2.0f };
const Vector2 kStageSize = { kFrameSize.x * 32.0f,  kFrameSize.y * 18.0f };
const Vector2 kStageCenter = { 0.0f,0.0f };
const Vector2 kMoveableAreaSize = { kStageSize.x - kFrameSize.x * 2.0f, kStageSize.y - kFrameSize.y * 2.0f };
}