#pragma once

#include "Window.h"

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
};

