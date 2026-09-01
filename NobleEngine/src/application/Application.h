#pragma once
#include <memory>

class GameManager;

class Application
{
public:
	// 唯一のインスタンス取得
	static Application& Instance();

	// コピー・ムーブ禁止
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&&) = delete;

	void Initialize();
	void Update();
	void Draw();
	void DrawImGui();
	void Finalize();

public:


private:
	Application() = default;
	~Application() = default;

	// メインのゲームループ
	std::unique_ptr<GameManager> gameManager_;
};

