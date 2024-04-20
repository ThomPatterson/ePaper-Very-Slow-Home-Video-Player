#include "StorageController.h"
#include "Config.h"

int cacheFileNumber = 0;

void StorageController::init(DisplayController* displayController) {
  this->displayController = displayController;

  //SPIFFS.format();//this might only be needed the very first time a board is flashed.  Use partition scheme of "No OTA (1 MB APP/3MB SPIFFS)"

  if (!SPIFFS.begin(true)) {
    this->displayController->showMessage("An error occurred while mounting SPIFFS");
    return;
  }

  //showAvailableSpace();

  if (this->cacheHasImage()) {
    cacheFileNumber = this->getLargestFileNumber() + 1;
    Serial.println(String("StorageController: Lowest file number: ") + String(this->getSmallestFileNumber()));
    Serial.println(String("StorageController: Highest file number: ") + String(this->getLargestFileNumber()));
  }
}

void StorageController::purgeCache() {
  this->displayController->showMessage("Formatting SPIFFS");
  SPIFFS.format();
  cacheFileNumber = 0;
}

void StorageController::showAvailableSpace() {
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  size_t availableBytes = totalBytes - usedBytes;

  //TODO: look into combining this into a single multi-line string
  this->displayController->showMessage(("Total SPIFFS space: " + std::to_string(totalBytes / 1024) + " KB").c_str());
  this->displayController->showMessage(("Used SPIFFS space: " + std::to_string(usedBytes / 1024) + " KB").c_str());
  this->displayController->showMessage(("Available SPIFFS space: " + std::to_string(availableBytes / 1024) + " KB").c_str());
}

bool StorageController::cacheHasImage() {
  File root = SPIFFS.open("/");
  if (!root) {
    return false;
  }

  if (!root.isDirectory()) {
    return false;
  }

  File file = root.openNextFile();
  bool hasImage = (file != NULL);

  file.close();
  root.close();

  if (!hasImage) {
    cacheFileNumber = 0;  //reset the file number to 0 when the cache runs dry
  }

  return hasImage;
}

int StorageController::getLargestFileNumber() {
  File root = SPIFFS.open("/");
  if (!root) {
    return -1;
  }

  if (!root.isDirectory()) {
    return -1;
  }

  int largestNumber = -1;
  File file = root.openNextFile();
  while (file) {
    std::string filename = file.name();
    //Serial.println(filename.c_str());
    int start = filename.find("image") + 5;
    int end = filename.find(".bmp");
    int number = std::stoi(filename.substr(start, end - start));
    if (number > largestNumber) {
      largestNumber = number;
    }
    file = root.openNextFile();
  }

  //Serial.println("Largest number is ");
  //Serial.println(largestNumber);

  file.close();
  root.close();
  return largestNumber;
}

int StorageController::getSmallestFileNumber() {
  File root = SPIFFS.open("/");
  if (!root) {
    return -1;
  }

  if (!root.isDirectory()) {
    return -1;
  }

  int smallestNumber = INT_MAX;
  File file = root.openNextFile();
  while (file) {
    std::string filename = file.name();
    int start = filename.find("image") + 5;
    int end = filename.find(".bmp");
    int number = std::stoi(filename.substr(start, end - start));
    if (number < smallestNumber) {
      smallestNumber = number;
    }
    file = root.openNextFile();
  }

  file.close();
  root.close();
  return (smallestNumber == INT_MAX) ? -1 : smallestNumber;
}

//return a count of how many files are cached
int StorageController::getCacheSize() {
  File root = SPIFFS.open("/");
  if (!root) {
    return -1;
  }

  if (!root.isDirectory()) {
    return -1;
  }

  int fileCount = 0;
  File file = root.openNextFile();
  while (file) {
    fileCount++;
    file = root.openNextFile();
  }

  file.close();
  root.close();
  return fileCount;
}

bool StorageController::cacheHasRoomForAnotherImage() {
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  size_t availableBytes = totalBytes - usedBytes;
  Serial.println(String("StorageController: Available bytes of storage: ") + String(availableBytes));
  return (availableBytes > 49000);  //leave a little headroom
}

void StorageController::writeImageToCache(uint8_t* image) {
  // Generate a unique filename
  std::string filename = "/image" + zeroPad(cacheFileNumber++, 4) + ".bmp";

  // Open the file in write mode
  File file = SPIFFS.open(filename.c_str(), FILE_WRITE);

  if (!file) {
    this->displayController->showMessage("Failed to open file for writing");
    Serial.println(("StorageController: Failed to open file " + filename + " for writing.  Purging cache.").c_str());
    purgeCache();  //something is likely corrupt in flash memory, blow it all away
    return;
  }

  // Write the image data to the file and check the number of bytes written
  size_t bytesWritten = file.write(image, imageBytes);
  file.close();  // Close the file

  if (bytesWritten < imageBytes) {
    this->displayController->showMessage("Failed to write image!");
    Serial.println(("StorageController: Failed to write entire image to file " + filename + ". Bytes written: " + String(bytesWritten).c_str()).c_str());
    Serial.println("Assuming filesystem corruption.  Purging cache.");
    purgeCache();  //something is likely corrupt in flash memory, blow it all away
  } else {
    this->displayController->showMessage(("Image written to " + filename).c_str());
    Serial.println(("StorageController: Successfully wrote image to file " + filename).c_str());
  }
}

bool StorageController::getNextImage(uint8_t* image) {

  int lowestFileNumber = this->getSmallestFileNumber();
  if (lowestFileNumber == -1) {
    Serial.println("StorageController: Unable to determine lowest file number.");
    return false;
  }

  // Generate a unique filename
  std::string lowestFilename = "/image" + zeroPad(lowestFileNumber, 4) + ".bmp";

  if (!lowestFilename.empty()) {

    Serial.println(String("StorageController: Reading ") + lowestFilename.c_str());

    // Double check if the file exists
    if (!SPIFFS.exists(lowestFilename.c_str())) {
      Serial.println(String("StorageController: This shouldn't be possible.  File does not exist: ") + lowestFilename.c_str());
      return false;
    }

    // Open the file
    File imageFile = SPIFFS.open(lowestFilename.c_str());
    if (!imageFile) {
      Serial.println("StorageController: Failed to open image file.  Deleting it.");
      SPIFFS.remove(lowestFilename.c_str());  // Delete the file
      return false;
    }

    // Check the file size
    if (imageFile.size() == 0) {
      Serial.println("StorageController: File is empty.  Deleting it.");
      SPIFFS.remove(lowestFilename.c_str());  // Delete the file
      return false;
    }

    // Read the file
    size_t bytesRead = imageFile.read(image, imageBytes);
    if (bytesRead == 0) {
      Serial.println("StorageController: Failed to read image file.  Deleting it.");
      SPIFFS.remove(lowestFilename.c_str());  // Delete the file
      return false;
    } else if (bytesRead == -1) {
      Serial.println("StorageController: Error occurred while reading file.  Deleting it.");
      SPIFFS.remove(lowestFilename.c_str());  // Delete the file
      return false;
    }

    imageFile.close();                      //free resources
    SPIFFS.remove(lowestFilename.c_str());  // Delete the file.  Think of this as popping off the queue.
  } else {
    Serial.println("StorageController: This shouldn't be possible.  No file was found");
    return false;
  }

  return true;
}

std::string StorageController::zeroPad(int num, int size) {
  std::string s = std::to_string(num);
  while (s.size() < size) s = "0" + s;
  return s;
}