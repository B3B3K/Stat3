#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>

#pragma comment(lib, "pdh.lib")

// Configuration
const int BAUD_RATE = 115200;
const int UPDATE_INTERVAL_MS = 100;

class SystemMonitor {
private:
    HANDLE hSerial;
    PDH_HQUERY cpuQuery;
    PDH_HCOUNTER cpuCounter;
    PDH_HQUERY ramQuery;
    PDH_HCOUNTER ramCounter;
    PDH_HQUERY diskReadQuery, diskWriteQuery;
    PDH_HCOUNTER diskReadCounter, diskWriteCounter;
    PDH_HQUERY netQuery;
    PDH_HCOUNTER netSentCounter, netRecvCounter;
    
    double lastDiskRead = 0, lastDiskWrite = 0;
    double lastNetSent = 0, lastNetRecv = 0;
    
public:
    SystemMonitor() : hSerial(INVALID_HANDLE_VALUE) {
        initPDH();
    }
    
    ~SystemMonitor() {
        if (hSerial != INVALID_HANDLE_VALUE) {
            CloseHandle(hSerial);
        }
        PdhCloseQuery(cpuQuery);
        PdhCloseQuery(ramQuery);
        PdhCloseQuery(diskReadQuery);
        PdhCloseQuery(diskWriteQuery);
        PdhCloseQuery(netQuery);
    }
    
    void initPDH() {
        // CPU - Using ANSI strings and explicit 0 for DWORD_PTR
        PdhOpenQueryA(NULL, 0, &cpuQuery);
        PdhAddEnglishCounterA(cpuQuery, "\\Processor(_Total)\\% Processor Time", 0, &cpuCounter);
        PdhCollectQueryData(cpuQuery);
        
        // RAM
        PdhOpenQueryA(NULL, 0, &ramQuery);
        PdhAddEnglishCounterA(ramQuery, "\\Memory\\% Committed Bytes In Use", 0, &ramCounter);
        PdhCollectQueryData(ramQuery);
        
        // Disk I/O
        PdhOpenQueryA(NULL, 0, &diskReadQuery);
        PdhAddEnglishCounterA(diskReadQuery, "\\PhysicalDisk(_Total)\\Disk Read Bytes/sec", 0, &diskReadCounter);
        PdhCollectQueryData(diskReadQuery);
        
        PdhOpenQueryA(NULL, 0, &diskWriteQuery);
        PdhAddEnglishCounterA(diskWriteQuery, "\\PhysicalDisk(_Total)\\Disk Write Bytes/sec", 0, &diskWriteCounter);
        PdhCollectQueryData(diskWriteQuery);
        
        // Network I/O - Using wildcard to get all network interfaces
        PdhOpenQueryA(NULL, 0, &netQuery);
        PdhAddEnglishCounterA(netQuery, "\\Network Interface(*)\\Bytes Sent/sec", 0, &netSentCounter);
        PdhAddEnglishCounterA(netQuery, "\\Network Interface(*)\\Bytes Received/sec", 0, &netRecvCounter);
        PdhCollectQueryData(netQuery);
    }
    
    std::vector<std::string> listSerialPorts() {
        std::vector<std::string> ports;
        char portName[20];
        
        for (int i = 1; i <= 256; i++) {
            sprintf(portName, "\\\\.\\COM%d", i);
            HANDLE hPort = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 
                                      0, NULL, OPEN_EXISTING, 0, NULL);
            
            if (hPort != INVALID_HANDLE_VALUE) {
                CloseHandle(hPort);
                sprintf(portName, "COM%d", i);
                ports.push_back(portName);
            }
        }
        return ports;
    }
    
    bool connectSerial() {
        auto ports = listSerialPorts();
        
        if (ports.empty()) {
            std::cout << "No serial ports found!" << std::endl;
            return false;
        }
        
        std::cout << "\nAvailable serial ports:" << std::endl;
        for (size_t i = 0; i < ports.size(); i++) {
            std::cout << (i + 1) << ". " << ports[i] << std::endl;
        }
        
        int choice;
        std::cout << "\nSelect port (1-" << ports.size() << "): ";
        if(!(std::cin >> choice)) return false;
        
        if (choice < 1 || choice > (int)ports.size()) {
            return false;
        }
        
        std::string portPath = "\\\\.\\" + ports[choice - 1];
        
        hSerial = CreateFileA(portPath.c_str(), 
                             GENERIC_READ | GENERIC_WRITE,
                             0, NULL, OPEN_EXISTING, 
                             FILE_ATTRIBUTE_NORMAL, NULL);
        
        if (hSerial == INVALID_HANDLE_VALUE) {
            std::cout << "Failed to open port" << std::endl;
            return false;
        }
        
        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            CloseHandle(hSerial);
            return false;
        }
        
        dcbSerialParams.BaudRate = BAUD_RATE;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
        
        if (!SetCommState(hSerial, &dcbSerialParams)) {
            CloseHandle(hSerial);
            return false;
        }
        
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;
        
        if (!SetCommTimeouts(hSerial, &timeouts)) {
            CloseHandle(hSerial);
            return false;
        }
        
        std::cout << "Connected to " << ports[choice - 1] << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        return true;
    }
    
    float getCPUTemp() {
        return 50.0f;
    }
    
    void getGPUInfo(float& temp, float& vramUsage) {
        temp = 50.0f;
        vramUsage = 0.0f;
        
        FILE* pipe = _popen("nvidia-smi --query-gpu=temperature.gpu,memory.used,memory.total --format=csv,noheader,nounits", "r");
        if (!pipe) return;
        
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            float gpuTemp, memUsed, memTotal;
            if (sscanf(buffer, "%f, %f, %f", &gpuTemp, &memUsed, &memTotal) == 3) {
                temp = gpuTemp;
                if (memTotal > 0) {
                    vramUsage = (memUsed / memTotal) * 100.0f;
                }
            }
        }
        _pclose(pipe);
    }
    
    uint16_t tempToColor(float temp, float minTemp = 30.0f, float maxTemp = 90.0f) {
        temp = std::max(minTemp, std::min(maxTemp, temp));
        float ratio = (temp - minTemp) / (maxTemp - minTemp);
        
        int r, g, b;
        if (ratio < 0.33f) {
            r = 0; g = (int)(255 * (ratio / 0.33f)); b = (int)(255 * (1 - ratio / 0.33f));
        } else if (ratio < 0.66f) {
            r = (int)(255 * ((ratio - 0.33f) / 0.33f)); g = 255; b = 0;
        } else {
            r = 255; g = (int)(255 * (1 - (ratio - 0.66f) / 0.34f)); b = 0;
        }
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
    
    struct SystemData {
        float cpuUsage;
        float cpuTemp;
        uint16_t cpuColor;
        float ramUsage;
        float gpuUsage;
        float gpuTemp;
        uint16_t gpuColor;
        float vramUsage;
        uint16_t vramColor;
        float diskRead;
        float diskWrite;
        float netUp;
        float netDown;
    };
    
    SystemData getSystemData() {
        SystemData data;
        PDH_FMT_COUNTERVALUE counterVal;
        
        // CPU
        PdhCollectQueryData(cpuQuery);
        PdhGetFormattedCounterValue(cpuCounter, PDH_FMT_DOUBLE, NULL, &counterVal);
        data.cpuUsage = static_cast<float>(counterVal.doubleValue);
        data.cpuTemp = getCPUTemp();
        data.cpuColor = tempToColor(data.cpuTemp);
        
        // RAM
        PdhCollectQueryData(ramQuery);
        PdhGetFormattedCounterValue(ramCounter, PDH_FMT_DOUBLE, NULL, &counterVal);
        data.ramUsage = static_cast<float>(counterVal.doubleValue);
        
        // GPU
        getGPUInfo(data.gpuTemp, data.vramUsage);
        data.gpuColor = tempToColor(data.gpuTemp);
        data.vramColor = data.gpuColor;
        data.gpuUsage = data.vramUsage; 
        
        // Disk
        PdhCollectQueryData(diskReadQuery);
        PdhGetFormattedCounterValue(diskReadCounter, PDH_FMT_DOUBLE, NULL, &counterVal);
        data.diskRead = static_cast<float>(counterVal.doubleValue / (1024.0 * 1024.0));
        
        PdhCollectQueryData(diskWriteQuery);
        PdhGetFormattedCounterValue(diskWriteCounter, PDH_FMT_DOUBLE, NULL, &counterVal);
        data.diskWrite = static_cast<float>(counterVal.doubleValue / (1024.0 * 1024.0));
        
        // Network - FIXED: Actually collect the network data
        PdhCollectQueryData(netQuery);
        PdhGetFormattedCounterValue(netSentCounter, PDH_FMT_DOUBLE, NULL, &counterVal);
        data.netUp = static_cast<float>(counterVal.doubleValue / (1024.0 * 1024.0));
        
        PdhGetFormattedCounterValue(netRecvCounter, PDH_FMT_DOUBLE, NULL, &counterVal);
        data.netDown = static_cast<float>(counterVal.doubleValue / (1024.0 * 1024.0));
        
        return data;
    }
    
    bool sendData(const SystemData& data) {
        if (hSerial == INVALID_HANDLE_VALUE) return false;
        
        uint8_t packet[22];
        int idx = 0;
        packet[idx++] = 0xFF; packet[idx++] = 0xFF; // Start Marker
        
        packet[idx++] = (uint8_t)std::min(99.0f, data.cpuUsage);
        packet[idx++] = data.cpuColor & 0xFF;
        packet[idx++] = (data.cpuColor >> 8) & 0xFF;
        
        packet[idx++] = (uint8_t)std::min(99.0f, data.ramUsage);
        
        packet[idx++] = (uint8_t)std::min(99.0f, data.gpuUsage);
        packet[idx++] = data.gpuColor & 0xFF;
        packet[idx++] = (data.gpuColor >> 8) & 0xFF;
        
        packet[idx++] = (uint8_t)std::min(99.0f, data.vramUsage);
        packet[idx++] = data.vramColor & 0xFF;
        packet[idx++] = (data.vramColor >> 8) & 0xFF;
        
        packet[idx++] = (uint8_t)std::min(99.0f, data.diskRead);
        packet[idx++] = (uint8_t)std::min(99.0f, data.diskWrite);
        
        packet[idx++] = (uint8_t)std::min(99.0f, data.netUp);
        packet[idx++] = (uint8_t)std::min(99.0f, data.netDown);
        
        DWORD bytesWritten;
        return WriteFile(hSerial, packet, idx, &bytesWritten, NULL) && bytesWritten == (DWORD)idx;
    }
    
    void printData(const SystemData& data) {
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "\rCPU: " << std::setw(5) << data.cpuUsage << "% (" 
                  << std::setw(4) << data.cpuTemp << "C) | "
                  << "RAM: " << std::setw(5) << data.ramUsage << "% | "
                  << "GPU: " << std::setw(5) << data.gpuUsage << "% (" 
                  << std::setw(4) << data.gpuTemp << "C) | "
                  << "VRAM: " << std::setw(5) << data.vramUsage << "% | "
                  << std::setprecision(2)
                  << "Disk: R:" << std::setw(6) << data.diskRead 
                  << " W:" << std::setw(6) << data.diskWrite << " MB/s | "
                  << "Net: U:" << std::setw(6) << data.netUp 
                  << " D:" << std::setw(6) << data.netDown << " MB/s  " << std::flush;
    }
    
    void run() {
        std::cout << "================================================================================\n";
        std::cout << "System Monitor - Starting...\n";
        std::cout << "================================================================================\n";
        
        if (!connectSerial()) {
            std::cout << "\nFailed to connect. Exiting...\n";
            return;
        }
        
        std::cout << "\nMonitoring started. Press Ctrl+C to stop.\n\n";
        
        while (true) {
            SystemData data = getSystemData();
            if (!sendData(data)) {
                std::cout << "\nConnection lost.\n";
                break;
            }
            printData(data);
            std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_INTERVAL_MS));
        }
        
        std::cout << "\n\nStopping monitor...\n";
    }
};

int main() {
    SystemMonitor monitor;
    monitor.run();
    return 0;
}
