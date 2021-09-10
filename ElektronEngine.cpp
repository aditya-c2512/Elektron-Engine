#include "ElektronEngine.h"
#include "ElekGFX.h"
#include "ElekTimer.h"

ElektronEngine::ElektronEngine()
{
}

ElektronEngine::~ElektronEngine()
{
}

void ElektronEngine::onCreate()
{
	Window::onCreate();

	elekTimer = new ElekTimer();
	//Create ElekGFX
	elekGFX = new ElekGFX(GetModuleHandle(nullptr), this->m_hwnd);
	//Initialise GFX Engine
	elekGFX->InitD3D12();
	elekGFX->OnResize();
}

void ElektronEngine::onUpdate()
{
	Window::onUpdate();
	elekGFX->Draw();
}

void ElektronEngine::onDestroy()
{
	Window::onDestroy();
}

void ElektronEngine::onFocus()
{
	Window::onFocus();
}

void ElektronEngine::onKillFocus()
{
	Window::onKillFocus();
}

void ElektronEngine::onSize()
{
	Window::onSize();
}
