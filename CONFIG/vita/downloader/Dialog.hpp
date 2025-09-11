#pragma once

#include <paf.h>

class Dialog : public paf::ui::Widget::UserData {
private:
    paf::string title;
    paf::string text;
    paf::string button1_text;
    paf::string button2_text;
    std::function<void()> button1_click;
    std::function<void()> button2_click;

    paf::ui::Widget* parent;
    paf::ui::Widget* dialog_box;
    paf::ui::Button* dialog_button1;
    paf::ui::Button* dialog_button2;

    static void ButtonCallback(int32_t type, paf::ui::Handler *self, paf::ui::Event *e, void *userdata);
    static void HideCallback(int32_t type, paf::ui::Handler *self, paf::ui::Event *e, void *userdata);
public:

    Dialog() = default;
    virtual ~Dialog();

    void SetTitle(const paf::string& title);
    void SetText(const paf::string& text);

    void SetButton1(const paf::string& text, const std::function<void()> click = nullptr);
    void SetButton2(const paf::string& text, const std::function<void()> click = nullptr);

    void Show(paf::Plugin* plugin, paf::ui::Widget* parent);
};
