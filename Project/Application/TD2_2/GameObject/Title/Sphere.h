#pragma once
#include "Object3d.h"

class Sphere : public Object3d {
public:

	void Initialize();

	void Update() override;

	void Draw(const ICamera* camera) override;

	bool DrawImGui() override { return true; }

	const char* GetObjectName() const override { return "Sphere"; }



private:
};

