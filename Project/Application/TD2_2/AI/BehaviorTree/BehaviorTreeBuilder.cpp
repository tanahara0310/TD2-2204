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
