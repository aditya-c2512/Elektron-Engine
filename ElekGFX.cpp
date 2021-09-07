#include "ElekGFX.h"

using Microsoft::WRL::ComPtr;
using namespace std;
//using namespace DirectX;

ElekGFX* ElekGFX::elekGFX = nullptr;

ElekGFX::ElekGFX(HINSTANCE inst, HWND wind) : hAppInst(inst), hMainWnd(wind)
{
	if (elekGFX == nullptr) elekGFX = this;
}

ElekGFX::~ElekGFX()
{
}

void ElekGFX::InitD3D12()
{
	//Enable Debug Layer if in Debug Mode.
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
		debugController->EnableDebugLayer();
	}
#endif

	//Create the DXGI Factory.
	CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	//Create D3D12 Device.
	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&elekDevice));

	//Use Warp Adapter if Physical Device not available.
	if (FAILED(hr))
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
		D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&elekDevice));
	}

	//Create D3D12 Fence
	elekDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&elekFence));

	//Get Descriptor Sizes.
	RTVDescriptorSize = elekDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DSVDescriptorSize = elekDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CbvSrvUavDescriptorSize = elekDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//Check for 4x MSAA Support
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQL;
	msaaQL.Format = BackBufferFormat;
	msaaQL.SampleCount = 4;
	msaaQL.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaQL.NumQualityLevels = 0;

	elekDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQL, sizeof(msaaQL));

	elek4xMsaaQuality = msaaQL.NumQualityLevels;
	//assert(m4xMsaaQuality > 0 && “Unexpected MSAA quality level.”);

	CreateCommandObjects();
}

void ElekGFX::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	// Create Command Queue.
	elekDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&elekCommandQueue));
	// Create Command Allocator.
	elekDevice->CreateCommandAllocator(cmdQueueDesc.Type, IID_PPV_ARGS(elekDirectCmdListAlloc.GetAddressOf()));
	// Create Command List.
	elekDevice->CreateCommandList(0, cmdQueueDesc.Type, elekDirectCmdListAlloc.Get(), nullptr, IID_PPV_ARGS(elekCommandList.GetAddressOf()));

	// Close the command list before start up.
	elekCommandList->Close();
}

void ElekGFX::CreateSwapChain()
{
	elekSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = mClientWidth;
	swapChainDesc.BufferDesc.Height = mClientHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = BackBufferFormat;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = is4xMsaaState ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = is4xMsaaState ? (elek4xMsaaQuality - 1) : 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SwapChainBufferCount;
	swapChainDesc.OutputWindow = hMainWnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//Create Swap Chain
	dxgiFactory->CreateSwapChain(elekCommandQueue.Get(), &swapChainDesc, elekSwapChain.GetAddressOf());
}

void ElekGFX::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	elekDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(elekRTVHeap.GetAddressOf()));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	elekDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(elekDSVHeap.GetAddressOf()));
}

D3D12_CPU_DESCRIPTOR_HANDLE ElekGFX::CurrentBackBufferRTV() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(elekRTVHeap->GetCPUDescriptorHandleForHeapStart(),CurrBackBuffer,RTVDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE ElekGFX::DepthStencilView() const
{
	return elekDSVHeap->GetCPUDescriptorHandleForHeapStart();
}
