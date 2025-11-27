#include "DecoratorNode.h"

// DecoratorNode
DecoratorNode::DecoratorNode(std::unique_ptr<BaseNode> child)
   : child_(std::move(child)) {}

void DecoratorNode::SetChild(std::unique_ptr<BaseNode> child) {
   child_ = std::move(child);
}

// InverterNode
InverterNode::InverterNode(std::unique_ptr<BaseNode> child)
   : DecoratorNode(std::move(child)) {}

NodeState InverterNode::Tick() {
   if (!child_) {
      return NodeState::Failure;
   }

   NodeState result = child_->Tick();

   if (result == NodeState::Success) {
      return NodeState::Failure;
   } else if (result == NodeState::Failure) {
      return NodeState::Success;
   }

   return result; // Running
}

// SucceederNode
SucceederNode::SucceederNode(std::unique_ptr<BaseNode> child)
   : DecoratorNode(std::move(child)) {}

NodeState SucceederNode::Tick() {
   if (!child_) {
      return NodeState::Success;
   }

   child_->Tick();
   return NodeState::Success;
}

// RepeaterNode
RepeaterNode::RepeaterNode(std::unique_ptr<BaseNode> child, int repeatCount)
   : DecoratorNode(std::move(child)), repeatCount_(repeatCount), currentCount_(0) {}

NodeState RepeaterNode::Tick() {
   if (!child_) {
      return NodeState::Failure;
   }

   // 無限ループの場合
   if (repeatCount_ < 0) {
      child_->Tick();
      return NodeState::Running;
   }

   while (currentCount_ < repeatCount_) {
      NodeState result = child_->Tick();

      if (result == NodeState::Running) {
         return NodeState::Running;
      }

      currentCount_++;

      if (currentCount_ >= repeatCount_) {
         currentCount_ = 0;
         return NodeState::Success;
      }
   }

   currentCount_ = 0;
   return NodeState::Success;
}

void RepeaterNode::Reset() {
   currentCount_ = 0;
}

RetryNode::RetryNode(std::unique_ptr<BaseNode> child)
   : DecoratorNode(std::move(child)) {}

NodeState RetryNode::Tick() {
   if (!child_) {
      return NodeState::Failure;
   }

   NodeState result = child_->Tick();

   if (result == NodeState::Success) {
      return NodeState::Success;
   }

   // FailureまたはRunningの場合は継続
   return NodeState::Running;
}
