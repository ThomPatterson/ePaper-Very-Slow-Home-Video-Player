#include "HttpController.h"
#include "Config.h"


void HttpController::init(DisplayController* displayController) {
  this->displayController = displayController;
}

void HttpController::connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    this->displayController->showMessage("Connecting to WiFi...");
    Serial.println("HttpController: Connecting to WiFi...");
  }
  this->displayController->showMessage("Connected to WiFi");
  Serial.println("HttpController: Connected to WiFi");
}

void HttpController::disconnectWiFi() {
  WiFi.disconnect();
  this->displayController->showMessage("Disconnected from WiFi");
  Serial.println("HttpController: Disconnected from WiFi");
}

//A pointer is passed in of an array to be populated by this function.
void HttpController::fetchImage(uint8_t* image) {
  int payloadSize = imageBytes;
  HTTPClient http;
  http.begin(httpEndpoint);
  int httpCode = http.GET();

  if (httpCode > 0) {  // Check for the returning code
    if (httpCode == HTTP_CODE_OK) {
      WiFiClient stream = http.getStream();

      int contentLength = http.getSize();

      int headerLength = contentLength - payloadSize;

      // Create a dummy buffer
      uint8_t dummyBuffer[headerLength];

      // Read and discard the first headerLength bytes
      stream.readBytes(dummyBuffer, headerLength);

      // Now you can read the image data
      stream.readBytes(image, payloadSize);
      Serial.println("HttpController: Successfully fetched image.");
    }
  } else {
    Serial.println("HttpController: Error on HTTP request");
  }

  http.end();  // Free the resources
}