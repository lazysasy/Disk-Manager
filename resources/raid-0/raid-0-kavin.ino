#include <SPI.h>
#include <SD.h>

#define DISK_SIZE 200
#define DISK1_FILE "disk1.bin"
#define DISK2_FILE "disk2.bin"
#define CS_PIN 4
#define STRIPE_SIZE 16  // Size of each stripe block

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

void raid0_striping() {
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

  // Write data with striping
  for (int i = 0; i < data.length(); i++) {
    int current_address = address + i;
    
    // Determine which disk to write to based on stripe
    if ((current_address / STRIPE_SIZE) % 2 == 0) {
      disk1 = SD.open(DISK1_FILE, FILE_WRITE);
      if (!disk1) {
        Serial.println("Error opening disk1 for writing!");
        return;
      }
      disk1.seek(current_address / 2);  // Divide by 2 because we're striping
      disk1.write(data[i]);  // Write single character
      disk1.close();
    } else {
      disk2 = SD.open(DISK2_FILE, FILE_WRITE);
      if (!disk2) {
        Serial.println("Error opening disk2 for writing!");
        return;
      }
      disk2.seek(current_address / 2);
      disk2.write(data[i]);  // Write single character
      disk2.close();
    }
  }

  Serial.print("Data written in RAID 0 configuration at address ");
  Serial.print(address);
  Serial.print(": ");
  Serial.println(data);

  // Read back the data to verify
  Serial.println("\nReading back data in RAID 0 configuration...");
  
  String reconstructed_data = "";
  for (int i = 0; i < data.length(); i++) {
    int current_address = address + i;
    
    // Determine which disk to read from based on stripe
    if ((current_address / STRIPE_SIZE) % 2 == 0) {
      disk1 = SD.open(DISK1_FILE, FILE_READ);
      if (!disk1) {
        Serial.println("Error opening disk1 for reading!");
        return;
      }
      disk1.seek(current_address / 2);
      char c = disk1.read();
      reconstructed_data += c;
      disk1.close();
    } else {
      disk2 = SD.open(DISK2_FILE, FILE_READ);
      if (!disk2) {
        Serial.println("Error opening disk2 for reading!");
        return;
      }
      disk2.seek(current_address / 2);
      char c = disk2.read();
      reconstructed_data += c;
      disk2.close();
    }
  }

  Serial.print("Data read from RAID 0 at address ");
  Serial.print(address);
  Serial.print(": ");
  Serial.println(reconstructed_data);

  // Simulate disk failure
  Serial.println("\nSimulating disk1 failure...");
  Serial.println("Trying to read complete data...");
  
  String partial_data = "";
  for (int i = 0; i < data.length(); i++) {
    int current_address = address + i;
    
    // Skip reading from disk1 (simulated failure)
    if ((current_address / STRIPE_SIZE) % 2 == 0) {
      partial_data += '?';  // Mark as unreadable
    } else {
      disk2 = SD.open(DISK2_FILE, FILE_READ);
      if (!disk2) {
        Serial.println("Error opening disk2 for reading!");
        return;
      }
      disk2.seek(current_address / 2);
      char c = disk2.read();
      partial_data += c;
      disk2.close();
    }
  }

  Serial.print("Partial data recovered (disk1 failed): ");
  Serial.println(partial_data);
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial connection

  Serial.println("\n--- RAID 0: Striping ---");
  
  if (!SD.begin(CS_PIN)) {
    Serial.println("SD card initialization failed!");
    Serial.println("Check wiring and CS pin (default: pin 4)");
    return;
  }
  
  // Create files if they don't exist
  if (!SD.exists(DISK1_FILE)) {
    disk1 = SD.open(DISK1_FILE, FILE_WRITE);
    disk1.close();
  }
  if (!SD.exists(DISK2_FILE)) {
    disk2 = SD.open(DISK2_FILE, FILE_WRITE);
    disk2.close();
  }
  
  raid0_striping();
}

void loop() {
  // Empty - runs once
}