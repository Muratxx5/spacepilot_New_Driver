# spacepilot_New_Driver
An alternative way to use the 3Dconnexion SpacePilot device with the new version of 3DxWare software and drivers.

Setup Steps:
- Install the latest version of the 3DxWare software on your computer. You can download it from the following link: 3DxWare 10 - Official 3Dconnexion UK website
- First, you’ll need an Arduino Pro Micro device based on the 32u4 chip. A low-cost clone will work fine. This device only needs to be plugged into the computer—no further action is required.
- In the board settings file of the Arduino Pro Micro, set the PID and VID values to match those of the SpaceMouse Pro Wireless (cabled) device. Then upload the program to the Arduino.
- VID = 0x256f
- PID = 0xc631
(SpaceMouse Pro Wireless (cabled))
- Connect the SpacePilot device to your computer. It won’t be recognized by the 3DxWare software, but its lights will turn on.
- Next, move the 3dxWareConnectApp folder to your C drive and run the 3dxWareConnect_V005.exe application. The app is currently in Turkish and not fully completed. I’ll finish it when I have time, but it works in its current state.
- Once the Arduino is connected, you’ll see a notification in the system tray. If no Arduino board is found, the software will notify you that the device is missing.
- After the connection is established, you’ll see that the SpaceMouse Pro Wireless (cabled) device is listed instead of the SpacePilot in the 3DxWare software. The system is now ready—you can use the SpacePilot device.
- The Esc, Shift, Ctrl, and Alt keys are set as default keyboard keys. Beyond that, you can assign all the buttons on the SpacePilot to match those of the SpaceMouse Pro Wireless in the 3DxWare software.
- I’ll prepare a visual guide later showing which SpacePilot button corresponds to which SpaceMouse Pro Wireless button. For now, this setup should suffice.
If you run into any issues, feel free to reach out.
Good luck!

simple tutorial:

Install "3DxWare64_v10-9-8_b715.exe"

Install "arduino-1.8.19-windows.exe"

Edit "C:\Program Files (x86)\Arduino\hardware\arduino\avr\boards.txt"

Change VID -> 0x256f & PID -> 0xc631 in the following section:

micro.build.mcu=atmega32u4
micro.build.f_cpu=16000000L
micro.build.vid=0x2341
micro.build.pid=0x8037
micro.build.usb_product="Arduino Micro"
micro.build.board=AVR_MICRO
micro.build.core=arduino
micro.build.variant=micro
micro.build.extra_flags={build.usb_flags}

Arduino IDE

Tools -> Board -> Arduino Micro
Tools -> Port -> Select Arduino COM-Port
File -> Open... -> "fromspacepilottocomputer_V005.ino"
Upload

Copy "3dxWareConnectApp" folder to "C:"

Connect legacy 3DConnexion Device

Run "C:\3dxWareConnectApp\3dxWareConnect_V005.exe"

<img src="spacePilot_to_SpaceMouse Pro Wireless-2.jpg" alt="Key Mappings for spacepilot" width="400"/> <img src="spacePilot_to_SpaceMouse Pro Wireless.jpg" alt="Key Mappings for spacepilot" width="400"/>
