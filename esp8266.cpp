#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// =====================================================
// WIFI
// =====================================================

const char* WIFI_SSID = "<<wifi name>>";
const char* WIFI_PASSWORD = "<<password>>";


// India timezone
const long GMT_OFFSET_SEC = 19800;
const int DAYLIGHT_OFFSET_SEC = 0;


// =====================================================
// OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1

// NodeMCU ESP8266
// SDA -> D2
// SCL -> D1

#define OLED_SDA D2
#define OLED_SCL D1


Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);


// =====================================================
// HTTP SERVER
// =====================================================

ESP8266WebServer server(80);


// =====================================================
// SCREEN MODES
// =====================================================

enum ScreenMode {

  CLOCK_SCREEN,
  NOTIFICATION_SCREEN,
  CALL_SCREEN
};

ScreenMode currentScreen = CLOCK_SCREEN;


// =====================================================
// NOTIFICATION
// =====================================================

struct NotificationItem {

  String app;
  String chat;
  String message;
};


#define MAX_NOTIFICATIONS 10

NotificationItem notificationQueue[
  MAX_NOTIFICATIONS
];

int queueCount = 0;

unsigned long notificationCount = 0;


// Notification display time
// 10 seconds

const unsigned long NOTIFICATION_TIME = 10000;

unsigned long notificationStartTime = 0;


// =====================================================
// CALL
// =====================================================

bool callActive = false;

String callerName = "";


// =====================================================
// CALL ANIMATION
// =====================================================

// Animation:
//
// INCOMING CALL
// >
// >>
// >>>
// >>>>
// >>>>>

int callAnimationStep = 0;

unsigned long lastCallAnimation = 0;

const unsigned long CALL_ANIMATION_SPEED = 200;


// =====================================================
// CALLER NAME SCROLL
// =====================================================

int callScrollPosition = 0;

unsigned long lastCallScroll = 0;

const unsigned long CALL_SCROLL_SPEED = 250;


// =====================================================
// FUNCTION DECLARATIONS
// =====================================================

String urlDecode(String input);

String removeEmoji(String text);

String cleanText(String text);

bool isAllowedNotification(
  String app,
  String chat,
  String message
);

void addNotification(
  String app,
  String chat,
  String message
);

void showClock();

void showNotification(
  String app,
  String chat,
  String message
);

void showCall();

void updateCallAnimation();

void updateCallerScroll();

void handleNotification();

void handleCallStart();

void handleCallEnd();

void processNotification();


// =====================================================
// REMOVE EMOJI
// =====================================================

String removeEmoji(String text) {

  String result = "";

  for (
    int i = 0;
    i < text.length();
    i++
  ) {

    unsigned char c =
      text[i];

    // Keep printable ASCII
    // Remove emoji / Unicode characters

    if (
      c >= 32 &&
      c <= 126
    ) {

      result += char(c);
    }
  }

  return result;
}


// =====================================================
// URL DECODE
// =====================================================

String urlDecode(String input) {

  String decoded = "";

  for (
    int i = 0;
    i < input.length();
    i++
  ) {

    char c =
      input[i];


    // + becomes space

    if (c == '+') {

      decoded += ' ';
    }


    // %XX

    else if (
      c == '%' &&
      i + 2 < input.length()
    ) {

      char hex1 =
        input[i + 1];

      char hex2 =
        input[i + 2];


      char hexString[3];

      hexString[0] = hex1;
      hexString[1] = hex2;
      hexString[2] = '\0';


      char value =
        (char)strtol(
          hexString,
          NULL,
          16
        );


      decoded += value;

      i += 2;
    }


    else {

      decoded += c;
    }
  }

  return decoded;
}


// =====================================================
// CLEAN TEXT
// =====================================================

String cleanText(String text) {

  text = urlDecode(text);

  text = removeEmoji(text);

  text.trim();

  return text;
}


// =====================================================
// NOTIFICATION FILTER
// =====================================================

bool isAllowedNotification(
  String app,
  String chat,
  String message
) {

  String a = app;
  String c = chat;
  String m = message;


  a.toLowerCase();
  c.toLowerCase();
  m.toLowerCase();


  // -----------------------------------------------
  // Android data-limit notification
  // -----------------------------------------------

  if (
    m.indexOf(
      "data limit exceeded"
    ) >= 0
  ) {

    return false;
  }


  if (
    c.indexOf(
      "data limit exceeded"
    ) >= 0
  ) {

    return false;
  }


  // -----------------------------------------------
  // Data usage
  // -----------------------------------------------

  if (
    m.indexOf(
      "data usage"
    ) >= 0
  ) {

    return false;
  }


  // -----------------------------------------------
  // Data warning
  // -----------------------------------------------

  if (
    m.indexOf(
      "data warning"
    ) >= 0
  ) {

    return false;
  }


  // -----------------------------------------------
  // Empty notification
  // -----------------------------------------------

  if (
    a.length() == 0 &&
    c.length() == 0 &&
    m.length() == 0
  ) {

    return false;
  }


  return true;
}


// =====================================================
// ADD NOTIFICATION TO QUEUE
// =====================================================

void addNotification(
  String app,
  String chat,
  String message
) {

  if (
    queueCount >=
    MAX_NOTIFICATIONS
  ) {

    Serial.println(
      "Notification queue full"
    );

    return;
  }


  notificationQueue[
    queueCount
  ].app = app;


  notificationQueue[
    queueCount
  ].chat = chat;


  notificationQueue[
    queueCount
  ].message = message;


  queueCount++;


  Serial.print(
    "Queue count: "
  );

  Serial.println(
    queueCount
  );
}


// =====================================================
// CLOCK SCREEN
// =====================================================

void showClock() {

  struct tm timeInfo;


  if (
    !getLocalTime(
      &timeInfo
    )
  ) {

    display.clearDisplay();

    display.setTextColor(
      SSD1306_WHITE
    );

    display.setTextSize(1);

    display.setCursor(
      0,
      0
    );

    display.println(
      "Waiting for time..."
    );

    display.display();

    return;
  }


  // -----------------------------------------------
  // TIME
  // -----------------------------------------------

  char timeText[10];

  strftime(
    timeText,
    sizeof(timeText),
    "%H:%M",
    &timeInfo
  );


  // -----------------------------------------------
  // DATE
  // -----------------------------------------------

  char dateText[20];

  strftime(
    dateText,
    sizeof(dateText),
    "%d/%m/%Y",
    &timeInfo
  );


  // -----------------------------------------------
  // DAY
  // -----------------------------------------------

  char dayText[20];

  strftime(
    dayText,
    sizeof(dayText),
    "%A",
    &timeInfo
  );


  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );


  // -----------------------------------------------
  // BIG TIME
  // -----------------------------------------------

  display.setTextSize(3);

  display.setCursor(
    15,
    0
  );

  display.println(
    timeText
  );


  // -----------------------------------------------
  // DATE
  // -----------------------------------------------

  display.setTextSize(1);

  display.setCursor(
    30,
    32
  );

  display.println(
    dateText
  );


  // -----------------------------------------------
  // DAY
  // -----------------------------------------------

  display.setCursor(
    35,
    43
  );

  display.println(
    dayText
  );


  // -----------------------------------------------
  // NOTIFICATION COUNT
  // -----------------------------------------------

  display.setCursor(
    0,
    56
  );

  display.print(
    "NOTIF "
  );

  display.print(
    notificationCount
  );


  display.display();
}


// =====================================================
// INCOMING CALL SCREEN
// =====================================================

void showCall() {

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );


  // =================================================
  // LARGE "CALL"
  // =================================================

  display.setTextSize(2);

  display.setCursor(
    35,
    0
  );

  display.println(
    "CALL"
  );


  // =================================================
  // ANIMATED ARROWS
  // =================================================

  display.setTextSize(1);

  display.setCursor(
    0,
    17
  );


  for (
    int i = 0;
    i < callAnimationStep;
    i++
  ) {

    display.print(">");
  }


  // =================================================
  // SEPARATOR
  // =================================================

  display.drawLine(
    0,
    25,
    127,
    25,
    SSD1306_WHITE
  );


  // =================================================
  // CALLER NAME
  // =================================================

  display.setTextSize(2);


  String name =
    callerName;


  // 10 characters approximately
  // fit at text size 2

  const int visibleChars = 10;


  // -------------------------------------------------
  // SHORT NAME
  // -------------------------------------------------

  if (
    name.length() <=
    visibleChars
  ) {

    int width =
      name.length() * 12;


    int x =
      (128 - width) / 2;


    if (x < 0) {

      x = 0;
    }


    display.setCursor(
      x,
      30
    );


    display.println(
      name
    );
  }


  // -------------------------------------------------
  // LONG NAME
  // -------------------------------------------------

  else {

    if (
      callScrollPosition >=
      name.length()
    ) {

      callScrollPosition = 0;
    }


    int end =
      callScrollPosition +
      visibleChars;


    if (
      end > name.length()
    ) {

      end =
        name.length();
    }


    String visibleName =
      name.substring(
        callScrollPosition,
        end
      );


    display.setCursor(
      0,
      30
    );


    display.println(
      visibleName
    );
  }


  // =================================================
  // RINGING
  // =================================================

  display.setTextSize(1);

  display.setCursor(
    35,
    54
  );

  display.println(
    "RINGING"
  );


  display.display();
}


// =====================================================
// CALL ANIMATION
// =====================================================

void updateCallAnimation() {

  if (
    !callActive
  ) {

    return;
  }


  // -----------------------------------------------
  // Animate arrows
  // -----------------------------------------------

  if (
    millis() -
    lastCallAnimation
    >=
    CALL_ANIMATION_SPEED
  ) {

    lastCallAnimation =
      millis();


    callAnimationStep++;


    if (
      callAnimationStep > 5
    ) {

      callAnimationStep = 0;
    }
  }


  // -----------------------------------------------
  // Scroll caller name
  // -----------------------------------------------

  if (
    callerName.length() > 10
  ) {

    if (
      millis() -
      lastCallScroll
      >=
      CALL_SCROLL_SPEED
    ) {

      lastCallScroll =
        millis();


      callScrollPosition++;


      if (
        callScrollPosition >=
        callerName.length()
      ) {

        callScrollPosition = 0;
      }
    }
  }


  // Redraw

  showCall();
}


// =====================================================
// NOTIFICATION SCREEN
// =====================================================

void showNotification(
  String app,
  String chat,
  String message
) {

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );


  // -----------------------------------------------
  // APP
  // -----------------------------------------------

  display.setTextSize(1);

  display.setCursor(
    0,
    0
  );

  display.println(
    app
  );


  // Separator

  display.drawLine(
    0,
    10,
    127,
    10,
    SSD1306_WHITE
  );


  // -----------------------------------------------
  // CHAT NAME
  // -----------------------------------------------

  display.setTextSize(2);

  display.setCursor(
    0,
    13
  );


  if (
    chat.length() > 10
  ) {

    display.println(
      chat.substring(
        0,
        10
      )
    );
  }

  else {

    display.println(
      chat
    );
  }


  // -----------------------------------------------
  // MESSAGE
  // -----------------------------------------------

  display.setTextSize(1);

  display.setCursor(
    0,
    32
  );


  const int charsPerLine = 21;

  int start = 0;

  int line = 0;


  while (
    start < message.length() &&
    line < 4
  ) {

    int end =
      start +
      charsPerLine;


    if (
      end >
      message.length()
    ) {

      end =
        message.length();
    }


    display.println(
      message.substring(
        start,
        end
      )
    );


    start = end;

    line++;
  }


  display.display();
}


// =====================================================
// NOTIFICATION HTTP
// =====================================================

void handleNotification() {

  String app =
    cleanText(
      server.arg(
        "app"
      )
    );


  String chat =
    cleanText(
      server.arg(
        "chat"
      )
    );


  String message =
    cleanText(
      server.arg(
        "message"
      )
    );


  // -----------------------------------------------
  // SERIAL DEBUG
  // -----------------------------------------------

  Serial.println();

  Serial.println(
    "=========================="
  );

  Serial.println(
    "NEW NOTIFICATION"
  );

  Serial.print(
    "App: "
  );

  Serial.println(
    app
  );

  Serial.print(
    "Chat: "
  );

  Serial.println(
    chat
  );

  Serial.print(
    "Message: "
  );

  Serial.println(
    message
  );


  // -----------------------------------------------
  // FILTER
  // -----------------------------------------------

  if (
    !isAllowedNotification(
      app,
      chat,
      message
    )
  ) {

    Serial.println(
      "IGNORED"
    );

    server.send(
      200,
      "text/plain",
      "Ignored"
    );

    return;
  }


  // -----------------------------------------------
  // COUNT
  // -----------------------------------------------

  notificationCount++;


  // -----------------------------------------------
  // QUEUE
  // -----------------------------------------------

  addNotification(
    app,
    chat,
    message
  );


  server.send(
    200,
    "text/plain",
    "Notification received"
  );
}


// =====================================================
// CALL START
// =====================================================

void handleCallStart() {

  callerName =
    cleanText(
      server.arg(
        "caller"
      )
    );


  if (
    callerName.length() == 0
  ) {

    callerName =
      "Unknown Caller";
  }


  // -----------------------------------------------
  // SERIAL
  // -----------------------------------------------

  Serial.println();

  Serial.println(
    "=========================="
  );

  Serial.println(
    "INCOMING CALL"
  );

  Serial.print(
    "Caller: "
  );

  Serial.println(
    callerName
  );


  // -----------------------------------------------
  // START CALL
  // -----------------------------------------------

  callActive = true;


  currentScreen =
    CALL_SCREEN;


  // Reset animation

  callAnimationStep = 0;

  callScrollPosition = 0;


  lastCallAnimation =
    millis();

  lastCallScroll =
    millis();


  showCall();


  server.send(
    200,
    "text/plain",
    "Call started"
  );
}


// =====================================================
// CALL END
// =====================================================

void handleCallEnd() {

  Serial.println();

  Serial.println(
    "=========================="
  );

  Serial.println(
    "CALL ENDED"
  );


  callActive = false;


  currentScreen =
    CLOCK_SCREEN;


  // -----------------------------------------------
  // Continue pending notification
  // -----------------------------------------------

  if (
    queueCount > 0
  ) {

    processNotification();
  }

  else {

    showClock();
  }


  server.send(
    200,
    "text/plain",
    "Call ended"
  );
}


// =====================================================
// PROCESS NOTIFICATION
// =====================================================

void processNotification() {

  // Call has highest priority

  if (
    callActive
  ) {

    return;
  }


  // -----------------------------------------------
  // Current notification
  // -----------------------------------------------

  if (
    currentScreen ==
    NOTIFICATION_SCREEN
  ) {

    if (
      millis() -
      notificationStartTime
      >=
      NOTIFICATION_TIME
    ) {

      currentScreen =
        CLOCK_SCREEN;

      showClock();
    }


    return;
  }


  // -----------------------------------------------
  // Show next notification
  // -----------------------------------------------

  if (
    queueCount > 0
  ) {

    NotificationItem item =
      notificationQueue[0];


    // Shift queue

    for (
      int i = 0;
      i < queueCount - 1;
      i++
    ) {

      notificationQueue[i] =
        notificationQueue[
          i + 1
        ];
    }


    queueCount--;


    // Display

    showNotification(
      item.app,
      item.chat,
      item.message
    );


    currentScreen =
      NOTIFICATION_SCREEN;


    notificationStartTime =
      millis();
  }
}


// =====================================================
// ROOT
// =====================================================

void handleRoot() {

  server.send(
    200,
    "text/plain",
    "ESP8266 Notification Server"
  );
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(
    115200
  );

  delay(1000);


  Serial.println();

  Serial.println(
    "================================"
  );

  Serial.println(
    "ESP8266 NOTIFICATION SYSTEM"
  );

  Serial.println(
    "================================"
  );


  // =================================================
  // OLED
  // =================================================

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );


  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      0x3C
    )
  ) {

    Serial.println(
      "OLED FAILED"
    );


    while (true) {

      delay(1000);
    }
  }


  Serial.println(
    "OLED OK"
  );


  display.clearDisplay();

  display.display();


  // =================================================
  // WIFI
  // =================================================

  WiFi.mode(
    WIFI_STA
  );


  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );


  Serial.print(
    "Connecting to WiFi"
  );


  while (
    WiFi.status() !=
    WL_CONNECTED
  ) {

    delay(500);

    Serial.print(".");
  }


  Serial.println();

  Serial.println(
    "WiFi connected"
  );


  Serial.print(
    "ESP8266 IP: "
  );

  Serial.println(
    WiFi.localIP()
  );


  // =================================================
  // NTP TIME
  // =================================================

  configTime(
    GMT_OFFSET_SEC,
    DAYLIGHT_OFFSET_SEC,
    "pool.ntp.org",
    "time.nist.gov",
    "time.google.com"
  );


  // =================================================
  // HTTP ROUTES
  // =================================================

  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );


  // Notification

  server.on(
    "/notify",
    HTTP_GET,
    handleNotification
  );


  // Call start

  server.on(
    "/call/start",
    HTTP_GET,
    handleCallStart
  );


  // Call end

  server.on(
    "/call/end",
    HTTP_GET,
    handleCallEnd
  );


  // Start HTTP server

  server.begin();


  Serial.println(
    "HTTP server started"
  );


  // Initial clock

  showClock();
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // Process HTTP

  server.handleClient();


  // =================================================
  // CALL HAS HIGHEST PRIORITY
  // =================================================

  if (
    callActive
  ) {

    updateCallAnimation();

    return;
  }


  // =================================================
  // NOTIFICATION
  // =================================================

  processNotification();


  // =================================================
  // CLOCK
  // =================================================

  if (
    currentScreen ==
    CLOCK_SCREEN
  ) {

    static unsigned long lastClock =
      0;


    if (
      millis() -
      lastClock >=
      1000
    ) {

      lastClock =
        millis();


      showClock();
    }
  }
}
