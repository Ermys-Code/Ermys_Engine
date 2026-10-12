#pragma once

#include "Core/Globals.h"
#include "Passes/IRenderPass.h"
#include "Rendering/RenderContext.h"
#include "Rendering/FrameContext.h"

class TrianglePass : public IRenderPass
{
public:
    explicit TrianglePass();

    void prepare(RenderContext* renderContext, FrameContext* frameContext) override;
    void render(ID3D12GraphicsCommandList4* commandList) override;

private:
    void createRootSignature(ID3D12Device4* device);
    void createPipelineState(ID3D12Device4* device);
    void createVertexBuffer(ID3D12Device4* device);

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;

    RenderContext* m_renderContext;
    FrameContext* m_frameContext;

    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
};

