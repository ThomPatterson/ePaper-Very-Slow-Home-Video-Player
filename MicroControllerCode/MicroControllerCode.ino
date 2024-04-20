#include "Config.h"
#include "StorageController.h"
#include "HttpController.h"
#include "DisplayController.h"
#include <esp_sleep.h>

// image array for fetched images
uint8_t image[imageBytes];

StorageController storageController;
HttpController httpController;
DisplayController displayController;
int failCount;

void setup() {
  Serial.begin(115200);
  delay(1000);  //short delay as next line wasn't showing up in Serial Monitor
  Serial.println("MainController: Waking Up.");
  delay(100);  //short as display init is very fast and was clobbering above println
  displayController.init();
  httpController.init(&displayController);
  storageController.init(&displayController);
  failCount = 0;
  Serial.println("MainController: Setup complete");
}

void loop() {
  if (failCount >= 3) {
    Serial.println("MainController: Too many failures.  Purging cache.");
    storageController.purgeCache();
    failCount = 0;
  }
  if (storageController.cacheHasImage()) {
    bool gotImage = storageController.getNextImage(image);
    if (gotImage) {
      displayController.displayImage(image);

      if (!storageController.cacheHasImage()) {//repopulate cache if necessary
        populateCache();
      }

      displayNumberOfImagesInCache();
      delay(5000);
      displayController.dismissMessage();
      goToSleep();
    } else {
      failCount++;
      Serial.println("MainController: Failed to get image.  Allowing another loop iteration.");
    }
  } else {
    populateCache();
  }
}

void displayNumberOfImagesInCache() {
  int cacheSize = storageController.getCacheSize();
  std::string cacheSizeStr = std::to_string(cacheSize);
  const char* numberCStr = cacheSizeStr.c_str();
  displayController.showMessage((String("Images in cache: ") + numberCStr).c_str());
}

void populateCache() {
  httpController.connectWiFi();
  while (storageController.cacheHasRoomForAnotherImage()) {
    httpController.fetchImage(image);
    storageController.writeImageToCache(image);
  }
  httpController.disconnectWiFi();
}

void goToSleep() {
  Serial.println("MainController: Going to sleep.");
  esp_sleep_enable_timer_wakeup(deepSleepTime);  // Time in microseconds
  displayController.powerDown();                 //power down the display
  esp_deep_sleep_start();                        // Enter deep sleep mode
}