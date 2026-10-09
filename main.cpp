#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <sstream>
#include <memory>
#include <array>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

// =========================================================
// Cyberpunk / Pro Matrix Color & Style Palette
// =========================================================
namespace Color {
    const std::string RESET        = "\033[0m";
    const std::string BOLD         = "\033[1m";
    const std::string DIM          = "\033[2m";
    const std::string ITALIC       = "\033[3m";
    const std::string UNDERLINE    = "\033[4m";
    
    // Foreground
    const std::string RED          = "\033[91m";
    const std::string GREEN        = "\033[92m";
    const std::string YELLOW       = "\033[93m";
    const std::string BLUE         = "\033[94m";
    const std::string MAGENTA      = "\033[95m";
    const std::string CYAN         = "\033[96m";
    const std::string WHITE        = "\033[97m";
    const std::string DARK_GRAY    = "\033[90m";
    
    // Backgrounds
    const std::string BG_RED       = "\033[41m";
    const std::string BG_GREEN     = "\033[42m";
    const std::string BG_DARK      = "\033[40m";
}

struct CheckResult {
    std::string moduleName;
    bool passed;
    std::string details;
};

// =========================================================
// Console Setup & Core Helper Functions
// =========================================================
void setupConsole() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
    SetConsoleOutputCP(65001); // UTF-8 Encoding
#endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printBanner() {
    std::cout << Color::CYAN << Color::BOLD << R"(
  ██████╗██╗  ██╗███████╗██╗     ██████╗██╗██╗███████╗██████╗███╗   ███╗██████╗ 
 ██╔════╝██║  ██║██╔════╝██║    ██╔════╝██║██║██╔════╝██╔════╝████╗ ████║██╔══██╗
 ██║     ███████║█████╗  ██║    ██║     █████║█████╗  ██║     ██╔████╔██║██║  ██║
 ██║     ██╔══██║██╔══╝  ██║    ██║     ██╔═██╗██╔══╝  ██║     ██║╚██╔╝██║██║  ██║
 ╚██████╗██║  ██║███████╗███████╗██████╗██║  ██║███████╗╚██████╗██║ ╚═╝ ██║██████╔╝
  ╚═════╝╚═╝  ╚═╝╚══════╝╚══════╝╚═════╝╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝╚═════╝ 
)" << Color::RESET;
    std::cout << Color::DARK_GRAY << " ═════════════════════════════════════════════════════════════════════════════════" << Color::RESET << std::endl;
    std::cout << Color::WHITE << Color::BOLD << "   [ ChelCheckCMD v1.0 ] " << Color::MAGENTA << "― Professional Anti-Cheat & Forensic Suite" << Color::RESET << std::endl;
    std::cout << Color::DARK_GRAY << " ═════════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n\n";
}

// Pro Animated Loading Bar
void loadingAnimation(const std::string& label, int durationMs = 600) {
    const char spinner[] = {'⠋', '⠙', '⠹', '⠸', '⠼', '⠴', '⠦', '⠧', '⠇', '⠏'};
    int steps = 20;
    int sleepTime = durationMs / steps;
    
    for (int i = 0; i <= steps; ++i) {
        int percent = (i * 100) / steps;
        int barWidth = 24;
        int filled = (i * barWidth) / steps;

        std::cout << "\r  " << Color::CYAN << spinner[i % 10] << Color::WHITE << Color::BOLD << " Analyzing " 
                  << Color::YELLOW << std::left << std::setw(38) << label 
                  << Color::DARK_GRAY << " [" << Color::GREEN;
        
        for (int j = 0; j < barWidth; ++j) {
            if (j < filled) std::cout << "█";
            else std::cout << "░";
        }
        std::cout << Color::DARK_GRAY << "] " << Color::CYAN << std::right << std::setw(3) << percent << "%" << Color::RESET << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    }
    std::cout << "\r  " << Color::GREEN << "✔" << Color::WHITE << Color::BOLD << " Completed " 
              << Color::YELLOW << std::left << std::setw(38) << label 
              << Color::DARK_GRAY << " [" << Color::GREEN << "████████████████████████" << Color::DARK_GRAY << "] " 
              << Color::GREEN << "100%" << Color::RESET << std::endl;
}

// Cross-Platform Execution Engine (Fixed MSVC popen/pclose Issue)
std::string execPowerShell(const std::string& cmd) {
    std::array<char, 256> buffer;
    std::string result;
    std::string fullCmd = "powershell -NoProfile -ExecutionPolicy Bypass -Command \"" + cmd + "\"";
    
#ifdef _WIN32
    FILE* pipe = _popen(fullCmd.c_str(), "r");
    if (!pipe) return "ERROR";
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result += buffer.data();
    }
    _pclose(pipe);
#else
    FILE* pipe = popen(fullCmd.c_str(), "r");
    if (!pipe) return "ERROR";
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
#endif

    return result;
}

// =========================================================
// Forensic & Security Scan Modules
// =========================================================

// 1. ตรวจสอบไฟล์ใน Temp / AppData
CheckResult checkSuspiciousFiles() {
    loadingAnimation("Suspicious Files & Temp Directory");
    std::string cmd = "Get-ChildItem -Path $env:TEMP, $env:LOCALAPPDATA, $env:APPDATA -Include *.exe,*.dll,*.bat,*.sys -Recurse -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -gt (Get-Date).AddDays(-3) } | Select-Object -First 5 -ExpandProperty Name";
    std::string res = execPowerShell(cmd);
    
    bool suspicious = (res.find(".exe") != std::string::npos || res.find(".dll") != std::string::npos || res.find(".sys") != std::string::npos);
    return { 
        "Suspicious Active Files", 
        !suspicious, 
        suspicious ? "Found executable/driver files created recently in Temp/AppData:\n" + res : "No suspicious executables or drivers detected in Temp/AppData." 
    };
}

// 2. ตรวจสอบ Process / Injected Tools
CheckResult checkMaliciousProcesses() {
    loadingAnimation("Running Processes Integrity");
    std::string cmd = "Get-Process | Where-Object { $_.ProcessName -match 'cheat|hack|injector|cheatengine|xenos|processhacker|dnspy|scylla|cheat' } | Select-Object -ExpandProperty ProcessName";
    std::string res = execPowerShell(cmd);
    
    bool found = !res.empty() && res.find_first_not_of(" \t\n\r") != std::string::npos;
    return { 
        "Malicious Processes Scan", 
        !found, 
        found ? "ALERT! Blacklisted tool/process detected running:\n" + res : "All active system processes clean." 
    };
}

// 3. ตรวจสอบประวัติถังขยะ (Recycle Bin)
CheckResult checkDeletedFilesHistory() {
    loadingAnimation("Deleted Files (Recycle Bin)");
    std::string cmd = "(New-Object -ComObject Shell.Application).NameSpace(0xa).Items() | Select-Object -First 5 -ExpandProperty Name";
    std::string res = execPowerShell(cmd);
    
    bool found = !res.empty() && res.find_first_not_of(" \t\n\r") != std::string::npos;
    return { 
        "Recycle Bin Artifacts", 
        true, 
        found ? "Recent deleted items found in Recycle Bin:\n" + res : "Recycle Bin is currently clean/empty." 
    };
}

// 4. ตรวจสอบประวัติ PowerShell
CheckResult checkPowerShellHistory() {
    loadingAnimation("PowerShell History Logs");
    std::string cmd = "Get-Content (Get-PSReadLineOption).HistorySavePath -ErrorAction SilentlyContinue | Select-Object -Last 5";
    std::string res = execPowerShell(cmd);
    
    bool deletedHistory = res.empty() || res.find("ERROR") != std::string::npos;
    return { 
        "PowerShell History Logs", 
        !deletedHistory, 
        deletedHistory ? "WARNING: PowerShell history log file missing, empty, or recently cleared!" : "Recent PowerShell Execution History:\n" + res 
    };
}

// 5. ตรวจสอบ Prefetch History
CheckResult checkExecutedHistory() {
    loadingAnimation("System Execution Cache (Prefetch)");
    std::string cmd = "Get-ChildItem -Path C:\\Windows\\Prefetch -Filter *.pf -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 5 -ExpandProperty Name";
    std::string res = execPowerShell(cmd);
    
    bool valid = !res.empty() && res.find_first_not_of(" \t\n\r") != std::string::npos;
    return { 
        "Prefetch Execution Log", 
        valid, 
        valid ? "Recently launched applications (Prefetch):\n" + res : "Prefetch log is empty or access denied." 
    };
}

// =========================================================
// UI Card Render Engine
// =========================================================
void renderResultCard(const CheckResult& res) {
    std::cout << "\n  " << Color::DARK_GRAY << "┌──────────────────────────────────────────────────────────────────────────┐" << Color::RESET << std::endl;
    std::cout << "  │ " << Color::BOLD << Color::WHITE << std::left << std::setw(45) << res.moduleName;
    if (res.passed) {
        std::cout << Color::BG_GREEN << Color::WHITE << Color::BOLD << "  ✔ PASSED  " << Color::RESET << Color::DARK_GRAY << " │" << Color::RESET << std::endl;
    } else {
        std::cout << Color::BG_RED << Color::WHITE << Color::BOLD << "  ✘ DETECTED " << Color::RESET << Color::DARK_GRAY << " │" << Color::RESET << std::endl;
    }
    std::cout << "  " << Color::DARK_GRAY << "├──────────────────────────────────────────────────────────────────────────┤" << Color::RESET << std::endl;
    
    std::stringstream ss(res.details);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty()) {
            if (line.size() > 70) line = line.substr(0, 67) + "...";
            std::cout << "  │ " << Color::DIM << Color::CYAN << "  › " << Color::WHITE << std::left << std::setw(65) << line << Color::RESET << Color::DARK_GRAY << " │" << Color::RESET << std::endl;
        }
    }
    std::cout << "  " << Color::DARK_GRAY << "└──────────────────────────────────────────────────────────────────────────┘" << Color::RESET << std::endl;
}

void runAllChecks() {
    clearScreen();
    printBanner();
    std::cout << Color::CYAN << Color::BOLD << "  [=== INITIATING FULL SYSTEM AUTO SCAN ===]\n\n" << Color::RESET;
    
    std::vector<CheckResult> results;
    results.push_back(checkSuspiciousFiles());
    results.push_back(checkMaliciousProcesses());
    results.push_back(checkDeletedFilesHistory());
    results.push_back(checkPowerShellHistory());
    results.push_back(checkExecutedHistory());

    std::cout << "\n\n  " << Color::MAGENTA << Color::BOLD << "=========================== [ SCAN REPORT SUMMARY ] ===========================" << Color::RESET << std::endl;
    for (const auto& res : results) {
        renderResultCard(res);
    }
    
    std::cout << "\n\n  " << Color::YELLOW << Color::BOLD << "  [!] Press Enter to return to main menu..." << Color::RESET;
    std::cin.ignore();
    std::cin.get();
}

// =========================================================
// Main Interactive Menu Loop
// =========================================================
void showMenu() {
    while (true) {
        clearScreen();
        printBanner();
        
        std::cout << Color::WHITE << Color::BOLD << "  [ SELECT SCAN MODULE ]\n" << Color::RESET << std::endl;
        std::cout << Color::CYAN << "   [1] " << Color::WHITE << "Check Suspicious Files & Paths " << Color::DARK_GRAY << "(Temp / AppData / Executables)" << std::endl;
        std::cout << Color::CYAN << "   [2] " << Color::WHITE << "Check Running Processes " << Color::DARK_GRAY << "(Injectors / Mod Tools / Hacks)" << std::endl;
        std::cout << Color::CYAN << "   [3] " << Color::WHITE << "Check Deleted Files History " << Color::DARK_GRAY << "(Recycle Bin Artifacts)" << std::endl;
        std::cout << Color::CYAN << "   [4] " << Color::WHITE << "Check PowerShell Command Logs " << Color::DARK_GRAY << "(History & Log Wiping)" << std::endl;
        std::cout << Color::CYAN << "   [5] " << Color::WHITE << "Check Program Execution History " << Color::DARK_GRAY << "(Prefetch Logs)" << std::endl;
        std::cout << Color::GREEN << Color::BOLD << "   [6] " << Color::WHITE << Color::BOLD << "RUN FULL AUTOMATED SCAN " << Color::GREEN << "(Recommended)" << Color::RESET << std::endl;
        std::cout << Color::RED << "   [0] " << Color::WHITE << "Exit Program" << std::endl;
        
        std::cout << "\n  " << Color::YELLOW << Color::BOLD << "ChelCheckCMD " << Color::CYAN << "❯ " << Color::RESET;
        
        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        if (choice == 0) break;
        
        clearScreen();
        printBanner();
        
        switch (choice) {
            case 1: renderResultCard(checkSuspiciousFiles()); break;
            case 2: renderResultCard(checkMaliciousProcesses()); break;
            case 3: renderResultCard(checkDeletedFilesHistory()); break;
            case 4: renderResultCard(checkPowerShellHistory()); break;
            case 5: renderResultCard(checkExecutedHistory()); break;
            case 6: runAllChecks(); continue;
            default: continue;
        }

        std::cout << "\n\n  " << Color::YELLOW << Color::BOLD << "  [!] Press Enter to return to main menu..." << Color::RESET;
        std::cin.ignore();
        std::cin.get();
    }
}

int main() {
    setupConsole();
    showMenu();
    return 0;
}
