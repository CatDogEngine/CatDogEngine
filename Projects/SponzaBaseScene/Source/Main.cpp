#include "Application/Engine.h"
#include "GameApp.h"

int main()
{
	using namespace engine;
	Engine* pEngine = Engine::Create(std::make_unique<game::GameApp>());

	EngineInitArgs initArgs;
	initArgs.pTitle = "Game";
	initArgs.width = 1200;
	initArgs.height = 900;
	initArgs.pIconFilePath = "editor_icon.png";
	pEngine->Init(std::move(initArgs));

	pEngine->Run();

	Engine::Destroy(pEngine);

	return 0;
}