#pragma once
#include "Object3d.h"

class Sphere : public Object3d {
public:

	void Initialize();

	void Update() override;

	void Draw(ICamera* camera) override;

	bool DrawImGui() override { return true; }

	const char* GetObjectName() const override { return "Sphere"; }

	bool Is2D() const override { return false; }



private:
};

