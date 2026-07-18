# Valkyrie's Pager

Valkyrie's Pager is firmware for an ESP32-C3 device with an ST7789 display. It connects to Wi-Fi, retrieves chat content from a remote HTTP API, and presents the content through a three-button interface.

## Hardware

- Board: ESP32-C3-DevKitM-1
- Display: ST7789, 320 x 240 pixels
- Input: enter/back, down, and up buttons
- Battery measurement: analogue input on GPIO 1

Pin assignments are defined in [include/AppConfig.h](include/AppConfig.h).

## Requirements

- PlatformIO Core
- An ESP32-C3-DevKitM-1 board
- The display and buttons wired according to `AppConfig.h`
- Wi-Fi access

## Build

Run the following command from the repository root:

```sh
platformio run
```

To upload firmware through the configured built-in debugger:

```sh
platformio run --target upload
```

The active PlatformIO environment is `esp32-c3-devkitm-1`.

## Configuration

The API host is configured once in `API_BASE_URL` in `include/AppConfig.h`.

At startup, the firmware loads the application username and password from ESP32 Preferences. Hold the enter/back button in the chat menu for three seconds to open Settings menu and then configuration portal. Connect to the `ESP32_SETUP` network and open `192.168.4.1` to configure Wi-Fi and application credentials.

## Controls

| Screen | Enter/back | Down | Up |
| --- | --- | --- | --- |
| Chat menu | Open selected chat | Next chat | Previous chat |
| Chat menu, hold for 3 seconds | Open settings | — | — |
| Chat view | Return to menu | Next page | Previous page |
| Settings | Select option | Next option | Previous option |

## Notes

- TLS certificate validation is currently disabled through `WiFiClientSecure::setInsecure()`.
- The project stores configuration in ESP32 non-volatile Preferences under the `creds` namespace.
