#include "LeafNode.h"

ConditionNode::ConditionNode(ConditionFunc condition)
   : condition_(std::move(condition)) {}

NodeState ConditionNode::Tick() {
   return condition_() ? NodeState::Success : NodeState::Failure;
}

ActionNode::ActionNode(ActionFunc action)
   : action_(std::move(action)) {}

NodeState ActionNode::Tick() {
   return action_();
}

WaitNode::WaitNode(float duration)
   : duration_(duration), elapsedTime_(0.0f), isRunning_(false) {}

NodeState WaitNode::Tick() {
   if (!isRunning_) {
      isRunning_ = true;
      elapsedTime_ = 0.0f;
   }

   elapsedTime_ += 1.0f / 60.0f; // フレーム時間を加算（必要に応じてdeltaTimeを渡すように変更可能）

   if (elapsedTime_ >= duration_) {
      isRunning_ = false;
      return NodeState::Success;
   }

   return NodeState::Running;
}

void WaitNode::Reset() {
   elapsedTime_ = 0.0f;
   isRunning_ = false;
}