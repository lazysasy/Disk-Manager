#include <SPI.h>
#include <SD.h>

#define JOURNAL_SIZE 1000
#define JOURNAL_FILE "/journal.txt"
#define FILESYSTEM_FILE "/fs.txt"
#define CS_PIN 17  

File journalFile;
File fsFile;

void setup() {
  Serial.begin(115200);
  while (!Serial); 

  if (!SD.begin(CS_PIN)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }

 
  SD.remove(JOURNAL_FILE);
  SD.remove(FILESYSTEM_FILE);
  
  journaling();
}

void journaling() {
    char entry[JOURNAL_SIZE];

    Serial.println("Enter journal entries (type 'exit' to stop):");

    while (1) {
        Serial.print("> ");
        readLine(entry, JOURNAL_SIZE);

        if (strcmp(entry, "exit") == 0) {
            break;
        }

        
        if (getFileSize(JOURNAL_FILE) + strlen(entry) >= JOURNAL_SIZE ||
            getFileSize(FILESYSTEM_FILE) + strlen(entry) >= JOURNAL_SIZE) {
            Serial.println("Error: Journal or filesystem is full! Cannot add more entries.");
            break;
        }

       
        journalFile = SD.open(JOURNAL_FILE, FILE_WRITE);
        if (journalFile) {
            journalFile.println(entry);
            journalFile.close();
        }

        // Commit to filesystem
        fsFile = SD.open(FILESYSTEM_FILE, FILE_WRITE);
        if (fsFile) {
            fsFile.println(entry);
            fsFile.close();
        }

        Serial.print("Journal entry: ");
        Serial.println(entry);
        Serial.print("Committed to filesystem: ");
        Serial.println(entry);
    }

    Serial.println("\nSimulating crash...");
    Serial.println("Recovering from journal...");

   
    if (SD.exists(JOURNAL_FILE)) {
        journalFile = SD.open(JOURNAL_FILE, FILE_READ);
        if (journalFile) {
            while (journalFile.available()) {
                String line = journalFile.readStringUntil('\n');
                fsFile = SD.open(FILESYSTEM_FILE, FILE_WRITE);
                if (fsFile) {
                    fsFile.println(line);
                    fsFile.close();
                }
            }
            journalFile.close();
        }
    }


    Serial.println("Filesystem after recovery:");
    fsFile = SD.open(FILESYSTEM_FILE, FILE_READ);
    if (fsFile) {
        while (fsFile.available()) {
            Serial.write(fsFile.read());
        }
        fsFile.close();
    }
}


void readLine(char* buffer, size_t length) {
    size_t pos = 0;
    while (1) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') {
                buffer[pos] = '\0';
                return;
            }
            buffer[pos++] = c;
            if (pos >= length - 1) {
                buffer[length - 1] = '\0';
                return;
            }
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

void loop() {
    // Empty
}