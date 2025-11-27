#pragma once
#include "BaseNode.h"

class LeafNode : public BaseNode {
   public:
	virtual ~LeafNode() = default;
};

class ConditionNode : public LeafNode {
public:
   using ConditionFunc = std::function<bool()>;

   explicit ConditionNode(ConditionFunc condition);

   NodeState Tick() override;

private:
   ConditionFunc condition_;
};

class ActionNode : public LeafNode {
public:
   using ActionFunc = std::function<NodeState()>;

   explicit ActionNode(ActionFunc action);

   NodeState Tick() override;

private:
   ActionFunc action_;
};

class WaitNode : public LeafNode {
public:
   explicit WaitNode(float duration);

   NodeState Tick() override;
   void Reset();

private:
   float duration_;      // 待機時間（秒）
   float elapsedTime_;   // 経過時間（秒）
   bool isRunning_;      // 実行中フラグ
};