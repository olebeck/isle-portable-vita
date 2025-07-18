#include "gxm_memory.h"
#include "tlsf.h"
#include "utils.h"

#include <SDL3/SDL_stdinc.h>
#include <psp2/gxm.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/sysmem.h>

#define CDRAM_POOL_SIZE 64 * 1024 * 1024

static SceUID cdramAllocatorUID = -1;
static tlsf_t cdramAllocator = nullptr;
int inuse_mem = 0;

void* patcher_host_alloc(void* user_data, unsigned int size)
{
	void* mem = SDL_malloc(size);
	(void) user_data;
	return mem;
}

void patcher_host_free(void* user_data, void* mem)
{
	(void) user_data;
	SDL_free(mem);
}

void* vita_mem_alloc(unsigned int type, size_t size, size_t alignment, int attribs, SceUID* uid, const char* name)
{
	void* mem;

	if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW) {
		size = ALIGN(size, 256 * 1024);
	}
	else if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW) {
		size = ALIGN(size, 1024 * 1024);
	}
	else {
		size = ALIGN(size, 4 * 1024);
	}

	*uid = sceKernelAllocMemBlock(name, type, size, NULL);

	if (*uid < 0) {
		SDL_Log("sceKernelAllocMemBlock: 0x%x", *uid);
		return NULL;
	}

	if (sceKernelGetMemBlockBase(*uid, &mem) < 0) {
		return NULL;
	}

	if (sceGxmMapMemory(mem, size, (SceGxmMemoryAttribFlags) attribs) < 0) {
		SDL_Log("sceGxmMapMemory 0x%x 0x%x %d failed", mem, size, attribs);
		return NULL;
	}

	return mem;
}

void vita_mem_free(SceUID uid)
{
	void* mem = NULL;
	if (sceKernelGetMemBlockBase(uid, &mem) < 0) {
		return;
	}
	sceGxmUnmapMemory(mem);
	sceKernelFreeMemBlock(uid);
}

void* vita_mem_vertex_usse_alloc(unsigned int size, SceUID* uid, unsigned int* usse_offset)
{
	void* mem = NULL;

	size = ALIGN(size, 4096);
	*uid = sceKernelAllocMemBlock("vertex_usse", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, size, NULL);

	if (sceKernelGetMemBlockBase(*uid, &mem) < 0) {
		return NULL;
	}
	if (sceGxmMapVertexUsseMemory(mem, size, usse_offset) < 0) {
		return NULL;
	}

	return mem;
}

void vita_mem_vertex_usse_free(SceUID uid)
{
	void* mem = NULL;
	if (sceKernelGetMemBlockBase(uid, &mem) < 0) {
		return;
	}
	sceGxmUnmapVertexUsseMemory(mem);
	sceKernelFreeMemBlock(uid);
}

void* vita_mem_fragment_usse_alloc(unsigned int size, SceUID* uid, unsigned int* usse_offset)
{
	void* mem = NULL;

	size = ALIGN(size, 4096);
	*uid = sceKernelAllocMemBlock("fragment_usse", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, size, NULL);

	if (sceKernelGetMemBlockBase(*uid, &mem) < 0) {
		return NULL;
	}
	if (sceGxmMapFragmentUsseMemory(mem, size, usse_offset) < 0) {
		return NULL;
	}

	return mem;
}

void vita_mem_fragment_usse_free(SceUID uid)
{
	void* mem = NULL;
	if (sceKernelGetMemBlockBase(uid, &mem) < 0) {
		return;
	}
	sceGxmUnmapFragmentUsseMemory(mem);
	sceKernelFreeMemBlock(uid);
}
