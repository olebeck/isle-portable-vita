#include "Page.hpp"

#include "util.hpp"

#include <psp2/kernel/clib.h>

void Page::Open(paf::Plugin* plugin)
{
	this->plugin = plugin;
	paf::Plugin::PageOpenParam openParam;
	openParam.timer = new paf::Timer(100);
	openParam.transition_type = paf::Plugin::TransitionType_SlideFromLeft;
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
		sceClibPrintf("PageClose(%s)\n", this->opened_id.GetID().c_str());
		this->plugin->PageClose(this->opened_id, closeParam);
		this->scene = nullptr;
	}
}

void func_button_callback(int32_t type, paf::ui::Handler* self, paf::ui::Event* e, void* userdata)
{
	auto* callback_ptr = static_cast<std::function<void()>*>(userdata);
	if (callback_ptr) {
		(*callback_ptr)();
	}
}

void AddClickCallback(paf::ui::ButtonBase* button, std::function<void()>* func)
{
	button->AddEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE, func_button_callback, func);
}
