/*
 * Copyright (C) 2023 Kyle Reed
 *
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/* TODO:
 * - Busy indicator while reading files.
 */

#ifndef __UI_TEXT_EDITOR_H__
#define __UI_TEXT_EDITOR_H__

#include "ui.hpp"
#include "ui_navigation.hpp"
#include "ui_painter.hpp"
#include "ui_styles.hpp"
#include "ui_widget.hpp"

#include "file_wrapper.hpp"
#include "optional.hpp"

#include <memory>
#include <string>

namespace ui {

enum class ScrollDirection : uint8_t {
    Vertical,
    Horizontal
};

/* Control that renders a text file. */
class TextViewer : public Widget {
   public:
    TextViewer(Rect parent_rect);

    TextViewer(const TextViewer&) = delete;
    TextViewer(TextViewer&&) = delete;
    TextViewer& operator=(const TextViewer&) = delete;
    TextViewer& operator=(TextViewer&&) = delete;

    std::function<void()> on_select{};
    std::function<void()> on_cursor_moved{};

    void paint(Painter& painter) override;
    bool on_key(KeyEvent key) override;
    bool on_encoder(EncoderEvent delta) override;

    void redraw(bool redraw_text = false);

    void set_file(FileWrapper& file) { reset_file(&file); }
    void clear_file() { reset_file(); }
    bool has_file() const { return file_ != nullptr; }

    uint32_t line() const { return cursor_.line; }
    uint32_t col() const { return cursor_.col; }
    uint32_t offset() const;

    // Gets the length of the current line.
    uint16_t line_length();

   private:
    static constexpr int8_t char_width = 5;
    static constexpr int8_t char_height = 8;

    const uint8_t max_line = 32;
    const uint8_t max_col = 48;

    /* Returns true if the cursor was updated. */
    bool apply_scrolling_constraints(
        int16_t delta_line,
        int16_t delta_col);

    void paint_text(Painter& painter, uint32_t line, uint16_t col);
    void paint_cursor(Painter& painter);

    void reset_file(FileWrapper* file = nullptr);

    FileWrapper* file_{};

    struct {
        // Previous cursor state.
        uint32_t line{};
        uint16_t col{};

        // Previous draw state.
        uint32_t first_line{};
        uint16_t first_col{};
        bool redraw_text{true};
    } paint_state_{};

    struct {
        uint32_t line{};
        uint16_t col{};
        ScrollDirection dir{ScrollDirection::Vertical};
    } cursor_{};
};

/* Menu control for the TextEditor. */
class TextEditorMenu : public View {
   public:
    TextEditorMenu();

    void on_show() override;
    void on_hide() override;

    std::function<void()>& on_cut() { return button_cut.on_select; }
    std::function<void()>& on_paste() { return button_paste.on_select; }
    std::function<void()>& on_copy() { return button_copy.on_select; }

    std::function<void()>& on_delete_line() { return button_delline.on_select; }
    std::function<void()>& on_edit_line() { return button_edit.on_select; }
    std::function<void()>& on_add_line() { return button_addline.on_select; }

    std::function<void()>& on_open() { return button_open.on_select; }
    std::function<void()>& on_save() { return button_save.on_select; }
    std::function<void()>& on_exit() { return button_exit.on_select; }

   private:
    void hide_children(bool hidden);

    Rectangle rect_frame{
        {0 * 8, 0 * 8, 23 * 8, 23 * 8},
        Color::dark_grey()};

    NewButton button_cut{
        {1 * 8, 1 * 8, 7 * 8, 7 * 8},
        "Cut",
        &bitmap_icon_cut,
        Color::dark_grey()};

    NewButton button_paste{
        {8 * 8, 1 * 8, 7 * 8, 7 * 8},
        "Paste",
        &bitmap_icon_paste,
        Color::dark_grey()};

    NewButton button_copy{
        {15 * 8, 1 * 8, 7 * 8, 7 * 8},
        "Copy",
        &bitmap_icon_copy,
        Color::dark_grey()};

    NewButton button_delline{
        {1 * 8, 8 * 8, 7 * 8, 7 * 8},
        "-Line",
        &bitmap_icon_delete,
        Color::dark_red()};

    NewButton button_edit{
        {8 * 8, 8 * 8, 7 * 8, 7 * 8},
        "Edit",
        &bitmap_icon_rename,
        Color::dark_blue()};

    NewButton button_addline{
        {15 * 8, 8 * 8, 7 * 8, 7 * 8},
        "+Line",
        &bitmap_icon_scanner,
        Color::dark_blue()};

    NewButton button_open{
        {1 * 8, 15 * 8, 7 * 8, 7 * 8},
        "Open",
        &bitmap_icon_load,
        Color::green()};

    NewButton button_save{
        {8 * 8, 15 * 8, 7 * 8, 7 * 8},
        "Save",
        &bitmap_icon_save,
        Color::green()};

    NewButton button_exit{
        {15 * 8, 15 * 8, 7 * 8, 7 * 8},
        "Exit",
        &bitmap_icon_previous,
        Color::dark_red()};
};

/* View viewing and minor edits on a text file. */
class TextEditorView : public View {
   public:
    TextEditorView(NavigationView& nav);
    TextEditorView(
        NavigationView& nav,
        const std::filesystem::path& path);
    ~TextEditorView();

    std::string title() const override {
        return "Notepad";
    };

    void on_show() override;

   private:
    static constexpr size_t max_edit_length = 1024;
    std::string edit_line_buffer_{};

    void open_file(const std::filesystem::path& path);
    void save_file();
    void refresh_ui();
    void update_position();
    void hide_menu(bool hidden = true);
    void show_file_picker(bool immediate = true);
    void show_edit_line();
    void show_nyi();
    void show_save_prompt(std::function<void()> continuation);

    void prepare_for_write();
    void create_temp_file() const;
    void delete_temp_file() const;
    void save_temp_file();
    std::filesystem::path get_temp_path() const;

    NavigationView& nav_;
    std::unique_ptr<FileWrapper> file_{};
    std::filesystem::path path_{};
    bool file_dirty_{false};
    bool has_temp_file_{false};

    TextViewer viewer{
        /* 272 = 320 - 16 (top bar) - 32 (bottom controls) */
        {0, 0, 240, 272}};

    TextEditorMenu menu{};

    NewButton button_menu{
        {26 * 8, 34 * 8, 4 * 8, 4 * 8},
        {},
        &bitmap_icon_controls,
        Color::dark_grey()};

    Text text_position{
        {0 * 8, 34 * 8, 26 * 8, 2 * 8},
        ""};

    Text text_size{
        {0 * 8, 36 * 8, 26 * 8, 2 * 8},
        ""};
};

}  // namespace ui

#endif  // __UI_TEXT_EDITOR_H__