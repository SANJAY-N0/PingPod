# 📟 PingPod

### A tiny notification companion for your phone

**PingPod** is a small ESP8266-based notification display that receives notifications from an Android phone and displays them on a **0.96-inch OLED screen**.

The idea is simple:

> **Phone notification → MacroDroid → HTTP → ESP8266 → OLED**

Instead of constantly checking your phone, PingPod gives you a quick glance at incoming notifications and calls on a small portable display.

---

## Why PingPod?

This project started from a simple real-world problem.

While travelling or riding, I may not notice phone calls or notifications even when my phone is on vibration.

So I thought:

> **"Why not build a small device that can tell me what's happening on my phone?"**

That idea became PingPod.

---

## ✨ Features

- 📞 Incoming call display
- 💬 Notification display
- 📱 App name display
- 👤 Caller/chat name display
- 📝 Message preview
- 🕐 Clock display
- 📅 Date display
- 📆 Day display
- 🔢 Notification counter
- 🔄 Notification queue
- 🎞️ Incoming-call animation
- ↔️ Caller-name scrolling for long names
- 🌐 HTTP-based communication
- 📡 Wi-Fi connectivity
- 🧹 Basic notification filtering
- 🔤 URL decoding
- 🚫 Emoji/Unicode filtering for OLED compatibility
- ⏱️ Automatic notification timeout

---

## 🧠 How It Works

PingPod uses an Android phone and an ESP8266 communicating over the same network.

```text
┌─────────────────────┐
│      Android Phone  │
│                     │
│  Call / Notification│
└──────────┬──────────┘
           │
           │ MacroDroid
           ▼
┌─────────────────────┐
│    MacroDroid       │
│                     │
│ Detect notification │
│ Create HTTP request │
└──────────┬──────────┘
           │
           │ HTTP GET
           ▼
┌─────────────────────┐
│    ESP8266 NodeMCU  │
│                     │
│ HTTP Server         │
│ Process notification│
│ Manage queue        │
└──────────┬──────────┘
           │
           │ I2C
           ▼
┌─────────────────────┐
│   0.96" OLED        │
│      128 × 64       │
│                     │
│  CALL                │
│  John                 │
│  RINGING              │
└─────────────────────┘
```

---

# 🛠️ Hardware

| Component | Purpose |
|---|---|
| **ESP8266 NodeMCU** | Main controller and Wi-Fi communication |
| **0.96-inch OLED** | Displays calls, notifications, and clock |
| USB cable | Power and programming |
| Android phone | Notification source |

### OLED

The project is designed for a:

- 0.96-inch OLED
- 128 × 64 resolution
- SSD1306 controller
- I2C interface
- I2C address: `0x3C`

---

# 🔌 OLED Wiring

The current code uses the following I2C pins:

| OLED | ESP8266 NodeMCU |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | D2 |
| SCL | D1 |

### Wiring

```text
OLED              ESP8266 NodeMCU
───────────────────────────────
VCC      ───────► 3.3V
GND      ───────► GND
SDA      ───────► D2
SCL      ───────► D1
```

The code initializes the display with address:

```cpp
0x3C
```

---

# 💻 Software

## Required

- Arduino IDE
- ESP8266 Board Package
- Android phone
- MacroDroid
- Wi-Fi network

## Arduino Libraries

The project uses:

```cpp
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
```

Install these libraries through the Arduino IDE Library Manager:

### Adafruit GFX Library

Used for drawing text and graphics on the OLED.

### Adafruit SSD1306

Used to control the SSD1306 OLED display.

The ESP8266 Wi-Fi and HTTP server libraries are provided through the ESP8266 Arduino core.

---

# 🚀 Getting Started

## 1. Clone the repository

```bash
git clone https://github.com/SANJAY-N0/PingPod.git
```

Move into the project:

```bash
cd PingPod
```

---

## 2. Open the project

Open:

```text
esp8266.cpp
```

in Arduino IDE.

---

## 3. Configure Wi-Fi

Find:

```cpp
const char* WIFI_SSID = "<<wifi name>>";
const char* WIFI_PASSWORD = "<<password>>";
```

Replace them with your Wi-Fi credentials:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

> Keep your real Wi-Fi credentials private. Do not commit them to a public GitHub repository.

---

## 4. Select ESP8266 board

In Arduino IDE:

```text
Tools
  ↓
Board
  ↓
ESP8266 Boards
  ↓
NodeMCU 1.0 (ESP-12E Module)
```

Select the correct serial port under:

```text
Tools → Port
```

---

## 5. Upload

Click:

```text
Upload
```

After uploading, open:

```text
Tools → Serial Monitor
```

Set the baud rate to:

```text
115200
```

You should see the ESP8266 connecting to Wi-Fi and printing its local IP address.

Example:

```text
WiFi connected
ESP8266 IP: 192.168.1.105
HTTP server started
```

Save this IP address.

You will use it in MacroDroid.

---

# 📱 MacroDroid Setup

MacroDroid is used as the bridge between the Android notification system and PingPod.

The basic workflow is:

```text
Android Notification
        ↓
   MacroDroid
        ↓
   HTTP GET Request
        ↓
   ESP8266
        ↓
     OLED
```

Create a MacroDroid macro that:

### Trigger

Use a notification-related trigger.

For example:

```text
Notification Received
```

### Action

Send an HTTP request to the ESP8266.

The PingPod server exposes:

```text
/notify
```

The request accepts:

```text
app
chat
message
```

So the request format is:

```text
http://ESP8266_IP/notify?app=APP&chat=CHAT&message=MESSAGE
```

Example:

```text
http://192.168.1.105/notify?app=WhatsApp&chat=John&message=Hello
```

The ESP8266 receives the request and displays the information on the OLED.

---

# 📞 Call Handling

PingPod also supports incoming-call events.

The call-start endpoint is:

```text
/call/start
```

The caller name is passed using:

```text
caller
```

Example:

```text
http://192.168.1.105/call/start?caller=John
```

The OLED switches to the call screen.

Example:

```text
       CALL

> > > > >

     John

   RINGING
```

The project also supports ending the call through:

```text
/call/end
```

Example:

```text
http://192.168.1.105/call/end
```

After the call ends, PingPod returns to the clock or continues pending notifications.

---

# 🌐 HTTP API

PingPod runs a lightweight HTTP server on port `80`.

## Root

```http
GET /
```

Response:

```text
ESP8266 Notification Server
```

---

## Send Notification

```http
GET /notify
```

Parameters:

```text
app
chat
message
```

Example:

```http
GET /notify?app=WhatsApp&chat=John&message=Hello
```

Response:

```text
Notification received
```

---

## Start Call

```http
GET /call/start
```

Parameter:

```text
caller
```

Example:

```http
GET /call/start?caller=John
```

Response:

```text
Call started
```

---

## End Call

```http
GET /call/end
```

Response:

```text
Call ended
```

---

# 🖥️ Display Modes

PingPod currently has three main display modes:

```text
CLOCK_SCREEN
NOTIFICATION_SCREEN
CALL_SCREEN
```

---

## 🕐 Clock Screen

When there are no active notifications or calls, the OLED displays:

```text
      10:45

   10/11/2026

    Tuesday

   NOTIF 5
```

The time is synchronized using NTP.

The project uses:

```text
pool.ntp.org
time.nist.gov
time.google.com
```

The configured timezone is:

```text
GMT +5:30
```

for India.

---

# 🔔 Notification Screen

When a notification arrives, PingPod displays:

```text
WhatsApp
────────────────

John

Hello, how are you?
Are you coming today?
```

The screen contains:

- Application name
- Chat/contact name
- Message preview

Notifications are displayed for approximately **10 seconds** before returning to the clock.

---

# 📞 Call Screen

Incoming calls have higher priority than normal notifications.

The display shows:

```text
       CALL

> > > >

     John

   RINGING
```

The caller name can also scroll when the name is too long for the OLED.

---

# 📥 Notification Queue

PingPod supports a small notification queue.

The current implementation allows up to:

```cpp
#define MAX_NOTIFICATIONS 10
```

notifications.

This prevents multiple notifications arriving at nearly the same time from immediately overwriting each other.

The queue works approximately like:

```text
Notification 1
      ↓
Notification 2
      ↓
Notification 3
      ↓
     Queue
      ↓
Display one at a time
```

Incoming calls have priority over normal notifications.

---

# 🧹 Notification Filtering

PingPod includes basic filtering to ignore unwanted notifications.

For example, certain Android system messages related to:

```text
Data limit exceeded
Data usage
Data warning
```

are ignored.

Empty notifications are also ignored.

This helps keep the OLED focused on useful notifications.

---

# 🔤 Text Processing

Because the OLED display has limited space, the project performs some text processing before displaying notification data.

It handles:

### URL decoding

For example:

```text
Hello%20World
```

becomes:

```text
Hello World
```

### Plus signs

```text
Hello+World
```

becomes:

```text
Hello World
```

### Emoji filtering

The current implementation keeps printable ASCII characters and removes unsupported Unicode/emoji characters before displaying them.

This helps avoid unexpected characters on the small OLED display.

---

# 🏗️ Project Structure

The repository is currently simple:

```text
PingPod/
│
├── README.md
│
└── esp8266.cpp
```

### `esp8266.cpp`

Contains the complete ESP8266 firmware, including:

- Wi-Fi setup
- OLED initialization
- HTTP server
- Notification processing
- Notification queue
- Call handling
- Clock
- NTP synchronization
- OLED rendering
- Text cleaning
- Notification filtering

---

# 🔄 Complete Data Flow

### Notification

```text
        Android Phone
             │
             │ Notification
             ▼
         MacroDroid
             │
             │ HTTP GET
             ▼
       ESP8266 NodeMCU
             │
             │ Parse parameters
             ▼
      Notification Queue
             │
             ▼
       OLED Display
             │
             ▼
       Notification
        displayed
```

### Incoming Call

```text
      Incoming Call
             │
             ▼
         MacroDroid
             │
             │ /call/start
             ▼
       ESP8266 NodeMCU
             │
             ▼
        Call Screen
             │
             ▼
          OLED
```

When the call ends:

```text
/call/end
     │
     ▼
ESP8266
     │
     ▼
Clock / Pending Notification
```

---

# 🧩 Architecture

```text
┌──────────────────────────────────────┐
│              Android                 │
│                                      │
│  Calls / Messages / Notifications   │
└──────────────────┬───────────────────┘
                   │
                   ▼
┌──────────────────────────────────────┐
│             MacroDroid               │
│                                      │
│ Notification Listener                │
│ HTTP Request                         │
└──────────────────┬───────────────────┘
                   │
              Wi-Fi / HTTP
                   │
                   ▼
┌──────────────────────────────────────┐
│           ESP8266 NodeMCU            │
│                                      │
│  HTTP Server                         │
│       ↓                              │
│  Request Processing                  │
│       ↓                              │
│  Notification Queue                 │
│       ↓                              │
│  Screen Controller                  │
└──────────────────┬───────────────────┘
                   │
                   │ I2C
                   ▼
┌──────────────────────────────────────┐
│            0.96" OLED                │
│             128 × 64                 │
└──────────────────────────────────────┘
```

---

# 🔐 Security Note

PingPod currently runs a simple HTTP server on the local network.

There is **no authentication or HTTPS implemented in the current prototype**.

Therefore:

- Use it on a trusted local network.
- Do not expose the ESP8266 HTTP server directly to the public internet.
- Do not share your Wi-Fi credentials.
- Do not commit Wi-Fi passwords to GitHub.

For a production version, authentication and encrypted communication should be considered.

---

# ⚠️ Current Limitations

This is currently a prototype.

Some limitations include:

- Requires Wi-Fi connectivity.
- Requires an Android phone running MacroDroid.
- HTTP communication is currently unauthenticated.
- OLED has limited display space.
- Long notification messages are truncated.
- Emoji/Unicode characters are filtered.
- Hardware enclosure is not yet finalized.
- Power/battery management still needs improvement.
- The device is currently dependent on the phone and local network.

---

# 🚧 Future Improvements

Planned improvements include:

- 🔋 Battery-powered version
- 📦 Compact enclosure
- 🔌 Better power management
- 🎨 Improved OLED UI
- 📞 Better call interface
- 💬 Better message formatting
- 🔔 Notification icons
- 🔐 HTTP authentication
- 🔒 HTTPS or secure communication
- 📡 More reliable connectivity
- ⚙️ Configuration through a web interface
- 🔄 Automatic Wi-Fi reconnection
- 🧠 Better notification filtering
- 📱 Companion Android application
- 🔕 Notification priority settings

---

# 🎯 Project Goal

The goal of PingPod is not to replace a smartphone.

It is to create a **small, focused notification companion** that provides important information at a glance.

Instead of:

```text
Phone → Take out → Unlock → Check notification
```

the goal is:

```text
Notification → Quick glance at PingPod
```

Simple.

Small.

Useful.

---

# 📚 What I Learned

Building PingPod helped me understand:

- ESP8266 development
- Embedded programming
- I2C communication
- OLED display programming
- HTTP servers on microcontrollers
- REST-like HTTP endpoints
- Android notification automation
- MacroDroid
- Wi-Fi communication
- NTP time synchronization
- Event-driven programming
- Notification queues
- Real-time display updates
- Hardware/software integration

Most importantly, I learned that a hardware project doesn't always have to start with a huge idea.

Sometimes:

```text
Real Problem
     ↓
Simple Idea
     ↓
Prototype
     ↓
Testing
     ↓
Improvement
     ↓
Shipping
```

---

# 🚀 Art of Shipping

PingPod started from a simple problem I experienced while travelling.

I wanted a small device that could tell me when someone was calling or when I received an important notification.

So instead of only thinking about the idea, I started building it.

**Problem → Idea → Build → Test → Break → Fix → Improve → Ship**

PingPod is still evolving, but the prototype is already working.

This is one step in my **Art of Shipping** journey.

---

# 📸 Project Status

**Current status:**

```text
[+] ESP8266 NodeMCU connected with OLED
[+] Basic display interface working
[+] Phone notifications detected
[+] Notification data sent through HTTP
[+] Notification displayed on OLED
[+] Incoming call display
[+] Notification queue
[+] Clock display
[ ] Improve hardware design
[ ] Make the device smaller
[ ] Improve power management
[ ] Build a proper portable enclosure
[ ] Finalize the portable version
```

---

# 🤝 Contributing

This is currently a personal prototype, but suggestions and improvements are welcome.

If you have an idea for:

- Better notification handling
- OLED UI
- Power management
- Hardware design
- Android integration
- Communication protocols

feel free to open an issue or submit a pull request.

---

# 📄 License

This project does not currently specify a license.

If you want others to freely use, modify, and distribute the project, consider adding an appropriate open-source license such as MIT.

---

# ⭐ Support

If you find the project interesting:

⭐ Star the repository  
🍴 Fork it  
🐛 Report issues  
💡 Suggest improvements  
🚀 Build your own version

---

## 🔗 Repository

**GitHub:**  
https://github.com/SANJAY-N0/PingPod

---

## 👨‍💻 Author

**SANJAY-N0**

Built as part of my **Art of Shipping** journey.

> **Don't wait for the perfect idea. Find a problem. Build a solution. Ship it.**
