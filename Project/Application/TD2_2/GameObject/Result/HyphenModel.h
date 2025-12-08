#pragma once
#include "../GameObject.h"
#include <memory>

class HyphenModel : public GameObject {
public:
	HyphenModel() = default;
	~HyphenModel() = default;
	void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);
	void Update() override;
	void Draw(const ICamera* camera) override;

	const char* GetObjectName() const override { return "HyphenModel"; }
};