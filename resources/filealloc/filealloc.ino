#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#define DISK_SIZE 1000

int disk[DISK_SIZE], next[DISK_SIZE], index_table[DISK_SIZE][10];
SemaphoreHandle_t xSerialMutex;

void init_disk() {
    for (int i = 0; i < DISK_SIZE; i++) {
        disk[i] = next[i] = -1;
    }
}

void allocate_contiguous(int start, int length) {
    for (int i = 0; i < length; i++) {
        disk[start + i] = 1;
    }
    Serial.printf("Contiguous Allocation: Start=%d, Length=%d\n", start, length);
}

void allocate_linked(int blocks[], int n) {
    for (int i = 0; i < n; i++) {
        disk[blocks[i]] = 1;
        next[blocks[i]] = (i < n - 1) ? blocks[i + 1] : -1;
    }

    Serial.print("Linked Allocation: ");
    for (int i = 0; i < n; i++) {
        Serial.print(blocks[i]);
        if (i < n - 1) Serial.print(" -> ");
    }
    Serial.println();
}

void allocate_indexed(int index_block, int blocks[], int n) {
    disk[index_block] = 1;
    for (int i = 0; i < n; i++) {
        disk[blocks[i]] = 1;
        index_table[index_block][i] = blocks[i];
    }

    Serial.printf("Indexed Allocation: Index Block = %d, Blocks = ", index_block);
    for (int i = 0; i < n; i++) {
        Serial.print(index_table[index_block][i]);
        Serial.print(" ");
    }
    Serial.println();
}

void disk_allocation_task(void *pvParameters) {
    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
        init_disk();
        allocate_contiguous(100, 5);
        int linked_blocks[] = {200, 305, 404, 123};
        allocate_linked(linked_blocks, 4);
        int indexed_blocks[] = {501, 502, 503};
        allocate_indexed(500, indexed_blocks, 3);
        xSemaphoreGive(xSerialMutex);
    }

    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("Starting Disk Allocation Simulation...");

    xSerialMutex = xSemaphoreCreateMutex();
    xTaskCreate(
        disk_allocation_task,
        "DiskAlloc",
        1024,
        NULL,
        1,
        NULL
    );
}

void loop() {
    // FreeRTOS handles the task
}
