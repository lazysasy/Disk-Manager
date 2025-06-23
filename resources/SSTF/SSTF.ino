#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#define MAX_REQUESTS 20
#define DISK_SIZE 200

SemaphoreHandle_t xSerialMutex;
TaskHandle_t xSSTFTaskHandle;

int requests[MAX_REQUESTS];
int request_count = 0;
int head = 0;

void read_input() {
    Serial.println("\nEnter disk requests (comma-separated, max 20, e.g., 98,183,37,122,14):");
    while (Serial.available() == 0) {
        delay(10);
    }
    String input = Serial.readStringUntil('\n');
    input.trim();

    char input_buffer[100];
    input.toCharArray(input_buffer, sizeof(input_buffer));

    request_count = 0;
    char *token = strtok(input_buffer, ",");
    while (token != NULL && request_count < MAX_REQUESTS) {
        requests[request_count++] = atoi(token);
        token = strtok(NULL, ",");
    }

    Serial.println("\nEnter initial head position (0-199):");
    while (Serial.available() == 0) {
        delay(10);
    }
    head = Serial.parseInt();
    while (Serial.read() != '\n'); 

    head = constrain(head, 0, DISK_SIZE - 1);

    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
        Serial.println("\nInput Summary:");
        Serial.print("Requests: ");
        for (int i = 0; i < request_count; i++) {
            Serial.print(requests[i]);
            if (i < request_count - 1) Serial.print(", ");
        }
        Serial.print("\nHead Position: ");
        Serial.println(head);
        xSemaphoreGive(xSerialMutex);
    }
}

void sstf_task(void *pvParameters) {
    int local_requests[MAX_REQUESTS];
    bool completed[MAX_REQUESTS] = { false };
    int local_request_count = request_count;
    int local_head = head;

    for (int i = 0; i < local_request_count; i++) {
        local_requests[i] = requests[i];
    }

    int seek_sequence[MAX_REQUESTS];
    int seek_count = 0;
    int sequence_index = 0;

    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
        Serial.println("\n--- SSTF Disk Scheduling ---");
        Serial.print("Initial Head Position: ");
        Serial.println(local_head);
        Serial.print("Requests: ");
        for (int i = 0; i < local_request_count; i++) {
            Serial.print(local_requests[i]);
            if (i < local_request_count - 1) Serial.print(", ");
        }
        Serial.println();
        xSemaphoreGive(xSerialMutex);
    }

    for (int i = 0; i < local_request_count; i++) {
        int min_seek = INT_MAX;
        int closest_index = -1;

        for (int j = 0; j < local_request_count; j++) {
            if (!completed[j]) {
                int distance = abs(local_requests[j] - local_head);
                if (distance < min_seek) {
                    min_seek = distance;
                    closest_index = j;
                }
            }
        }

        if (closest_index != -1) {
            completed[closest_index] = true;
            seek_sequence[sequence_index++] = local_requests[closest_index];
            seek_count += abs(local_requests[closest_index] - local_head);
            local_head = local_requests[closest_index];
        }
    }

    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
        Serial.println("\nSSTF Seek Sequence:");
        Serial.print(head);
        for (int i = 0; i < sequence_index; i++) {
            Serial.print(" -> ");
            Serial.print(seek_sequence[i]);
        }

        Serial.print("\nTotal Seek Operations: ");
        Serial.println(seek_count);
        Serial.print("Average Seek Length: ");
        Serial.println((float)seek_count / request_count, 2);
        Serial.println("SSTF scheduling complete!");
        xSemaphoreGive(xSerialMutex);
    }

    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("Disk Scheduling Algorithm Simulator");
    Serial.println("==================================");
    Serial.println("SSTF (Shortest Seek Time First) Algorithm");

    xSerialMutex = xSemaphoreCreateMutex();

    read_input();

    xTaskCreate(sstf_task,
                "SSTF Task",
                1024,
                NULL,
                1,
                &xSSTFTaskHandle);

    vTaskStartScheduler();
}

void loop() {
    // FreeRTOS handles tasks
}
