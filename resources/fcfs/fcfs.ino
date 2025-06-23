#include <SD.h>
#include <SPI.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <Arduino.h>

#define CS_PIN 17
#define JOURNAL_FILE "/journal.txt"
#define FILESYSTEM_FILE "/fs.txt"
#define JOURNAL_SIZE 1000

File journalFile, fsFile;

QueueHandle_t dataQueue;

struct SensorData {
  float temperature;
  int moisture;
};

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!SD.begin(CS_PIN)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }

  Serial.println("Checking for crash recovery...");
  recoverFromJournal();

  dataQueue = xQueueCreate(10, sizeof(SensorData));
  if (dataQueue == NULL) {
    Serial.println("Failed to create queue!");
    while (1);
  }

  xTaskCreate(sensorTask, "SensorTask", 1024, NULL, 1, NULL);
  xTaskCreate(logTask, "LogTask", 2048, NULL, 1, NULL);
}

void loop() {
  
}

void sensorTask(void *pvParameters) {
  SensorData data;
  while (1) {
   
    data.temperature = 24.0 + (rand() % 100) / 10.0;  
    data.moisture = 300 + rand() % 200;              

    xQueueSend(dataQueue, &data, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(5000));  
  }
}

void logTask(void *pvParameters) {
  SensorData data;
  char logEntry[100];

  while (1) {
    if (xQueueReceive(dataQueue, &data, portMAX_DELAY)) {
      sprintf(logEntry, "Temp: %.2f, Moisture: %d, Action: %s",
              data.temperature,
              data.moisture,
              (data.moisture < 400) ? "Pump ON" : "Pump OFF");

      Serial.println(logEntry);

      if (getFileSize(JOURNAL_FILE) + strlen(logEntry) >= JOURNAL_SIZE ||
          getFileSize(FILESYSTEM_FILE) + strlen(logEntry) >= JOURNAL_SIZE) {
        Serial.println("Storage full!");
        continue;
      }

      
      journalFile = SD.open(JOURNAL_FILE, FILE_WRITE);
      if (journalFile) {
        journalFile.println(logEntry);
        journalFile.close();
      }

      
      fsFile = SD.open(FILESYSTEM_FILE, FILE_WRITE);
      if (fsFile) {
        fsFile.println(logEntry);
        fsFile.close();
      }
    }
  }
}

void recoverFromJournal() {
  if (SD.exists(JOURNAL_FILE)) {
    journalFile = SD.open(JOURNAL_FILE, FILE_READ);
    if (journalFile) {
      fsFile = SD.open(FILESYSTEM_FILE, FILE_WRITE);  
      if (fsFile) {
        
        fsFile.seek(fsFile.size());
        
        while (journalFile.available()) {
          String line = journalFile.readStringUntil('\n');
          fsFile.println(line);
        }
        fsFile.close();
      }
      journalFile.close();
      SD.remove(JOURNAL_FILE);
      Serial.println("Recovery complete.");
    }
  }
}
uint32_t getFileSize(const char* filename) {
  File file = SD.open(filename, FILE_READ);
  if (!file) return 0;
  uint32_t size = file.size();
  file.close();
  return size;
}