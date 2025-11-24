#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <algorithm>

class StateMachine {
public:
   StateMachine();

   struct State {
      std::function<void()> onEnter;   // 状態切替時に1回
      std::function<void()> onUpdate;  // 状態中は毎フレーム
   };

   /// @brief 状態を登録
   void AddState(const std::string& name,
      std::function<void()> onEnter = nullptr,
      std::function<void()> onUpdate = nullptr);

   /// @brief 状態リクエストを追加
   void RequestState(const std::string& stateName, int priority);

   /// @brief 現在の状態名を取得
   const std::string& GetCurrentState() const { return currentState_; }

   /// @brief 遷移ルールを追加
   void AddTransitionRule(const std::string& from, const std::vector<std::string>& toList);

   /// @brief Update を呼ぶだけで Resolve + onUpdate を実行
   void Update();

   /// @brief リクエストをクリア
   void Clear();

private:
   const std::string& Resolve();
   bool CanTransition(const std::string& newState) const;

private:
   std::unordered_map<std::string, int> requests_;
   std::unordered_map<std::string, State> states_;
   std::string currentState_ = "Idle";
   std::unordered_map<std::string, std::vector<std::string>> transitionRules_;
};
