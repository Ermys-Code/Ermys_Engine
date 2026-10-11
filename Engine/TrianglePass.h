#pragma once

#include "Core/Globals.h"
#include "Passes/IRenderPass.h"

class TrianglePass : public IRenderPass
{
public:
    explicit TrianglePass(ID3D12Device4* device);

    void prepare(RenderContext& renderContext, FrameContext& frameContext) override;
    void apply(ID3D12GraphicsCommandList4* commandList) override;

private:
    void createRootSignature(ID3D12Device4* device);
    void createPipelineState(ID3D12Device4* device);

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;

    RenderContext m_renderContext;
    FrameContext m_frameContext;
};

