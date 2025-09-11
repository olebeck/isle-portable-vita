#include "StartupPage.hpp"

void StartupPage::Mount()
{
	auto download_button = this->GetElementById<paf::ui::Button>("download_button");
	auto verify_button = this->GetElementById<paf::ui::Button>("verify_button");
	AddClickCallback(download_button, &this->OnDownloadButton);
	AddClickCallback(verify_button, &this->OnVerifyButton);
}
