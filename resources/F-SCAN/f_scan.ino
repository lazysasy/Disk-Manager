#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#define MAX_REQUESTS 100
#define CYLINDER_LIMIT 199
#define MAX_SCANS 2 

SemaphoreHandle_t xSerialMutex;

const int test_requests[] = {176, 79, 34, 60, 92, 11, 41, 114};
const int test_request_count = 8;
const int test_head = 50;
const int test_direction = 1;

void sort(int arr[], int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

void fscan_task(void *pvParameters) {
    int active_queue[MAX_REQUESTS];
    int waiting_queue[MAX_REQUESTS];
    int active_size = 0, waiting_size = 0;

    
    for (int i = 0; i < test_request_count; i++) {
        active_queue[active_size++] = test_requests[i];
    }

    int head = test_head;
    int direction = test_direction;
    int total_seek = 0;

    for (int scan_cycle = 0; scan_cycle < MAX_SCANS; scan_cycle++) {
        if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
            Serial.print("\n--- F-SCAN Cycle ");
            Serial.print(scan_cycle + 1);
            Serial.println(" ---");
            Serial.print("Active Queue: ");
            for (int i = 0; i < active_size; i++) {
                Serial.print(active_queue[i]);
                Serial.print(" ");
            }
            Serial.println();
            xSemaphoreGive(xSerialMutex);
        }
    
       
        int left[MAX_REQUESTS], right[MAX_REQUESTS];
        int left_size = 0, right_size = 0;

        for (int i = 0; i < active_size; i++) {
            if (active_queue[i] < head)
                left[left_size++] = active_queue[i];
            else
                right[right_size++] = active_queue[i];
        }

        sort(left, left_size);
        sort(right, right_size);

        if (direction == 1) {
            for (int i = 0; i < right_size; i++) {
                total_seek += abs(right[i] - head);
                head = right[i];

                if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
                    Serial.print("Serviced: ");
                    Serial.println(head);
                    xSemaphoreGive(xSerialMutex);
                }

                
                if (waiting_size < MAX_REQUESTS)
                    waiting_queue[waiting_size++] = random(0, CYLINDER_LIMIT);
            }

            for (int i = left_size - 1; i >= 0; i--) {
                total_seek += abs(left[i] - head);
                head = left[i];

                if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
                    Serial.print("Serviced: ");
                    Serial.println(head);
                    xSemaphoreGive(xSerialMutex);
                }

                if (waiting_size < MAX_REQUESTS)
                    waiting_queue[waiting_size++] = random(0, CYLINDER_LIMIT);
            }
        } else {
            for (int i = left_size - 1; i >= 0; i--) {
                total_seek += abs(left[i] - head);
                head = left[i];

                if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
                    Serial.print("Serviced: ");
                    Serial.println(head);
                    xSemaphoreGive(xSerialMutex);
                }

                if (waiting_size < MAX_REQUESTS)
                    waiting_queue[waiting_size++] = random(0, CYLINDER_LIMIT);
            }

            for (int i = 0; i < right_size; i++) {
                total_seek += abs(right[i] - head);
                head = right[i];

                if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
                    Serial.print("Serviced: ");
                    Serial.println(head);
                    xSemaphoreGive(xSerialMutex);
                }

                if (waiting_size < MAX_REQUESTS)
                    waiting_queue[waiting_size++] = random(0, CYLINDER_LIMIT);
            }
        }

       
        memcpy(active_queue, waiting_queue, sizeof(int) * waiting_size);
        active_size = waiting_size;
        waiting_size = 0;

        
        direction = !direction;
    }

    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
        Serial.print("\nTotal seek operations after ");
        Serial.print(MAX_SCANS);
        Serial.print(" F-SCAN cycles: ");
        Serial.println(total_seek);
        xSemaphoreGive(xSerialMutex);
    }

    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("Initializing F-SCAN Disk Scheduling...");

    xSerialMutex = xSemaphoreCreateMutex();

    xTaskCreate(fscan_task, "FSCAN_Task", 2048, NULL, 1, NULL);
}

void loop() {
    // FreeRTOS handles scheduling
}
