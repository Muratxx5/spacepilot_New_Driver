#include "arduino_controller.h"
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "setupapi.lib")

static HANDLE hSerial = INVALID_HANDLE_VALUE;
static char current_port[32] = { 0 };
static ArduinoStatus status = ARDUINO_DISCONNECTED;

static int find_arduino_port(char* out_port, size_t out_len) {
    GUID guid = GUID_DEVCLASS_PORTS;
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE) return 0;

    SP_DEVINFO_DATA devInfoData = { 0 };
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    char hardwareID[256];

    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); i++) {
        if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &devInfoData, SPDRP_HARDWAREID,
            NULL, (BYTE*)hardwareID, sizeof(hardwareID), NULL)) {

            if ((strstr(hardwareID, "VID_256F&PID_C631") != NULL) ||
                (strstr(hardwareID, "VID_256F&PID_C633") != NULL)) {

                HKEY hKey = SetupDiOpenDevRegKey(hDevInfo, &devInfoData,
                    DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);

                if (hKey != INVALID_HANDLE_VALUE) {
                    char portName[128];
                    DWORD size = sizeof(portName);
                    DWORD type = 0;

                    if (RegQueryValueExA(hKey, "PortName", NULL, &type,
                        (LPBYTE)portName, &size) == ERROR_SUCCESS) {
                        snprintf(out_port, out_len, "%s", portName);
                        RegCloseKey(hKey);
                        SetupDiDestroyDeviceInfoList(hDevInfo);
                        return 1;
                    }
                    RegCloseKey(hKey);
                }
            }
        }
    }
    SetupDiDestroyDeviceInfoList(hDevInfo);
    return 0;
}

ArduinoStatus init_arduino() {
    if (status == ARDUINO_CONNECTED) return status;

    if (!find_arduino_port(current_port, sizeof(current_port))) {
        status = ARDUINO_ERROR;
        return status;
    }

    char full_port_name[32];
    snprintf(full_port_name, sizeof(full_port_name), "\\\\.\\%s", current_port);

    hSerial = CreateFileA(full_port_name, GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        status = ARDUINO_ERROR;
        return status;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        status = ARDUINO_ERROR;
        return status;
    }

    dcbSerialParams.BaudRate = 115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        status = ARDUINO_ERROR;
        return status;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    SetCommTimeouts(hSerial, &timeouts);
    status = ARDUINO_CONNECTED;
    return status;
}

void send_to_arduino(const char* message) {
    if (get_arduino_status() != ARDUINO_CONNECTED) return;

    DWORD bytesWritten;
    DWORD msg_len = (DWORD)strlen(message);

    // Buffer'ý temizle
    PurgeComm(hSerial, PURGE_TXCLEAR);

    // Veriyi gönder
    if (!WriteFile(hSerial, message, msg_len, &bytesWritten, NULL)) {
        status = ARDUINO_ERROR;
        return;
    }

    // Gönderimi tamamla
    FlushFileBuffers(hSerial);
}

void close_arduino() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
    }
    status = ARDUINO_DISCONNECTED;
}

const char* get_arduino_port() {
    return current_port;
}

ArduinoStatus get_arduino_status() {
    // Portun hala açýk olduðunu kontrol et
    if (hSerial == INVALID_HANDLE_VALUE) {
        status = ARDUINO_DISCONNECTED;
    }
    return status;
}