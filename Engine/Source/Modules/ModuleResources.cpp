#include "Core/Globals.h"
#include "ModuleResources.h"

#include "Core/Application.h"
#include "Modules/ModuleD3D12.h"

#include "D3D12/CommandQueue.h"

#include <stdexcept>

ModuleResources::ModuleResources()
{
}

ModuleResources::~ModuleResources()
{
}

bool ModuleResources::init()
{
	bool ok = true;
	
	m_device = app->GetModuleD3D12()->GetDevice();
	m_commandQueue = app->GetModuleD3D12()->GetCommandQueue();

	ok = ok && SUCCEEDED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_allocator)));
	ok = ok && SUCCEEDED(m_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_commandList)));

	return ok;
}

void ModuleResources::preRender()
{
	CollectCompletedResources();
}

bool ModuleResources::cleanUp()
{
	if (m_uploadBatchOpen)
	{
		m_commandList->Close();
		m_uploadBatchOpen = false;
		m_pendingUploadResources.clear();
	}

	m_commandQueue->Flush();

	m_deferredResources.clear();
	m_pendingUploadResources.clear();

	m_commandList.Reset();
	m_allocator.Reset();

	m_lastUploadFence = 0;

	return true;
}

ComPtr<ID3D12Resource> ModuleResources::CreateUploadBuffer(size_t size)
{
	ComPtr<ID3D12Resource> buffer;
	
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);

	m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer));
	
	return buffer;
}

ComPtr<ID3D12Resource> ModuleResources::CreateIndividualDefaultBuffer(const void* data, size_t size, const char* name)
{
	BeginUploadCommands();

	ComPtr<ID3D12Resource> buffer = CreateDefaultBuffer(data, size, name);

	SubmitUploadBatch();

	return buffer;
}

void ModuleResources::StartUploadBatch()
{
	BeginUploadCommands();
}

ComPtr<ID3D12Resource> ModuleResources::CreateDefaultBufferForBatch(const void* data, size_t size, const char* name)
{
	return CreateDefaultBuffer(data, size, name);
}

void ModuleResources::SubmitUploadBatch()
{
	SubmitUploadCommands();
}

ComPtr<ID3D12Resource> ModuleResources::CreateDefaultBuffer(const void* data, size_t size, const char* name)
{
	ComPtr<ID3D12Resource> buffer;
	CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
	m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));

	ComPtr<ID3D12Resource> uploadBuffer = CreateUploadBuffer(size);

	BYTE* pData = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
	memcpy(pData, data, size);
	uploadBuffer->Unmap(0, nullptr);

	m_commandList->CopyResource(buffer.Get(), uploadBuffer.Get());

	if (name && name[0] != '\0')
	{
		buffer->SetName(std::wstring(name, name + strlen(name)).c_str());
	}
	
	m_pendingUploadResources.push_back(std::move(uploadBuffer));


	return buffer;
}

void ModuleResources::BeginUploadCommands()
{
	if (m_uploadBatchOpen)
	{
		char message[256];
		std::snprintf(message, sizeof(message), "Can't open a UploadBatch when there is one UploadBatch opened already.");
		DEBUG_ERROR("%s", message);
		throw std::runtime_error(message);
	}

	if (m_lastUploadFence != 0) m_commandQueue->WaitForFenceValue(m_lastUploadFence);

	m_allocator->Reset();
	m_commandList->Reset(m_allocator.Get(), nullptr);

	m_uploadBatchOpen = true;
}

void ModuleResources::SubmitUploadCommands()
{
	if (!m_uploadBatchOpen)
	{
		char message[256];
		std::snprintf(message, sizeof(message), "Can't submit a UploadBatch when there is no UploadBatch opened.");
		DEBUG_ERROR("%s", message);
		throw std::runtime_error(message);
	}

	m_commandList->Close();
	
	m_lastUploadFence = m_commandQueue->ExecuteCommandListAndSignal(m_commandList.Get());

	for (auto& resource : m_pendingUploadResources)
	{
		DeferredResourcesRelease(std::move(resource), m_lastUploadFence);
	}

	m_pendingUploadResources.clear();

	m_uploadBatchOpen = false;
}

void ModuleResources::DeferredResourcesRelease(ComPtr<ID3D12Resource> resource, uint64_t fenceValue)
{
	if (!resource) return;

	DeferredResource deferredResource{};
	deferredResource.fenceValue = fenceValue;
	deferredResource.resource = resource;

	m_deferredResources.push_back(std::move(deferredResource));
}

void ModuleResources::CollectCompletedResources()
{
	const uint64_t completedFence = m_commandQueue->GetCompletedFenceValue();

	auto it = m_deferredResources.begin();

	while (it != m_deferredResources.end())
	{
		if (it->fenceValue <= completedFence)
		{
			it = m_deferredResources.erase(it);
		}
		else
		{
			it++;
		}
	}
}
