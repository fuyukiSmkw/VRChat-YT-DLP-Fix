#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "tray.hpp"

std::filesystem::path GetSelfDirectory()
{
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
}

bool DownloadFile(const std::string& url, const std::filesystem::path& outPath)
{
    std::string command =
        "powershell -NoProfile -Command \""
        "$ProgressPreference='SilentlyContinue'; "
        "Invoke-WebRequest -Uri '" + url +
        "' -OutFile '" + outPath.string() +
        "'\"";

    std::cout << "Downloading via PowerShell..." << std::endl;

    int result = std::system(command.c_str());
    if (result != 0)
    {
        std::cerr << "PowerShell download failed. Exit code: " << result << std::endl;
        return false;
    }

    if (!std::filesystem::exists(outPath))
    {
        std::cerr << "Download reported success but file is missing: "
                  << outPath << std::endl;
        return false;
    }

    return true;
}

void UnblockFile(const std::filesystem::path& file)
{
    std::wstring cmd = L"powershell -NoProfile -Command \"Unblock-File -Path '"
        + file.wstring() + L"'\"";
    _wsystem(cmd.c_str());
    cmd = L"icacls \"" + file.wstring() + L"\" /setintegritylevel M >nul";
    _wsystem(cmd.c_str());
}


// Function to check if a process is running
bool IsProcessRunning(const std::string & processName)
{
    bool isRunning = false;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &processEntry))
        {
            do
            {
                if (processName == processEntry.szExeFile)
                {
                    isRunning = true;
                    break;
                }
            } while (Process32Next(hSnapshot, &processEntry));
        }
    }
    CloseHandle(hSnapshot);
    return isRunning;
}

bool CheckIfGameRunning()
{
    if (!IsProcessRunning("VRChat.exe"))
    {
        std::cout << "VRChat no longer running, exiting!" << std::endl;
        exit(0);
        return false;
    }
    return true;
}


bool IsFileInUse(const std::filesystem::path& filePath)
{
    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return GetLastError() == ERROR_SHARING_VIOLATION;
    }
    CloseHandle(hFile);
    return false;
}

std::string GetDefaultWebBrowser()
{
    HKEY hKey;
    const char* subKey = R"(Software\Microsoft\Windows\Shell\Associations\UrlAssociations\http\UserChoice)";
    const char* valueName = "ProgId";
    char value[256];
    DWORD valueLength = sizeof(value);
    if (RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueExA(hKey, valueName, nullptr, nullptr, (LPBYTE)value, &valueLength) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            std::string progId(value);
            if (progId.find("ChromeHTML") != std::string::npos)
                return "chrome";
            else if (progId.find("FirefoxURL") != std::string::npos)
                return "firefox";
            else if (progId.find("MSEdgeHTM") != std::string::npos)
                return "edge";
            else if (progId.find("VivaldiHTM") != std::string::npos)
                return "vivaldi";
            else if (progId.find("OperaHTML") != std::string::npos)
                return "opera";
            else if (progId.find("ChromiumHTM") != std::string::npos)
                return "chromium";
            else if (progId.find("BraveHTML") != std::string::npos)
                return "brave";
            else
                return {};
        }
        RegCloseKey(hKey);
    }
    return {};
}

bool IsSameFileQuick(const std::filesystem::path& a, const std::filesystem::path& b)
{
    if (!std::filesystem::exists(a) || !std::filesystem::exists(b))
        return false;

    if (std::filesystem::file_size(a) != std::filesystem::file_size(b))
        return false;

    std::ifstream fa(a, std::ios::binary);
    std::ifstream fb(b, std::ios::binary);
    if (!fa || !fb) return false;

    constexpr size_t BUF = 4096;
    char ba[BUF], bb[BUF];

    while (fa && fb)
    {
        fa.read(ba, BUF);
        fb.read(bb, BUF);
        if (std::memcmp(ba, bb, fa.gcount()) != 0)
            return false;
    }

    return true;
}

template<typename... Args>
void ErrorExit(Args&&... args)
{
    ShowFromOtherProcess();
    (std::cerr << ... << std::forward<Args>(args)) << std::endl << std::endl;
    system("pause");
    exit(1);
}

void GuardLoop(
    const std::filesystem::path& localYtDlp,
    const std::filesystem::path& ytDlpPath
)
{
    std::cout << "\nEntering guard loop..." << std::endl;

    while (true)
    {
        if (!CheckIfGameRunning())
            return;

        bool needReplace = false;

        if (!std::filesystem::exists(ytDlpPath))
        {
            needReplace = true;
            std::cout << "VRChat's yt-dlp doesn't exist" << std::endl;
        }
        else if (!IsSameFileQuick(localYtDlp, ytDlpPath))
        {
            needReplace = true;
            if (std::filesystem::exists(ytDlpPath)) {
                // avoid exceptions thrown (if VRChat is using the file)
                std::error_code _;
                std::filesystem::remove(ytDlpPath, _) && std::cout << "\nDeleting VRChat's custom YT-DLP from " << ytDlpPath << std::endl;
            }
        }

        if (needReplace)
        {
            std::cout << "\nWait 20s for response..." << std::endl;

            bool appeared = false;
            for (int i = 0; i < 20; i++)
            {
                if (std::filesystem::exists(ytDlpPath))
                {
                    appeared = true;
                    break;
                }
                Sleep(1000);
            }

            if (appeared)
            {
                std::cout << "Waiting for VRChat to finish writing its custom YT-DLP..." << std::endl;
                while (IsFileInUse(ytDlpPath))
                {
                    if (!CheckIfGameRunning())
                        return;
                    Sleep(100);
                }
                std::filesystem::remove(ytDlpPath);
                std::cout << "Deleting VRChat's custom YT-DLP again lol." << std::endl;
            }
            else
                std::cout << "Waited 20s without VRChat replacing YT-DLP, proceeding.\n" << std::endl;

            if (std::filesystem::exists(localYtDlp)) {
                std::filesystem::copy_file(
                    localYtDlp,
                    ytDlpPath,
                    std::filesystem::copy_options::overwrite_existing
                );

                UnblockFile(ytDlpPath);
            }
            else {
                ErrorExit("ERROR: local yt-dlp doesn't exist!");
            }

            std::cout << "yt-dlp replaced!" << std::endl;
            std::cout << "Waiting for next yt-dlp replacement..." << std::endl;
        }

        Sleep(3000); // check every 3s
    }
}

int worker()
{
    std::cout << "======\nVRChat-YT-DLP-Fix by ShizCalev, fuyukiS' fork\nhttps://github.com/fuyukiSmkw/VRChat-YT-DLP-Fix\n======\n" << std::endl;

    std::cout << "Checking if VRChat is running." << std::endl;

    int count = 0;
    while (!IsProcessRunning("VRChat.exe"))
    {
        if (count >= 30) // seconds
        {
            ErrorExit("ERROR: Exceeded maximum wait time, VRChat.exe not found.");
        }
        if (!count)
        {
            std::cout << "VRChat.exe not open. Waiting 30 seconds." << std::endl;
        }
        Sleep(1000);
        count++;
    }

    std::cout << "VRChat.exe found." << "\n" << std::endl;

    // Retrieve the AppData path (mingw)
    const char* appDataPath = std::getenv("LOCALAPPDATA");
    if (!appDataPath)
    {
        ErrorExit("ERROR: Unable to retrieve LOCALAPPDATA environment variable.");
    }

    std::string appDataPathStr(appDataPath);
    std::filesystem::path ytDlpConfig = std::filesystem::weakly_canonical(appDataPathStr + R"(\..\Roaming\yt-dlp\config)");
    if (std::filesystem::exists(ytDlpConfig))
    {
        std::ifstream file(ytDlpConfig);
        if (!file.is_open())
        {
            ErrorExit("ERROR: Error opening file: ", ytDlpConfig);
        }
        std::cout << "yt-dlp config found at " << ytDlpConfig << "\nFile Contents:" << std::endl;
        std::ostringstream buffer;
        std::string line;
        bool containsCookiesFromBrowser = false;
        bool containsSleepRequests = false;
        bool containsMinSleepInterval = false;
        bool containsMaxSleepInterval = false;
        // Read the file and check for parameters
        while (std::getline(file, line))
        {
            buffer << line << "\n";
            std::cout << line << std::endl;
            if (line.find("--cookies-from-browser") != std::string::npos)
            {
                containsCookiesFromBrowser = true;
            }
            if (line.find("--sleep-requests") != std::string::npos)
            {
                containsSleepRequests = true;
            }
            if (line.find("--min-sleep-interval") != std::string::npos)
            {
                containsMinSleepInterval = true;
            }
            if (line.find("--max-sleep-interval") != std::string::npos)
            {
                containsMaxSleepInterval = true;
            }
        }
        std::cout << std::endl;
        file.close();
        // Open the file for writing and prepend missing parameters
        std::ofstream outFile(ytDlpConfig, std::ios::trunc);
        if (!outFile.is_open())
        {
            ErrorExit("Error opening file for writing: ", ytDlpConfig);
        }
        // not enforcing --cookies-from-browser for those who want to use given local cookies with --cookies
        /*if (!containsCookiesFromBrowser)
        {
            std::cout << "--cookies-from-browser parameter not located in config. Checking system registry for default web browser ProgId." << std::endl;
            std::string defaultBrowser = GetDefaultWebBrowser();
            if (defaultBrowser.empty())
            {
                std::cerr << "ERROR: Failed to detect default web browser from registry or browser not recognized." << std::endl;
                Sleep(10000);
                return 0;
            }
            std::cout << "Browser detected as: " << defaultBrowser << std::endl;
            outFile << "--cookies-from-browser " << defaultBrowser << " ";
            std::cout << "\"--cookies-from-browser " << defaultBrowser << "\" prepended to the yt-dlp config file." << std::endl;
        }*/
        if (!containsSleepRequests)
        {
            outFile << "--sleep-requests 1.5 ";
            std::cout << "\"--sleep-requests 1.5\" prepended to the yt-dlp config file." << std::endl;
        }
        if (!containsMinSleepInterval)
        {
            outFile << "--min-sleep-interval 15 ";
            std::cout << "\"--min-sleep-interval 15\" prepended to the yt-dlp config file." << std::endl;
        }
        if (!containsMaxSleepInterval)
        {
            outFile << "--max-sleep-interval 45 ";
            std::cout << "\"--max-sleep-interval 45\" prepended to the yt-dlp config file." << std::endl;
        }
        // Write the original content after the prepended parameters
        outFile << buffer.str();
        outFile.close();
    }
    else
    {
        std::cout << "yt-dlp config file not found. Creating default config in " << ytDlpConfig.parent_path() << "\n" << std::endl;

        if (!std::filesystem::exists(ytDlpConfig.parent_path()))
            std::filesystem::create_directories(ytDlpConfig.parent_path());
        std::ofstream outFile(ytDlpConfig);
        if (!outFile.is_open())
        {
            ErrorExit("ERROR: Error creating file: ", ytDlpConfig);
        }

        std::string defaultBrowser = GetDefaultWebBrowser();
        if (defaultBrowser.empty())
        {
            std::cerr << "WARNING: Failed to detect default web browser from registry or browser not recognized.\nWARNING: Ignoring browser cookies. You're likely to fail to play videos; please edit " << ytDlpConfig <<" ." << std::endl;
        }
        else
            outFile << "--cookies-from-browser " << defaultBrowser << " ";
        outFile << "--sleep-requests 1.5 --min-sleep-interval 15 --max-sleep-interval 45";
        outFile.close();
    }

    // path to VRC's yt-dlp
    const std::filesystem::path ytDlpPath = std::filesystem::weakly_canonical(appDataPathStr + R"(\..\LocalLow\VRChat\VRChat\Tools\yt-dlp.exe)");

    // path to local yt-dlp
    const std::filesystem::path selfDir = GetSelfDirectory();
    const std::filesystem::path localYtDlp = selfDir / "yt-dlp.exe";

    if (std::filesystem::exists(localYtDlp))
    {
        std::cout << "Using local yt-dlp.exe from: " << localYtDlp << std::endl;
    }
    else
    {
        std::cout << "Local yt-dlp.exe not found. Downloading latest version..." << std::endl;

        const std::string downloadUrl =
            "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe";

        if (!DownloadFile(downloadUrl, localYtDlp))
        {
            ErrorExit("ERROR: Failed to download yt-dlp.exe!");
        }

        std::cout << "yt-dlp.exe downloaded to: " << localYtDlp << std::endl;
    }

    if (!std::filesystem::exists(localYtDlp))
    {
        ErrorExit("ERROR: yt-dlp.exe still missing after download attempt!");
    }

    GuardLoop(localYtDlp, ytDlpPath);
    return 0;
}

void workerProcess()
{
    int ret = 0;
    try {
        ret = worker();
    } catch (std::exception &e) {
        ErrorExit("ERROR: Uncaught exception: ", e.what());
    } catch (...) {
        ErrorExit("ERROR: Unknown exception.");
    }
    exit(ret);
}

int main()
{
    InitAndHide();
    std::thread(workerProcess).detach();
    TrayMessageLoop();
}
