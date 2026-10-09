#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <array>

#ifdef _WIN32
#include <windows.h>
#else
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0400
#endif

// =========================================================
// ANSI Color & Style Codes (สำหรับ UI/UX เท่ๆ)
// =========================================================
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string DIM     = "\033[2m";
    const std::string RED     = "\033[91m";
    const std::string GREEN   = "\033[92m";
    const std::string YELLOW  = "\033[93m";
    const std::string BLUE    = "\033[94m";
    const std::string MAGENTA = "\033[95m";
    const std::string CYAN    = "\033[96m";
    const std::string WHITE   = "\033[97m";
    const std::string BG_DARK = "\033[40m";
}

// Struct สำหรับเก็บผลลัพธ์การตรวจ
struct CheckResult {
    std::string moduleName;
    bool passed;
    std::string details;
};

// =========================================================
// Helper Functions (UI & System Exec)
// =========================================================
void setupConsole() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    SetConsoleOutputCP(65001); // UTF-8
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
    std::cout << Color::MAGENTA << "             ================================================" << Color::RESET << std::endl;
    std::cout << Color::WHITE << Color::BOLD << "             [ ChelCheckCMD v1.0 ] - Ultimate Forensic Tool" << Color::RESET << std::endl;
    std::cout << Color::MAGENTA << "             ================================================" << Color::RESET << "\n\n";
}

void loadingAnimation(const std::string& label, int durationMs = 800) {
    std::cout << Color::YELLOW << "  [>] " << Color::WHITE << std::left << std::setw(38) << label << Color::CYAN << " [";
    const char spinner[] = {'|', '/', '-', '\\'};
    int steps = 15;
    int sleepTime = durationMs / steps;
    
    for (int i = 0; i <= steps; ++i) {
        int pos = (i * 20) / steps;
        std::cout << "\r" << Color::YELLOW << "  [>] " << Color::WHITE << std::left << std::setw(38) << label << Color::CYAN << " [";
        for (int j = 0; j < 20; ++j) {
            if (j < pos) std::cout << "=";
            else if (j == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << spinner[i % 4] << " " << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    }
    std::cout << "\r" << Color::YELLOW << "  [>] " << Color::WHITE << std::left << std::setw(38) << label << Color::CYAN << " [====================] " << Color::GREEN << "DONE!" << Color::RESET << std::endl;
}

std::string execPowerShell(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::string fullCmd = "powershell -NoProfile -ExecutionPolicy Bypass -Command \"" + cmd + "\"";
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCmd.c_str(), "r"), pclose);
    if (!pipe) return "ERROR";
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// =========================================================
// System Checking Modules
// =========================================================

// 1. ตรวจสอบไฟล์แปลกๆ ใน Temp / AppData / Prefetch
CheckResult checkSuspiciousFiles() {
    loadingAnimation("Checking Suspicious Files & Paths");
    std::string cmd = "Get-ChildItem -Path $env:TEMP, $env:LOCALAPPDATA, $env:APPDATA -Include *.exe,*.dll,*.bat,*.sys -Recurse -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -gt (Get-Date).AddDays(-3) } | Select-Object -First 5 | Format-Table -HideTableHeaders Name";
    std::string res = execPowerShell(cmd);
    
    bool suspicious = (res.find(".exe") != std::string::npos || res.find(".dll") != std::string::npos);
    return { "Suspicious Files Check", !suspicious, suspicious ? "Found recent executables in Temp/AppData:\n" + res : "No suspicious active files in Temp/AppData" };
}

// 2. ตรวจสอบ Process ทำร้ายระบบ / Injection Tools
CheckResult checkMaliciousProcesses() {
    loadingAnimation("Checking Running Processes");
    std::string cmd = "Get-Process | Where-Object { $_.ProcessName -match 'cheat|hack|injector|cheatengine|xenos|processhacker|dnspy|scylla' } | Select-Object -ExpandProperty ProcessName";
    std::string res = execPowerShell(cmd);
    
    bool found = !res.empty() && res.find_first_not_of(" \t\n\r") != std::string::npos;
    return { "Process Integrity Check", !found, found ? "ALERT: Suspicious Process Running: " + res : "All running processes clean" };
}

// 3. ตรวจสอบประวัติไฟล์ที่ถูกลบไปแล้ว (Recycle Bin & USN Journal)
CheckResult checkDeletedFilesHistory() {
    loadingAnimation("Checking Deleted Files History");
    std::string cmd = "(New-Object -ComObject Shell.Application).NameSpace(0xa).Items() | Select-Object -First 5 | ForEach-Object { $_.Name }";
    std::string res = execPowerShell(cmd);
    
    bool found = !res.empty() && res.find_first_not_of(" \t\n\r") != std::string::npos;
    return { "Recycle Bin History Check", true, found ? "Recent Recycle Bin items:\n" + res : "Recycle Bin empty or clean" };
}

// 4. ตรวจสอบประวัติคำสั่ง PowerShell (PowerShell History)
CheckResult checkPowerShellHistory() {
    loadingAnimation("Checking PowerShell History Log");
    std::string cmd = "Get-History; Get-Content (Get-PSReadLineOption).HistorySavePath -ErrorAction SilentlyContinue | Select-Object -Last 10";
    std::string res = execPowerShell(cmd);
    
    bool deletedHistory = res.empty() || res.find("ERROR") != std::string::npos;
    return { "PowerShell History Check", !deletedHistory, deletedHistory ? "WARNING: PowerShell history log is missing or cleared!" : "PowerShell history retrieved successfully" };
}

// 5. ตรวจสอบ Prefetch / Recent Executed Programs
CheckResult checkExecutedHistory() {
    loadingAnimation("Checking System Executed History (Prefetch)");
    std::string cmd = "Get-ChildItem -Path C:\\Windows\\Prefetch -Filter *.pf -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 5 -ExpandProperty Name";
    std::string res = execPowerShell(cmd);
    
    return { "Prefetch History Check", true, "Recently Executed (Prefetch):\n" + res };
}

// =========================================================
// Main UI & Menu System
// =========================================================
void renderResultCard(const CheckResult& res) {
    std::cout << "\n  " << Color::BOLD << Color::WHITE << "┌─────────────────────────────────────────────────────────────┐" << Color::RESET << std::endl;
    std::cout << "  │ " << Color::BOLD << std::left << std::setw(32) << res.moduleName;
    if (res.passed) {
        std::cout << Color::GREEN << Color::BOLD << " [ PASSED ] " << Color::WHITE << " │" << Color::RESET << std::endl;
    } else {
        std::cout << Color::RED << Color::BOLD << " [ DETECTED ] " << Color::WHITE << "│" << Color::RESET << std::endl;
    }
    std::cout << "  ├─────────────────────────────────────────────────────────────┤" << Color::RESET << std::endl;
    
    std::stringstream ss(res.details);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty()) {
            std::cout << "  │ " << Color::DIM << std::left << std::setw(59) << line.substr(0, 58) << Color::RESET << " │" << std::endl;
        }
    }
    std::cout << "  " << Color::BOLD << Color::WHITE << "└─────────────────────────────────────────────────────────────┘" << Color::RESET << std::endl;
}

void runAllChecks() {
    clearScreen();
    printBanner();
    std::cout << Color::CYAN << Color::BOLD << "  [=== RUNNING FULL SYSTEM CHECKS ===]\n\n" << Color::RESET;
    
    std::vector<CheckResult> results;
    results.push_back(checkSuspiciousFiles());
    results.push_back(checkMaliciousProcesses());
    results.push_back(checkDeletedFilesHistory());
    results.push_back(checkPowerShellHistory());
    results.push_back(checkExecutedHistory());

    std::cout << "\n" << Color::MAGENTA << Color::BOLD << "  ================== SUMMARY REPORT ==================" << Color::RESET << std::endl;
    for (const auto& res : results) {
        renderResultCard(res);
    }
    
    std::cout << "\n" << Color::YELLOW << "  กด Enter เพื่อกลับสู่เมนูหลัก..." << Color::RESET;
    std::cin.ignore();
    std::cin.get();
}

void showMenu() {
    while (true) {
        clearScreen();
        printBanner();
        
        std::cout << Color::WHITE << Color::BOLD << "  กรุณาเลือกรายการที่ต้องการเช็ค:\n" << Color::RESET << std::endl;
        std::cout << Color::CYAN << "   [1] " << Color::WHITE << "เช็คไฟล์แปลกๆ (Temp/AppData/Executable)" << std::endl;
        std::cout << Color::CYAN << "   [2] " << Color::WHITE << "เช็ค Process ทำร้ายระบบ / Injection" << std::endl;
        std::cout << Color::CYAN << "   [3] " << Color::WHITE << "เช็คประวัติไฟล์ที่ถูกลบ (Recycle Bin)" << std::endl;
        std::cout << Color::CYAN << "   [4] " << Color::WHITE << "เช็คประวัติคำสั่ง PowerShell / การลบ Log" << std::endl;
        std::cout << Color::CYAN << "   [5] " << Color::WHITE << "เช็คประวัติการเปิดโปรแกรมย้อนหลัง (Prefetch)" << std::endl;
        std::cout << Color::GREEN << Color::BOLD << "   [6] " << Color::WHITE << Color::BOLD << "ตรวจครบทุกระบบ (FULL AUTO SCAN)" << Color::RESET << std::endl;
        std::cout << Color::RED << "   [0] " << Color::WHITE << "ออกจากโปรแกรม" << std::endl;
        
        std::cout << "\n" << Color::YELLOW << "  ChelCheckCMD > " << Color::RESET;
        
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

        std::cout << "\n" << Color::YELLOW << "  กด Enter เพื่อกลับสู่เมนูหลัก..." << Color::RESET;
        std::cin.ignore();
        std::cin.get();
    }
}

int main() {
    setupConsole();
    showMenu();
    return 0;
}
