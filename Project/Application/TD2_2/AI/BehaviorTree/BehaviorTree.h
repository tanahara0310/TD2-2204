#pragma once
#include "Application/TD2_2/AI/Node/BaseNode.h"
#include "BehaviorTreeBuilder.h"
#include <memory>
#include <string>

/// @brief ビヘイビアツリー管理クラス
/// ボスやエネミーのAIを管理するためのクラス
class BehaviorTree {
public:
   BehaviorTree() = default;
   ~BehaviorTree() = default;

   /// @brief ルートノードを設定
   /// @param root ルートノード
   void SetRoot(std::unique_ptr<BaseNode> root);

   /// @brief ビヘイビアツリーを実行
   /// @return ノードの実行結果
   NodeState Tick();

   /// @brief ビヘイビアツリーをリセット
   void Reset();

   /// @brief ルートノードが設定されているか
   /// @return true: 設定済み, false: 未設定
   bool HasRoot() const { return root_ != nullptr; }

   /// @brief デバッグ情報の取得
   /// @return 実行回数
   uint32_t GetTickCount() const { return tickCount_; }

   /// @brief 名前を設定
   /// @param name ツリーの名前
   void SetName(const std::string& name) { name_ = name; }

   /// @brief 名前を取得
   /// @return ツリーの名前
   const std::string& GetName() const { return name_; }

private:
   std::unique_ptr<BaseNode> root_;   // ルートノード
   uint32_t tickCount_ = 0;           // 実行回数
   std::string name_ = "BehaviorTree"; // ツリーの名前（デバッグ用）
};

/// @brief ビヘイビアツリービルダーヘルパー
/// BehaviorTreeBuilderを使って直接BehaviorTreeを構築する
class BehaviorTreeFactory {
public:
   /// @brief 新しいビルダーを作成
   /// @return BehaviorTreeBuilderのインスタンス
   static BehaviorTreeBuilder CreateBuilder() {
      return BehaviorTreeBuilder();
   }

   /// @brief ビルダーからBehaviorTreeを構築
   /// @param builder ビルダー
   /// @param name ツリーの名前（オプション）
   /// @return 構築されたBehaviorTree
   static std::unique_ptr<BehaviorTree> Build(BehaviorTreeBuilder& builder, const std::string& name = "BehaviorTree") {
      auto tree = std::make_unique<BehaviorTree>();
      tree->SetRoot(builder.Build());
      tree->SetName(name);
      return tree;
   }

   /// @brief BehaviorTreeBuilderから直接構築
   /// @param buildFunc ビルド関数
   /// @param name ツリーの名前（オプション）
   /// @return 構築されたBehaviorTree
   template<typename BuildFunc>
   static std::unique_ptr<BehaviorTree> Create(BuildFunc buildFunc, const std::string& name = "BehaviorTree") {
      auto builder = CreateBuilder();
      buildFunc(builder);
      return Build(builder, name);
   }
};
