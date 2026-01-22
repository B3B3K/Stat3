#include <SPI.h>

// --- TFT Pins ---
#define TFT_CS    0
#define TFT_RST   2
#define TFT_DC    1
#define TFT_MOSI  21
#define TFT_SCLK  20

#define W 160
#define H 80
#define X_OFFSET 1
#define Y_OFFSET 26
#define ROW_HEIGHT 10

// --- ST7735 Commands ---
#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT  0x11
#define ST7735_INVON   0x21
#define ST7735_MADCTL  0x36
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_DISPON  0x29

#define BLACK   0x0000
#define WHITE   0xFFFF

// --- Complete 5x7 ASCII Font ---
const uint8_t font5x7[][5] = {
  {0x00, 0x00, 0x00, 0x00, 0x00}, // space (32)
  {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
  {0x00, 0x07, 0x00, 0x07, 0x00}, // "
  {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
  {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
  {0x23, 0x13, 0x08, 0x64, 0x62}, // %
  {0x36, 0x49, 0x55, 0x22, 0x50}, // &
  {0x00, 0x05, 0x03, 0x00, 0x00}, // '
  {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
  {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
  {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // *
  {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
  {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
  {0x08, 0x08, 0x08, 0x08, 0x08}, // -
  {0x00, 0x30, 0x30, 0x00, 0x00}, // .
  {0x20, 0x10, 0x08, 0x04, 0x02}, // /
  {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0 (48)
  {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
  {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
  {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
  {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
  {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
  {0x00, 0x36, 0x36, 0x00, 0x00}, // :
  {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
  {0x00, 0x08, 0x14, 0x22, 0x41}, // <
  {0x14, 0x14, 0x14, 0x14, 0x14}, // =
  {0x41, 0x22, 0x14, 0x08, 0x00}, // >
  {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
  {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
  {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A (65)
  {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
  {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
  {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
  {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
  {0x7F, 0x09, 0x09, 0x01, 0x01}, // F
  {0x3E, 0x41, 0x41, 0x49, 0x3A}, // G
  {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
  {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
  {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
  {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
  {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
  {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
  {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
  {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
  {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
  {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
  {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
  {0x46, 0x49, 0x49, 0x49, 0x31}, // S
  {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
  {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
  {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
  {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
  {0x63, 0x14, 0x08, 0x14, 0x63}, // X
  {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
  {0x61, 0x51, 0x49, 0x45, 0x43}  // Z
};

struct SystemData {
  uint8_t cpu_usage, ram_usage, gpu_usage, vram_usage;
  uint8_t disk_read, disk_write, net_up, net_down;
  uint8_t cpu_temp, gpu_temp;
};

SystemData data;
uint8_t max_net_up = 1, max_net_down = 1;

#define GRAPH_WIDTH 160
#define GRAPH_HEIGHT 40
#define GRAPH_X 0
#define GRAPH_Y 40

uint8_t graph_cpu[GRAPH_WIDTH], graph_ram[GRAPH_WIDTH], graph_gpu[GRAPH_WIDTH], graph_vram[GRAPH_WIDTH];
uint8_t graph_index = 0, update_counter = 0;

#define COLOR_CPU    0xF800   // kırmızı (değişmedi)
#define COLOR_RAM    0x001F   // mavi
#define COLOR_GPU    0xFFE0   // sarı (değişmedi, istersen başka yaparız)
#define COLOR_VRAM   0xFD60   // turuncu (yaklaşık #FF8C00)

// --- SPI & Display Functions ---
void writeCmd(uint8_t c) {
  digitalWrite(TFT_DC, LOW); digitalWrite(TFT_CS, LOW);
  SPI.transfer(c); digitalWrite(TFT_CS, HIGH);
}

void writeData(uint8_t d) {
  digitalWrite(TFT_DC, HIGH); digitalWrite(TFT_CS, LOW);
  SPI.transfer(d); digitalWrite(TFT_CS, HIGH);
}

void setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
  writeCmd(ST7735_CASET); writeData(0); writeData(x0 + X_OFFSET); writeData(0); writeData(x1 + X_OFFSET);
  writeCmd(ST7735_RASET); writeData(0); writeData(y0 + Y_OFFSET); writeData(0); writeData(y1 + Y_OFFSET);
  writeCmd(ST7735_RAMWR);
}

void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
  if (x >= W || y >= H) return;
  setAddrWindow(x, y, x + w - 1, y + h - 1);
  uint8_t hi = color >> 8, lo = color & 0xFF;
  digitalWrite(TFT_DC, HIGH); digitalWrite(TFT_CS, LOW);
  for (uint32_t i = 0; i < (uint32_t)w * h; i++) { SPI.transfer(hi); SPI.transfer(lo); }
  digitalWrite(TFT_CS, HIGH);
}

// Flicker-Free Char: Draws background pixels instead of clearing
void drawCharOpaque(uint8_t x, uint8_t y, char c, uint16_t color, uint16_t bg) {
  if (c < 32 || c > 95) return;
  uint8_t idx = c - 32;
  setAddrWindow(x, y, x + 4, y + 7);
  digitalWrite(TFT_DC, HIGH); digitalWrite(TFT_CS, LOW);
  for (int8_t row = 0; row < 8; row++) {
    for (int8_t col = 0; col < 5; col++) {
      uint16_t pColor = (font5x7[idx][col] & (1 << row)) ? color : bg;
      SPI.transfer(pColor >> 8); SPI.transfer(pColor & 0xFF);
    }
  }
  digitalWrite(TFT_CS, HIGH);
}

void drawTextOpaque(uint8_t x, uint8_t y, const char* text, uint16_t color, uint16_t bg) {
  while (*text) { drawCharOpaque(x, y, *text++, color, bg); x += 6; }
}

void drawHalfRow(uint8_t row, bool left, const char* label, uint8_t value, uint16_t textColor, uint8_t maxVal = 100) {
  uint8_t x = left ? 0 : (W / 2);
  uint8_t y = row * ROW_HEIGHT;
  
  // Draw label and value using Opaque text (No fillRect call needed)
  drawTextOpaque(x + 2, y + 1, label, textColor, BLACK);
  uint8_t displayVal = (value > 99) ? 99 : value;
  char buf[3]; sprintf(buf, "%02d", displayVal);
  drawTextOpaque(x + 10, y + 1, buf, WHITE, BLACK);
  
  // Bar area differential update
  uint8_t barStartX = x + 22;
  uint8_t barMaxWidth = (W/2) - 24;
  uint8_t barWidth = (displayVal * barMaxWidth) / maxVal;
  if (barWidth > barMaxWidth) barWidth = barMaxWidth;
  
  if (barWidth > 0) fillRect(barStartX, y + 2, barWidth, 6, WHITE);
  if (barWidth < barMaxWidth) fillRect(barStartX + barWidth, y + 2, barMaxWidth - barWidth, 6, BLACK);
}

// Optimized Graph: No full clear, draws column by column
void drawGraph() {
  for (uint8_t x = 0; x < GRAPH_WIDTH; x++) {
    uint8_t screenX = GRAPH_WIDTH - 1 - x;
    uint8_t idx = (graph_index - x + GRAPH_WIDTH) % GRAPH_WIDTH;
    uint8_t prevIdx = (graph_index - x - 1 + GRAPH_WIDTH) % GRAPH_WIDTH;

    // Clear only this vertical column
    fillRect(screenX, GRAPH_Y, 1, GRAPH_HEIGHT, BLACK);

    auto drawSegment = [&](uint8_t val1, uint8_t val2, uint16_t color) {
      uint8_t y1 = GRAPH_Y + GRAPH_HEIGHT - 1 - (val1 * (GRAPH_HEIGHT - 2) / 100);
      uint8_t y2 = GRAPH_Y + GRAPH_HEIGHT - 1 - (val2 * (GRAPH_HEIGHT - 2) / 100);
      fillRect(screenX, min(y1, y2), 1, abs(y1 - y2) + 2, color); // +2 for thickness
    };

    drawSegment(graph_vram[prevIdx], graph_vram[idx], COLOR_VRAM);   // turuncu
    drawSegment(graph_gpu[prevIdx], graph_gpu[idx], COLOR_GPU);
    drawSegment(graph_ram[prevIdx], graph_ram[idx], COLOR_RAM);     // mavi
    drawSegment(graph_cpu[prevIdx], graph_cpu[idx], COLOR_CPU);
    }
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_CS, OUTPUT); pinMode(TFT_RST, OUTPUT); pinMode(TFT_DC, OUTPUT);
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));

  digitalWrite(TFT_RST, HIGH); delay(10);
  digitalWrite(TFT_RST, LOW);  delay(10);
  digitalWrite(TFT_RST, HIGH); delay(150);

  writeCmd(ST7735_SWRESET); delay(150);
  writeCmd(ST7735_SLPOUT);  delay(200);
  writeCmd(ST7735_INVON);
  writeCmd(ST7735_MADCTL);  writeData(0x68); 
  writeCmd(0x3A);           writeData(0x05); 
  writeCmd(ST7735_DISPON);

  fillRect(0, 0, W, H, BLACK);
}

void loop() {
  if (Serial.available() >= 16) {
    if (Serial.read() == 0xFF && Serial.read() == 0xFF) {
      uint8_t buf[16]; Serial.readBytes(buf, 16);
      
      data.cpu_usage = buf[0];
      data.ram_usage = buf[3];
      data.gpu_usage = buf[4];
      data.vram_usage = buf[7];
      data.disk_read = buf[10];
      data.disk_write = buf[11];
      data.net_up = buf[12];
      data.net_down = buf[13];
      data.cpu_temp = buf[14];
      data.gpu_temp = buf[15];
      
      if (data.net_up > max_net_up) max_net_up = data.net_up;
      if (data.net_down > max_net_down) max_net_down = data.net_down;
      
      update_counter++;
      if (update_counter >= 2) { // 1 Second update for graph
        update_counter = 0;
        graph_index = (graph_index + 1) % GRAPH_WIDTH;
        graph_cpu[graph_index] = data.cpu_usage;
        graph_ram[graph_index] = data.ram_usage;
        graph_gpu[graph_index] = data.gpu_usage;
        graph_vram[graph_index] = data.vram_usage;
        drawGraph();
      }
      
      drawHalfRow(0, true, "C", data.cpu_usage, COLOR_CPU);
      drawHalfRow(0, false, "R", data.ram_usage, COLOR_RAM);
      drawHalfRow(1, true, "G", data.gpu_usage, COLOR_GPU);
      drawHalfRow(1, false, "V", data.vram_usage, COLOR_VRAM);
      drawHalfRow(2, true, "D", data.disk_read, WHITE, 99);
      drawHalfRow(2, false, "D", data.disk_write, WHITE, 99);
      drawHalfRow(3, true, "N", data.net_up, WHITE, max_net_up);
      drawHalfRow(3, false, "N", data.net_down, WHITE, max_net_down);
    }
  }
}
