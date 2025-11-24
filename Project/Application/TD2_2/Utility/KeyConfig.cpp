#include "KeyConfig.h"
#include "EngineSystem.h"

namespace {
KeyboardInput* sKeyboard = nullptr;
GamepadInput* sGamepad = nullptr;
}

void InputSource::Initialize(EngineSystem* engine) {
   sKeyboard = engine->GetComponent<KeyboardInput>();
   sGamepad = engine->GetComponent<GamepadInput>();
}

bool KeyboardButtonSource::GetBool() const {
   return sKeyboard->IsKeyPressed(key_);
}

bool KeyboardButtonSource::GetBoolDown() const {
   return sKeyboard->IsKeyTriggered(key_);
}

bool KeyboardButtonSource::GetBoolUp() const {
   return sKeyboard->IsKeyReleased(key_);
}

bool GamepadButtonSource::GetBool() const {
   return sGamepad->IsButtonPressed(btn_);
}

bool GamepadButtonSource::GetBoolDown() const {
   return sGamepad->IsButtonTriggered(btn_);
}
bool GamepadButtonSource::GetBoolUp() const {
   return sGamepad->IsButtonReleased(btn_);
}

Vector2 KeyboardAxis2DSource::GetVector2() const {
   float x = 0.0f;
   float y = 0.0f;

   if (sKeyboard->IsKeyPressed(left_))  x -= 1;
   if (sKeyboard->IsKeyPressed(right_)) x += 1;
   if (sKeyboard->IsKeyPressed(up_))    y += 1;
   if (sKeyboard->IsKeyPressed(down_))  y -= 1;

   return { x, y };
}

Vector2 GamepadAxis2DSource::GetVector2() const {
   Stick s = sGamepad->GetLeftStick();
   return { s.x, s.y };
}
