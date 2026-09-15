#pragma once
#include<Game.h>

class Bar
{
public:
	Bar();
	~Bar();
	void Initialize();
	void Update(const int cameraID, const LightDataForGPU& lightData);
	void Draw(const int renderTexture3DId);
	void DrawImGui();
private:
	// バー
	std::unique_ptr<RenderObject> bar_;
	Material barMaterial_;
	int32_t barTextureID_ = -1;
	EulerTransforms barTransforms_;

};

