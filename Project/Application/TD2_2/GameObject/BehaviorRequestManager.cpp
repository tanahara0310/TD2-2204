#include "BehaviorRequestManager.h"

BehaviorRequestManager::BehaviorRequestManager() {}

// --------------------------------------------------------
// 行動登録
// --------------------------------------------------------
void BehaviorRequestManager::AddBehavior(const std::string& name,
   std::function<void()> onEnter,
   std::function<void()> onUpdate)
{
   behaviors_[name] = { onEnter, onUpdate };
}

// --------------------------------------------------------
// 行動リクエスト追加
// --------------------------------------------------------
void BehaviorRequestManager::Request(const std::string& behaviorName, int priority)
{
   if (!CanInterrupt(behaviorName)) return;

   auto it = requests_.find(behaviorName);
   if (it != requests_.end()) {
      if (priority > it->second) it->second = priority;
   } else {
      requests_.insert({ behaviorName, priority });
   }
}

// --------------------------------------------------------
// 最優先行動を決定
// --------------------------------------------------------
const std::string& BehaviorRequestManager::Resolve()
{
   if (requests_.empty()) return currentBehavior_;

   int bestPriority = -999999;
   std::string bestBehavior = currentBehavior_;

   for (const auto& req : requests_) {
      if (req.second > bestPriority) {
         bestPriority = req.second;
         bestBehavior = req.first;
      }
   }

   // 行動が切り替わった場合に onEnter を呼ぶ
   if (bestBehavior != currentBehavior_) {
      currentBehavior_ = bestBehavior;
      auto it = behaviors_.find(bestBehavior);
      if (it != behaviors_.end() && it->second.onEnter) {
         it->second.onEnter();
      }
   }

   requests_.clear();
   return currentBehavior_;
}

// --------------------------------------------------------
// Update 呼び出しで Resolve + onUpdate
// --------------------------------------------------------
void BehaviorRequestManager::Update()
{
   const std::string& state = Resolve();
   auto it = behaviors_.find(state);
   if (it != behaviors_.end() && it->second.onUpdate) {
      it->second.onUpdate();
   }
}

// --------------------------------------------------------
// 割り込みルール追加
// --------------------------------------------------------
void BehaviorRequestManager::AddInterruptRule(const std::string& from,
   const std::vector<std::string>& toList)
{
   interruptRules_[from] = toList;
}

// --------------------------------------------------------
// 割り込み判定
// --------------------------------------------------------
bool BehaviorRequestManager::CanInterrupt(const std::string& newBehavior) const
{
   auto it = interruptRules_.find(currentBehavior_);
   if (it == interruptRules_.end()) return true;

   const auto& allowedList = it->second;
   return std::find(allowedList.begin(), allowedList.end(), newBehavior) != allowedList.end();
}

// --------------------------------------------------------
// リクエストクリア
// --------------------------------------------------------
void BehaviorRequestManager::Clear()
{
   requests_.clear();
}
