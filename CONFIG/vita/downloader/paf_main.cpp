#include "App.hpp"

#include <paf.h>

int paf_main()
{
	paf::Framework::InitParam fwParam;
	fwParam.mode = paf::Framework::Mode_Normal;
	paf::Framework* paf_fw = new paf::Framework(fwParam);
	paf_fw->LoadCommonResourceSync();
	paf::job::JobQueue::Init();

	App app(paf_fw);
	paf_fw->Run();
	return 0;
}
