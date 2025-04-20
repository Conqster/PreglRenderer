#pragma once

#include "NonCopyable.h"
#include "Log.h"


#define ENABLE_HEAP_MEM_TRACKING 0
#define ENABLE_FULL_PROFILING 0

namespace Util {
	namespace Memory {

		struct MemAllocation
		{
			size_t allocatedCount = 0;
			size_t freeCount = 0;
			size_t allocatedSize = 0;
			size_t freeSize = 0;
	
			size_t CurrentUsage() { return allocatedSize - freeSize; }
			size_t CurrentAllocation() { return allocatedCount - freeCount; }
		};

		inline MemAllocation& GetMemAllocTracker() {
			static MemAllocation mem_alloc_inst;
			return mem_alloc_inst;
		}


		struct ScopeMemProfile : NonCopyable
		{
			//stack allocated 
			const char* nameID;
			Util::Memory::MemAllocation tracker;
			ScopeMemProfile(const char* name_id) : nameID(name_id)
			{

#if ENABLE_HEAP_MEM_TRACKING
				tracker = Util::Memory::GetMemAllocTracker();
				DEBUG_LOG("====[OPEN PROFILE SCOPE]: ", name_id, " ====");
#if ENABLE_FULL_PROFILING
				DEBUG_LOG("====[CURRENT PROGRAM STATE]====");
				DEBUG_LOG("Current Heap Memory Allocation Count: ", tracker.CurrentAllocation(), ".");
				DEBUG_LOG("Current Heap Memory Allocation Size: ", tracker.CurrentUsage(), " bytes.");
				DEBUG_LOG("Program Overall Allocation Count: ", tracker.allocatedCount, ".");
				DEBUG_LOG("Program Overall Allocation Size: ", tracker.allocatedSize, " bytes.");
				DEBUG_LOG("Program Overall Freed Count: ", tracker.freeCount, ".");
				DEBUG_LOG("Program Overall Freed Size: ", tracker.freeSize, " bytes.");
#endif // ENABLE_FULL_PROFILING
#endif // ENABLE_HEAP_MEM_TRACKING

			}
			~ScopeMemProfile()
			{

#if ENABLE_HEAP_MEM_TRACKING
				DEBUG_LOG("====[CONCLUDING PROFILE SCOPE]: ", nameID, " ====");
				auto& mem_alloc_tracker = Util::Memory::GetMemAllocTracker();
				auto block_mem_alloc_count = mem_alloc_tracker.allocatedCount - tracker.allocatedCount;
				auto block_mem_alloc_size = mem_alloc_tracker.allocatedSize - tracker.allocatedSize;
				auto block_mem_free_count = mem_alloc_tracker.freeCount - tracker.freeCount;
				auto block_mem_free_size = mem_alloc_tracker.freeSize - tracker.freeSize;
				DEBUG_LOG("{Scope - ", nameID, "}: allocated mem count: ", block_mem_alloc_count, ".");
				DEBUG_LOG("{Scope - ", nameID, "}: allocated mem size: ", block_mem_alloc_size, " bytes.");
				DEBUG_LOG("{Scope - ", nameID, "}: freed mem count: ", block_mem_free_count, ".");
				DEBUG_LOG("{Scope - ", nameID, "}: freed mem Size: ", block_mem_free_size, " bytes.");
				DEBUG_LOG("{Scope - ", nameID, "}: Still allocated mem count: ", mem_alloc_tracker.CurrentAllocation() - tracker.CurrentAllocation(), ".");
				DEBUG_LOG("{Scope - ", nameID, "}: freed count out of allocated {must similar to above} ", block_mem_alloc_count - block_mem_free_count, ".");
				DEBUG_LOG("{Scope - ", nameID, "}: Still allocated mem size: ", mem_alloc_tracker.CurrentUsage() - tracker.CurrentUsage(), " bytes.");
				DEBUG_LOG("{Scope - ", nameID, "}: freed size out of allocated {must similar to above} ", block_mem_alloc_size - block_mem_free_size, ".");
				PGL_ASSERT_WARN((block_mem_alloc_count - block_mem_free_count <= 0), "Potential Memory leak {Block -- ", nameID, "}, not all allocated memeory was freed");
	#if ENABLE_FULL_PROFILING
				DEBUG_LOG("====[CURRENT PROGRAM STATE]====");
				DEBUG_LOG("Current Heap Memory Allocation Count: ", mem_alloc_tracker.CurrentAllocation(), ".");
				DEBUG_LOG("Current Heap Memory Allocation Size: ", mem_alloc_tracker.CurrentUsage(), " bytes.");
				DEBUG_LOG("Program Overall Allocation Count: ", mem_alloc_tracker.allocatedCount, ".");
				DEBUG_LOG("Program Overall Allocation Size: ", mem_alloc_tracker.allocatedSize, " bytes.");
				DEBUG_LOG("Program Overall Freed Count: ", mem_alloc_tracker.freeCount, ".");
				DEBUG_LOG("Program Overall Freed Size: ", mem_alloc_tracker.freeSize, " bytes.");
	#endif // ENABLE_FULL_PROFILING
				DEBUG_LOG("====[CLOSE PROFILE SCOPE]: ", nameID, " ====");
	#endif // ENABLE_HEAP_MEM_TRACKING

			}

		}; // ScopeMemProfile struct 
	} // Memory namespace
} // Util namespace








#if ENABLE_HEAP_MEM_TRACKING

	void* operator new(size_t in_size);
	void operator delete(void* in_ptr_memory, size_t in_size) noexcept;
	void operator delete(void* in_ptr_memory) noexcept;

//USAGE: SCOPE_MEM_ALLOC_PROFILE("MainLoop")
//
#define SCOPE_MEM_ALLOC_PROFILE(name) Util::Memory::ScopeMemProfile scope_profile = Util::Memory::ScopeMemProfile(name);

#if ENABLE_FULL_PROFILING
//USAGE:	OPEN_BLOCK_MEM_TRACKING_PROFILE(test_block_tracking);
//	Test();
//	CLOSE_BLOCK_MEM_TRACKING_PROFILE(test_block_tracking);
//	test_block_tracking :- serves as name & id to create prev_mem_allocated
//
#define OPEN_BLOCK_MEM_TRACKING_PROFILE(name_id) \
		DEBUG_LOG("====[OPEN PROFILE BLOCK]: ", #name_id, " ===="); \
		Util::Memory::MemAllocation pre_mem_alloc##_##name_id = Util::Memory::GetMemAllocTracker(); \
		DEBUG_LOG("====[CURRENT PROGRAM STATE]===="); \
		DEBUG_LOG("Current Heap Memory Allocation Count: ", pre_mem_alloc##_##name_id.CurrentAllocation(), "."); \
		DEBUG_LOG("Current Heap Memory Allocation Size: ", pre_mem_alloc##_##name_id.CurrentUsage(), " bytes."); \
		DEBUG_LOG("Program Overall Allocation Count: ", pre_mem_alloc##_##name_id.allocatedCount, "."); \
		DEBUG_LOG("Program Overall Allocation Size: ", pre_mem_alloc##_##name_id.allocatedSize, " bytes."); \
		DEBUG_LOG("Program Overall Freed Count: ", pre_mem_alloc##_##name_id.freeCount, "."); \
		DEBUG_LOG("Program Overall Freed Size: ", pre_mem_alloc##_##name_id.freeSize, " bytes.")

//USAGE:	OPEN_BLOCK_MEM_TRACKING_PROFILE(test_block_tracking);
//	Test();
//	CLOSE_BLOCK_MEM_TRACKING_PROFILE(test_block_tracking);
//	test_block_tracking :- serves as name & id to create prev_mem_allocated
//
#define CLOSE_BLOCK_MEM_TRACKING_PROFILE(name_id) \
		DEBUG_LOG("====[CONCLUDING PROFILE BLOCK]: ", #name_id, " ===="); \
		auto& mem_alloc_tracker##_##name_id = Util::Memory::GetMemAllocTracker(); \
		auto block_mem_alloc_count##_##name_id = mem_alloc_tracker##_##name_id.allocatedCount - pre_mem_alloc##_##name_id.allocatedCount; \
		auto block_mem_alloc_size##_##name_id = mem_alloc_tracker##_##name_id.allocatedSize - pre_mem_alloc##_##name_id.allocatedSize; \
		auto block_mem_free_count##_##name_id = mem_alloc_tracker##_##name_id.freeCount - pre_mem_alloc##_##name_id.freeCount; \
		auto block_mem_free_size##_##name_id = mem_alloc_tracker##_##name_id.freeSize - pre_mem_alloc##_##name_id.freeSize; \
		DEBUG_LOG("{Block - ", #name_id, "}: allocated mem count: ", block_mem_alloc_count##_##name_id, "."); \
		DEBUG_LOG("{Block - ", #name_id, "}: allocated mem size: ", block_mem_alloc_size##_##name_id, " bytes."); \
		DEBUG_LOG("{Block - ", #name_id, "}: freed mem count: ", block_mem_free_count##_##name_id, "."); \
		DEBUG_LOG("{Block - ", #name_id, "}: freed mem Size: ", block_mem_free_size##_##name_id, " bytes."); \
		DEBUG_LOG("{Block - ", #name_id, "}: Still allocated mem count: ", mem_alloc_tracker##_##name_id.CurrentAllocation() - pre_mem_alloc##_##name_id.CurrentAllocation(), "."); \
		DEBUG_LOG("{Block - ", #name_id, "}: freed count out of allocated {must similar to above} ", block_mem_alloc_count##_##name_id - block_mem_free_count##_##name_id, "."); \
		DEBUG_LOG("{Block - ", #name_id, "}: Still allocated mem size: ", mem_alloc_tracker##_##name_id.CurrentUsage() - pre_mem_alloc##_##name_id.CurrentUsage(), " bytes."); \
		DEBUG_LOG("{Block - ", #name_id, "}: freed size out of allocated {must similar to above} ", block_mem_alloc_size##_##name_id - block_mem_free_size##_##name_id, "."); \
		PGL_ASSERT_WARN((block_mem_alloc_count##_##name_id - block_mem_free_count##_##name_id <= 0), "Potential Memory leak {Block -- ", #name_id, "}, not all allocated memeory was freed"); \
		DEBUG_LOG("====[CURRENT PROGRAM STATE]===="); \
		DEBUG_LOG("Current Heap Memory Allocation Count: ", mem_alloc_tracker##_##name_id.CurrentAllocation(), "."); \
		DEBUG_LOG("Current Heap Memory Allocation Size: ", mem_alloc_tracker##_##name_id.CurrentUsage(), " bytes."); \
		DEBUG_LOG("Program Overall Allocation Count: ", mem_alloc_tracker##_##name_id.allocatedCount, "."); \
		DEBUG_LOG("Program Overall Allocation Size: ", mem_alloc_tracker##_##name_id.allocatedSize, " bytes."); \
		DEBUG_LOG("Program Overall Freed Count: ", mem_alloc_tracker##_##name_id.freeCount, "."); \
		DEBUG_LOG("Program Overall Freed Size: ", mem_alloc_tracker##_##name_id.freeSize, " bytes."); \
		DEBUG_LOG("====[CLOSE PROFILE BLOCK]: ", #name_id, " ====")

#else


//USAGE:	OPEN_BLOCK_MEM_TRACKING_PROFILE(test_block_tracking);
//	Test();
//	CLOSE_BLOCK_MEM_TRACKING_PROFILE(test_block_tracking);
//	test_block_tracking :- serves as name & id to create prev_mem_allocated
//
#define OPEN_BLOCK_MEM_TRACKING_PROFILE(name_id) \
		DEBUG_LOG("====[OPEN PROFILE BLOCK]: ", #name_id, " ===="); \
		Util::Memory::MemAllocation pre_mem_alloc##_##name_id = Util::Memory::GetMemAllocTracker()


//USAGE:	OPEN_BLOCK_MEM_TRACKING_PROFILE(test_block_tracking);
//	Test();
//	CLOSE_BLOCK_MEM_TRACKING_PROFILE(test_block_tracking);
//	test_block_tracking :- serves as name & id to create prev_mem_allocated
//
#define CLOSE_BLOCK_MEM_TRACKING_PROFILE(name_id) \
		DEBUG_LOG("====[CONCLUDING PROFILE BLOCK]: ", #name_id, " ===="); \
		auto& mem_alloc_tracker##_##name_id = Util::Memory::GetMemAllocTracker(); \
		auto block_mem_alloc_count##_##name_id = mem_alloc_tracker##_##name_id.allocatedCount - pre_mem_alloc##_##name_id.allocatedCount; \
		auto block_mem_alloc_size##_##name_id = mem_alloc_tracker##_##name_id.allocatedSize - pre_mem_alloc##_##name_id.allocatedSize; \
		auto block_mem_free_count##_##name_id = mem_alloc_tracker##_##name_id.freeCount - pre_mem_alloc##_##name_id.freeCount; \
		auto block_mem_free_size##_##name_id = mem_alloc_tracker##_##name_id.freeSize - pre_mem_alloc##_##name_id.freeSize; \
		DEBUG_LOG("{Block - ", #name_id, "}: allocated mem count: ", block_mem_alloc_count##_##name_id, "."); \
		DEBUG_LOG("{Block - ", #name_id, "}: allocated mem size: ", block_mem_alloc_size##_##name_id, " bytes."); \
		DEBUG_LOG("{Block - ", #name_id, "}: freed mem count: ", block_mem_free_count##_##name_id, "."); \
		DEBUG_LOG("{Block - ", #name_id, "}: freed mem Size: ", block_mem_free_size##_##name_id, " bytes."); \
		DEBUG_LOG("{Block - ", #name_id, "}: Still allocated mem count: ", mem_alloc_tracker##_##name_id.CurrentAllocation() - pre_mem_alloc##_##name_id.CurrentAllocation(), "."); \
		DEBUG_LOG("{Block - ", #name_id, "}: freed count out of allocated {must similar to above} ", block_mem_alloc_count##_##name_id - block_mem_free_count##_##name_id, "."); \
		DEBUG_LOG("{Block - ", #name_id, "}: Still allocated mem size: ", mem_alloc_tracker##_##name_id.CurrentUsage() - pre_mem_alloc##_##name_id.CurrentUsage(), " bytes."); \
		DEBUG_LOG("{Block - ", #name_id, "}: freed size out of allocated {must similar to above} ", block_mem_alloc_size##_##name_id - block_mem_free_size##_##name_id, "."); \
		PGL_ASSERT_WARN((block_mem_alloc_count##_##name_id - block_mem_free_count##_##name_id <= 0), "Potential Memory leak {Block -- ", #name_id, "}, not all allocated memeory was freed"); \
		DEBUG_LOG("====[CLOSE PROFILE BLOCK]: ", #name_id, " ====")
#endif // ENABLE_FULL_PROFILING


//USAGE:	PROFILE_FUNCTION_MEM_ALLOC(func_name_id, func_block)
//		func_name_id :- tracker given name as id for multple tracking on same frame stack call
//		func_block :- function call to within open close tracking.
#define PROFILE_FUNCTION_MEM_ALLOC(name_id, func_block) \
	do { \
		OPEN_BLOCK_MEM_TRACKING_PROFILE(name_id); \
		func_block \
		CLOSE_BLOCK_MEM_TRACKING_PROFILE(name_id); \
	} while(0)
#else
#define PROFILE_FUNCTION_MEM_ALLOC(name, func_block) func_block
#define OPEN_BLOCK_MEM_TRACKING_PROFILE(name)
#define CLOSE_BLOCK_MEM_TRACKING_PROFILE(name)
#define SCOPE_MEM_ALLOC_PROFILE(name)
#endif // ENABLE_HEAP_MEM_TRACKING