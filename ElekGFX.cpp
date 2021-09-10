#include "ElekGFX.h"

using Microsoft::WRL::ComPtr;
using namespace std;
//using namespace DirectX;

//ElekGFX* ElekGFX::elekGFX = nullptr;

ElekGFX::ElekGFX(HINSTANCE inst, HWND wind) : hAppInst(inst), hMainWnd(wind)
{
	//if (elekGFX == nullptr) elekGFX = this;
}

ElekGFX::~ElekGFX()
{
	if (elekDevice != nullptr)
		FlushCommandQueue();
}

ElekGFX* ElekGFX::GetGFX()
{
	return nullptr;
}

void ElekGFX::OnResize()
{
	FlushCommandQueue();
	elekCommandList->Reset(elekDirectCmdListAlloc.Get(), nullptr);

	for (int i = 0; i < SwapChainBufferCount; ++i)
		elekSwapChainBuffer[i].Reset();

	elekDepthStencilBuffer.Reset();

	elekSwapChain->ResizeBuffers(SwapChainBufferCount, mClientWidth, mClientHeight, BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	CurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(elekRTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		elekSwapChain->GetBuffer(i, IID_PPV_ARGS(&elekSwapChainBuffer[i]));

		//Create Render Target View
		elekDevice->CreateRenderTargetView(elekSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		// Next entry in heap.
		rtvHeapHandle.Offset(1, RTVDescriptorSize);

	}

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = is4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = is4xMsaaState ? (elek4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;


	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	elekDevice->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
										D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(elekDepthStencilBuffer.GetAddressOf()));

	elekDevice->CreateDepthStencilView(elekDepthStencilBuffer.Get(), nullptr, DepthStencilView());

	auto resBar = CD3DX12_RESOURCE_BARRIER::Transition(elekDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	elekCommandList->ResourceBarrier(1, &resBar);

	elekCommandList->Close();
	ID3D12CommandList* cmdsList[] = { elekCommandList.Get() };
	elekCommandQueue->ExecuteCommandLists(_countof(cmdsList), cmdsList);

	FlushCommandQueue();

	elekScreenViewport.TopLeftX = 0;
	elekScreenViewport.TopLeftY = 0;
	elekScreenViewport.Width = static_cast<float>(mClientWidth);
	elekScreenViewport.Height = static_cast<float>(mClientHeight);
	elekScreenViewport.MinDepth = 0.0f;
	elekScreenViewport.MaxDepth = 1.0f;

	elekScissorRect = { 0, 0, mClientWidth, mClientHeight };

	elekCommandList->RSSetViewports(1, &elekScreenViewport);
	elekCommandList->RSSetScissorRects(1, &elekScissorRect);
}

void ElekGFX::Update()
{
}

void ElekGFX::Draw()
{
	elekDirectCmdListAlloc->Reset();
	elekCommandList->Reset(elekDirectCmdListAlloc.Get(), nullptr);

	auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(elekSwapChainBuffer[CurrBackBuffer].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	elekCommandList->ResourceBarrier(1, &Barrier);

	elekCommandList->RSSetViewports(1, &elekScreenViewport);
	elekCommandList->RSSetScissorRects(1, &elekScissorRect);

	FLOAT color[4] = { 0,1,0,1 };
	elekCommandList->ClearRenderTargetView(CurrentBackBufferRTV(), color, 0, nullptr);
	elekCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	auto currBackBufferView = CurrentBackBufferRTV();
	auto depthBufferView = DepthStencilView();
	elekCommandList->OMSetRenderTargets(1, &currBackBufferView, true, &depthBufferView);

	Barrier = CD3DX12_RESOURCE_BARRIER::Transition(elekSwapChainBuffer[CurrBackBuffer].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	elekCommandList->ResourceBarrier(1, &Barrier);

	elekCommandList->Close();

	ID3D12CommandList* cmdsLists[] = {elekCommandList.Get() };
	elekCommandQueue -> ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	elekSwapChain->Present(0, 0);

	CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;

	FlushCommandQueue();
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
	CreateSwapChain();
	CreateDescriptorHeaps();
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

void ElekGFX::FlushCommandQueue()
{
	CurrentFence++;
	elekCommandQueue->Signal(elekFence.Get(), CurrentFence);

	if (elekFence->GetCompletedValue() < CurrentFence)
	{
		//HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		elekFence->SetEventOnCompletion(CurrentFence, nullptr);

		//WaitForSingleObject(eventHandle, INFINITE);
		//CloseHandle(eventHandle);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE ElekGFX::CurrentBackBufferRTV() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(elekRTVHeap->GetCPUDescriptorHandleForHeapStart(),CurrBackBuffer,RTVDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE ElekGFX::DepthStencilView() const
{
	return elekDSVHeap->GetCPUDescriptorHandleForHeapStart();
}
