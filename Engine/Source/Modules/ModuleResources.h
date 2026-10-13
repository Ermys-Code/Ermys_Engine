#pragma once
#include "IModule.h"

class CommandQueue;

struct DeferredResource
{
	uint64_t			   fenceValue = 0;
	ComPtr<ID3D12Resource> resource;
};

class ModuleResources : public IModule
{
public:
	ModuleResources();
	~ModuleResources();

	bool init()		  override;
	void preRender()  override;
	bool cleanUp()	  override;

	ComPtr<ID3D12Resource> CreateUploadBuffer(size_t size);
	
	ComPtr<ID3D12Resource> CreateIndividualDefaultBuffer(const void* data, size_t size, const char* name);

	void StartUploadBatch();
	ComPtr<ID3D12Resource> CreateDefaultBufferForBatch(const void* data, size_t size, const char* name);
	void SubmitUploadBatch();


private:
	ID3D12Device4* m_device;
	CommandQueue* m_commandQueue;

	ComPtr<ID3D12CommandAllocator> m_allocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	uint64_t m_lastUploadFence = 0;

	std::vector<DeferredResource> m_deferredResources;
	std::vector<ComPtr<ID3D12Resource>> m_pendingUploadResources;

	bool m_uploadBatchOpen = false;

	ComPtr<ID3D12Resource> CreateDefaultBuffer(const void* data, size_t size, const char* name);

	void BeginUploadCommands();
	void SubmitUploadCommands();

	void DeferredResourcesRelease(ComPtr<ID3D12Resource> resource, uint64_t fenceValue);
	void CollectCompletedResources();
};