#pragma once
#include <functional>
#include <memory>
#include <vector>
#include <random>
#include "EngineSystem.h"

// ノードの実行結果を表す列挙体
enum class NodeState {
   Success,  // 成功した
   Failure,  // 条件を満たさなかった
   Running   // 実行中（次フレームも継続）
};

class BaseNode {
public:
   //static void Initialize(const EngineSystem* engine);

   virtual ~BaseNode() = default;

   virtual NodeState Tick() = 0;
};
