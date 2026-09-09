#pragma once
#include <Game.h> 

class CreateMaskTexture
{
public:
	CreateMaskTexture();
	~CreateMaskTexture();

	int32_t GetMaskTextureID() { return maskTextureID_; };

private:
	int32_t maskTextureID_ = -1;

	std::unique_ptr<RenderObject> render;
};

