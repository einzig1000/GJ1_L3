#pragma once
#include<Game.h>

class Bar
{
public:
	Bar();
	~Bar();
	void Initialize();
	void Update(const int cameraID);
	void Draw(const int renderTexture3DId);
	void DrawImGui();
	void SetLightData(LightDataForGPU* lightData) { lightData_ = lightData; };
private:
	// バー
	std::unique_ptr<RenderObject> bar_;
	Material barMaterial_;
	int32_t barTextureID_ = -1;
	EulerTransforms barTransforms_;
	LightDataForGPU* lightData_ = nullptr;
};

