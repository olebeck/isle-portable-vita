#include "App.hpp"

#include <paf.h>
#include <psp2/libssl.h>
#include <psp2/net/http.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>

int main()
{
	int ret;
	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_COMMON_GUI_DIALOG);

	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	SceNetInitParam netInitParam;
	int size = 1 * 1024 * 1024;
	netInitParam.memory = malloc(size);
	netInitParam.size = size;
	netInitParam.flags = 0;
	ret = sceNetInit(&netInitParam);
	if(ret < 0) {
		sceClibPrintf("sceNetInit: %08x\n", ret);
		return 0;
	}
	ret = sceNetCtlInit();
	if(ret < 0) {
		sceClibPrintf("sceNetCtlInit: %08x\n", ret);
		return 0;
	}

	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
	ret = sceHttpInit(1 * 1024 * 1024);
	if(ret < 0) {
		sceClibPrintf("sceHttpInit: %08x\n", ret);
		return 0;
	}

	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTPS);
	ret = sceSslInit(100 * 1024);
	if(ret < 0) {
		sceClibPrintf("sceSslInit: %08x\n", ret);
		return 0;
	}

	paf::Framework::InitParam fwParam;
	fwParam.mode = paf::Framework::Mode_Normal;
	paf::Framework* paf_fw = new paf::Framework(fwParam);
	paf_fw->LoadCommonResourceSync();
	paf::job::JobQueue::Init();

	App app(paf_fw);
	paf_fw->Run();
	return 0;
}
