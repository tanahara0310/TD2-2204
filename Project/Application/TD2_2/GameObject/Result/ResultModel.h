#pragma once
#include "../GameObject.h"
#include <memory>

class ResultModel : public GameObject {
public:
	ResultModel() = default;
	~ResultModel() = default;
	void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);
	void Draw(const ICamera* camera) override;

	const char* GetObjectName() const override { return "ResultModel"; }
};