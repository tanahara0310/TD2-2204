#define NOMINMAX
#include "CompositeNode.h"
#include <random>
#include <algorithm>

namespace {
std::mt19937 rng_{ std::random_device{}() };
}

void CompositeNode::AddChild(std::unique_ptr<BaseNode> child) {
   children_.push_back(std::move(child));
}

NodeState SelectorNode::Tick() {
   // 子が1つもない場合
   if (children_.empty()) return NodeState::Failure;

   // 前回Runningだった子から再開
   while (currentIndex_ < children_.size()) {
	  NodeState state = children_[currentIndex_]->Tick();

	  switch (state) {
		 case NodeState::Running:
			// この子がまだ実行中 → 状態維持
			return NodeState::Running;

		 case NodeState::Success:
			// 成功したら全体も成功
			currentIndex_ = 0; // 次回に備えてリセット
			return NodeState::Success;

		 case NodeState::Failure:
			// 失敗したので次の子を試す
			currentIndex_++;
			break;
	  }
   }

   // 全ての子が失敗したら、全体も失敗
   currentIndex_ = 0;
   return NodeState::Failure;
}

NodeState SequenceNode::Tick() {
   while (currentIndex_ < children_.size()) {
	  NodeState state = children_[currentIndex_]->Tick();
	  if (state == NodeState::Running) return NodeState::Running;
	  if (state == NodeState::Failure) {
		 currentIndex_ = 0;
		 return NodeState::Failure;
	  }
	  currentIndex_++;
   }
   currentIndex_ = 0;
   return NodeState::Success;
}

ParallelNode::ParallelNode(ParallelPolicy policy)
   : policy_(policy) {}

NodeState ParallelNode::Tick() {
   // 子が1つもない場合
   if (children_.empty()) return NodeState::Failure;

   // 初回または状態リセット時に各子の状態を初期化
   if (childrenStates_.size() != children_.size()) {
	  childrenStates_.resize(children_.size(), NodeState::Running);
   }

   // すべての子ノードを実行
   size_t successCount = 0;
   size_t failureCount = 0;
   size_t runningCount = 0;

   for (size_t i = 0; i < children_.size(); ++i) {
	  // 既に終了している子はスキップ（状態を保持）
	  if (childrenStates_[i] != NodeState::Running) {
		 if (childrenStates_[i] == NodeState::Success) {
			successCount++;
		 } else if (childrenStates_[i] == NodeState::Failure) {
			failureCount++;
		 }
		 continue;
	  }

	  // 子ノードを実行
	  NodeState state = children_[i]->Tick();
	  childrenStates_[i] = state;

	  switch (state) {
		 case NodeState::Success:
			successCount++;
			break;
		 case NodeState::Failure:
			failureCount++;
			break;
		 case NodeState::Running:
			runningCount++;
			break;
	  }
   }

   // ポリシーに応じて全体の状態を決定
   switch (policy_) {
	  case ParallelPolicy::SuccessWhenAllSucceed:
		 // すべての子がSuccessなら成功
		 if (successCount == children_.size()) {
			childrenStates_.clear(); // 次回に備えてリセット
			return NodeState::Success;
		 }
		 // 1つでもFailureがあれば失敗
		 if (failureCount > 0) {
			childrenStates_.clear(); // 次回に備えてリセット
			return NodeState::Failure;
		 }
		 // それ以外はまだ実行中
		 return NodeState::Running;

	  case ParallelPolicy::SuccessWhenAnySucceed:
		 // 1つでもSuccessなら成功
		 if (successCount > 0) {
			childrenStates_.clear(); // 次回に備えてリセット
			return NodeState::Success;
		 }
		 // すべての子がFailureなら失敗
		 if (failureCount == children_.size()) {
			childrenStates_.clear(); // 次回に備えてリセット
			return NodeState::Failure;
		 }
		 // それ以外はまだ実行中
		 return NodeState::Running;

	  case ParallelPolicy::StopWhenOneFails:
		 // 1つでもFailureが出たら全体失敗
		 if (failureCount > 0) {
			childrenStates_.clear(); // 次回に備えてリセット
			return NodeState::Failure;
		 }
		 // すべての子がSuccessなら成功
		 if (successCount == children_.size()) {
			childrenStates_.clear(); // 次回に備えてリセット
			return NodeState::Success;
		 }
		 // それ以外はまだ実行中
		 return NodeState::Running;
   }

   // デフォルトでRunning（到達しないはず）
   return NodeState::Running;
}

NodeState RandomSelectorNode::Tick() {
   if (children_.empty()) return NodeState::Failure;
   std::uniform_int_distribution<size_t> dist(0, children_.size() - 1);
   size_t index = dist(rng_);
   return children_[index]->Tick();
}

void WeightedRandomSelectorNode::AddChild(std::unique_ptr<BaseNode> child, float staticWeight) {
   auto eval = std::make_unique<LambdaEvaluator>([=]() { return staticWeight; });
   entries_.push_back({ std::move(child), std::move(eval) });
}

void WeightedRandomSelectorNode::AddChild(std::unique_ptr<BaseNode> child, std::unique_ptr<IEvaluator> evaluator){
   entries_.push_back({ std::move(child), std::move(evaluator) });
}

NodeState WeightedRandomSelectorNode::Tick() {
   if (entries_.empty()) return NodeState::Failure;

   // 現在 Running 中のノードがある場合は継続
   if (currentIndex_ >= 0 && currentIndex_ < static_cast<int>(entries_.size())) {
	  auto& entry = entries_[currentIndex_];
	  NodeState state = entry.node->Tick();
	  if (state == NodeState::Running) return NodeState::Running;
	  currentIndex_ = -1;
	  return state;
   }

   // 各ノードの評価値を計算
   std::vector<float> weights;
   weights.reserve(entries_.size());
   for (auto& e : entries_) {
	  float w = e.evaluator ? e.evaluator->Evaluate() : 0.0f;
	  weights.push_back(std::max(0.0f, w));
   }

   // 合計
   float totalWeight = std::accumulate(weights.begin(), weights.end(), 0.0f);
   if (totalWeight <= 0.0f) return NodeState::Failure;

   // ランダム選択
   std::uniform_real_distribution<float> dist(0.0f, totalWeight);
   float r = dist(rng_);

   float cumulative = 0.0f;
   for (size_t i = 0; i < entries_.size(); ++i) {
	  cumulative += weights[i];
	  if (r <= cumulative) {
		 currentIndex_ = static_cast<int>(i);
		 return entries_[i].node->Tick();
	  }
   }

   return NodeState::Failure;
}
