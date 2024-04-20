#ifndef STORAGECONTROLLER_H
#define STORAGECONTROLLER_H

#include <SPIFFS.h>
#include "DisplayController.h"
#include <string>


class StorageController {
private:
  DisplayController* displayController;
  std::string zeroPad(int num, int size);
public:
  void init(DisplayController* displayController);
  void showAvailableSpace();
  void purgeCache();
  bool cacheHasImage();
  int getLargestFileNumber();
  int getSmallestFileNumber();
  int getCacheSize();
  bool cacheHasRoomForAnotherImage();
  void writeImageToCache(uint8_t* image);
  bool getNextImage(uint8_t* image);
};

#endif