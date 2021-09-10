#pragma once

#include "Window.h"

class ElekGFX;
class ElekTimer;

class ElektronEngine : public Window
{
public :
	ElektronEngine();
	~ElektronEngine();

	void onCreate() override;
	void onUpdate() override;
	void onDestroy() override;
	void onFocus() override;
	void onKillFocus() override;
	void onSize() override;

protected :
	ElekGFX* elekGFX;
	ElekTimer* elekTimer;
};

