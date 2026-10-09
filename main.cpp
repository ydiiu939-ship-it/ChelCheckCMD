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

namespace Color {
    const std::string RESET        = "\033[0m";
    const std::string BOLD         = "\033[1m";
    const std::string DIM          = "\033[2m";
    const std::string RED          = "\033[91m";
    const std::string GREEN        = "\033[92m";
    const std::string YELLOW       = "\033[93m";
    const std::string BLUE         = "\033[94m";
    const std::string MAGENTA      = "\033[95m";
    const std::string CYAN         = "\033[96m";
    const std::string WHITE        = "\033[97m";
    const std::string DARK_GRAY    = "\033[90m";
    const std::string BG_RED       = "\033[41m";
    const std::string BG_GREEN     = "\033[42m";
}

struct CheckResult {
    std::string moduleName;
    bool passed;
    std::string details;
};

void setupConsole() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
    SetConsoleOutputCP(65001);
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
    std::cout << Color::WHITE << Color::BOLD << "   [ ChelCheckCMD v2.0 ] " << Color::MAGENTA << "― Precision Anti-Cheat & Forensic Suite" << Color::RESET << std::endl;
    std::cout << Color::DARK_GRAY << " ═════════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n\n";
}

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
// Advanced Precision Scan Modules
// =========================================================

// 1. ตรวจสอบเฉพาะไฟล์ที่มีพฤติกรรมโกง (คัดแยกไฟล์เกมออก 100%)
CheckResult checkSuspiciousFiles() {
    loadingAnimation("Precision File & Cheat Signature Scan");
    
    // กรองโฟลเดอร์เกมออก (Steam, Epic, Valorant, FiveM, Garena) และเช็คเฉพาะคำต้องห้าม
    std::string cmd = R"(
        $exclude = 'Steam|Epic Games|Riot Games|Garena|Valorant|FiveM|GTA|PUBG|Apex'
        $cheatKeywords = 'cheat|injector|spoofer|bypass|aimbot|wallhack|silent|esp|cheatengine|xenos|scylla'
        Get-ChildItem -Path $env:TEMP, $env:LOCALAPPDATA, "$env:USERPROFILE\Downloads" -Include *.exe,*.dll,*.sys,*.bat,*.ps1 -Recurse -ErrorAction SilentlyContinue |
        Where-Object { 
            $_.FullName -notmatch $exclude -and 
            $_.Name -match $cheatKeywords -and
            $_.LastWriteTime -gt (Get-Date).AddDays(-7)
        } | Select-Object -First 5 -ExpandProperty FullName
    )";

    std::string res = execPowerShell(cmd);
    bool detected = !res.empty() && res.find_first_not_of(" \t\n\r") != std::string::npos;

    return { 
        "Cheat Signature & Path Scan", 
        !detected, 
        detected ? "DETECTED: Suspicious cheat binaries/injectors found:\n" + res : "No suspicious cheat signatures or unauthorized executables found." 
    };
}

// 2. ตรวจสอบ Process ทำร้ายระบบ / Injected Tools แบบเจาะจง
CheckResult checkMaliciousProcesses() {
    loadingAnimation("Targeted Process Integrity Scan");
    
    // รายชื่อ Process เครื่องมือโกงจริง (ไม่ใช่ชื่อโปรแกรมทั่วไป)
    std::string cmd = R"(
        $blackList = 'cheatengine|xenos|processhacker|dnspy|scylla|cheatengine-x86_64|extremeinjector|ksdumper|dumper|reshade-bypass'
        Get-Process | Where-Object { $_.ProcessName -match $blackList } | Select-Object -ExpandProperty ProcessName
    )";
    
    std::string res = execPowerShell(cmd);
    bool found = !res.empty() && res.find_first_not_of(" \t\n\r") != std::string::npos;

    return { 
        "Malicious Process Scan", 
        !found, 
        found ? "ALERT: High-risk cheat tool/injector currently running:\n" + res : "All running processes verified clean." 
    };
}

// 3. ตรวจสอบประวัติถังขยะแบบค้นหาเฉพาะไฟล์โกงที่เพิ่งลบไป
CheckResult checkDeletedFilesHistory() {
    loadingAnimation("Deleted Cheat Artifacts Analysis");
    
    std::string cmd = R"(
        $cheatKeywords = 'cheat|injector|spoofer|bypass|aimbot|wallhack|esp|cheatengine|xenos'
        (New-Object -ComObject Shell.Application).NameSpace(0xa).Items() | 
        Where-Object { $_.Name -match $cheatKeywords } | 
        Select-Object -First 5 -ExpandProperty Name
    )";
    
    std::string res = execPowerShell(cmd);
    bool found = !res.empty() && res.find_first_not_of(" \t\n\r") != std::string::npos;

    return { 
        "Deleted Cheat Artifacts", 
        !found, 
        found ? "DETECTED: Traces of deleted cheat files found in Recycle Bin:\n" + res : "No deleted cheat artifacts detected in Recycle Bin." 
    };
}

// 4. ตรวจสอบประวัติคำสั่ง PowerShell และคำสั่งทำลายหลักฐาน
CheckResult checkPowerShellHistory() {
    loadingAnimation("PowerShell Bypasses & Anti-Forensic Commands");
    
    std::string cmd = R"(
        $historyPath = (Get-PSReadLineOption).HistorySavePath
        if (Test-Path $historyPath) {
            $content = Get-Content $historyPath -ErrorAction SilentlyContinue
            $suspicious = $content | Where-Object { $_ -match 'Remove-Item|Clear-History|DownloadString|bypass|Unrestricted|Set-ExecutionPolicy' }
            if ($suspicious) { $suspicious | Select-Object -Last 5 } else { "CLEAN" }
        } else {
            "LOG_CLEARED"
        }
    )";
    
    std::string res = execPowerShell(cmd);
    bool isCleared = (res.find("LOG_CLEARED") != std::string::npos);
    bool hasSuspiciousCmd = (!isCleared && res.find("CLEAN") == std::string::npos && !res.empty());

    std::string details;
    if (isCleared) {
        details = "WARNING: PowerShell history log file was deliberately deleted/cleared!";
    } else if (hasSuspiciousCmd) {
        details = "DETECTED: Anti-forensic or bypass commands executed in PowerShell:\n" + res;
    } else {
        details = "PowerShell logs verified. No bypass or anti-forensic commands detected.";
    }

    return { 
        "PowerShell Anti-Forensic Check", 
        (!isCleared && !hasSuspiciousCmd), 
        details 
    };
}

// 5. ตรวจสอบ Prefetch ย้อนหลัง ค้นหาการรันโปรแกรมโกงที่เพิ่งปิดไป
CheckResult checkExecutedHistory() {
    loadingAnimation("Prefetch Artifacts (History Scan)");
    
    std::string cmd = R"(
        $cheatKeywords = 'CHEAT|INJECTOR|SPOOFER|BYPASS|AIMBOT|WALLHACK|CHEATENGINE|XENOS|PROCESSHACKER'
        Get-ChildItem -Path C:\Windows\Prefetch -Filter *.pf -ErrorAction SilentlyContinue | 
        Where-Object { $_.Name -match $cheatKeywords } | 
        Sort-Object LastWriteTime -Descending | 
        Select-Object -First 5 -ExpandProperty Name
    )";
    
    std::string res = execPowerShell(cmd);
    bool found = !res.empty() && res.find_first_not_of(" \t\n\r") != std::string::npos;

    return { 
        "Prefetch Execution History", 
        !found, 
        found ? "DETECTED: Evidence of executed cheat programs in Prefetch cache:\n" + res : "No cheat execution traces found in Prefetch history." 
    };
}

// =========================================================
// Render Engine & UI
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
    std::cout << Color::CYAN << Color::BOLD << "  [=== INITIATING FULL PRECISION ANTI-CHEAT SCAN ===]\n\n" << Color::RESET;
    
    std::vector<CheckResult> results;
    results.push_back(checkSuspiciousFiles());
    results.push_back(checkMaliciousProcesses());
    results.push_back(checkDeletedFilesHistory());
    results.push_back(checkPowerShellHistory());
    results.push_back(checkExecutedHistory());

    std::cout << "\n\n  " << Color::MAGENTA << Color::BOLD << "=========================== [ FINAL SCAN REPORT ] ===========================" << Color::RESET << std::endl;
    for (const auto& res : results) {
        renderResultCard(res);
    }
    
    std::cout << "\n\n  " << Color::YELLOW << Color::BOLD << "  [!] Press Enter to return to main menu..." << Color::RESET;
    std::cin.ignore();
    std::cin.get();
}

void showMenu() {
    while (true) {
        clearScreen();
        printBanner();
        
        std::cout << Color::WHITE << Color::BOLD << "  [ PRECISION SCAN MODULES ]\n" << Color::RESET << std::endl;
        std::cout << Color::CYAN << "   [1] " << Color::WHITE << "Scan Cheat Signatures & Paths " << Color::DARK_GRAY << "(Excludes Game Directories)" << std::endl;
        std::cout << Color::CYAN << "   [2] " << Color::WHITE << "Scan Running Cheat Processes " << Color::DARK_GRAY << "(Injectors / Cheat Tools)" << std::endl;
        std::cout << Color::CYAN << "   [3] " << Color::WHITE << "Scan Deleted Cheat Files " << Color::DARK_GRAY << "(Recycle Bin Artifacts)" << std::endl;
        std::cout << Color::CYAN << "   [4] " << Color::WHITE << "Scan PowerShell Bypasses " << Color::DARK_GRAY << "(Anti-Forensic History Checks)" << std::endl;
        std::cout << Color::CYAN << "   [5] " << Color::WHITE << "Scan Prefetch History " << Color::DARK_GRAY << "(Executed Cheat Logs)" << std::endl;
        std::cout << Color::GREEN << Color::BOLD << "   [6] " << Color::WHITE << Color::BOLD << "RUN FULL PRECISION SCAN " << Color::GREEN << "(Recommended)" << Color::RESET << std::endl;
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
