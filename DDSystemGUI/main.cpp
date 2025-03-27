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


// Execute Callbacks
void exec_ddp2_pack_cb(Fl_Widget* w, void* data) {
    Fl_Input** inputs = (Fl_Input**)data; // 0: input folder, 1: output file
    const char* input_folder = inputs[0]->value();
    const char* output_file = inputs[1]->value();

    if (strlen(input_folder) == 0 || strlen(output_file) == 0) {
        append_output("Error: Please select both input folder and output file.\n");
        return;
    }
    // Command: DDP2_pack.exe "input_folder" "output_file"
    std::string command = "\"" + std::string(DDP2_PACK_EXE) + "\" \"" + std::string(input_folder) + "\" \"" + std::string(output_file) + "\"";
    append_output("Executing: " + command + "\n");
    std::string result = execute_command(command);
    append_output(result);
}

void exec_ddp2_unpack_cb(Fl_Widget* w, void* data) {
    Fl_Input** inputs = (Fl_Input**)data; // 0: input file, 1: output folder
    const char* input_file = inputs[0]->value();
    const char* output_folder = inputs[1]->value();

    if (strlen(input_file) == 0 || strlen(output_folder) == 0) {
        append_output("Error: Please select both input file and output folder.\n");
        return;
    }
    // Command: DDP2_unpack.exe "input_file" "output_folder"
    std::string command = "\"" + std::string(DDP2_UNPACK_EXE) + "\" \"" + std::string(input_file) + "\" \"" + std::string(output_folder) + "\"";
    append_output("Executing: " + command + "\n");
    std::string result = execute_command(command);
    append_output(result);
}

void exec_ddp3_pack_cb(Fl_Widget* w, void* data) {
    Fl_Input** inputs = (Fl_Input**)data; // 0: input folder, 1: output file
    const char* input_folder = inputs[0]->value();
    const char* output_file = inputs[1]->value();

    if (strlen(input_folder) == 0 || strlen(output_file) == 0) {
        append_output("Error: Please select both input folder and output file.\n");
        return;
    }
    // Command: DDP3_pack_wchar.exe "input_folder" "output_file"
    std::string command = "\"" + std::string(DDP3_PACK_EXE) + "\" \"" + std::string(input_folder) + "\" \"" + std::string(output_file) + "\"";
    append_output("Executing: " + command + "\n");
    std::string result = execute_command(command);
    append_output(result);
}

void exec_ddp3_unpack_cb(Fl_Widget* w, void* data) {
    Fl_Input** inputs = (Fl_Input**)data; // 0: input file, 1: output folder
    const char* input_file = inputs[0]->value();
    const char* output_folder = inputs[1]->value();

    if (strlen(input_file) == 0 || strlen(output_folder) == 0) {
        append_output("Error: Please select both input file and output folder.\n");
        return;
    }
    // Command: DDP3_unpack_wchar.exe "input_file" "output_folder"
    std::string command = "\"" + std::string(DDP3_UNPACK_EXE) + "\" \"" + std::string(input_file) + "\" \"" + std::string(output_folder) + "\"";
    append_output("Executing: " + command + "\n");
    std::string result = execute_command(command);
    append_output(result);
}


// --- Main Function ---
int main(int argc, char **argv) {
    Fl::scheme("gtk+"); // Optional: Use a different theme

    Fl_Window *window = new Fl_Window(600, 450, "DDSystem GUI");
    window->begin();

    Fl_Tabs *tabs = new Fl_Tabs(10, 10, 580, 200);
    tabs->begin();

    // --- DDP2 Pack Tab ---
    Fl_Group *grp_ddp2_pack = new Fl_Group(10, 35, 580, 175, "DDP2 Pack");
    grp_ddp2_pack->begin();
    Fl_Input* in_folder_d2p = new Fl_Input(100, 50, 380, 25, "Input Folder:");
    Fl_Button* btn_browse_folder_d2p = new Fl_Button(490, 50, 80, 25, "Browse...");
    Fl_Input* out_file_d2p = new Fl_Input(100, 85, 380, 25, "Output File:");
    Fl_Button* btn_browse_file_d2p = new Fl_Button(490, 85, 80, 25, "Browse...");
    Fl_Button* btn_exec_d2p = new Fl_Button(250, 130, 100, 30, "Pack");
    static Fl_Input* d2p_inputs[] = {in_folder_d2p, out_file_d2p}; // For callback
    btn_browse_folder_d2p->callback(browse_folder_cb, in_folder_d2p);
    btn_browse_file_d2p->callback(browse_file_output_cb, out_file_d2p);
    btn_exec_d2p->callback(exec_ddp2_pack_cb, d2p_inputs);
    grp_ddp2_pack->end();

    // --- DDP2 Unpack Tab ---
    Fl_Group *grp_ddp2_unpack = new Fl_Group(10, 35, 580, 175, "DDP2 Unpack");
    grp_ddp2_unpack->begin();
    Fl_Input* in_file_d2u = new Fl_Input(100, 50, 380, 25, "Input File:");
    Fl_Button* btn_browse_file_d2u = new Fl_Button(490, 50, 80, 25, "Browse...");
    Fl_Input* out_folder_d2u = new Fl_Input(100, 85, 380, 25, "Output Folder:");
    Fl_Button* btn_browse_folder_d2u = new Fl_Button(490, 85, 80, 25, "Browse...");
    Fl_Button* btn_exec_d2u = new Fl_Button(250, 130, 100, 30, "Unpack");
    static Fl_Input* d2u_inputs[] = {in_file_d2u, out_folder_d2u}; // For callback
    btn_browse_file_d2u->callback(browse_file_input_cb, in_file_d2u);
    btn_browse_folder_d2u->callback(browse_folder_cb, out_folder_d2u);
    btn_exec_d2u->callback(exec_ddp2_unpack_cb, d2u_inputs);
    grp_ddp2_unpack->end();

    // --- DDP3 Pack Tab ---
    Fl_Group *grp_ddp3_pack = new Fl_Group(10, 35, 580, 175, "DDP3 Pack (wchar)");
    grp_ddp3_pack->begin();
    Fl_Input* in_folder_d3p = new Fl_Input(100, 50, 380, 25, "Input Folder:");
    Fl_Button* btn_browse_folder_d3p = new Fl_Button(490, 50, 80, 25, "Browse...");
    Fl_Input* out_file_d3p = new Fl_Input(100, 85, 380, 25, "Output File:");
    Fl_Button* btn_browse_file_d3p = new Fl_Button(490, 85, 80, 25, "Browse...");
    Fl_Button* btn_exec_d3p = new Fl_Button(250, 130, 100, 30, "Pack");
    static Fl_Input* d3p_inputs[] = {in_folder_d3p, out_file_d3p}; // For callback
    btn_browse_folder_d3p->callback(browse_folder_cb, in_folder_d3p);
    btn_browse_file_d3p->callback(browse_file_output_cb, out_file_d3p);
    btn_exec_d3p->callback(exec_ddp3_pack_cb, d3p_inputs);
    grp_ddp3_pack->end();

    // --- DDP3 Unpack Tab ---
    Fl_Group *grp_ddp3_unpack = new Fl_Group(10, 35, 580, 175, "DDP3 Unpack (wchar)");
    grp_ddp3_unpack->begin();
    Fl_Input* in_file_d3u = new Fl_Input(100, 50, 380, 25, "Input File:");
    Fl_Button* btn_browse_file_d3u = new Fl_Button(490, 50, 80, 25, "Browse...");
    Fl_Input* out_folder_d3u = new Fl_Input(100, 85, 380, 25, "Output Folder:");
    Fl_Button* btn_browse_folder_d3u = new Fl_Button(490, 85, 80, 25, "Browse...");
    Fl_Button* btn_exec_d3u = new Fl_Button(250, 130, 100, 30, "Unpack");
    static Fl_Input* d3u_inputs[] = {in_file_d3u, out_folder_d3u}; // For callback
    btn_browse_file_d3u->callback(browse_file_input_cb, in_file_d3u);
    btn_browse_folder_d3u->callback(browse_folder_cb, out_folder_d3u);
    btn_exec_d3u->callback(exec_ddp3_unpack_cb, d3u_inputs);
    grp_ddp3_unpack->end();

    tabs->end();

    // --- Output Area ---
    output_buffer = new Fl_Text_Buffer();
    output_display = new Fl_Text_Display(10, 220, 580, 220, "Output");
    output_display->buffer(output_buffer);
    output_display->textfont(FL_COURIER); // Use a monospaced font
    output_display->textsize(12);

    window->end();
    window->resizable(output_display); // Allow resizing, affecting the output area
    window->show(argc, argv);

    return Fl::run();
}