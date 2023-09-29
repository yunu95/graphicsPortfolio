#pragma once
#include "Yunu3DRenderer.h"
#include "framework.h"

using namespace Microsoft::WRL;

class YunuDX12Renderer : Yunu3DRenderer
{
public:
    YunuDX12Renderer() {}
    virtual ~YunuDX12Renderer();
    // DX�� �ʱ�ȭ�Ѵ�.
    virtual bool Initialize(HWND hWnd);
    // ������ ����
    virtual void BeginRender();
    virtual void Render();
    // ������ ������. �׸� �� �� �׸� ���¿��� Present
    virtual void EndRender();
    // ������ �����Ѵ�.
    virtual void Release();
protected:
    // â ũ�Ⱑ ������ �� ó���ؾ� �� �͵�
    virtual void OnResize();
private:
    void WaitForPreviousFrame();
    void UpdatePipeline(); // add commands to change the state of render target, set root siganature
    IDXGIAdapter1* FindDx12SupportiveAdapter(IDXGIFactory4* factory);
    static constexpr int frameBufferCount = 2;
    ComPtr<ID3D12Device> device;
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<ID3D12CommandQueue> commandQueue; // container for command lists
    ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap; // a descriptor heap to hold resources, such as render targets
    ComPtr<ID3D12Resource> renderTargets[frameBufferCount]; // number of render targets equals to buffer count
    ComPtr<ID3D12CommandAllocator> commandAllocator[frameBufferCount]; // allocators for each buffer
    ComPtr<ID3D12GraphicsCommandList> commandList; // a command list we can record commands into. then exectue them to render the frame.
    // an object that is locked while our command list is being excuted by the gpu.
    // we need these as many as we have allocators.
    ComPtr<ID3D12Fence> fence[frameBufferCount];

    HANDLE fenceEvent;// a handle to an event when our fence is unlocked by the gpu.
    // ? : shouldn't there be fence events as many as fences?
    UINT64 fenceValue[frameBufferCount]; // the value is incremented each frame. each fence will have its own value
    int frameIndex; // current rtv we are on.
    int rtvDescriptorSize; // size of the rtv descriptor on the device
    bool running = false;
};
