#include <iostream>
#include <windows.h>
#include <urlmon.h>
#include <filesystem>
#include <cstdlib>
#include <string>

#pragma comment(lib, "urlmon.lib")

namespace fs = std::filesystem;

const std::string repo_url = "https://github.com/RoeiAvneri/MinecraftDownloader/archive/refs/heads/main.zip";
const std::string zip_path = std::getenv("TEMP") + std::string("\\MinecraftDownloader.zip");
const std::string extract_folder = std::getenv("TEMP") + std::string("\\MinecraftDownloader");
const std::string dest_folder = std::getenv("APPDATA") + std::string("\\.minecraft\\mods");

// Function to delete all existing mods
void clearModsFolder() {
    if (fs::exists(dest_folder)) {
        fs::remove_all(dest_folder);
    }
    fs::create_directories(dest_folder);
}

// Function to download the ZIP file
bool downloadMods() {
    std::cout << "Downloading mods...\n";
    HRESULT hr = URLDownloadToFile(NULL, repo_url.c_str(), zip_path.c_str(), 0, NULL);
    return hr == S_OK;
}

// Function to extract the ZIP file using PowerShell
bool extractZip() {
    // Added double quotes around paths to handle spaces (like "Program Files")
    std::string command = "powershell -Command \"Expand-Archive -Path '" + zip_path + "' -DestinationPath '" + extract_folder + "' -Force\"";
    // std::cout << "Executing command: " << command << "\n";  // For debugging purposes
    return system(command.c_str()) == 0;
}

// Function to move extracted mods to the mods folder
int moveMods() {
    int mod_count = 0;
    std::string mod_source_folder = extract_folder + "\\MinecraftDownloader-main";

    for (const auto& entry : fs::directory_iterator(mod_source_folder)) {
        fs::path source = entry.path();
        fs::path destination = dest_folder + "\\" + source.filename().string();

        if (fs::is_directory(source)) {
            fs::copy(source, destination, fs::copy_options::recursive);
            for (const auto& sub_entry : fs::recursive_directory_iterator(source))
                mod_count++;
        } else {
            fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
            mod_count++;
        }
    }
    return mod_count;
}

int main() {
    clearModsFolder();

    if (!downloadMods()) {
        std::cout << "Download failed! Check your internet connection.\n";
        return 1;
    }

    if (!extractZip()) {
        std::cout << "Extraction failed!\n";
        return 1;
    }

    int mod_count = moveMods();

    // Cleanup
    fs::remove(zip_path);
    fs::remove_all(extract_folder);

    // Show message box
    std::string msg = "Mods have been successfully installed!\nTotal mods added: " + std::to_string(mod_count);
    MessageBox(NULL, msg.c_str(), "Installation Complete", MB_ICONINFORMATION | MB_OK);

    std::cout << "Download complete! Mods are in " << dest_folder << "\n";
    return 0;
}
