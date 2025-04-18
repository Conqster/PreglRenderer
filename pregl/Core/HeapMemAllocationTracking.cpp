#include "HeapMemAllocationTracking.h"

#include <memory>
#include "Log.h"


#if ENABLE_HEAP_MEM_TRACKING

void* operator new(size_t in_size)
{
	void* m = malloc(in_size);
	if (!m) throw std::bad_alloc();
	auto& alloc_tracker = Util::Memory::GetMemAllocTracker();
	alloc_tracker.allocatedCount++;
	alloc_tracker.allocatedSize += in_size;
	//DEBUG_LOG_ERROR("[HEAP ALLOCATION]: ", (int)in_size, "bytes.");
	printf("[NEW -- HEAP] Allocated: %d bytes, mem address: %p.\n", int(in_size), m);
	return m;
}


void operator delete(void* in_memory, size_t in_size) noexcept
{
	auto& alloc_tracker = Util::Memory::GetMemAllocTracker();
	alloc_tracker.freeCount++;
	alloc_tracker.freeSize += in_size;
	printf("[DEL -- HEAP] freed: %d bytes, mem address: %p.\n", int(in_size), in_memory);
	free(in_memory);
}


void operator delete(void* in_memory) noexcept
{
	auto& alloc_tracker = Util::Memory::GetMemAllocTracker();
	alloc_tracker.freeCount++;
	printf("[DEL -- HEAP] freed: unknown size bytes, mem address: %p.\n", in_memory);
	free(in_memory);
}


#endif // ENABLE_HEAP_MEM_TRACKING



