#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <string>
#include "d3dx12.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

/*
* 1. Create the ID3D12Device object using D3D12CreateDevice function.
* 2. Create an ID3D12Fence object and query descriptor sizes.
* 3. Check 4X MSAA quality level support.
* 4. Create the command queue, command list allocator, and main command list.
* 5. Describe and create the swap chain.
* 6. Create the descriptor heaps the application requires.
* 7. Resize the back buffer and create a render target view to the back buffer.
* 8. Create the depth/stencil buffer and its associated depth/stencil view.
* 9. Set the viewport and scissor rectangles.
*/

class ElekGFX
{
public :
	ElekGFX(HINSTANCE inst, HWND wind);
	~ElekGFX();

	void InitD3D12();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeaps();
	
protected :
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferRTV() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

protected :
	static ElekGFX* elekGFX;

	HINSTANCE hAppInst = nullptr; // application instance handle
	HWND hMainWnd = nullptr; // main window handle
	bool isAppPaused = false;  // is the application paused?
	bool isMinimized = false;  // is the application minimized?
	bool isMaximized = false;  // is the application maximized?
	bool isResizing = false;   // are the resize bars being dragged?
	bool isFullscreenState = false;// fullscreen enabled

	// Set true to use 4X MSAA. The default is false.
	bool is4xMsaaState = false;// 4X MSAA enabled
	UINT elek4xMsaaQuality = 0;// quality level of 4X MSAA

	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> elekSwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> elekDevice;
	Microsoft::WRL::ComPtr<ID3D12Fence> elekFence;
	UINT64 CurrentFence = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> elekCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> elekDirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> elekCommandList;

	static const int SwapChainBufferCount = 2;
	int CurrBackBuffer = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> elekSwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> elekDepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> elekRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> elekDSVHeap;

	D3D12_VIEWPORT elekScreenViewport;
	D3D12_RECT elekScissorRect;

	UINT RTVDescriptorSize = 0;
	UINT DSVDescriptorSize = 0;
	UINT CbvSrvUavDescriptorSize = 0;

	std::wstring MainWndCaption = L"Elektron Engine";
	D3D_DRIVER_TYPE elekDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int mClientWidth = 1080;
	int mClientHeight = 1920;
};