#include "Core/Globals.h"
#include "CameraTrianglePass.h"

#include "Core/Application.h"
#include "Modules/ModuleD3D12.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"

#include "Rendering/FrameContext.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Vertex.h"

#include "3rdParty/DebugDraw/DebugDrawPass.h"
#include "3rdParty/SimpleMath/SimpleMath.h"

#include <d3dcompiler.h>
#include <Input/PlatformHelpers.h>

CameraTrianglePass::CameraTrianglePass()
{
    m_device = app->GetModuleD3D12()->GetDevice();
    createRootSignature(m_device);
    createPipelineState(m_device);
    createVertexBuffer(m_device);

    m_debugDrawPass = std::make_unique<DebugDrawPass>(m_device, app->GetModuleRender()->GetCommandQueue()->GetCommandQueue());
}

void CameraTrianglePass::prepare(RenderContext* renderContext, FrameContext* frameContext)
{
    m_renderContext = renderContext;
    m_frameContext = frameContext;
}

void CameraTrianglePass::render(ID3D12GraphicsCommandList4* commandList)
{
    ModuleCamera* moduleCamera = app->GetModuleCamera();

    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    
    Matrix model     = Matrix::Identity;
    Matrix viewProj = moduleCamera->GetViewProjectionMatrix();

    m_mvpMatrix = (model * viewProj).Transpose();
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &m_mvpMatrix, 0);

    commandList->RSSetViewports(1, &m_renderContext->viewport);
    commandList->RSSetScissorRects(1, &m_renderContext->scissorRect);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->OMSetRenderTargets(1, &m_renderContext->rtvCpuHandle, FALSE, &m_renderContext->dsvCpuHandle);

    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    commandList->DrawInstanced(3, 1, 0, 0);

    dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray);
    dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);

    m_debugDrawPass->record(commandList, m_renderContext->width, m_renderContext->height, moduleCamera->GetViewMatrix(), moduleCamera->GetProjectionMatrix());
}

void CameraTrianglePass::createRootSignature(ID3D12Device4* device)
{
    CD3DX12_ROOT_SIGNATURE_DESC desc;
    CD3DX12_ROOT_PARAMETER rootParams[1];

    rootParams[0].InitAsConstants(sizeof(Matrix) / sizeof(UINT32), 0);
    
    desc.Init(_countof(rootParams), rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
}

void CameraTrianglePass::createPipelineState(ID3D12Device4* device)
{
    ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> ps;
    ThrowIfFailed(D3DReadFileToBlob(L"CameraTriangleVS.cso", &vs));
    ThrowIfFailed(D3DReadFileToBlob(L"CameraTrianglePS.cso", &ps));

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
    desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    desc.NumRenderTargets = 1;
    desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    desc.SampleDesc = { 1, 0 };
    desc.SampleMask = UINT_MAX;
    desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pipelineState));
}

void CameraTrianglePass::createVertexBuffer(ID3D12Device4* device)
{
    Vertex vertices[3] =
    {
        Vector3(-1.0f, -1.0f, 0.0f),  // 0
        Vector3(0.0f, 1.0f, 0.0f),    // 1
        Vector3(1.0f, -1.0f, 0.0f)    // 2
    };

    m_vertexBuffer = app->GetModuleResources()->CreateIndividualDefaultBuffer(vertices, sizeof(vertices), "Triangle Vertex Buffer");


    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();

    m_vertexBufferView.SizeInBytes = sizeof(vertices);
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
}
