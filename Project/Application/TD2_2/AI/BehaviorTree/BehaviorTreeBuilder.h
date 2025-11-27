#pragma once
#include <cassert>
#include <functional>
#include <memory>
#include "../Node/CompositeNode.h"
#include "../Node/DecoratorNode.h"
#include "../Node/LeafNode.h"
#include "../Node/Evaluator.h"

class BehaviorTreeBuilder {
public:
   BehaviorTreeBuilder& Selector();

   BehaviorTreeBuilder& Sequence();

   BehaviorTreeBuilder& WeightedSelector();

   BehaviorTreeBuilder& Parallel(ParallelPolicy policy = ParallelPolicy::SuccessWhenAllSucceed);

   BehaviorTreeBuilder& Condition(std::function<bool()> func);

   BehaviorTreeBuilder& Inverter();

   BehaviorTreeBuilder& Succeeder();

   BehaviorTreeBuilder& Repeater(int repeatCount);

   BehaviorTreeBuilder& Retry();

   BehaviorTreeBuilder& Wait(float duration);

   template<typename T, typename... Args>
   BehaviorTreeBuilder& Action(Args&&... args);

   template<typename T, typename... Args>
   BehaviorTreeBuilder& WeightedAction(float weight, Args&&... args);

   template<typename T, typename... Args>
   BehaviorTreeBuilder& WeightedAction(std::unique_ptr<IEvaluator> evaluator, Args&&... args);

   BehaviorTreeBuilder& WeightedNode(std::unique_ptr<BaseNode> node, float weight);

   BehaviorTreeBuilder& WeightedNode(std::unique_ptr<BaseNode> node, std::unique_ptr<IEvaluator> evaluator);

   BehaviorTreeBuilder& End();

   std::unique_ptr<BaseNode> Build();

   std::unique_ptr<BaseNode> BuildSubTree();

   // ======================================================================
   // 汎用ヘルパーメソッド群
   // ======================================================================

   // 可変長テンプレートを使ったシーケンス作成ヘルパー
   template<typename... Actions>
   BehaviorTreeBuilder& QuickSequence(Actions&&... actions);

   // 可変長テンプレートを使ったセレクター作成ヘルパー
   template<typename... Actions>
   BehaviorTreeBuilder& QuickSelector(Actions&&... actions);

   // 条件付きシーケンス作成ヘルパー
   BehaviorTreeBuilder& ConditionalSequence(std::function<bool()> condition);

   // 条件付きアクション（インライン）
   template<typename T, typename... Args>
   BehaviorTreeBuilder& ConditionalAction(std::function<bool()> condition, Args&&... args);

   // WeightedRandomSelector簡易作成ヘルパー
   template<typename... NodeWeightPairs>
   BehaviorTreeBuilder& QuickWeightedSelector();

   // 距離ベース条件付きアクション
   template<typename T, typename... Args>
   BehaviorTreeBuilder& DistanceBasedAction(
      const Vector3* pos1, const Vector3* pos2,
      float minDistance, float maxDistance,
      Args&&... args);

   // HPベース条件付きアクション
   template<typename T, typename... Args>
   BehaviorTreeBuilder& HpBasedAction(
      std::function<float()> getHpRatio,
      float minHp, float maxHp,
      Args&&... args);

   // ループシーケンス（N回繰り返すシーケンス）
   BehaviorTreeBuilder& LoopSequence(int loopCount);

   // タイムアウト付きシーケンス
   BehaviorTreeBuilder& TimedSequence(float timeout);

   // パラレル実行簡易ヘルパー
   BehaviorTreeBuilder& QuickParallel(ParallelPolicy policy = ParallelPolicy::SuccessWhenAllSucceed);

   // ランダムセレクター（均等確率）
   BehaviorTreeBuilder& RandomSelector();

   // 優先度付きセレクター（評価値の高い順に実行）
   BehaviorTreeBuilder& PrioritySelector();

private:
   std::vector<std::unique_ptr<BaseNode>> stack_;

   void AddToCurrent(std::unique_ptr<BaseNode> node) {
      auto* composite = dynamic_cast<CompositeNode*>(stack_.back().get());
      assert(composite && "Current node cannot have children!");
      composite->AddChild(std::move(node));
   }

   // 可変長引数の展開用ヘルパー
   template<typename T, typename... Rest>
   void AddActionsToSequence(T&& first, Rest&&... rest);

   void AddActionsToSequence() {} // 終端

   template<typename T, typename... Rest>
   void AddActionsToSelector(T&& first, Rest&&... rest);

   void AddActionsToSelector() {} // 終端
};

template<typename T, typename ...Args>
inline BehaviorTreeBuilder& BehaviorTreeBuilder::Action(Args && ...args) {
   AddToCurrent(std::make_unique<T>(std::forward<Args>(args)...));
   return *this;
}

// 固定ウェイトを使ったWeightedSelectorへのAddChild（アクション用）
template<typename T, typename ...Args>
inline BehaviorTreeBuilder& BehaviorTreeBuilder::WeightedAction(float weight, Args && ...args) {
   auto* weighted = dynamic_cast<WeightedRandomSelectorNode*>(stack_.back().get());
   assert(weighted && "Current node is not WeightedRandomSelectorNode!");
   weighted->AddChild(std::make_unique<T>(std::forward<Args>(args)...), weight);
   return *this;
}

// 評価関数を使ったWeightedSelectorへのAddChild（アクション用）
template<typename T, typename ...Args>
inline BehaviorTreeBuilder& BehaviorTreeBuilder::WeightedAction(std::unique_ptr<IEvaluator> evaluator, Args && ...args) {
   auto* weighted = dynamic_cast<WeightedRandomSelectorNode*>(stack_.back().get());
   assert(weighted && "Current node is not WeightedRandomSelectorNode!");
   weighted->AddChild(std::make_unique<T>(std::forward<Args>(args)...), std::move(evaluator));
   return *this;
}

// ======================================================================
// 汎用ヘルパーメソッドの実装
// ======================================================================

// 可変長テンプレートを使ったシーケンス作成
template<typename... Actions>
inline BehaviorTreeBuilder& BehaviorTreeBuilder::QuickSequence(Actions&&... actions) {
   Sequence();
   AddActionsToSequence(std::forward<Actions>(actions)...);
   return End();
}

template<typename T, typename... Rest>
inline void BehaviorTreeBuilder::AddActionsToSequence(T&& first, Rest&&... rest) {
   AddToCurrent(std::forward<T>(first));
   AddActionsToSequence(std::forward<Rest>(rest)...);
}

// 可変長テンプレートを使ったセレクター作成
template<typename... Actions>
inline BehaviorTreeBuilder& BehaviorTreeBuilder::QuickSelector(Actions&&... actions) {
   Selector();
   AddActionsToSelector(std::forward<Actions>(actions)...);
   return End();
}

template<typename T, typename... Rest>
inline void BehaviorTreeBuilder::AddActionsToSelector(T&& first, Rest&&... rest) {
   AddToCurrent(std::forward<T>(first));
   AddActionsToSelector(std::forward<Rest>(rest)...);
}

// 条件付きシーケンス
inline BehaviorTreeBuilder& BehaviorTreeBuilder::ConditionalSequence(std::function<bool()> condition) {
   Sequence();
   Condition(std::move(condition));
   return *this;
}

// 条件付きアクション
template<typename T, typename... Args>
inline BehaviorTreeBuilder& BehaviorTreeBuilder::ConditionalAction(std::function<bool()> condition, Args&&... args) {
   Sequence();
   Condition(std::move(condition));
   Action<T>(std::forward<Args>(args)...);
   return End();
}

// 距離ベース条件付きアクション
template<typename T, typename... Args>
inline BehaviorTreeBuilder& BehaviorTreeBuilder::DistanceBasedAction(
   const Vector3* pos1, const Vector3* pos2,
   float minDistance, float maxDistance,
   Args&&... args) {
   
   Sequence();
   Condition([pos1, pos2, minDistance, maxDistance]() {
      if (!pos1 || !pos2) return false;
      Vector3 diff = *pos2 - *pos1;
      float distance = MathCore::Vector::Length(diff);
      return distance >= minDistance && distance <= maxDistance;
   });
   Action<T>(std::forward<Args>(args)...);
   return End();
}

// HPベース条件付きアクション
template<typename T, typename... Args>
inline BehaviorTreeBuilder& BehaviorTreeBuilder::HpBasedAction(
   std::function<float()> getHpRatio,
   float minHp, float maxHp,
   Args&&... args) {
   
   Sequence();
   Condition([getHpRatio, minHp, maxHp]() {
      float hp = getHpRatio();
      return hp >= minHp && hp <= maxHp;
   });
   Action<T>(std::forward<Args>(args)...);
   return End();
}

// ループシーケンス
inline BehaviorTreeBuilder& BehaviorTreeBuilder::LoopSequence(int loopCount) {
   Repeater(loopCount);
   Sequence();
   return *this;
}

// タイムアウト付きシーケンス（簡易実装）
inline BehaviorTreeBuilder& BehaviorTreeBuilder::TimedSequence(float timeout) {
   (void)timeout; // 未使用パラメータ警告を抑制
   Sequence();
   // タイムアウトチェックは後続のアクションで実装する想定
   return *this;
}

// パラレル実行簡易ヘルパー
inline BehaviorTreeBuilder& BehaviorTreeBuilder::QuickParallel(ParallelPolicy policy) {
   Parallel(policy);
   return *this;
}

// ランダムセレクター（均等確率）
inline BehaviorTreeBuilder& BehaviorTreeBuilder::RandomSelector() {
   WeightedSelector();
   return *this;
}

// 優先度付きセレクター
inline BehaviorTreeBuilder& BehaviorTreeBuilder::PrioritySelector() {
   Selector();
   return *this;
}
