#pragma once
#include "../GameObject.h"
#include <memory>

class Background : public GameObject {
public:
	Background() = default;
	~Background() = default;
	void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);
	void Draw(const ICamera* camera) override;

	const char* GetObjectName() const override { return "Background"; }
};