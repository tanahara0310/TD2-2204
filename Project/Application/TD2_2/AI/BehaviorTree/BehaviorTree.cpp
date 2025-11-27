#include "BehaviorTree.h"

void BehaviorTree::SetRoot(std::unique_ptr<BaseNode> root) {
   root_ = std::move(root);
   tickCount_ = 0;
}

NodeState BehaviorTree::Tick() {
   if (!root_) {
      return NodeState::Failure;
   }

   tickCount_++;
   return root_->Tick();
}

void BehaviorTree::Reset() {
   tickCount_ = 0;
   // 必要に応じてルートノードのリセット処理を追加
}
