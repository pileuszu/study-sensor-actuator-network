#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "gpio_utils.h"
#include "network_utils.h"

#define WATER_SENSOR_PIN 23
#define MOTION_SENSOR_PIN 26

int water_state = 0, motion_state = 0;

/**
 * Thread function to read water sensor
 */
void *water_thd() {
    water_state = GPIORead(WATER_SENSOR_PIN);
    usleep(500000);
    return NULL;
}

/**
 * Thread function to read motion sensor
 */
void *motion_thd() {
    motion_state = GPIORead(MOTION_SENSOR_PIN);
    usleep(500000);
    return NULL;
}

int main(int argc, char *argv[]) {
    int state;
    int serv_sock, clnt_sock = -1;
    char msg[3];
    
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    
    // Initialize GPIO pins
    if (-1 == GPIOExport(WATER_SENSOR_PIN) || -1 == GPIOExport(MOTION_SENSOR_PIN))
        return 1;
    usleep(100000);
    
    if (-1 == GPIODirection(WATER_SENSOR_PIN, IN) || -1 == GPIODirection(MOTION_SENSOR_PIN, IN))
        return 2;
    usleep(10000);
   
    // Setup server socket
    serv_sock = setup_server_socket(atoi(argv[1]));
    
    // Accept client connection
    if (clnt_sock < 0) {
        clnt_sock = accept_client(serv_sock);
    }
    
    // Main loop - read sensors and send data to client
    while (1) {       
        pthread_t p_thread[2];
        int thr_id;
        int status;

        // Create thread for water sensor reading
        thr_id = pthread_create(&p_thread[0], NULL, water_thd, NULL);
        if (thr_id < 0) {
            perror("thread create error : ");
            exit(0);
        }
        
        // Create thread for motion sensor reading
        thr_id = pthread_create(&p_thread[1], NULL, motion_thd, NULL);
        if (thr_id < 0) {
            perror("thread create error : ");
            exit(0);
        }
        
        // Wait for threads to complete
        pthread_join(p_thread[0], (void**)&status);
        pthread_join(p_thread[1], (void**)&status);
        
        printf("water: %d   motion: %d\n", water_state, motion_state);
        
        // Encode state into a message: water_state*10 + motion_state
        state = water_state * 10 + motion_state;
        snprintf(msg, 3, "%d", state);
        
        // Send the state to the client
        write(clnt_sock, msg, sizeof(msg));

        usleep(5000);
    }
    
    // Cleanup
    close(clnt_sock);
    close(serv_sock);
    
    if (-1 == GPIOUnexport(WATER_SENSOR_PIN) || -1 == GPIOUnexport(MOTION_SENSOR_PIN))
        return 7;
    
    return 0;
}
