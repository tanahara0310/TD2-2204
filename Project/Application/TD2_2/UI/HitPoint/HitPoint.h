#pragma once
#include "Engine/ObjectCommon/SpriteObject.h"
#include <memory>
#include <vector>

class EngineSystem;
class IDrawable;

enum class SettingObject {
	PLAYER = 0,
	BOSS = 1,
};

class HitPoint {
public:
	HitPoint() = default;
	~HitPoint() = default;

	/// @brief 初期化（スプライトを作成してvectorで返す）
	/// @param pivot 基点
	/// @param setObj 指定するオブジェクト(使用したいスプライト)
	/// @param hpCount HPの数
	/// @return 作成したスプライトのunique_ptrのvector
	std::vector<std::unique_ptr<IDrawable>> Initialize(Vector2 pivot, SettingObject setObj, int hpCount);

	/// @brief 更新
	void Update();

	/// @brief 新しいHPをセット
	void SetHP(int currentHPCount);

private:
	// HPアイコンを作成
	std::unique_ptr<SpriteObject> CreateHPIcon();

private:
	// HPアイコンの最大数
	int maxHPCount_ = 0;

	// 現在のHPアイコンの数
	int currentHPCount_ = 0;

	// テクスチャファイルパス
	std::string playerIconFilePath_ = "Resources/Textures/HPIcon/HiyokoIcon.png";
	std::string playerDamageIconFilePath_ = "Resources/Textures/HPIcon/HiyokoIconDamage.png";
	std::string bossIconFilePath_ = "Resources/Textures/HPIcon/BossIcon.png";
	std::string bossDamageIconFilePath_ = "Resources/Textures/HPIcon/BossIconDamage.png";

	// UI要素のポインタ（所有権はgameObjects_が持つ）
	std::vector<SpriteObject*> hpIcon_;

	// 指定するオブジェクト
	SettingObject setObj_ = SettingObject::PLAYER;
};
