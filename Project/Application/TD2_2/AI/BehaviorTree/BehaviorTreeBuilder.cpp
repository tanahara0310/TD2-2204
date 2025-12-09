#include "BehaviorTreeBuilder.h"

BehaviorTreeBuilder& BehaviorTreeBuilder::Selector() {
    stack_.push_back(std::make_unique<SelectorNode>());
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::Sequence() {
   stack_.push_back(std::make_unique<SequenceNode>());
   return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::WeightedSelector() {
   stack_.push_back(std::make_unique<WeightedRandomSelectorNode>());
   return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::Parallel(ParallelPolicy policy) {
   stack_.push_back(std::make_unique<ParallelNode>(policy));
   return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::Condition(std::function<bool()> func) {
   AddToCurrent(std::make_unique<ConditionNode>(std::move(func)));
   return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::Inverter() {
   stack_.push_back(std::make_unique<InverterNode>(nullptr));
   return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::Succeeder() {
   stack_.push_back(std::make_unique<SucceederNode>(nullptr));
   return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::Repeater(int repeatCount) {
   stack_.push_back(std::make_unique<RepeaterNode>(nullptr, repeatCount));
   return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::Retry() {
   stack_.push_back(std::make_unique<RetryNode>(nullptr));
   return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::Wait(float duration) {
   AddToCurrent(std::make_unique<WaitNode>(duration));
   return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::WeightedNode(std::unique_ptr<BaseNode> node, float weight) {
   auto* weighted = dynamic_cast<WeightedRandomSelectorNode*>(stack_.back().get());
   assert(weighted && "Current node is not WeightedRandomSelectorNode!");
   weighted->AddChild(std::move(node), weight);
   return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::WeightedNode(std::unique_ptr<BaseNode> node, std::unique_ptr<IEvaluator> evaluator) {
   auto* weighted = dynamic_cast<WeightedRandomSelectorNode*>(stack_.back().get());
   assert(weighted && "Current node is not WeightedRandomSelectorNode!");
   weighted->AddChild(std::move(node), std::move(evaluator));
   return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::End() {
   if (stack_.size() > 1) {
      auto node = std::move(stack_.back());
      stack_.pop_back();
      
      // Decoratorノードの場合、その前のスタック要素を子として設定
      if (auto* decorator = dynamic_cast<DecoratorNode*>(node.get())) {
         // Decoratorは直前に構築されたノードを子とする
         // しかし、この設計では直前のノードがスタックにないため、
         // Decoratorの使用方法を変更する必要がある
         // 代わりに、Decoratorノードをスタックに残して、次のEnd()で子を設定する
      }
      
      // 親がDecoratorの場合は特別処理
      if (stack_.size() > 0) {
         if (auto* decorator = dynamic_cast<DecoratorNode*>(stack_.back().get())) {
            // Decoratorに子ノードを設定
            decorator->SetChild(std::move(node));
            return *this;
         }
      }
      
      AddToCurrent(std::move(node));
   }
   return *this;
}

std::unique_ptr<BaseNode> BehaviorTreeBuilder::Build() {
   assert(stack_.size() == 1 && "Unbalanced Begin/End calls in builder!");
   return std::move(stack_.front());
}

std::unique_ptr<BaseNode> BehaviorTreeBuilder::BuildSubTree() {
   assert(stack_.size() == 1 && "Unbalanced Begin/End calls in sub-tree builder!");
   return std::move(stack_.front());
}

BehaviorTreeBuilder& BehaviorTreeBuilder::HpCondition(
   std::function<float()> getHpRatio,
   float threshold,
   CompareType compareType) {

   // 判定ロジックを作成し、Conditionノードとして追加
   return Condition([getHpRatio, threshold, compareType]() {
      float hp = getHpRatio();
      switch (compareType) {
         case CompareType::LessThan:    return hp < threshold;
         case CompareType::LessEqual:   return hp <= threshold;
         case CompareType::GreaterThan: return hp > threshold;
         case CompareType::GreaterEqual:return hp >= threshold;
      }
      return false;
      });
}

// 【追加】条件付き実行ノード (If-Then構造)
BehaviorTreeBuilder& BehaviorTreeBuilder::IfThenNode(
   std::function<bool()> condition,
   std::unique_ptr<BaseNode> thenBranch) {

   // Sequence(Condition, ThenBranch)のノード構造を構築し、親に追加
   auto sequence = std::make_unique<SequenceNode>();
   sequence->AddChild(std::make_unique<ConditionNode>(std::move(condition)));
   sequence->AddChild(std::move(thenBranch));

   AddToCurrent(std::move(sequence));

   return *this;
}

// 【追加】条件分岐ノード (If-Else構造)
BehaviorTreeBuilder& BehaviorTreeBuilder::IfElseNode(
   std::function<bool()> condition,
   std::unique_ptr<BaseNode> trueBranch,
   std::unique_ptr<BaseNode> falseBranch) {

   // Selectorノードをルートとする構造を構築
   // Selector
   //   ├─ Sequence(Condition, TrueBranch) <- 条件が真の場合に成功
   //   └─ FalseBranch                     <- Sequenceが失敗（条件が偽）の場合に実行

   auto selector = std::make_unique<SelectorNode>();

   // 1. True Branch: Sequence(Condition, TrueBranch)
   auto conditionSequence = std::make_unique<SequenceNode>();
   conditionSequence->AddChild(std::make_unique<ConditionNode>(std::move(condition)));
   conditionSequence->AddChild(std::move(trueBranch));

   // 2. SelectorにConditionSequenceとFalseBranchを追加
   selector->AddChild(std::move(conditionSequence));
   selector->AddChild(std::move(falseBranch));

   AddToCurrent(std::move(selector));

   return *this;
}