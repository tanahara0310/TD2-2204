#pragma once
#include "BaseNode.h"

// Decoratorノードの基底クラス
class DecoratorNode : public BaseNode {
public:
   explicit DecoratorNode(std::unique_ptr<BaseNode> child);
   virtual ~DecoratorNode() = default;

   // ビルダーパターン用：子ノードを設定
   void SetChild(std::unique_ptr<BaseNode> child);

protected:
   std::unique_ptr<BaseNode> child_;
};

// 子ノードの結果を反転させるDecorator
class InverterNode : public DecoratorNode {
public:
   explicit InverterNode(std::unique_ptr<BaseNode> child);

   NodeState Tick() override;
};

// 子ノードの結果に関わらず常にSuccessを返すDecorator
class SucceederNode : public DecoratorNode {
public:
   explicit SucceederNode(std::unique_ptr<BaseNode> child);

   NodeState Tick() override;
};

// 子ノードを指定回数繰り返すDecorator
class RepeaterNode : public DecoratorNode {
public:
   RepeaterNode(std::unique_ptr<BaseNode> child, int repeatCount);

   NodeState Tick() override;
   void Reset();

private:
   int repeatCount_;     // 繰り返し回数（-1で無限）
   int currentCount_;    // 現在のカウント
};

// 子ノードがSuccessになるまで繰り返すDecorator
class RetryNode : public DecoratorNode {
public:
   explicit RetryNode(std::unique_ptr<BaseNode> child);

   NodeState Tick() override;
};
