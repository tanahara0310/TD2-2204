#include "LightningEffectManager.h"
#include "Application/TD2_2/GameObject/Voxel/Voxel.h"

void LightningEffectManager::Initialize(ModelManager* modelManager, TextureManager* textureManager)
{
	modelManager->LoadModelResource("Resources/Models/Voxel/", "voxel.obj");
	voxelModelResource_ = modelManager->GetModelResource("Resources/Models/Voxel/voxel.obj");
	voxelTexture_ = textureManager->Load("Resources/SampleResources/white1x1.png");
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

	effectData.lightning = lightning.get();
	gameObjects.push_back(std::move(lightning));

	int effectId = nextEffectId_++;
	effects_.push_back(std::move(effectData));
	return effectId;
}

void LightningEffectManager::UpdateAllEffects()
{
	for (auto& effect : effects_) {
		if (effect.lightning && effect.isActive) {
			effect.lightning->GetTransform().translate = effect.position;
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

void LightningEffectManager::SetEffectActive(int effectId, bool active)
{
	if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
		return;
	}
	effects_[effectId].isActive = active;
	if (effects_[effectId].lightning) {
		effects_[effectId].lightning->SetActive(active);
	}
}

bool LightningEffectManager::IsEffectActive(int effectId) const
{
	if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
		return false;
	}
	return effects_[effectId].isActive;
}
