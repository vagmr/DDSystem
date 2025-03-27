#define _CRT_SECURE_NO_WARNINGS // 禁用警告

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <string>
#include <cstdio> // For _popen, _pclose
#include <cstdlib> // For system, EXIT_FAILURE, EXIT_SUCCESS (though _popen is preferred)
#include <stdexcept> // For std::runtime_error
#include <vector>
#include <iostream> // For cerr
#include <windows.h> // 用于文件检查
#pragma execution_character_set("utf-8")

// --- Custom Color Scheme ---
// 定义现代化配色方案
#define COLOR_BG            fl_rgb_color(240, 240, 245)    // 淡蓝灰色背景
#define COLOR_PRIMARY       fl_rgb_color(41, 128, 185)     // 主色调：靓蓝色
#define COLOR_SECONDARY     fl_rgb_color(29, 188, 156)     // 辅助色：绿松石色
#define COLOR_ACCENT        fl_rgb_color(155, 89, 182)     // 强调色：薰衣草色
#define COLOR_TEXT_DARK     fl_rgb_color(52, 73, 94)       // 深色文字
#define COLOR_TEXT_LIGHT    fl_rgb_color(236, 240, 241)    // 浅色文字
#define COLOR_SUCCESS       fl_rgb_color(46, 204, 113)     // 成功色：绿色
#define COLOR_WARNING       fl_rgb_color(230, 126, 34)     // 警告色：橙色
#define COLOR_INPUT_BG      fl_rgb_color(248, 248, 255)    // 输入框背景色

// --- Configuration ---
const char* DDP2_PACK_EXE = "lib\\DDP2_pack.exe";
const char* DDP2_UNPACK_EXE = "lib\\DDP2_unpack.exe";
const char* DDP3_PACK_EXE = "lib\\DDP3_pack_wchar.exe";
const char* DDP3_UNPACK_EXE = "lib\\DDP3_unpack_wchar.exe";

// --- Global Widgets ---
Fl_Text_Display* output_display = nullptr;
Fl_Text_Buffer* output_buffer = nullptr;

// --- 检查可执行程序是否存在 ---
bool file_exists(const char* path) {
    DWORD fileAttributes = GetFileAttributesA(path);
    return (fileAttributes != INVALID_FILE_ATTRIBUTES && 
            !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

void check_executables() {
    if (output_buffer && output_display) {
        output_buffer->append("正在检查必要的可执行程序...\n");
        
        struct ExeInfo {
            const char* path;
            const char* name;
            bool found;
        };
        
        ExeInfo exes[] = {
            { DDP2_PACK_EXE, "DDP2打包程序", false },
            { DDP2_UNPACK_EXE, "DDP2解包程序", false },
            { DDP3_PACK_EXE, "DDP3打包程序(wchar)", false },
            { DDP3_UNPACK_EXE, "DDP3解包程序(wchar)", false }
        };
        
        int missing_count = 0;
        
        for (auto& exe : exes) {
            exe.found = file_exists(exe.path);
            if (exe.found) {
                output_buffer->append("✅ ");
                output_buffer->append(exe.name);
                output_buffer->append(" 已找到: ");
            } else {
                output_buffer->append("❌ ");
                output_buffer->append(exe.name);
                output_buffer->append(" 未找到: ");
                missing_count++;
            }
            output_buffer->append(exe.path);
            output_buffer->append("\n");
        }
        
        output_buffer->append("\n");
        if (missing_count == 0) {
            output_buffer->append("✨ 所有程序检查完毕，系统准备就绪！\n");
        } else {
            output_buffer->append("⚠️ 警告：有 ");
            output_buffer->append(std::to_string(missing_count).c_str());
            output_buffer->append(" 个程序未找到，部分功能可能无法使用。\n");
            output_buffer->append("请确保所有可执行文件都在正确的位置。\n");
        }
        output_buffer->append("\n");
        
        output_display->scroll(output_buffer->length(), 0); // 滚动到底部
    }
}

// --- Helper Function to Execute Command and Capture Output ---
std::string execute_command(const std::string& command) {
    std::string result = "";
    std::vector<char> buffer(128);
    FILE* pipe = _popen(command.c_str(), "r"); // Execute command and open read pipe

    if (!pipe) {
        return "Error: Failed to execute command: " + command + "\n";
    }

    try {
        size_t bytesread;
        while ((bytesread = fread(buffer.data(), 1, buffer.size(), pipe)) != 0) {
            result += std::string(buffer.data(), bytesread);
        }
    } catch (...) {
        _pclose(pipe);
        return "Error: Exception while reading command output.\n";
    }

    int exit_code = _pclose(pipe); // Close pipe and get exit code
    result += "\n--------------------\n";
    result += "命令执行结束，退出码: " + std::to_string(exit_code) + "\n";

    return result;
}

// --- 输出提示信息 ---
void append_output(const std::string& text) {
    if (output_buffer && output_display) {
        output_buffer->append(text.c_str());
        output_display->scroll(output_buffer->length(), 0); // Scroll to end
    }
}

// --- Callback Functions ---

// Generic Browse Folder Callback
void browse_folder_cb(Fl_Widget* w, void* data) {
    Fl_Input* input_field = (Fl_Input*)data;
    Fl_Native_File_Chooser fnfc;
    fnfc.title("选择文件夹");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    if (fnfc.show() == 0) {
        input_field->value(fnfc.filename());
    }
}

// 选择输入文件
void browse_file_input_cb(Fl_Widget* w, void* data) {
    Fl_Input* input_field = (Fl_Input*)data;
    Fl_Native_File_Chooser fnfc;
    fnfc.title("选择输入文件");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fnfc.filter("DDP Files\t*.ddp\nAll Files\t*.*"); // Adjust filter if needed
    if (fnfc.show() == 0) {
        input_field->value(fnfc.filename());
    }
}

// Generic Browse File Callback (for output DDP)
// 选择输出文件
void browse_file_output_cb(Fl_Widget* w, void* data) {
    Fl_Input* input_field = (Fl_Input*)data;
    Fl_Native_File_Chooser fnfc;
    fnfc.title("选择输出文件");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fnfc.filter("DDP Files\t*.ddp\nAll Files\t*.*"); // Adjust filter if needed
    fnfc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM);
    if (fnfc.show() == 0) {
        std::string filename = fnfc.filename();
        // Ensure .ddp extension (optional)
        if (filename.length() < 4 || filename.substr(filename.length() - 4) != ".ddp") {
             filename += ".ddp";
        }
        input_field->value(filename.c_str());
    }
}


// Execute Callbacks (Wrap execution logic)
void run_tool(const char* exe_path, const char* input_arg, const char* output_arg) {
     if (strlen(input_arg) == 0 || strlen(output_arg) == 0) {
        append_output("Error: 请选择输入和输出路径.\n");
        append_output("Error: Please select both input and output paths.\n");
        return;
    }
    std::string command = "\"" + std::string(exe_path) + "\" \"" + std::string(input_arg) + "\" \"" + std::string(output_arg) + "\"";
    append_output("Executing: " + command + "\n");
    std::string result = execute_command(command);
    append_output(result);
}

void exec_ddp2_pack_cb(Fl_Widget* w, void* data) {
    Fl_Input** inputs = (Fl_Input**)data; // 0: input folder, 1: output file
    run_tool(DDP2_PACK_EXE, inputs[0]->value(), inputs[1]->value());
}

void exec_ddp2_unpack_cb(Fl_Widget* w, void* data) {
    Fl_Input** inputs = (Fl_Input**)data; // 0: input file, 1: output folder
    run_tool(DDP2_UNPACK_EXE, inputs[0]->value(), inputs[1]->value());
}

void exec_ddp3_pack_cb(Fl_Widget* w, void* data) {
    Fl_Input** inputs = (Fl_Input**)data; // 0: input folder, 1: output file
    run_tool(DDP3_PACK_EXE, inputs[0]->value(), inputs[1]->value());
}

void exec_ddp3_unpack_cb(Fl_Widget* w, void* data) {
    Fl_Input** inputs = (Fl_Input**)data; // 0: input file, 1: output folder
    run_tool(DDP3_UNPACK_EXE, inputs[0]->value(), inputs[1]->value());
}


// --- Main Function ---
int main(int argc, char **argv) {
    Fl::scheme("gleam"); // Try the gleam scheme
    Fl::get_system_colors(); // Ensure system colors are loaded for the scheme
    Fl::background(240, 240, 245); // 设置应用整体背景色
    Fl::background2(248, 248, 255); // 设置输入控件背景色
    Fl::foreground(52, 73, 94); // 设置文本颜色

    const int padding = 10;
    const int widget_h = 25;
    const int label_w = 90;
    const int input_w = 350;
    const int browse_w = 75;
    const int button_w = 120;
    const int button_h = 30;

    const int win_w = label_w + input_w + browse_w + padding * 4; // Calculate window width
    const int win_h = 480; // Increased height for padding
    const int tabs_h = 220; // Increased height for tabs content
    const int output_y = tabs_h + padding * 2;
    const int output_h = win_h - output_y - padding;


    Fl_Window *window = new Fl_Window(win_w, win_h, "DDSystem GUI");
    window->box(FL_FLAT_BOX); // Use flat box for window background
    window->color(COLOR_BG); // 设置窗口背景色
    window->begin();

    Fl_Tabs *tabs = new Fl_Tabs(padding, padding, win_w - padding * 2, tabs_h);
    tabs->box(FL_FLAT_BOX); // Use flat box for tabs background
    tabs->color(COLOR_BG); // 标签页背景色
    tabs->selection_color(COLOR_PRIMARY); // 设置选中标签颜色
    tabs->labelcolor(COLOR_TEXT_DARK); // 标签文字颜色
    tabs->begin();

    // --- DDP2 Pack Tab ---
    { // Scope for local variables
        Fl_Group *grp_ddp2_pack = new Fl_Group(padding, padding + widget_h, win_w - padding * 2, tabs_h - widget_h, "DDP2 Pack");
        grp_ddp2_pack->box(FL_FLAT_BOX);
        grp_ddp2_pack->color(COLOR_BG);
        grp_ddp2_pack->labelcolor(COLOR_TEXT_DARK);
        grp_ddp2_pack->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
        grp_ddp2_pack->begin();
        int current_y = padding * 2 + widget_h; // Start y position inside the group
        Fl_Input* in_folder_d2p = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Input Folder:");
        in_folder_d2p->align(FL_ALIGN_LEFT);
        in_folder_d2p->color(COLOR_INPUT_BG);
        in_folder_d2p->textcolor(COLOR_TEXT_DARK);
        in_folder_d2p->labelcolor(COLOR_TEXT_DARK);
        Fl_Button* btn_browse_folder_d2p = new Fl_Button(in_folder_d2p->x() + input_w + padding, current_y, browse_w, widget_h, "浏览...");
        btn_browse_folder_d2p->box(FL_GLEAM_UP_BOX);
        btn_browse_folder_d2p->color(COLOR_SECONDARY);
        btn_browse_folder_d2p->labelcolor(COLOR_TEXT_DARK);

        current_y += widget_h + padding;
        Fl_Input* out_file_d2p = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Output File:");
        out_file_d2p->align(FL_ALIGN_LEFT);
        out_file_d2p->color(COLOR_INPUT_BG);
        out_file_d2p->textcolor(COLOR_TEXT_DARK);
        out_file_d2p->labelcolor(COLOR_TEXT_DARK);
        Fl_Button* btn_browse_file_d2p = new Fl_Button(out_file_d2p->x() + input_w + padding, current_y, browse_w, widget_h, "浏览...");
        btn_browse_file_d2p->box(FL_GLEAM_UP_BOX);
        btn_browse_file_d2p->color(COLOR_SECONDARY);
        btn_browse_file_d2p->labelcolor(COLOR_TEXT_DARK);

        current_y += widget_h + padding * 2;
        Fl_Button* btn_exec_d2p = new Fl_Button( (grp_ddp2_pack->w() - button_w) / 2, current_y, button_w, button_h, "Pack");
        btn_exec_d2p->box(FL_GLEAM_UP_BOX);
        btn_exec_d2p->color(COLOR_PRIMARY);
        btn_exec_d2p->labelcolor(COLOR_TEXT_LIGHT);
        btn_exec_d2p->labelfont(FL_BOLD);

        static Fl_Input* d2p_inputs[] = {in_folder_d2p, out_file_d2p}; // For callback
        btn_browse_folder_d2p->callback(browse_folder_cb, in_folder_d2p);
        btn_browse_file_d2p->callback(browse_file_output_cb, out_file_d2p);
        btn_exec_d2p->callback(exec_ddp2_pack_cb, d2p_inputs);
        grp_ddp2_pack->end();
    }

    // --- DDP2 Unpack Tab ---
    {
        Fl_Group *grp_ddp2_unpack = new Fl_Group(padding, padding + widget_h, win_w - padding * 2, tabs_h - widget_h, "DDP2 Unpack");
        grp_ddp2_unpack->box(FL_FLAT_BOX);
        grp_ddp2_unpack->color(COLOR_BG);
        grp_ddp2_unpack->labelcolor(COLOR_TEXT_DARK);
        grp_ddp2_unpack->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
        grp_ddp2_unpack->begin();
        int current_y = padding * 2 + widget_h;
        Fl_Input* in_file_d2u = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Input File:");
        in_file_d2u->align(FL_ALIGN_LEFT);
        in_file_d2u->color(COLOR_INPUT_BG);
        in_file_d2u->textcolor(COLOR_TEXT_DARK);
        in_file_d2u->labelcolor(COLOR_TEXT_DARK);
        Fl_Button* btn_browse_file_d2u = new Fl_Button(in_file_d2u->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_file_d2u->box(FL_GLEAM_UP_BOX);
        btn_browse_file_d2u->color(COLOR_SECONDARY);
        btn_browse_file_d2u->labelcolor(COLOR_TEXT_LIGHT);

        current_y += widget_h + padding;
        Fl_Input* out_folder_d2u = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Output Folder:");
        out_folder_d2u->align(FL_ALIGN_LEFT);
        out_folder_d2u->color(COLOR_INPUT_BG);
        out_folder_d2u->textcolor(COLOR_TEXT_DARK);
        out_folder_d2u->labelcolor(COLOR_TEXT_DARK);
        Fl_Button* btn_browse_folder_d2u = new Fl_Button(out_folder_d2u->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_folder_d2u->box(FL_GLEAM_UP_BOX);
        btn_browse_folder_d2u->color(COLOR_SECONDARY);
        btn_browse_folder_d2u->labelcolor(COLOR_TEXT_LIGHT);

        current_y += widget_h + padding * 2;
        Fl_Button* btn_exec_d2u = new Fl_Button( (grp_ddp2_unpack->w() - button_w) / 2, current_y, button_w, button_h, "Unpack");
        btn_exec_d2u->box(FL_GLEAM_UP_BOX);
        btn_exec_d2u->color(COLOR_PRIMARY);
        btn_exec_d2u->labelcolor(COLOR_TEXT_LIGHT);
        btn_exec_d2u->labelfont(FL_BOLD);

        static Fl_Input* d2u_inputs[] = {in_file_d2u, out_folder_d2u}; // For callback
        btn_browse_file_d2u->callback(browse_file_input_cb, in_file_d2u);
        btn_browse_folder_d2u->callback(browse_folder_cb, out_folder_d2u);
        btn_exec_d2u->callback(exec_ddp2_unpack_cb, d2u_inputs);
        grp_ddp2_unpack->end();
    }

    // --- DDP3 Pack Tab ---
    {
        Fl_Group *grp_ddp3_pack = new Fl_Group(padding, padding + widget_h, win_w - padding * 2, tabs_h - widget_h, "DDP3 Pack (wchar)");
        grp_ddp3_pack->box(FL_FLAT_BOX);
        grp_ddp3_pack->color(COLOR_BG);
        grp_ddp3_pack->labelcolor(COLOR_TEXT_DARK);
        grp_ddp3_pack->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
        grp_ddp3_pack->begin();
        int current_y = padding * 2 + widget_h;
        Fl_Input* in_folder_d3p = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Input Folder:");
        in_folder_d3p->align(FL_ALIGN_LEFT);
        in_folder_d3p->color(COLOR_INPUT_BG);
        in_folder_d3p->textcolor(COLOR_TEXT_DARK);
        in_folder_d3p->labelcolor(COLOR_TEXT_DARK);
        Fl_Button* btn_browse_folder_d3p = new Fl_Button(in_folder_d3p->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_folder_d3p->box(FL_GLEAM_UP_BOX);
        btn_browse_folder_d3p->color(COLOR_SECONDARY);
        btn_browse_folder_d3p->labelcolor(COLOR_TEXT_LIGHT);

        current_y += widget_h + padding;
        Fl_Input* out_file_d3p = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Output File:");
        out_file_d3p->align(FL_ALIGN_LEFT);
        out_file_d3p->color(COLOR_INPUT_BG);
        out_file_d3p->textcolor(COLOR_TEXT_DARK);
        out_file_d3p->labelcolor(COLOR_TEXT_DARK);
        Fl_Button* btn_browse_file_d3p = new Fl_Button(out_file_d3p->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_file_d3p->box(FL_GLEAM_UP_BOX);
        btn_browse_file_d3p->color(COLOR_SECONDARY);
        btn_browse_file_d3p->labelcolor(COLOR_TEXT_LIGHT);

        current_y += widget_h + padding * 2;
        Fl_Button* btn_exec_d3p = new Fl_Button( (grp_ddp3_pack->w() - button_w) / 2, current_y, button_w, button_h, "Pack");
        btn_exec_d3p->box(FL_GLEAM_UP_BOX);
        btn_exec_d3p->color(COLOR_PRIMARY);
        btn_exec_d3p->labelcolor(COLOR_TEXT_LIGHT);
        btn_exec_d3p->labelfont(FL_BOLD);

        static Fl_Input* d3p_inputs[] = {in_folder_d3p, out_file_d3p}; // For callback
        btn_browse_folder_d3p->callback(browse_folder_cb, in_folder_d3p);
        btn_browse_file_d3p->callback(browse_file_output_cb, out_file_d3p);
        btn_exec_d3p->callback(exec_ddp3_pack_cb, d3p_inputs);
        grp_ddp3_pack->end();
    }

    // --- DDP3 Unpack Tab ---
    {
        Fl_Group *grp_ddp3_unpack = new Fl_Group(padding, padding + widget_h, win_w - padding * 2, tabs_h - widget_h, "DDP3 Unpack (wchar)");
        grp_ddp3_unpack->box(FL_FLAT_BOX);
        grp_ddp3_unpack->color(COLOR_BG);
        grp_ddp3_unpack->labelcolor(COLOR_TEXT_DARK);
        grp_ddp3_unpack->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
        grp_ddp3_unpack->begin();
        int current_y = padding * 2 + widget_h;
        Fl_Input* in_file_d3u = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Input File:");
        in_file_d3u->align(FL_ALIGN_LEFT);
        in_file_d3u->color(COLOR_INPUT_BG);
        in_file_d3u->textcolor(COLOR_TEXT_DARK);
        in_file_d3u->labelcolor(COLOR_TEXT_DARK);
        Fl_Button* btn_browse_file_d3u = new Fl_Button(in_file_d3u->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_file_d3u->box(FL_GLEAM_UP_BOX);
        btn_browse_file_d3u->color(COLOR_SECONDARY);
        btn_browse_file_d3u->labelcolor(COLOR_TEXT_LIGHT);

        current_y += widget_h + padding;
        Fl_Input* out_folder_d3u = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Output Folder:");
        out_folder_d3u->align(FL_ALIGN_LEFT);
        out_folder_d3u->color(COLOR_INPUT_BG);
        out_folder_d3u->textcolor(COLOR_TEXT_DARK);
        out_folder_d3u->labelcolor(COLOR_TEXT_DARK);
        Fl_Button* btn_browse_folder_d3u = new Fl_Button(out_folder_d3u->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_folder_d3u->box(FL_GLEAM_UP_BOX);
        btn_browse_folder_d3u->color(COLOR_SECONDARY);
        btn_browse_folder_d3u->labelcolor(COLOR_TEXT_LIGHT);

        current_y += widget_h + padding * 2;
        Fl_Button* btn_exec_d3u = new Fl_Button( (grp_ddp3_unpack->w() - button_w) / 2, current_y, button_w, button_h, "Unpack");
        btn_exec_d3u->box(FL_GLEAM_UP_BOX);
        btn_exec_d3u->color(COLOR_PRIMARY);
        btn_exec_d3u->labelcolor(COLOR_TEXT_LIGHT);
        btn_exec_d3u->labelfont(FL_BOLD);

        static Fl_Input* d3u_inputs[] = {in_file_d3u, out_folder_d3u}; // For callback
        btn_browse_file_d3u->callback(browse_file_input_cb, in_file_d3u);
        btn_browse_folder_d3u->callback(browse_folder_cb, out_folder_d3u);
        btn_exec_d3u->callback(exec_ddp3_unpack_cb, d3u_inputs);
        grp_ddp3_unpack->end();
    }

    tabs->end();

    // --- Output Area ---
    output_buffer = new Fl_Text_Buffer();
    output_display = new Fl_Text_Display(padding, output_y, win_w - padding * 2, output_h, "Output");
    output_display->buffer(output_buffer);
    output_display->box(FL_DOWN_FRAME); // Add a frame around output
    output_display->color(COLOR_INPUT_BG); // 输出区背景色
    output_display->textfont(FL_COURIER); // Use a monospaced font
    output_display->textcolor(COLOR_TEXT_DARK); // 输出文本颜色
    output_display->textsize(12);
    output_display->labelcolor(COLOR_TEXT_DARK);
    output_display->align(FL_ALIGN_TOP | FL_ALIGN_LEFT); // Align label to top-left

    window->end();
    window->resizable(output_display); // Allow resizing, affecting the output area
    window->show(argc, argv);
    
    // 程序启动后检查可执行文件
    check_executables();

    return Fl::run();
}