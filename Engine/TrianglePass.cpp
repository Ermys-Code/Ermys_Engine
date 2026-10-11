#include "Core/Globals.h"
#include "TrianglePass.h"

#include "Rendering/FrameContext.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Vertex.h"

#include <d3dcompiler.h>
#include <Input/PlatformHelpers.h>

TrianglePass::TrianglePass(ID3D12Device4* device)
{
	createRootSignature(device);
	createPipelineState(device);
}

void TrianglePass::prepare(RenderContext& renderContext, FrameContext& frameContext)
{
    m_renderContext = renderContext;
    m_frameContext  = frameContext;
}

void TrianglePass::apply(ID3D12GraphicsCommandList4* commandList)
{
    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    D3D12_VIEWPORT viewport{ 0.0, 0.0, float(m_frameContext.width), float(m_frameContext.height) , 0.0, 1.0 };
    D3D12_RECT scissor{ 0, 0, m_frameContext.width, m_frameContext.height };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->OMSetRenderTargets(1, &m_frameContext.rtvCpuHandle, FALSE, nullptr);
    
    Vertex vertices[3] =
    {
        {Vector3(-1.0f, -1.0f, 0.0f)},
        {Vector3(0.0f,  1.0f,  0.0f)},
        {Vector3(1.0f,  -1.0f, 0.0f)}
    };

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
