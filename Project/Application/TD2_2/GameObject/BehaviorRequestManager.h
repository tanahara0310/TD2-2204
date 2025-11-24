#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <algorithm>

class BehaviorRequestManager {
public:
   BehaviorRequestManager();

   struct Behavior {
      std::function<void()> onEnter;   // 行動切替時に1回
      std::function<void()> onUpdate;  // 行動中は毎フレーム
   };

   /// @brief 行動を登録
   void AddBehavior(const std::string& name,
      std::function<void()> onEnter = nullptr,
      std::function<void()> onUpdate = nullptr);

   /// @brief 行動リクエストを追加
   void Request(const std::string& behaviorName, int priority);

   /// @brief 現在の行動名を取得
   const std::string& GetCurrentBehavior() const { return currentBehavior_; }

   /// @brief 割り込みルールを追加
   void AddInterruptRule(const std::string& from, const std::vector<std::string>& toList);

   /// @brief Update を呼ぶだけで Resolve + onUpdate を実行
   void Update();

   /// @brief リクエストをクリア
   void Clear();

private:
   const std::string& Resolve();
   bool CanInterrupt(const std::string& newBehavior) const;

private:
   std::unordered_map<std::string, int> requests_;
   std::unordered_map<std::string, Behavior> behaviors_;
   std::string currentBehavior_ = "Idle";
   std::unordered_map<std::string, std::vector<std::string>> interruptRules_;
};
