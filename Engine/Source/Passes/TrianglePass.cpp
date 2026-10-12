#include "Core/Globals.h"
#include "TrianglePass.h"

#include "Core/Application.h"
#include "Modules/ModuleD3D12.h"

#include "Rendering/FrameContext.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Vertex.h"

#include <d3dcompiler.h>
#include <Input/PlatformHelpers.h>

TrianglePass::TrianglePass()
{
    auto* device = app->GetModuleD3D12()->GetDevice();
	createRootSignature(device);
	createPipelineState(device);
    createVertexBuffer(device);
}

void TrianglePass::prepare(RenderContext* renderContext, FrameContext* frameContext)
{
    m_renderContext = renderContext;
    m_frameContext  = frameContext;
}

void TrianglePass::render(ID3D12GraphicsCommandList4* commandList)
{
    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    commandList->RSSetViewports(1, &m_renderContext->viewport);
    commandList->RSSetScissorRects(1, &m_renderContext->scissorRect);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->OMSetRenderTargets(1, &m_renderContext->rtvCpuHandle, FALSE, nullptr);

    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    commandList->DrawInstanced(3, 1, 0, 0);
}

void TrianglePass::createRootSignature(ID3D12Device4* device)
{

    CD3DX12_ROOT_SIGNATURE_DESC desc;
    desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
}

void TrianglePass::createPipelineState(ID3D12Device4* device)
{
    ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> ps;
    ThrowIfFailed(D3DReadFileToBlob(L"TriangleVS.cso", &vs));
    ThrowIfFailed(D3DReadFileToBlob(L"TrianglePS.cso", &ps));

    D3D12_INPUT_ELEMENT_DESC input[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
    desc.InputLayout = { input, _countof(input) };
    desc.pRootSignature = m_rootSignature.Get();
    desc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
    desc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.NumRenderTargets = 1;
    desc.SampleDesc = { 1, 0 };
    desc.SampleMask = UINT_MAX;
    desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pipelineState));
}

void TrianglePass::createVertexBuffer(ID3D12Device4* device)
{
    Vertex vertices[3] =
    {
        { Vector3(-1.0f, -1.0f, 0.0f) },
        { Vector3(0.0f,  1.0f, 0.0f) },
        { Vector3(1.0f, -1.0f, 0.0f) }
    };

    const UINT bufferSize = sizeof(vertices);

    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);

    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer)));

    void* mappedData = nullptr;
    const D3D12_RANGE readRange{ 0, 0 };

    ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, &mappedData));

    memcpy(mappedData, vertices, bufferSize);

    m_vertexBuffer->Unmap(0, nullptr);

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();

    m_vertexBufferView.SizeInBytes = bufferSize;
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
}
