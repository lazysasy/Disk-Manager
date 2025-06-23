#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#define MAX_REQUESTS 100
#define CYLINDER_LIMIT 199


SemaphoreHandle_t xSerialMutex;


const int test_requests[] = {176, 79, 34, 60, 92, 11, 41, 114};
const int test_request_count = 8;
const int test_head = 50;
const int test_direction = 1; 

void cscan_task(void *pvParameters) {
    int requests[MAX_REQUESTS];
    int request_count = test_request_count;
    int head = test_head;
    int direction = test_direction;
    
   
    for (int i = 0; i < request_count; i++) {
        requests[i] = test_requests[i];
    }

    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
        Serial.println("--- C-SCAN Disk Scheduling ---");
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

    int left[MAX_REQUESTS + 1], right[MAX_REQUESTS + 1];
    int left_size = 0, right_size = 0;
    int seek_sequence[MAX_REQUESTS + 2];
    int seek_sequence_size = 0;
    int seek_count = 0;

    right[right_size++] = CYLINDER_LIMIT;
    left[left_size++] = 0;

    for (int i = 0; i < request_count; i++) {
        if (requests[i] < head) {
            left[left_size++] = requests[i];
        } else {
            right[right_size++] = requests[i];
        }
    }

   
    for (int i = 0; i < left_size - 1; i++) {
        for (int j = 0; j < left_size - i - 1; j++) {
            if (left[j] > left[j + 1]) {
                int temp = left[j];
                left[j] = left[j + 1];
                left[j + 1] = temp;
            }
        }
    }


    for (int i = 0; i < right_size - 1; i++) {
        for (int j = 0; j < right_size - i - 1; j++) {
            if (right[j] > right[j + 1]) {
                int temp = right[j];
                right[j] = right[j + 1];
                right[j + 1] = temp;
            }
        }
    }

    if (direction == 1) {  
        for (int i = 0; i < right_size; i++) {
            seek_sequence[seek_sequence_size++] = right[i];
            seek_count += abs(right[i] - head);
            head = right[i];
        }

        seek_count += abs(CYLINDER_LIMIT - 0);  
        head = 0;

        for (int i = 0; i < left_size; i++) {
            seek_sequence[seek_sequence_size++] = left[i];
            seek_count += abs(left[i] - head);
            head = left[i];
        }
    } else {  
        for (int i = left_size - 1; i >= 0; i--) {
            seek_sequence[seek_sequence_size++] = left[i];
            seek_count += abs(left[i] - head);
            head = left[i];
        }

        seek_count += abs(0 - CYLINDER_LIMIT);  
        head = CYLINDER_LIMIT;

        for (int i = right_size - 1; i >= 0; i--) {
            seek_sequence[seek_sequence_size++] = right[i];
            seek_count += abs(right[i] - head);
            head = right[i];
        }
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
    while (!Serial); 

    Serial.println("Initializing C-SCAN Disk Scheduling...");

 
    xSerialMutex = xSemaphoreCreateMutex();

    // Create C-SCAN task
    xTaskCreate(cscan_task,
                "C-SCAN Task",
                1024,  
                NULL,
                1,     
                NULL);
}

void loop() {
    // Not used - FreeRTOS scheduler handles tasks
}