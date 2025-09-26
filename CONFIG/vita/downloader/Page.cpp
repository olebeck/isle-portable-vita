#include "Page.hpp"

#include "util.hpp"

#include <psp2/kernel/clib.h>

void Page::Open(paf::Plugin* plugin)
{
	this->plugin = plugin;
	paf::Plugin::PageOpenParam openParam;
	openParam.fade = true;
	openParam.fade_time_ms = 100;
	if (!plugin->IsRegisteredID(this->PageId())) {
		sceClibPrintf("Page not found %s\n", this->PageId().GetID().c_str());
		this->scene = plugin->PageOpen("FallbackPage", openParam);
		this->opened_id = "FallbackPage";
		return;
	}
	this->scene = plugin->PageOpen(this->PageId(), openParam);
	this->opened_id = this->PageId();
	this->Mount();
}

void Page::Close()
{
	if (this->scene != nullptr) {
		paf::Plugin::PageCloseParam closeParam;
		closeParam.fade = true;
		closeParam.fade_time_ms = 100;
		sceClibPrintf("PageClose(%s)\n", this->opened_id.GetID().c_str());
		this->plugin->PageClose(this->opened_id, closeParam);
		this->scene = nullptr;
	}
}
