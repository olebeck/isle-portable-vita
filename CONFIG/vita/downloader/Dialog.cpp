#include "Dialog.hpp"

#include "util.hpp"

#include <psp2/kernel/clib.h>

Dialog::~Dialog()
{
	sceClibPrintf("Dialog::~Dialog\n");
}

void Dialog::ButtonCallback(int32_t type, paf::ui::Handler* self, paf::ui::Event* e, void* userdata)
{
	Dialog* dialog = static_cast<Dialog*>(userdata);
	if (self == dialog->dialog_button1) {
		if (dialog->button1_click) {
			dialog->button1_click();
		}
	}
	else {
		if (dialog->button2_click) {
			dialog->button2_click();
		}
	}
	dialog->dialog_box->Hide(paf::common::transition::Type_Popup5, 0, Dialog::HideCallback, dialog);
}

void Dialog::HideCallback(int32_t type, paf::ui::Handler* self, paf::ui::Event* e, void* userdata)
{
	sceClibPrintf("HideCallback\n");
	// Dialog* dialog = static_cast<Dialog*>(userdata);
	// dialog->dialog_box->GetParent()->RemoveChild(dialog->dialog_box); // crashes for some reason when switching pages
}

void Dialog::SetTitle(const paf::string& title)
{
	this->title = title;
}

void Dialog::SetText(const paf::string& text)
{
	this->text = text;
}

void Dialog::SetButton1(const paf::string& text, const std::function<void()> click)
{
	this->button1_text = text;
	this->button1_click = click;
}

void Dialog::SetButton2(const paf::string& text, const std::function<void()> click)
{
	this->button2_text = text;
	this->button2_click = click;
}

void Dialog::Show(paf::Plugin* plugin, paf::ui::Widget* parent)
{
	paf::Plugin::TemplateOpenParam openParam;
	plugin->TemplateOpen(parent, "_common_template_dialog_base", openParam);
	this->dialog_box = parent->GetChild(parent->GetChildrenNum() - 1);
	this->dialog_box->SetUserData2(this, 0);
	this->dialog_box->SetName(rand()); // multiple dialogs with same id crashes

	bool twoButton = !this->button2_text.empty();
	paf::IDParam templateId = twoButton ? "_common_template_t1b2_dialog" : "_common_template_t1b1_dialog";
	plugin->TemplateOpen(this->dialog_box, templateId, openParam);
	auto dialog = this->dialog_box->GetChild(this->dialog_box->GetChildrenNum() - 1);

	auto dialog_title = static_cast<paf::ui::Text*>(dialog->FindChild("dialog_title"));
	auto dialog_text = dialog->FindChild("dialog_text1");

	dialog_title->SetString(StringToWString(this->title));
	dialog_text->SetString(StringToWString(this->text));

	this->dialog_button1 = static_cast<paf::ui::Button*>(dialog->FindChild("dialog_button1"));
	this->dialog_button1->AddEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE, Dialog::ButtonCallback, this);
	this->dialog_button1->SetString(StringToWString(this->button1_text));

	if (twoButton) {
		this->dialog_button2 = static_cast<paf::ui::Button*>(dialog->FindChild("dialog_button2"));
		this->dialog_button2->AddEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE, Dialog::ButtonCallback, this);
		this->dialog_button2->SetString(StringToWString(this->button2_text));
	}
	dialog->Show();

	this->dialog_box->Show(paf::common::transition::Type_Popup1);
}
