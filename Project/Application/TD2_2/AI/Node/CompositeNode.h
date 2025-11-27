#pragma once
#include "BaseNode.h"
#include <vector>
#include <memory>
#include <numeric>
#include "Evaluator.h"

/// @brief 子ノードを複数持つノードの基底クラス
class CompositeNode : public BaseNode {
public:
   virtual ~CompositeNode() = default;

   virtual void AddChild(std::unique_ptr<BaseNode> child);

protected:
   std::vector<std::unique_ptr<BaseNode>> children_;
};

/// @brief セレクターノード - 子ノードを順に実行し、最初に成功したノードの結果を返す
class SelectorNode : public CompositeNode {
public:
   NodeState Tick() override;
private:
   size_t currentIndex_ = 0;
};

/// @brief シーケンスノード - 子ノードを順に実行し、最初に失敗したノードの結果を返す
class SequenceNode : public CompositeNode {
public:
   NodeState Tick() override;
private:
   size_t currentIndex_ = 0;
};

/// @brief パラレルノード - 複数の子ノードを同時に実行し、ポリシーに基づいて結果を決定する
enum class ParallelPolicy {
   SuccessWhenAllSucceed,  // すべての子がSuccessのとき全体Success
   SuccessWhenAnySucceed,  // 1つでもSuccessなら全体Success
   StopWhenOneFails        // 1つでもFailureが出たら全体Failure
};

/// @brief パラレルノードクラス・並列実行ノード
class ParallelNode : public CompositeNode {
public:
   explicit ParallelNode(ParallelPolicy policy = ParallelPolicy::SuccessWhenAllSucceed);

   NodeState Tick() override;

private:
   std::vector<NodeState> childrenStates_;
   ParallelPolicy policy_;
};

/// @brief ランダムセレクターノード - 子ノードをランダムに選択して実行する
class RandomSelectorNode : public CompositeNode {
public:
   NodeState Tick() override;
};

/// @brief 重み付きランダムセレクターノード - 子ノードを重みに基づいてランダムに選択して実行する
class WeightedRandomSelectorNode : public CompositeNode {
public:
   struct Entry {
      std::unique_ptr<BaseNode> node;
      std::unique_ptr<IEvaluator> evaluator;
   };

   // 通常の固定重み
   void AddChild(std::unique_ptr<BaseNode> child, float staticWeight = 1.0f);

   // 動的な評価関数
   void AddChild(std::unique_ptr<BaseNode> child, std::unique_ptr<IEvaluator> evaluator);

   NodeState Tick() override;

private:
   std::vector<Entry> entries_;
   int currentIndex_ = -1;
};