#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <string.h>
#include <psapi.h>
#include "spnavdev.h"
#include "lcd_controller.h"
#include "arduino_controller.h"

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1001
#define ID_TRAY_EXIT 1002
#define ID_BALLOON_TIMER 2001
#define ID_APPNAME_TIMER 2002

struct spndev* dev = NULL;
int last_button_pressed = -1;
int quit = 0;

wchar_t* GetActiveAppName() {
    static wchar_t appName[MAX_PATH] = L"3dxwareAraBaðlantý";
    HWND hwndForeground = GetForegroundWindow();
    if (hwndForeground) {
        DWORD processId;
        GetWindowThreadProcessId(hwndForeground, &processId);

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
        if (hProcess) {
            wchar_t fileName[MAX_PATH];
            if (GetProcessImageFileNameW(hProcess, fileName, MAX_PATH) > 0) {
                wchar_t* baseName = wcsrchr(fileName, L'\\');
                if (baseName) {
                    swprintf(appName, MAX_PATH, L"%s - 3dxwareAraBaðlantý", baseName + 1);
                }
            }
            CloseHandle(hProcess);
        }
    }
    return appName;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Çýkýþ");
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
        else if (lParam == WM_LBUTTONUP) {
            MessageBoxW(hwnd, L"3dxWareConnect Çalýþýyor", L"Durum", MB_OK);
        }
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT) {
            quit = 1;

            NOTIFYICONDATA nid = { sizeof(nid) };
            nid.hWnd = hwnd;
            nid.uID = ID_TRAYICON;
            Shell_NotifyIcon(NIM_DELETE, &nid);

            PostQuitMessage(0);
        }
        break;

    case WM_TIMER:
        if (wParam == ID_BALLOON_TIMER) {
            KillTimer(hwnd, ID_BALLOON_TIMER);

            NOTIFYICONDATA nidBalloon = { sizeof(nidBalloon) };
            nidBalloon.hWnd = hwnd;
            nidBalloon.uID = ID_TRAYICON;
            nidBalloon.uFlags = NIF_INFO;
            wcscpy_s(nidBalloon.szInfo, ARRAYSIZE(nidBalloon.szInfo), L"Arduino ve SpaceMouse baðlantýsý kuruluyor...");
            wcscpy_s(nidBalloon.szInfoTitle, ARRAYSIZE(nidBalloon.szInfoTitle), L"3dxWareConnect Baþlatýldý");
            nidBalloon.dwInfoFlags = NIIF_INFO;
            Shell_NotifyIcon(NIM_MODIFY, &nidBalloon);
        }
        else if (wParam == ID_APPNAME_TIMER) {
            NOTIFYICONDATA nidUpdate = { sizeof(nidUpdate) };
            nidUpdate.hWnd = hwnd;
            nidUpdate.uID = ID_TRAYICON;
            nidUpdate.uFlags = NIF_TIP;
            wcscpy_s(nidUpdate.szTip, ARRAYSIZE(nidUpdate.szTip), GetActiveAppName());
            Shell_NotifyIcon(NIM_MODIFY, &nidUpdate);
        }
        break;

    case WM_DESTROY:
        quit = 1;
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

DWORD WINAPI SpaceMouseThread(LPVOID lpParam) {
    union spndev_event ev;
    HWND hwnd = (HWND)lpParam;

    while (!quit) {
        if (dev && spndev_process(dev, &ev)) {
            char buffer[128];

            if (ev.type == SPNDEV_MOTION) {
                if (last_button_pressed != -1) {
                    snprintf(buffer, sizeof(buffer),
                        "%d,%d,%d,%d,%d,%d,%d,%d\n",
                        -ev.mot.v[3], -ev.mot.v[4], -ev.mot.v[5],
                        ev.mot.v[0], -ev.mot.v[1], -ev.mot.v[2],
                        0, last_button_pressed);
                    last_button_pressed = -1;
                }
                else {
                    snprintf(buffer, sizeof(buffer),
                        "%d,%d,%d,%d,%d,%d,0,-1\n",
                        -ev.mot.v[3], -ev.mot.v[4], -ev.mot.v[5],
                        ev.mot.v[0], -ev.mot.v[1], -ev.mot.v[2]);
                }

                send_to_arduino(buffer);
            }
            else if (ev.type == SPNDEV_BUTTON) {
                // Tuþ emülasyonlarý (0-based index)
                if (ev.bn.num == 10) { // 11. tuþ - ESC
                    INPUT ip;
                    ip.type = INPUT_KEYBOARD;
                    ip.ki.wScan = 0;
                    ip.ki.time = 0;
                    ip.ki.dwExtraInfo = 0;
                    ip.ki.wVk = VK_ESCAPE;
                    ip.ki.dwFlags = ev.bn.press ? 0 : KEYEVENTF_KEYUP;
                    SendInput(1, &ip, sizeof(INPUT));
                }
                else if (ev.bn.num == 11) { // 12. tuþ - ALT
                    INPUT ip;
                    ip.type = INPUT_KEYBOARD;
                    ip.ki.wScan = 0;
                    ip.ki.time = 0;
                    ip.ki.dwExtraInfo = 0;
                    ip.ki.wVk = VK_MENU; // ALT tuþu VK_MENU olarak tanýmlý
                    ip.ki.dwFlags = ev.bn.press ? 0 : KEYEVENTF_KEYUP;
                    SendInput(1, &ip, sizeof(INPUT));
                }
                else if (ev.bn.num == 12) { // 13. tuþ - SHIFT
                    INPUT ip;
                    ip.type = INPUT_KEYBOARD;
                    ip.ki.wScan = 0;
                    ip.ki.time = 0;
                    ip.ki.dwExtraInfo = 0;
                    ip.ki.wVk = VK_SHIFT;
                    ip.ki.dwFlags = ev.bn.press ? 0 : KEYEVENTF_KEYUP;
                    SendInput(1, &ip, sizeof(INPUT));
                }
                else if (ev.bn.num == 13) { // 14. tuþ - CTRL
                    INPUT ip;
                    ip.type = INPUT_KEYBOARD;
                    ip.ki.wScan = 0;
                    ip.ki.time = 0;
                    ip.ki.dwExtraInfo = 0;
                    ip.ki.wVk = VK_CONTROL;
                    ip.ki.dwFlags = ev.bn.press ? 0 : KEYEVENTF_KEYUP;
                    SendInput(1, &ip, sizeof(INPUT));
                }

                if (ev.bn.press) {
                    last_button_pressed = ev.bn.num + 1;
                    spndev_set_led(dev, 1);
                    char status_msg[32];
                    snprintf(status_msg, sizeof(status_msg), "Buton %d basildi", ev.bn.num + 1);
                    update_cell_content(0, 0, status_msg);
                }

                snprintf(buffer, sizeof(buffer),
                    "0,0,0,0,0,0,%d,%d\n",
                    ev.bn.num + 1, ev.bn.press ? 1 : 0);
                send_to_arduino(buffer);
            }
        }
    }
    return 0;
}

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"3dxWareConnectApp";

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"3dxWareConnect",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    // Ýlk kontrol: SpaceMouse baðlý mý?
    dev = spndev_open(NULL);
    if (!dev) {
        MessageBoxW(NULL, L"SpaceMouse/SpacePilot cihazý bulunamadý. Lütfen cihazý baðlayýn ve tekrar deneyin.", L"Baðlantý Hatasý", MB_ICONERROR | MB_OK);
        return 1;
    }

    // Tepsi ikonu oluþtur
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.hWnd = hwnd;
    nid.uID = ID_TRAYICON;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = (HICON)LoadImageW(NULL, L"C:\\Program Files\\3Dconnexion\\3DxWare\\3DxWinCore\\Cfg\\Images\\3DxService\\Straumann_Nova.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    wcscpy_s(nid.szTip, ARRAYSIZE(nid.szTip), GetActiveAppName());
  //  wcscpy_s(nid.szTip, ARRAYSIZE(nid.szTip) / sizeof(wchar_t), L"3dxWareConnect");

    Shell_NotifyIcon(NIM_ADD, &nid);

    // Timer'larý baþlat
    SetTimer(hwnd, ID_BALLOON_TIMER, 1000, NULL);
    SetTimer(hwnd, ID_APPNAME_TIMER, 500, NULL);

    // Init LCD
    if (!init_lcd()) {
        show_custom_message("Hata!", "LCD Baþlatýlamadý", NULL);
        Sleep(3000);
        return 1;
    }

    // Init Arduino
    if (init_arduino() != ARDUINO_CONNECTED) {
        show_custom_message("Hata!", "Arduino baðlantýsý kurulamadý", NULL);
        MessageBoxW(NULL, L"Arduino baðlantýsý kurulamadý. Lütfen cihazý baðlayýn ve tekrar deneyin.", L"Baðlantý Hatasý", MB_ICONERROR | MB_OK);
        Sleep(3000);
        return 1;
    }



    // Ýsteðe baðlý: Sadece dikdörtgen çizmek için
   // draw_rectangle(0, 0, 240, 64, 2);  // (x, y, width, height, thickness)

    // Baþlangýç mesajý
    char com_info[32];
    snprintf(com_info, sizeof(com_info), "Baglanti: %s", get_arduino_port());

    show_custom_message(
        "Merhaba Murat!",
        "Iyi calismalar...",
        com_info
    );
    Sleep(2000);
    

    //---TAblo
// Baþlangýç içerikleri
    const char* initial_contents[3][2] = {
        {"1. Modeling", "2. Shaded"},
        {"3. Undo", "4. Redo"},
        {"5. Hide", "6. Show"}
    };

    // Tabloyu tüm içerikle birlikte çiz
    draw_full_table("SISTEM HAZIR", initial_contents);
    //---TAblo

    // Thread baþlat
    HANDLE hThread = CreateThread(NULL, 0, SpaceMouseThread, hwnd, 0, NULL);
    if (!hThread) {
        show_custom_message("Hata!", "Thread baþlatýlamadý", NULL);
        if (dev) spndev_close(dev);
        close_arduino();
        return 1;
    }

    // Mesaj döngüsü
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Temizlik
    quit = 1;
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    if (dev) spndev_close(dev);
    close_arduino();
    Shell_NotifyIcon(NIM_DELETE, &nid);

    return 0;
}