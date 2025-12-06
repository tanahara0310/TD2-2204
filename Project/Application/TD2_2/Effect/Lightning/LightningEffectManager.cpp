#include "LightningEffectManager.h"
#include "Application/TD2_2/GameObject/Voxel/Voxel.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Utility/Random/RandomGenerator.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include <cmath>
#include <numbers>
#include <algorithm>

// Avoid Windows min/max macro collisions
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

void LightningEffectManager::Initialize(ModelManager* modelManager, TextureManager* textureManager)
{
	modelManager->LoadModelResource("Resources/Models/Voxel/", "voxel.obj");
	voxelModelResource_ = modelManager->GetModelResource("Resources/Models/Voxel/voxel.obj");
	voxelTexture_ = textureManager->Load("Resources/SampleResources/white1x1.png");
}

std::vector<Vector3> LightningEffectManager::GenerateSpherePoints(int count, float radius)
{
	std::vector<Vector3> points;
	auto& random = RandomGenerator::GetInstance();
	
	// Fibonacci球面分布で均等な基準点を生成し、そこにランダムなオフセットを追加
	const float goldenRatio = (1.0f + std::sqrt(5.0f)) / 2.0f;
	const float angleIncrement = std::numbers::pi_v<float> * 2.0f * goldenRatio;
	
	for (int i = 0; i < count; ++i) {
		// 均等分布の基準点を計算
		float t = static_cast<float>(i) / static_cast<float>(count);
		float inclination = std::acos(1.0f - 2.0f * t);
		float azimuth = angleIncrement * static_cast<float>(i);
		
		// ランダムなオフセットを追加（球面上でバランスを保つ）
		float offsetAngle = random.GetFloat(-0.3f, 0.3f); // 角度のオフセット
		float offsetInclination = random.GetFloat(-0.3f, 0.3f);
		
		inclination += offsetInclination;
		azimuth += offsetAngle;
		
		// 球面座標から直交座標に変換
		float x = std::sin(inclination) * std::cos(azimuth);
		float y = std::sin(inclination) * std::sin(azimuth);
		float z = std::cos(inclination);
		
		// 半径にも少しランダム性を追加
		float radiusVariation = random.GetFloat(0.95f, 1.05f);
		
		points.push_back({ 
			x * radius * radiusVariation, 
			y * radius * radiusVariation, 
			z * radius * radiusVariation 
		});
	}
	
	return points;
}

int LightningEffectManager::CreateEffect(const Vector3& position, const EffectConfig& config,
	std::vector<std::unique_ptr<IDrawable>>& gameObjects)
{
	if (!voxelModelResource_) {
		return -1;
	}

	EffectData effectData;
	effectData.position = position;
	effectData.config = config;
	effectData.state = AnimationState::Hidden;

	if (config.useSphereDistribution) {
		auto& random = RandomGenerator::GetInstance();
		
		// 球面配置モード：複数の雷を球面上にバランスよく配置
		auto spherePoints = GenerateSpherePoints(config.lightningCount, config.sphereRadius);
		
		for (const auto& point : spherePoints) {
			// 開始点と終点を計算（内側から外側へ）
			Vector3 direction = point;
			float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
			if (length > 0.0f) {
				direction.x /= length;
				direction.y /= length;
				direction.z /= length;
			}
			
			// 開始点：球面の内側から（ランダム性を抑えて均等に）
			float startRadiusVariation = random.GetFloat(0.9f, 1.1f);
			Vector3 startPoint = {
				direction.x * config.sphereRadius * config.sphereStartRadiusRatio * startRadiusVariation,
				direction.y * config.sphereRadius * config.sphereStartRadiusRatio * startRadiusVariation,
				direction.z * config.sphereRadius * config.sphereStartRadiusRatio * startRadiusVariation
			};
			
			// 終点：球面上に小さなランダムオフセット
			float endRadiusVariation = random.GetFloat(0.95f, 1.05f);
			Vector3 endPoint = {
				point.x * endRadiusVariation + random.GetFloat(-config.randomOffsetRange * 0.3f, config.randomOffsetRange * 0.3f),
				point.y * endRadiusVariation + random.GetFloat(-config.randomOffsetRange * 0.3f, config.randomOffsetRange * 0.3f),
				point.z * endRadiusVariation + random.GetFloat(-config.randomOffsetRange * 0.3f, config.randomOffsetRange * 0.3f)
			};
			
			Lightning::Config lightningConfig;
			// 初期状態は始点のみ（線の長さ0）
			lightningConfig.startPoint = startPoint;
			lightningConfig.endPoint = startPoint;
			lightningConfig.segmentCount = config.segmentCount;
			lightningConfig.noiseScale = config.noiseScale;
			lightningConfig.noiseSpeed = config.noiseSpeed;
			lightningConfig.enableAnimation = true;
			lightningConfig.color = config.hiddenColor; // 初期状態は非表示
			lightningConfig.pathType = Lightning::PathType::Linear;
			lightningConfig.voxelScale = config.voxelScale;

			auto lightning = std::make_unique<Lightning>();
			lightning->Initialize(voxelModelResource_, voxelTexture_, lightningConfig,
				"Lightning_" + std::to_string(nextEffectId_) + "_" + std::to_string(effectData.lightnings.size()));
			
			lightning->SetActive(true);
			lightning->GetTransform().translate = position;

			LightningData lightningData;
			lightningData.lightning = lightning.get();
			lightningData.originalStartPoint = startPoint;
			lightningData.originalEndPoint = endPoint;
			lightningData.delay = random.GetFloat(0.0f, 0.05f); // 各ライトニングに小さな遅延

			effectData.lightnings.push_back(lightningData);
			gameObjects.push_back(std::move(lightning));
		}
	} else {
		// 従来モード：単一の雷（常に表示）
		Lightning::Config lightningConfig;
		lightningConfig.startPoint = config.startOffset;
		lightningConfig.endPoint = config.endOffset;
		lightningConfig.segmentCount = config.segmentCount;
		lightningConfig.noiseScale = config.noiseScale;
		lightningConfig.noiseSpeed = config.noiseSpeed;
		lightningConfig.enableAnimation = true;
		lightningConfig.color = config.color;
		lightningConfig.pathType = Lightning::PathType::Linear;
		lightningConfig.voxelScale = config.voxelScale;

		auto lightning = std::make_unique<Lightning>();
		lightning->Initialize(voxelModelResource_, voxelTexture_, lightningConfig,
			"Lightning_" + std::to_string(nextEffectId_));
		
		lightning->SetActive(true);
		lightning->GetTransform().translate = position;

		LightningData lightningData;
		lightningData.lightning = lightning.get();
		lightningData.originalStartPoint = config.startOffset;
		lightningData.originalEndPoint = config.endOffset;
		lightningData.delay = 0.0f;

		effectData.lightnings.push_back(lightningData);
		gameObjects.push_back(std::move(lightning));
		
		effectData.state = AnimationState::Visible;
	}

	int effectId = nextEffectId_++;
	effects_.push_back(std::move(effectData));
	
	return effectId;
}

void LightningEffectManager::UpdateAllEffects()
{
	float deltaTime = GameUtils::GetDeltaTime();
	
	for (auto& effect : effects_) {
		// 位置を更新
		for (auto& lightningData : effect.lightnings) {
			if (lightningData.lightning) {
				lightningData.lightning->GetTransform().translate = effect.position;
			}
		}
		
		// アニメーション更新
		if (effect.state == AnimationState::FadingIn) {
			effect.animationTimer.Update(deltaTime);
			float progress = effect.animationTimer.GetProgress();
			
			// 各ライトニングを遅延付きで更新
			for (auto& lightningData : effect.lightnings) {
				if (!lightningData.lightning) continue;
				
				// 遅延を考慮した進行度
				float delayedProgress = std::max(0.0f, (progress - lightningData.delay) / (1.0f - lightningData.delay));
				delayedProgress = std::clamp(delayedProgress, 0.0f, 1.0f);
				float delayedEasedProgress = EasingUtil::Apply(delayedProgress, EasingUtil::Type::EaseOutCubic);
				
				// 始点から終点まで徐々に伸びる
				auto& config = lightningData.lightning->GetConfig();
				config.startPoint = lightningData.originalStartPoint;
				config.endPoint = {
					lightningData.originalStartPoint.x + (lightningData.originalEndPoint.x - lightningData.originalStartPoint.x) * delayedEasedProgress,
					lightningData.originalStartPoint.y + (lightningData.originalEndPoint.y - lightningData.originalStartPoint.y) * delayedEasedProgress,
					lightningData.originalStartPoint.z + (lightningData.originalEndPoint.z - lightningData.originalStartPoint.z) * delayedEasedProgress
				};
				
				// 色もフェードイン
				config.color = {
					effect.config.color.x,
					effect.config.color.y,
					effect.config.color.z,
					effect.config.color.w * delayedEasedProgress
				};
				
				lightningData.lightning->ApplyConfigChanges();
			}
			
			if (effect.animationTimer.IsFinished()) {
				effect.state = AnimationState::Visible;
			}
		}
		else if (effect.state == AnimationState::FadingOut) {
			effect.animationTimer.Update(deltaTime);
			float progress = effect.animationTimer.GetProgress();
			
			// 各ライトニングを遅延付きで更新
			for (auto& lightningData : effect.lightnings) {
				if (!lightningData.lightning) continue;
				
				// 遅延を考慮した進行度
				float delayedProgress = std::max(0.0f, (progress - lightningData.delay) / (1.0f - lightningData.delay));
				delayedProgress = std::clamp(delayedProgress, 0.0f, 1.0f);
				float delayedEasedProgress = EasingUtil::Apply(delayedProgress, EasingUtil::Type::EaseInCubic);
				
				// 始点から徐々に縮んでいく
				auto& config = lightningData.lightning->GetConfig();
				float remainingLength = 1.0f - delayedEasedProgress;
				config.startPoint = {
					lightningData.originalStartPoint.x + (lightningData.originalEndPoint.x - lightningData.originalStartPoint.x) * delayedEasedProgress,
					lightningData.originalStartPoint.y + (lightningData.originalEndPoint.y - lightningData.originalStartPoint.y) * delayedEasedProgress,
					lightningData.originalStartPoint.z + (lightningData.originalEndPoint.z - lightningData.originalStartPoint.z) * delayedEasedProgress
				};
				config.endPoint = lightningData.originalEndPoint;
				
				// 色もフェードアウト
				config.color = {
					effect.config.color.x,
					effect.config.color.y,
					effect.config.color.z,
					effect.config.color.w * remainingLength
				};
				
				lightningData.lightning->ApplyConfigChanges();
			}
			
			if (effect.animationTimer.IsFinished()) {
				effect.state = AnimationState::Hidden;
				// 完全に非表示
				for (auto& lightningData : effect.lightnings) {
					if (lightningData.lightning) {
						auto& config = lightningData.lightning->GetConfig();
						config.color = effect.config.hiddenColor;
						lightningData.lightning->ApplyConfigChanges();
					}
				}
			}
		}
	}
}

void LightningEffectManager::SetEffectPosition(int effectId, const Vector3& position)
{
	if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
		return;
	}
	effects_[effectId].position = position;
}

void LightningEffectManager::SetEffectVisible(int effectId, bool visible)
{
	if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
		return;
	}
	
	auto& effect = effects_[effectId];
	
	if (visible && (effect.state == AnimationState::Hidden || effect.state == AnimationState::FadingOut)) {
		// フェードイン開始
		effect.state = AnimationState::FadingIn;
		effect.animationTimer.Start(effect.config.fadeInDuration, false);
	}
	else if (!visible && (effect.state == AnimationState::Visible || effect.state == AnimationState::FadingIn)) {
		// フェードアウト開始
		effect.state = AnimationState::FadingOut;
		effect.animationTimer.Start(effect.config.fadeOutDuration, false);
	}
}

bool LightningEffectManager::IsEffectVisible(int effectId) const
{
	if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
		return false;
	}
	return effects_[effectId].state == AnimationState::Visible || 
	       effects_[effectId].state == AnimationState::FadingIn;
}
