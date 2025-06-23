#include <SPI.h>
#include <SD.h>

#define DISK_SIZE 200
#define DISK1_FILE "disk1.bin"
#define DISK2_FILE "disk2.bin"
#define CS_PIN 4

File disk1, disk2;

void clearSerialBuffer() {
  while (Serial.available()) {
    Serial.read();
  }
}

String readSerialString() {
  String input = "";
  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n') {
        return input;
      }
      input += c;
    }
  }
}

void raid1_mirroring() {
  int address;
  String data;
  char read_buf[DISK_SIZE];
  
  Serial.print("\nEnter the address to write data (0-");
  Serial.print(DISK_SIZE - 1);
  Serial.print("): ");
  
  address = readSerialString().toInt();
  Serial.println(address);

  if (address < 0 || address >= DISK_SIZE) {
    Serial.print("Error: Invalid address. Must be between 0 and ");
    Serial.println(DISK_SIZE - 1);
    return;
  }

  Serial.print("Enter the data to write (string up to ");
  Serial.print(DISK_SIZE - address - 1);
  Serial.print(" characters): ");
  
  data = readSerialString();
  Serial.println(data);

  if (address + data.length() >= DISK_SIZE) {
    Serial.println("Error: Data exceeds disk size.");
    return;
  }

 
  disk1 = SD.open(DISK1_FILE, FILE_WRITE);
  if (!disk1) {
    Serial.println("Error opening disk1 for writing!");
    return;
  }
  disk1.seek(address);
  disk1.print(data);
  disk1.close();
  
  disk2 = SD.open(DISK2_FILE, FILE_WRITE);
  if (!disk2) {
    Serial.println("Error opening disk2 for writing!");
    return;
  }
  disk2.seek(address);
  disk2.print(data);
  disk2.close();

  Serial.print("Data written to both disks at address ");
  Serial.print(address);
  Serial.print(": ");
  Serial.println(data);

 
  disk1 = SD.open(DISK1_FILE, FILE_READ);
  if (!disk1) {
    Serial.println("Error opening disk1 for reading!");
    return;
  }
  disk1.seek(address);
  int bytesRead = disk1.readBytes(read_buf, data.length());
  read_buf[bytesRead] = '\0';
  disk1.close();
  
  Serial.print("Data read from disk1 at address ");
  Serial.print(address);
  Serial.print(": ");
  Serial.println(read_buf);

 
  Serial.println("\nSimulating disk1 failure...");
  Serial.println("Trying to read from disk1...");
  Serial.println("Error: disk1 failed! Switching to disk2...");

  disk2 = SD.open(DISK2_FILE, FILE_READ);
  if (!disk2) {
    Serial.println("Error opening disk2 for reading!");
    return;
  }
  disk2.seek(address);
  bytesRead = disk2.readBytes(read_buf, data.length());
  read_buf[bytesRead] = '\0';
  disk2.close();
  
  Serial.print("Data read from disk2 at address ");
  Serial.print(address);
  Serial.print(": ");
  Serial.println(read_buf);
}

void setup() {
  Serial.begin(115200);
  while (!Serial); 

  Serial.println("\n--- RAID 1: Mirroring ---");
  
  if (!SD.begin(CS_PIN)) {
    Serial.println("SD card initialization failed!");
    Serial.println("Check wiring and CS pin (default: pin 4)");
    return;
  }
  

  if (!SD.exists(DISK1_FILE)) {
    disk1 = SD.open(DISK1_FILE, FILE_WRITE);
    disk1.close();
  }
  if (!SD.exists(DISK2_FILE)) {
    disk2 = SD.open(DISK2_FILE, FILE_WRITE);
    disk2.close();
  }
  
  raid1_mirroring();
}

void loop() {
  // Empty - runs once
}