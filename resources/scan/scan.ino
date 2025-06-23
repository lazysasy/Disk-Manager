#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#define MAX_REQUESTS 100
#define CYLINDER_LIMIT 199

// Global variables
SemaphoreHandle_t xSerialMutex;

// Simulated disk requests
const int test_requests[] = {176, 79, 34, 60, 92, 11, 41, 114};
const int test_request_count = 8;
const int test_head = 50;
const int test_direction = 1; // 1 for right, 0 for left

void scan_task(void *pvParameters) {
    int requests[MAX_REQUESTS];
    int request_count = test_request_count;
    int head = test_head;
    int direction = test_direction;
    
    // Copy test requests
    for (int i = 0; i < request_count; i++) {
        requests[i] = test_requests[i];
    }

    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
        Serial.println("--- SCAN Disk Scheduling ---");
        Serial.print("Requests: ");
        for (int i = 0; i < request_count; i++) {
            Serial.print(requests[i]);
            Serial.print(" ");
        }
        Serial.println();
        Serial.print("Head: ");
        Serial.print(head);
        Serial.print(", Direction: ");
        Serial.println(direction ? "Right" : "Left");
        xSemaphoreGive(xSerialMutex);
    }

    int left[MAX_REQUESTS], right[MAX_REQUESTS];
    int left_size = 0, right_size = 0;
    int seek_sequence[MAX_REQUESTS + 1];
    int seek_sequence_size = 0;
    int seek_count = 0;

    // Add end points if needed (SCAN doesn't automatically go to cylinder limit)
    for (int i = 0; i < request_count; i++) {
        if (requests[i] < head) {
            left[left_size++] = requests[i];
        } else {
            right[right_size++] = requests[i];
        }
    }

    // Sort left array (descending order for left movement)
    for (int i = 0; i < left_size - 1; i++) {
        for (int j = 0; j < left_size - i - 1; j++) {
            if (left[j] < left[j + 1]) {
                int temp = left[j];
                left[j] = left[j + 1];
                left[j + 1] = temp;
            }
        }
    }

    // Sort right array (ascending order for right movement)
    for (int i = 0; i < right_size - 1; i++) {
        for (int j = 0; j < right_size - i - 1; j++) {
            if (right[j] > right[j + 1]) {
                int temp = right[j];
                right[j] = right[j + 1];
                right[j + 1] = temp;
            }
        }
    }

    // Run the while loop twice - once for each direction
    int run = 2;
    while (run--) {
        if (direction == 1) {  // Moving right
            for (int i = 0; i < right_size; i++) {
                seek_sequence[seek_sequence_size++] = right[i];
                seek_count += abs(right[i] - head);
                head = right[i];
            }
            
            // Only go to end if there are requests in left direction
            if (left_size > 0) {
                seek_count += abs(CYLINDER_LIMIT - head);
                head = CYLINDER_LIMIT;
            }
        } else {  // Moving left
            for (int i = 0; i < left_size; i++) {
                seek_sequence[seek_sequence_size++] = left[i];
                seek_count += abs(left[i] - head);
                head = left[i];
            }
            
            // Only go to start if there are requests in right direction
            if (right_size > 0) {
                seek_count += abs(0 - head);
                head = 0;
            }
        }
        
        // Reverse direction for next pass
        direction = !direction;
    }

    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
        Serial.print("\nTotal number of seek operations = ");
        Serial.println(seek_count);
        Serial.print("Seek Sequence: ");
        for (int i = 0; i < seek_sequence_size; i++) {
            Serial.print(seek_sequence[i]);
            if (i < seek_sequence_size - 1) Serial.print(" -> ");
        }
        Serial.println();
        xSemaphoreGive(xSerialMutex);
    }

    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);
    while (!Serial); // Wait for serial connection

    Serial.println("Initializing SCAN Disk Scheduling...");

    // Create mutex for serial output
    xSerialMutex = xSemaphoreCreateMutex();

    // Create SCAN task
    xTaskCreate(scan_task,
                "SCAN Task",
                1024,  // Stack size
                NULL,
                1,     // Priority
                NULL);
}

void loop() {
    // Not used - FreeRTOS scheduler handles tasks
}