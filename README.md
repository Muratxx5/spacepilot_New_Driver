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

