//The code below is a combination of code written by a guest Pastebin user, and the source code for the Space-Mushroom project by 'Shiura' on thingiverse and instructables. Thank you for creating this project!! Amazing work!
//Also note, this is my blind attempt at splicing the two bits of code together until they work. It is definitely not perfect, or tidy, but it works for me at the moment. I will look at tidying up the code and possibly streamlining the operation at a later date.
 
// This code makes your Arduino act as a 3DConnexion SpaceMouse (32u4-based board required).
// To make this work you also need to set the USB Vendor ID and Product ID to values matching a real 3DConnexion device.
// You can do this by editing appropriate entries in the boards.txt file in your Arduino installation.
// Example values: vid=0x256f, pid=0xc631 (SpaceMouse Pro Wireless (cabled))
// Then install the 3DxWare software from 3DConnexion on your computer and you can use you Arduino device in software like solidworks, siemens NX, Fusion 360 as it it were a real SpaceMouse.
// Last update-V005_04/12/2024 - @muratozcan
#include "HID.h"

// Buton numaralarını SpaceMouse'e özgü düzenle
#define BUTTON_COUNT 15

/// SpacemouseProWireless Buton eşlemeri
enum SpaceMouseButtons {
    SM_MENU     = 0,  // Menu
    SM_FIT      = 1,  // Fit
    SM_T        = 2,  // Top
    SM_R        = 4,  // Right
    SM_F        = 5,  // Front
    SM_RCW      = 8,  // Roll 90°CW
    SM_1        = 12, // 1
    SM_2        = 13, // 2
    SM_3        = 14, // 3
    SM_4        = 15, // 4
    SM_ESC      = 22, // ESC
    SM_ALT      = 23, // ALT
    SM_SHFT     = 24, // SHIFT
    SM_CTRL     = 25, // CTRL
    SM_ROT      = 26  // Rotate
};


// 3DConnexion SpaceMouse HID tanımlayıcısı
static const uint8_t _hidReportDescriptor[] PROGMEM = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x08,       // Usage (Multi-Axis)
    0xA1, 0x01,       // Collection (Application)

    // Report 1: Translation
    0xa1, 0x00,       // Collection (Physical)
    0x85, 0x01,       // Report ID (1)
    0x16, 0x00, 0x80, // Logical Minimum (-32768)
    0x26, 0xFF, 0x7F, // Logical Maximum (32767)
    0x36, 0x00, 0x80, // Physical Minimum (-32768)
    0x46, 0xFF, 0x7F, // Physical Maximum (32767)
    0x09, 0x30,       // Usage (X)
    0x09, 0x31,       // Usage (Y)
    0x09, 0x32,       // Usage (Z)
    0x75, 0x10,       // Report Size (16)
    0x95, 0x03,       // Report Count (3)
    0x81, 0x02,       // Input (Data,Var,Abs)
    0xC0,             // End Collection

    // Report 2: Rotation
    0xa1, 0x00,       // Collection (Physical)
    0x85, 0x02,       // Report ID (2)
    0x16, 0x00, 0x80, // Logical Minimum (-32768)
    0x26, 0xFF, 0x7F, // Logical Maximum (32767)
    0x36, 0x00, 0x80, // Physical Minimum (-32768)
    0x46, 0xFF, 0x7F, // Physical Maximum (32767)
    0x09, 0x33,       // Usage (RX)
    0x09, 0x34,       // Usage (RY)
    0x09, 0x35,       // Usage (RZ)
    0x75, 0x10,       // Report Size (16)
    0x95, 0x03,       // Report Count (3)
    0x81, 0x02,       // Input (Data,Var,Abs)
    0xC0,             // End Collection

    // Report 3: Buttons (32 button)
    0xa1, 0x00,       // Collection (Physical)
    0x85, 0x03,       // Report ID (3)
    0x15, 0x00,       // Logical Minimum (0)
    0x25, 0x01,       // Logical Maximum (1)
    0x75, 0x01,       // Report Size (1)
    0x95, 0x20,       // Report Count (32)
    0x05, 0x09,       // Usage Page (Button)
    0x19, 0x01,       // Usage Minimum (Button 1)
    0x29, 0x20,       // Usage Maximum (Button 32)
    0x81, 0x02,       // Input (Data,Var,Abs)
    0xC0,             // End Collection

    0xC0              // End Application Collection
};

// Yön ters çevirme ayarları
bool invX = false;
bool invY = true;
bool invZ = true;
bool invRX = true;
bool invRY = true;
bool invRZ = true;

#define BUFFER_SIZE 128
char serialBuffer[BUFFER_SIZE];
byte bufferIndex = 0;

uint32_t lastButtonsState = 0;

uint32_t mapButtonToSpaceMouse(uint8_t buttonNumber, uint8_t state) {
    uint32_t buttons = 0;

    if (state == 0) return 0;

    switch(buttonNumber) {
        case 0:  break; // Kullanılmayan buton
       // case 0:  buttons |= (1UL << SM_1); break;
        case 1:  buttons |= (1UL << SM_1); break; //1
        case 2:  buttons |= (1UL << SM_2); break; //2
        case 3:  buttons |= (1UL << SM_3); break; //3
        case 4:  buttons |= (1UL << SM_4); break; //4
        case 5:  buttons |= (1UL << SM_ESC); break; //5
       // case 5:  buttons |= (1UL << SM_4); break;
      //  case 6:  buttons |= (1UL << SM_T); break;
        case 6:  buttons |= (1UL << SM_SHFT); break; //6
        //case 7:  buttons |= (1UL << SM_R); break;
        case 7:  buttons |= (1UL << SM_T); break;  //T
        case 8:  buttons |= (1UL << SM_RCW); break;  //L
        case 9:  buttons |= (1UL << SM_R); break;  //R
       // case 0:  buttons |= (1UL << SM_MENU); break;
       // case 1:  buttons |= (1UL << SM_FIT); break;
       // case 2:  buttons |= (1UL << SM_T); break;
       // case 4:  buttons |= (1UL << SM_R); break;
        case 10: buttons |= (1UL << SM_F); break;  //F
        //case 11: buttons |= (1UL << SM_ESC); break;
        //case 11: buttons |= (1UL << SM_ESC); break;
      //  case 12: buttons |= (1UL << SM_ALT); break;
        //case 13: buttons |= (1UL << SM_SHFT); break;
        //case 13: buttons |= (1UL << SM_SHFT); break;
       // case 14: buttons |= (1UL << SM_CTRL); break;
        case 15: buttons |= (1UL << SM_FIT); break;    //Fit    
        case 16: buttons |= (1UL << SM_MENU); break;  //Config
        case 17: buttons |= (1UL << SM_CTRL); break;  //+
        case 18: buttons |= (1UL << SM_ALT); break;   //-
        case 19: buttons |= (1UL << SM_ALT); break;  //Doom
       // case 17: break; // Kullanılmayan buton
        case 20: buttons |= (1UL << SM_ROT); break;  //3dLock
        case 21: buttons |= (1UL << SM_MENU); break;  //Panel
        default: break;
    }

    return buttons;
}

void setup() {
    static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
    HID().AppendDescriptor(&node);

    Serial.begin(115200);
    memset(serialBuffer, 0, BUFFER_SIZE);
    delay(1000);
}

void send_command(int16_t rx, int16_t ry, int16_t rz, int16_t x, int16_t y, int16_t z, uint32_t buttonMask = 0) {
    if (invX) x = -x;
    if (invY) y = -y;
    if (invZ) z = -z;
    if (invRX) rx = -rx;
    if (invRY) ry = -ry;
    if (invRZ) rz = -rz;

    int16_t trans_data[3] = {x, y, z};
    HID().SendReport(1, trans_data, sizeof(trans_data));

    int16_t rot_data[3] = {rx, ry, rz};
    HID().SendReport(2, rot_data, sizeof(rot_data));

    HID().SendReport(3, &buttonMask, sizeof(buttonMask));
}

void loop() {
    while (Serial.available() > 0) {
        char incomingChar = Serial.read();

        if (incomingChar != '\n') {
            if (bufferIndex < BUFFER_SIZE - 1) {
                serialBuffer[bufferIndex++] = incomingChar;
            }
        } else {
            serialBuffer[bufferIndex] = '\0';
            processReceivedData(serialBuffer);
            bufferIndex = 0;
        }
    }
}

void processReceivedData(char* data) {
    char* token = strtok(data, ",");
    if (token == NULL) return;
    int16_t rotX = atoi(token);

    token = strtok(NULL, ","); if (token == NULL) return;
    int16_t rotY = atoi(token);

    token = strtok(NULL, ","); if (token == NULL) return;
    int16_t rotZ = atoi(token);

    token = strtok(NULL, ","); if (token == NULL) return;
    int16_t transX = atoi(token);

    token = strtok(NULL, ","); if (token == NULL) return;
    int16_t transY = atoi(token);

    token = strtok(NULL, ","); if (token == NULL) return;
    int16_t transZ = atoi(token);

    uint32_t buttonMask = 0;
    while ((token = strtok(NULL, ",")) != NULL) {
        uint8_t key = atoi(token);
        token = strtok(NULL, ",");
        if (token == NULL) break;
        uint8_t state = atoi(token);
        buttonMask |= mapButtonToSpaceMouse(key, state);
    }

    send_command(rotX, rotY, rotZ, transX, transY, transZ, buttonMask);
}
