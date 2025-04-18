//#pragma once
//#include "Core/Log.h"
//#include "MemoryAllocation.h"
//
//
//#define PROFILE_FUNCTION_MEM_ALLOC(func) \
//	printf("===============================================start profile##func===============================================\n"); \
//	Util::MemAllocation prev_alloc = Util::gsMemAllocation; \
//	func() \
//	printf("===============================================end profile##function_name===============================================\n"); \
//	printf("function_name##_##USAGE\n"); \
//	DEBUG_LOG("Heap Memory Allocation Count: ", Util::gsMemAllocation.CurrentAllocation() - prev_alloc.CurrentAllocation(), "."); \
//	DEBUG_LOG("Heap Memory Allocation Size: ", Util::gsMemAllocation.CurrentUsage() - prev_alloc.CurrentUsage(), " bytes.\n"); \
//	DEBUG_LOG("Program Overall Allocation Count: ", Util::gsMemAllocation.allocatedCount - prev_alloc.allocatedCount, ".\n"); \
//	DEBUG_LOG("Program Overall Allocation Size: ", Util::gsMemAllocation.allocatedSize - prev_alloc.allocatedSize, " bytes.\n"); \
//	DEBUG_LOG("Program Overall Freed Count: ", Util::gsMemAllocation.freeCount - prev_alloc.freeCount, ".\n"); \
//	DEBUG_LOG("Program Overall Freed Size: ", Util::gsMemAllocation.freeSize - prev_alloc.freeSize, " bytes.\n"); \
//	printf("progarm##_##USAGE\n"); \
//	DEBUG_LOG("Current Heap Memory Allocation Count: ", Util::gsMemAllocation.CurrentAllocation(), "."); \
//	DEBUG_LOG("Current Heap Memory Allocation Size: ", Util::gsMemAllocation.CurrentUsage(), " bytes.\n"); \
//	DEBUG_LOG("Program Overall Allocation Count: ", Util::gsMemAllocation.allocatedCount, ".\n"); \
//	DEBUG_LOG("Program Overall Allocation Size: ", Util::gsMemAllocation.allocatedSize, " bytes.\n"); \
//	DEBUG_LOG("Program Overall Freed Count: ", Util::gsMemAllocation.freeCount, ".\n"); \
//	DEBUG_LOG("Program Overall Freed Size: ", Util::gsMemAllocation.freeSize, " bytes.\n"); \
//	printf("===============================================end profile##function_name===============================================\n"); \

//void Profile_LetsGo()
//{
//	DEBUG_LOG("Test");
//	
//	printf("===============================================start profile##function_name===============================================\n");
//	//cache mem allocation 
//	Util::MemAllocation prev_alloc = Util::gsMemAllocation;
//	//function_name()
//	printf("===============================================end profile##function_name===============================================\n");
//	printf("function_name##_##USAGE\n");
//	DEBUG_LOG("Heap Memory Allocation Count: ", Util::gsMemAllocation.CurrentAllocation() - prev_alloc.CurrentAllocation(), ".");
//	DEBUG_LOG("Heap Memory Allocation Size: ", Util::gsMemAllocation.CurrentUsage() - prev_alloc.CurrentUsage(), " bytes.\n");
//	DEBUG_LOG("Program Overall Allocation Count: ", Util::gsMemAllocation.allocatedCount - prev_alloc.allocatedCount, ".\n");
//	DEBUG_LOG("Program Overall Allocation Size: ", Util::gsMemAllocation.allocatedSize - prev_alloc.allocatedSize, " bytes.\n");
//	DEBUG_LOG("Program Overall Freed Count: ", Util::gsMemAllocation.freeCount - prev_alloc.freeCount, ".\n");
//	DEBUG_LOG("Program Overall Freed Size: ", Util::gsMemAllocation.freeSize - prev_alloc.freeSize, " bytes.\n");
//	printf("progarm##_##USAGE\n");
//	DEBUG_LOG("Current Heap Memory Allocation Count: ", Util::gsMemAllocation.CurrentAllocation(), ".");
//	DEBUG_LOG("Current Heap Memory Allocation Size: ", Util::gsMemAllocation.CurrentUsage(), " bytes.\n");
//	DEBUG_LOG("Program Overall Allocation Count: ", Util::gsMemAllocation.allocatedCount, ".\n");
//	DEBUG_LOG("Program Overall Allocation Size: ", Util::gsMemAllocation.allocatedSize, " bytes.\n");
//	DEBUG_LOG("Program Overall Freed Count: ", Util::gsMemAllocation.freeCount, ".\n");
//	DEBUG_LOG("Program Overall Freed Size: ", Util::gsMemAllocation.freeSize, " bytes.\n");
//	printf("===============================================end profile##function_name===============================================\n");
//}
//
