#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "CELIK2";
const char* password = "CelikCelikAilesi";
WebServer server(80);

// Pin Configuration (ESP32-C3)
#define TFT_CS    0
#define TFT_RST   2
#define TFT_DC    1
#define TFT_MOSI  21
#define TFT_SCLK  20

#define W 80
#define H 160
#define X_OFFSET 26
#define Y_OFFSET 1
#define IMG_BYTES 12800 // 80*80*2

// Colors
#define BLACK 0x0000
#define WHITE 0xFFFF
#define GRAY  0x4208
#define GREEN 0x07E0
#define RED   0xF800

// 5x7 Font (ASCII 32-90)
const uint8_t Font5x7[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, // Space
  0x00, 0x00, 0x5F, 0x00, 0x00, // !
  0x00, 0x07, 0x00, 0x07, 0x00, // "
  0x14, 0x7F, 0x14, 0x7F, 0x14, // #
  0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
  0x23, 0x13, 0x08, 0x64, 0x62, // %
  0x36, 0x49, 0x56, 0x20, 0x50, // &
  0x00, 0x08, 0x07, 0x03, 0x00, // '
  0x00, 0x1C, 0x22, 0x41, 0x00, // (
  0x00, 0x41, 0x22, 0x1C, 0x00, // )
  0x2A, 0x1C, 0x7F, 0x1C, 0x2A, // *
  0x08, 0x08, 0x3E, 0x08, 0x08, // +
  0x00, 0x80, 0x70, 0x30, 0x00, // ,
  0x08, 0x08, 0x08, 0x08, 0x08, // -
  0x00, 0x00, 0x60, 0x60, 0x00, // .
  0x20, 0x10, 0x08, 0x04, 0x02, // /
  0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
  0x00, 0x42, 0x7F, 0x40, 0x00, // 1
  0x72, 0x49, 0x49, 0x49, 0x46, // 2
  0x21, 0x41, 0x49, 0x4D, 0x33, // 3
  0x18, 0x14, 0x12, 0x7F, 0x10, // 4
  0x27, 0x45, 0x45, 0x45, 0x39, // 5
  0x3C, 0x4A, 0x49, 0x49, 0x31, // 6
  0x41, 0x21, 0x11, 0x09, 0x07, // 7
  0x36, 0x49, 0x49, 0x49, 0x36, // 8
  0x46, 0x49, 0x49, 0x29, 0x1E, // 9
  0x00, 0x00, 0x14, 0x00, 0x00, // :
  0x00, 0x40, 0x34, 0x00, 0x00, // ;
  0x00, 0x08, 0x14, 0x22, 0x41, // <
  0x14, 0x14, 0x14, 0x14, 0x14, // =
  0x00, 0x41, 0x22, 0x14, 0x08, // >
  0x02, 0x01, 0x59, 0x09, 0x06, // ?
  0x3E, 0x41, 0x5D, 0x59, 0x4E, // @
  0x7C, 0x12, 0x11, 0x12, 0x7C, // A
  0x7F, 0x49, 0x49, 0x49, 0x36, // B
  0x3E, 0x41, 0x41, 0x41, 0x22, // C
  0x7F, 0x41, 0x41, 0x41, 0x3E, // D
  0x7F, 0x49, 0x49, 0x49, 0x41, // E
  0x7F, 0x09, 0x09, 0x09, 0x01, // F
  0x3E, 0x41, 0x41, 0x51, 0x73, // G
  0x7F, 0x08, 0x08, 0x08, 0x7F, // H
  0x00, 0x41, 0x7F, 0x41, 0x00, // I
  0x20, 0x40, 0x41, 0x3F, 0x01, // J
  0x7F, 0x08, 0x14, 0x22, 0x41, // K
  0x7F, 0x40, 0x40, 0x40, 0x40, // L
  0x7F, 0x02, 0x1C, 0x02, 0x7F, // M
  0x7F, 0x04, 0x08, 0x10, 0x7F, // N
  0x3E, 0x41, 0x41, 0x41, 0x3E, // O
  0x7F, 0x09, 0x09, 0x09, 0x06, // P
  0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
  0x7F, 0x09, 0x19, 0x29, 0x46, // R
  0x26, 0x49, 0x49, 0x49, 0x32, // S
  0x03, 0x01, 0x7F, 0x01, 0x03, // T
  0x3F, 0x40, 0x40, 0x40, 0x3F, // U
  0x1F, 0x20, 0x40, 0x20, 0x1F, // V
  0x3F, 0x40, 0x38, 0x40, 0x3F, // W
  0x63, 0x14, 0x08, 0x14, 0x63, // X
  0x03, 0x04, 0x78, 0x04, 0x03, // Y
  0x61, 0x59, 0x49, 0x4D, 0x43, // Z
};

void writeCmd(uint8_t c) {
  digitalWrite(TFT_DC, LOW); 
  digitalWrite(TFT_CS, LOW);
  SPI.transfer(c); 
  digitalWrite(TFT_CS, HIGH);
}

void writeData(uint8_t d) {
  digitalWrite(TFT_DC, HIGH); 
  digitalWrite(TFT_CS, LOW);
  SPI.transfer(d); 
  digitalWrite(TFT_CS, HIGH);
}

void setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
  writeCmd(0x2A); 
  writeData(0x00); writeData(x0 + X_OFFSET);
  writeData(0x00); writeData(x1 + X_OFFSET);
  writeCmd(0x2B); 
  writeData(0x00); writeData(y0 + Y_OFFSET);
  writeData(0x00); writeData(y1 + Y_OFFSET);
  writeCmd(0x2C); 
}

void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
  setAddrWindow(x, y, x + w - 1, y + h - 1);
  uint8_t hi = color >> 8, lo = color & 0xFF;
  digitalWrite(TFT_DC, HIGH); 
  digitalWrite(TFT_CS, LOW);
  for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
    SPI.transfer(hi); 
    SPI.transfer(lo);
  }
  digitalWrite(TFT_CS, HIGH);
}

void printTextArea(String text, uint16_t color) {
  fillRect(0, 81, 80, 79, BLACK); // Clear bottom half
  fillRect(0, 80, 80, 1, GRAY);   // Divider line
  
  uint8_t curX = 2;
  uint8_t curY = 85;

  for (int i = 0; i < text.length(); i++) {
    char c = toupper(text[i]);
    if (c == '\n' || curX > 74) { 
      curX = 2; 
      curY += 10; 
    }
    if (curY > 150) break;
    if (c < 32 || c > 90) continue;

    uint8_t charIdx = c - 32;
    for (uint8_t col = 0; col < 5; col++) {
      uint8_t line = pgm_read_byte(&Font5x7[charIdx * 5 + col]);
      for (uint8_t row = 0; row < 7; row++) {
        if (line & (1 << row)) {
          setAddrWindow(curX + col, curY + row, curX + col, curY + row);
          writeData(color >> 8); 
          writeData(color & 0xFF);
        }
      }
    }
    curX += 6;
  }
}

void handleUpload() {
  HTTPUpload& upload = server.upload();
  static uint32_t imageReceived = 0;
  
  if (upload.status == UPLOAD_FILE_START) {
    imageReceived = 0;
    setAddrWindow(0, 0, 79, 79);
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    for (size_t i = 0; i < upload.currentSize; i++) {
      SPI.transfer(upload.buf[i]);
      imageReceived++;
    }
    
  } else if (upload.status == UPLOAD_FILE_END) {
    digitalWrite(TFT_CS, HIGH);
    
    if (server.hasArg("text")) {
      String text = server.arg("text");
      printTextArea(text, WHITE);
    }
    
    server.send(200, "text/plain", "OK");
  }
}

void handleText() {
  if (server.hasArg("text")) {
    String text = server.arg("text");
    printTextArea(text, WHITE);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing text parameter");
  }
}

void handleRoot() {
  server.send(200, "text/html", 
    "<html><body>"
    "<h1>ESP32 Display Server</h1>"
    "<p>Ready to receive images</p>"
    "<p>POST to /upload with multipart form data</p>"
    "</body></html>");
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_CS, OUTPUT); 
  pinMode(TFT_RST, OUTPUT); 
  pinMode(TFT_DC, OUTPUT);
  
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));

  // Hardware Reset
  digitalWrite(TFT_RST, HIGH); delay(50);
  digitalWrite(TFT_RST, LOW); delay(50);
  digitalWrite(TFT_RST, HIGH); delay(150);

  // Display Initialization
  writeCmd(0x01); delay(150);
  writeCmd(0x11); delay(200);
  writeCmd(0x3A); writeData(0x05);
  writeCmd(0x36); writeData(0xC8);
  writeCmd(0x21);
  writeCmd(0x29);
  
  fillRect(0, 0, 80, 160, BLACK);

  // WiFi Connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
  }
  
  // Setup HTTP server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/upload", HTTP_POST, 
    []() {}, 
    handleUpload
  );
  server.on("/text", HTTP_POST, handleText);
  
  server.begin();
  
  printTextArea("READY\nIP: " + WiFi.localIP().toString(), GREEN);
}

void loop() {
  server.handleClient();
}
