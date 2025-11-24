#pragma once
#include <unordered_map>
#include <string>
#include "EngineSystem.h"

enum class ActionType {
   Bool,
   Vector2
};

class InputSource {
public:
   static void Initialize(EngineSystem* engine);
   virtual ~InputSource() = default;
   virtual bool GetBool() const { return false; }
   virtual bool GetBoolDown() const { return false; }
   virtual bool GetBoolUp() const { return false; }
   virtual Vector2 GetVector2() const { return {}; }
};

class KeyboardButtonSource : public InputSource {
public:
   KeyboardButtonSource(uint8_t key) : key_(key) {}

   bool GetBool() const override;
   bool GetBoolDown() const override;
   bool GetBoolUp() const override;

private:
   uint8_t key_;
};

class GamepadButtonSource : public InputSource {
public:
   GamepadButtonSource(GamepadButton btn) : btn_(btn) {}

   bool GetBool() const override;
   bool GetBoolDown() const override;
   bool GetBoolUp() const override;
private:
   GamepadButton btn_;

};

class KeyboardAxis2DSource : public InputSource {
public:
   KeyboardAxis2DSource(uint8_t up, uint8_t down, uint8_t left, uint8_t right)
	  : up_(up), down_(down), left_(left), right_(right) {}

   Vector2 GetVector2() const override;

private:
   uint8_t up_, down_, left_, right_;
};


class GamepadAxis2DSource : public InputSource {
public:
   Vector2 GetVector2() const override;
};

class Action {
public:
   ActionType type;
   std::vector<std::unique_ptr<InputSource>> sources;

   // Bool の合成
   bool GetBool() const {
	  for (auto& s : sources)
		 if (s->GetBool()) return true;
	  return false;
   }
   bool GetBoolDown() const {
	  for (auto& s : sources)
		 if (s->GetBoolDown()) return true;
	  return false;
   }
   bool GetBoolUp() const {
	  for (auto& s : sources)
		 if (s->GetBoolUp()) return true;
	  return false;
   }

   // Vector2 の合成（加算）
   Vector2 GetVector2() const {
	  Vector2 result = { 0,0 };
	  for (auto& s : sources) {
		 result.x = result.x + s->GetVector2().x;
		 result.y = result.y + s->GetVector2().y;
	  }
	  return result;
   }
};


class KeyConfig {
public:
   Action& AddAction(const std::string& name, ActionType type) {
	  auto& act = actions_[name]; // map の中の実体
	  act.type = type;
	  return act; // 確実に map 内実体の参照を返す
   }

   template<typename T>
   T Get(const std::string& name) const;

   bool GetDown(const std::string& name) const {
	  return actions_.at(name).GetBoolDown();
   }
   bool GetUp(const std::string& name) const {
	  return actions_.at(name).GetBoolUp();
   }

   Action& GetAction(const std::string& name) {
	  return actions_.at(name);
   }

private:
   std::unordered_map<std::string, Action> actions_;
};

// 特化
template<>
inline bool KeyConfig::Get<bool>(const std::string& name) const {
   return actions_.at(name).GetBool();
}

template<>
inline Vector2 KeyConfig::Get<Vector2>(const std::string& name) const {
   return actions_.at(name).GetVector2();
}

class ActionBuilder {
   Action& action_;

public:
   ActionBuilder(Action& a) : action_(a) {}

   ActionBuilder& BindKey(uint8_t key) {
	  action_.sources.push_back(std::make_unique<KeyboardButtonSource>(key));
	  return *this;
   }

   ActionBuilder& BindGamepadButton(GamepadButton btn) {
	  action_.sources.push_back(std::make_unique<GamepadButtonSource>(btn));
	  return *this;
   }

   // WASD を uint8_t で指定
   ActionBuilder& BindKeyboardWASD(
	  uint8_t up, uint8_t down,
	  uint8_t left, uint8_t right)
   {
	  action_.sources.push_back(
		 std::make_unique<KeyboardAxis2DSource>(up, down, left, right)
	  );
	  return *this;
   }

   ActionBuilder& BindGamepadLeftStick() {
	  action_.sources.push_back(std::make_unique<GamepadAxis2DSource>());
	  return *this;
   }
};
