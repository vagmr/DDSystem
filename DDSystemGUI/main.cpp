#define _CRT_SECURE_NO_WARNINGS // Disable warnings for unsafe functions like strcpy

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

// --- Configuration ---
const char* DDP2_PACK_EXE = "lib\\DDP2_pack.exe";
const char* DDP2_UNPACK_EXE = "lib\\DDP2_unpack.exe";
const char* DDP3_PACK_EXE = "lib\\DDP3_pack_wchar.exe";
const char* DDP3_UNPACK_EXE = "lib\\DDP3_unpack_wchar.exe";

// --- Global Widgets ---
Fl_Text_Display* output_display = nullptr;
Fl_Text_Buffer* output_buffer = nullptr;

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
    result += "Command exited with code: " + std::to_string(exit_code) + "\n";

    return result;
}

// --- Helper to Append Output to Display ---
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
    fnfc.title("Select Folder");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    if (fnfc.show() == 0) {
        input_field->value(fnfc.filename());
    }
}

// Generic Browse File Callback (for input DDP)
void browse_file_input_cb(Fl_Widget* w, void* data) {
    Fl_Input* input_field = (Fl_Input*)data;
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Select Input File");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fnfc.filter("DDP Files\t*.ddp\nAll Files\t*.*"); // Adjust filter if needed
    if (fnfc.show() == 0) {
        input_field->value(fnfc.filename());
    }
}

// Generic Browse File Callback (for output DDP)
void browse_file_output_cb(Fl_Widget* w, void* data) {
    Fl_Input* input_field = (Fl_Input*)data;
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Select Output File");
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
    window->begin();

    Fl_Tabs *tabs = new Fl_Tabs(padding, padding, win_w - padding * 2, tabs_h);
    tabs->box(FL_FLAT_BOX); // Use flat box for tabs background
    tabs->selection_color(FL_WHITE); // Color for selected tab
    tabs->begin();

    // --- DDP2 Pack Tab ---
    { // Scope for local variables
        Fl_Group *grp_ddp2_pack = new Fl_Group(padding, padding + widget_h, win_w - padding * 2, tabs_h - widget_h, "DDP2 Pack");
        grp_ddp2_pack->box(FL_FLAT_BOX);
        grp_ddp2_pack->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
        grp_ddp2_pack->begin();
        int current_y = padding * 2 + widget_h; // Start y position inside the group
        Fl_Input* in_folder_d2p = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Input Folder:");
        in_folder_d2p->align(FL_ALIGN_LEFT);
        Fl_Button* btn_browse_folder_d2p = new Fl_Button(in_folder_d2p->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_folder_d2p->box(FL_GLEAM_UP_BOX);

        current_y += widget_h + padding;
        Fl_Input* out_file_d2p = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Output File:");
        out_file_d2p->align(FL_ALIGN_LEFT);
        Fl_Button* btn_browse_file_d2p = new Fl_Button(out_file_d2p->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_file_d2p->box(FL_GLEAM_UP_BOX);

        current_y += widget_h + padding * 2;
        Fl_Button* btn_exec_d2p = new Fl_Button( (grp_ddp2_pack->w() - button_w) / 2, current_y, button_w, button_h, "Pack");
        btn_exec_d2p->box(FL_GLEAM_UP_BOX);

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
        grp_ddp2_unpack->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
        grp_ddp2_unpack->begin();
        int current_y = padding * 2 + widget_h;
        Fl_Input* in_file_d2u = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Input File:");
        in_file_d2u->align(FL_ALIGN_LEFT);
        Fl_Button* btn_browse_file_d2u = new Fl_Button(in_file_d2u->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_file_d2u->box(FL_GLEAM_UP_BOX);

        current_y += widget_h + padding;
        Fl_Input* out_folder_d2u = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Output Folder:");
        out_folder_d2u->align(FL_ALIGN_LEFT);
        Fl_Button* btn_browse_folder_d2u = new Fl_Button(out_folder_d2u->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_folder_d2u->box(FL_GLEAM_UP_BOX);

        current_y += widget_h + padding * 2;
        Fl_Button* btn_exec_d2u = new Fl_Button( (grp_ddp2_unpack->w() - button_w) / 2, current_y, button_w, button_h, "Unpack");
        btn_exec_d2u->box(FL_GLEAM_UP_BOX);

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
        grp_ddp3_pack->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
        grp_ddp3_pack->begin();
        int current_y = padding * 2 + widget_h;
        Fl_Input* in_folder_d3p = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Input Folder:");
        in_folder_d3p->align(FL_ALIGN_LEFT);
        Fl_Button* btn_browse_folder_d3p = new Fl_Button(in_folder_d3p->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_folder_d3p->box(FL_GLEAM_UP_BOX);

        current_y += widget_h + padding;
        Fl_Input* out_file_d3p = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Output File:");
        out_file_d3p->align(FL_ALIGN_LEFT);
        Fl_Button* btn_browse_file_d3p = new Fl_Button(out_file_d3p->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_file_d3p->box(FL_GLEAM_UP_BOX);

        current_y += widget_h + padding * 2;
        Fl_Button* btn_exec_d3p = new Fl_Button( (grp_ddp3_pack->w() - button_w) / 2, current_y, button_w, button_h, "Pack");
        btn_exec_d3p->box(FL_GLEAM_UP_BOX);

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
        grp_ddp3_unpack->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
        grp_ddp3_unpack->begin();
        int current_y = padding * 2 + widget_h;
        Fl_Input* in_file_d3u = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Input File:");
        in_file_d3u->align(FL_ALIGN_LEFT);
        Fl_Button* btn_browse_file_d3u = new Fl_Button(in_file_d3u->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_file_d3u->box(FL_GLEAM_UP_BOX);

        current_y += widget_h + padding;
        Fl_Input* out_folder_d3u = new Fl_Input(padding + label_w, current_y, input_w, widget_h, "Output Folder:");
        out_folder_d3u->align(FL_ALIGN_LEFT);
        Fl_Button* btn_browse_folder_d3u = new Fl_Button(out_folder_d3u->x() + input_w + padding, current_y, browse_w, widget_h, "Browse...");
        btn_browse_folder_d3u->box(FL_GLEAM_UP_BOX);

        current_y += widget_h + padding * 2;
        Fl_Button* btn_exec_d3u = new Fl_Button( (grp_ddp3_unpack->w() - button_w) / 2, current_y, button_w, button_h, "Unpack");
        btn_exec_d3u->box(FL_GLEAM_UP_BOX);

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
    output_display->textfont(FL_COURIER); // Use a monospaced font
    output_display->textsize(12);
    output_display->align(FL_ALIGN_TOP | FL_ALIGN_LEFT); // Align label to top-left

    window->end();
    window->resizable(output_display); // Allow resizing, affecting the output area
    window->show(argc, argv);

    return Fl::run();
}