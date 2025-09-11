#include "DownloadPage.hpp"

#include "util.hpp"

DownloadPage::DownloadPage(uint64_t download_size)
{
	this->download_size = download_size;
	this->mb_done = 0;
}

void DownloadPage::Mount()
{
	auto dialog_base = this->GetElementById<paf::ui::Dialog>("dialog_base");
	paf::Plugin::TemplateOpenParam templateOpenParam;
	this->plugin->TemplateOpen(dialog_base, "template_downloading_dialog", templateOpenParam);
	auto dialog_box1 = dialog_base->GetChild(dialog_base->GetChildrenNum() - 1);

	auto dialog_title = dialog_box1->FindChild("dialog_title");
	dialog_title->SetString(L"Downloading Lego Island");
	this->dialog_text1 = static_cast<paf::ui::Text*>(dialog_box1->FindChild("dialog_text1"));

	// error
	this->error_message = static_cast<paf::ui::Text*>(dialog_box1->FindChild("dialog_error_message"));

	// bar 1
	auto progressbar_1 = dialog_box1->FindChild("dialog_progressbar_box_1");
	progressbar_1->FindChild("dialog_progressbar1_label1")->SetString(L"Total Progress");
	this->progress_bar_total = static_cast<paf::ui::ProgressBar*>(progressbar_1->FindChild("dialog_progressbar1"));

	// bar 2
	auto progressbar_2 = dialog_box1->FindChild("dialog_progressbar_box_2");
	progressbar_2->FindChild("dialog_progressbar1_label1")->SetString(L"File Progress");
	this->progress_bar_file = static_cast<paf::ui::ProgressBar*>(progressbar_2->FindChild("dialog_progressbar1"));
}

void DownloadPage::updateDialogText()
{
	wchar_t wbuf[256];
	sce_paf_swprintf(
		wbuf,
		sizeof(wbuf),
		L"Downloaded %d/%lldMB\nCurrent File: %ls",
		this->mb_done,
		this->download_size / 1000000,
		StringToWString(this->filename).c_str()
	);
	this->dialog_text1->SetString(wbuf);
	this->error_message->Hide();
}

void DownloadPage::UpdateFile(const paf::string& filename)
{
	this->filename = filename;
	this->updateDialogText();
}

void DownloadPage::UpdateProgress(uint64_t total_downloaded, float total_progress, float file_progress)
{
	this->progress_bar_total->SetValue(total_progress, true);
	this->progress_bar_file->SetValue(file_progress, true);

	int current_mb = total_downloaded / 1000000;
	if (current_mb != this->mb_done) {
		this->mb_done = current_mb;
		this->updateDialogText();
	}
}

void DownloadPage::DisplayError(const paf::string& filename, const paf::string& errorMessage)
{
	paf::string error_text = errorMessage;
	error_text += paf::string("\n");
	error_text += filename;

	auto errorMessageWide = StringToWString(error_text);
	this->error_message->SetString(errorMessageWide);
	this->error_message->Show();
}
