#include "GameObject.h"

namespace {
EngineSystem* sEngine = nullptr;
}

void GameObject::InitializeSystem(EngineSystem* engine) {
   if (sEngine == nullptr) {
	  sEngine = engine;
   }
}

EngineSystem* GameObject::GetEngineSystem() const {
   if (sEngine == nullptr) {
	  return sEngine;
   }
}
