#include "YunuDX12Renderer.h"

YunuDX12Renderer::~YunuDX12Renderer()
{
}

// DX를 초기화한다.
bool  YunuDX12Renderer::Initialize(HWND hWnd)
{
    HRESULT hr;
    IDXGIFactory4* dxgiFactory;
    DXCall(hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));
    if (FAILED(hr))
        return false;

    IDXGIAdapter1* adapter = FindDx12SupportiveAdapter(dxgiFactory);
    // if no adapter is found that supports dx12 features, then return false.
    if (!adapter)
        return false;

    hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    if (FAILED(hr))
        return false;

    // create the command queue
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{}; // means that i'll be using default values
    hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue)); //create the command queue
    if (FAILED(hr))
        return false;

    // create the swap chain
    DXGI_MODE_DESC backBufferDesc{};
    backBufferDesc.Width = 0;
    backBufferDesc.Height = 0;
    backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the bufer is rgba 32bits, 8 bits for each channel

    // describe multi-sampling, if not using multi-sampling, set the count to 1;
    DXGI_SAMPLE_DESC sampleDesc{};
    sampleDesc.Count = 1; // 1 sample is, by no means, multi, sampling.

    // describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    swapChainDesc.BufferCount = frameBufferCount;
    swapChainDesc.BufferDesc = backBufferDesc;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc = sampleDesc;
    swapChainDesc.Windowed = true;

    IDXGISwapChain* tempSwapChain;

    dxgiFactory->CreateSwapChain(commandQueue.Get(), &swapChainDesc, &tempSwapChain);
    swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
    frameIndex = swapChain->GetCurrentBackBufferIndex();

    // 3. create the back buffers descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.NumDescriptors = frameBufferCount; // number of descriptors for this heap.
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap.

    // this heap is not visible to shaders, as this will store the output from the pipeline.
    // that's why SHADER_VISIBLE flag is not set in flags.
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
    if (FAILED(hr))
        return false;

    // get the size of a descriptor in this heap (this is a rtv heap, so only descriptors should be stored in it.
    // descriptor sizes may vary from device to device, so it must be queried to device.
    // device size is used to increment a descriptor handle offset.
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // get a handle to the first descriptor in the descriptor heap.
    // ! slightly differnet from source source code.
    // https://www.braynzarsoft.net/viewtutorial/q16390-03-initializing-directx-12
    // in original code, type of rtvHandle is cd3dx12_cpu_descriptor_handle
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{ rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };

    // create a rtv for each buffer
    for (int i = 0; i < frameBufferCount; i++)
    {
        // get the n'th buffer into render targets n'th element,
        hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        if (FAILED(hr))
            return false;

        // create a render target view which binds the swap chain buffer to the rtv handle.
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    // create command allocators
    // command allocator is used to allocate memory on the gpu for the commands
    // we want to exectue on the command queue.
    for (int i = 0; i < frameBufferCount; i++)
    {
        hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
        if (FAILED(hr))
            return false;
    }
    // create the command list
    hr = device->CreateCommandList(
        0,                              // speicify the gpu we'll use, which is 0 index gpu.
        D3D12_COMMAND_LIST_TYPE_DIRECT, // type of command list
        commandAllocator[0].Get(),      // command allocator that will store the commands on the gpu made by the command list.
        nullptr,                        // starting pipeline state object for the command list.null will keep the pieplie state at its defualt values.
        IID_PPV_ARGS(&commandList)      // to draw anythoing on the screen, it will need at least a vertex shader.
    );
    if (FAILED(hr))
        return false;

    // command lists are created in recording state, main loop will set it up for recording again.
    commandList->Close();

    // create a fence & fence event
    // fence is used to prevent cpu 
    for (int i = 0; i < frameBufferCount; i++)
    {
        hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
        if (FAILED(hr))
            return false;
        fenceValue[i] = 0;
    }

    // create a handle to a fence event.
    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent == nullptr)
        return false;

    return true;
}
// 렌더링 시작
void  YunuDX12Renderer::BeginRender()
{
}
void  YunuDX12Renderer::Render()
{
    HRESULT hr;
    UpdatePipeline(); // update the pipeline by sending commands to the commmand queue.

    // create an arrray of command lists (only one command list here)
    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };

    // execute the array of command lists
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // commandQueue is executed on the GPU
    // this command goes in at the end of the command queue, in order to signal the fence that execution is done.
    hr = commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
    if (FAILED(hr))
        running = false;

    // present the current backbuffer
    hr = swapChain->Present(0, 0);
    if (FAILED(hr))
        running = false;
}
// 렌더링 마무리. 그릴 것 다 그린 상태에서 Present
void  YunuDX12Renderer::EndRender()
{
}
// 엔진을 종료한다.
void  YunuDX12Renderer::Release()
{
    // wait for the gpu to finish all frames
    for (int i = 0; i < frameBufferCount; ++i)
    {
        frameIndex = i;
        WaitForPreviousFrame();
    }
    // get swapchain out of full screen before exiting
    BOOL isFullscreen = false;
    if (swapChain->GetFullscreenState(&isFullscreen, nullptr))
        swapChain->SetFullscreenState(false, nullptr);

    device.Reset();
    swapChain.Reset();
    commandQueue.Reset();
    rtvDescriptorHeap.Reset();
    commandList.Reset();

    for (int i = 0; i < frameBufferCount; ++i)
    {
        renderTargets[i].Reset();
        commandAllocator[i].Reset();
        fence[i].Reset();
    }
}
void  YunuDX12Renderer::OnResize()
{
}
void YunuDX12Renderer::WaitForPreviousFrame()
{
    HRESULT hr;
    // swap the current rtv buffer index so we draw on the correct buffer
    frameIndex = swapChain->GetCurrentBackBufferIndex();

    // if the current fence value is still less then fence value, it implicates that gpus processing is not done.
    // fence value will change after command queue signals fence certain value.
    if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
    {
        hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
        if (FAILED(hr))
        {
            running = false;
        }
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    // increment fenceValue for next frame
    fenceValue[frameIndex]++;
}
void YunuDX12Renderer::UpdatePipeline()
{
    HRESULT hr;
    // command allocators cannot be reset while a gpu is executing commands from a command list associated with it.
    // because command allocators are gpu businesses, and these codes run on cpu.
    WaitForPreviousFrame();

    // we can only reset an allocator once the gpu is done with it.
    // resetting an allocator frees the memory that the command list was stored in
    hr = commandAllocator[frameIndex]->Reset();
    if (FAILED(hr))
        running = false;

    // resetting command list put it into a recording state, so that we can start recording commands into allocator.
    // one command allocator can be associated with multiple command lists. but only one can be recording at any time.
    // that is why only one command list should be in the recording state, while others are at closed state. 
    // this is the place to set initial pipeline state object
    hr = commandList->Reset(commandAllocator[frameIndex].Get(), nullptr);
    if (FAILED(hr))
        running = false;

    // start recording commands into the commandlist.
    // current renderTarget transitions from present state to the render target state.
    // only in render target state we can draw some shit.
    D3D12_RESOURCE_BARRIER transitionBarrier{};
    transitionBarrier.Type = { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION };
    transitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    transitionBarrier.Transition.pResource = renderTargets[frameIndex].Get();
    transitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    transitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    transitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_FLAG_NONE;

    commandList->ResourceBarrier(1, &transitionBarrier);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += frameIndex * rtvDescriptorSize;
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // clear render target.
    const float clearColor[] = { 1.0f,0.1f,0.1f,1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    transitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    transitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    hr = commandList->Close();
    if (FAILED(hr))
    {
        running = false;
    }

}
IDXGIAdapter1* YunuDX12Renderer::FindDx12SupportiveAdapter(IDXGIFactory4* dxgiFactory)
{
    IDXGIAdapter1* adapter = nullptr; // adapters are the graphics card (this includes the embedded graphics on the motherboard)
    int adapterIndex = 0; // index for iterating through graphics devices.

    // find first hardware gpu that supports d3d12
    while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        // if found adapter is a software device, then ignore it.
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            adapterIndex++;
            continue;
        }
        // only see if it's available to create device, which means, that the adapter supports d3d feature.
        auto hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
        if (SUCCEEDED(hr))
            return adapter;
        adapterIndex++;
    }
    return nullptr;
}
