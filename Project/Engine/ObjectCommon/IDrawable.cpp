#include "IDrawable.h"

namespace {
EngineSystem* sEngine = nullptr;
}

void IDrawable::Initialize(EngineSystem* engine) {
	if (sEngine == nullptr) {
		sEngine = engine;
	}
}

EngineSystem* IDrawable::GetEngineSystem() const {
	return sEngine;
}