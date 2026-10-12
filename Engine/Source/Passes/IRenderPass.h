#pragma once

#include "Core/Globals.h"

struct RenderContext;
struct FrameContext;

class IRenderPass
{
public:
    virtual void prepare(RenderContext* context, FrameContext* frameContext) = 0;
    virtual void render(ID3D12GraphicsCommandList4* commandList) = 0;
    virtual ~IRenderPass() = default;
};

