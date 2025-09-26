#include "VerifyPage.hpp"

#include "util.hpp"

VerifyPage::VerifyPage(uint64_t total_size)
{
	this->total_size = total_size;
	this->mb_done = 0;
	this->validCount = 0;
	this->brokenCount = 0;
	this->missingCount = 0;
}

void VerifyPage::Mount()
{
	auto dialog_base = this->GetElementById<paf::ui::Dialog>("dialog_base");
	paf::Plugin::TemplateOpenParam templateOpenParam;
	this->plugin->TemplateOpen(dialog_base, "template_downloading_dialog", templateOpenParam);
	auto dialog_box1 = dialog_base->GetChild(dialog_base->GetChildrenNum() - 1);

	auto dialog_title = dialog_box1->FindChild("dialog_title");
	dialog_title->SetString(L"Verifying Lego Island");
	this->dialog_text1 = static_cast<paf::ui::Text*>(dialog_box1->FindChild("dialog_text1"));

	// bar 1
	auto progressbar_1 = dialog_box1->FindChild("dialog_progressbar_box_1");
	progressbar_1->FindChild("dialog_progressbar1_label1")->SetString(L"Total Progress");
	this->progress_bar_total = static_cast<paf::ui::ProgressBar*>(progressbar_1->FindChild("dialog_progressbar1"));

	// bar 2
	auto progressbar_2 = dialog_box1->FindChild("dialog_progressbar_box_2");
	progressbar_2->FindChild("dialog_progressbar1_label1")->SetString(L"File Progress");
	this->progress_bar_file = static_cast<paf::ui::ProgressBar*>(progressbar_2->FindChild("dialog_progressbar1"));
}

void VerifyPage::updateDialogText()
{
	wchar_t wbuf[256];
	sce_paf_swprintf(
		wbuf,
		sizeof(wbuf),
		L"Checked %d/%dMB\nCurrent File: %ls\nValid: %d Broken: %d Missing: %d",
		this->mb_done,
		(int) (this->total_size / 1000000),
		StringToWString(this->filename).c_str(),
		validCount,
		brokenCount,
		missingCount
	);
	this->dialog_text1->SetString(wbuf);
}

void VerifyPage::UpdateFile(const paf::string& filename)
{
	this->filename = filename;
	this->updateDialogText();
}

void VerifyPage::UpdateProgress(uint64_t total_done, float total_progress, float file_progress)
{
	this->progress_bar_total->SetValue(total_progress, true);
	this->progress_bar_file->SetValue(file_progress, true);

	int current_mb = total_done / 1000000;
	if (current_mb != this->mb_done) {
		this->mb_done = current_mb;
		this->updateDialogText();
	}
}

void VerifyPage::UpdateVerifiedStatus(int validCount, int brokenCount, int missingCount)
{
	this->validCount = validCount;
	this->brokenCount = brokenCount;
	this->missingCount = missingCount;
}
