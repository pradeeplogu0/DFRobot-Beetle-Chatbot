#include <SoftwareSerial.h>
#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)

// Create a SoftwareSerial object on pins 2 (RX) and 3 (TX)
SoftwareSerial mySerial(2, 3); // RX, TX

// Initialize the TFT screen
TFT_eSPI tft = TFT_eSPI();  // Create object "tft"

// Screen dimensions
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);

  // Initialize the TFT screen
  tft.begin();
  tft.setRotation(3); // Set the screen orientation (1 for landscape mode)
  tft.fillScreen(TFT_BLACK); // Clear the screen with black color
  tft.setTextColor(TFT_WHITE); // Set text color to white
  tft.setTextSize(2); // Set text size

  // Print initial message to the screen
  tft.drawString("Waiting for data...", 10, 10);
}

void loop() {
  while (mySerial.available()) {
    String str = mySerial.readString(); // Read the incoming data as a string
    str.trim();
    Serial.println(str);

    // Clear the screen and display the new data with text wrapping
    tft.fillScreen(TFT_BLACK); // Clear the screen with black color
    printWrappedText(str, 10, 10, SCREEN_WIDTH - 20, 2);

    mySerial.println("initialization done.");
  }
}

// Function to print wrapped text on the screen
void printWrappedText(String str, int x, int y, int lineWidth, int textSize) {
  tft.setCursor(x, y);
  tft.setTextSize(textSize);

  int cursorX = x;
  int cursorY = y;
  int spaceWidth = tft.textWidth(" ");
  int maxLineWidth = lineWidth;

  String word;
  for (int i = 0; i < str.length(); i++) {
    if (str[i] == ' ' || str[i] == '\n') {
      int wordWidth = tft.textWidth(word);

      if (cursorX + wordWidth > maxLineWidth) {
        cursorX = x;
        cursorY += tft.fontHeight();
      }

      tft.drawString(word, cursorX, cursorY);
      cursorX += wordWidth + spaceWidth;

      if (str[i] == '\n') {
        cursorX = x;
        cursorY += tft.fontHeight();
      }

      word = "";
    } else {
      word += str[i];
    }
  }

  if (word.length() > 0) {
    int wordWidth = tft.textWidth(word);
    if (cursorX + wordWidth > maxLineWidth) {
      cursorX = x;
      cursorY += tft.fontHeight();
    }
    tft.drawString(word, cursorX, cursorY);
  }
}
