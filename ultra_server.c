#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "gpio_utils.h"
#include "network_utils.h"

#define TRIGGER_PIN1 23
#define ECHO_PIN1 24
#define TRIGGER_PIN2 5
#define ECHO_PIN2 6

double time1, distance1;
double time2, distance2;

/**
 * Thread function to measure distance using ultrasonic sensor 1
 */
void *ultrawave1_thd() {
    clock_t start_t, end_t;
    
    // Send trigger pulse
    if (-1 == GPIOWrite(TRIGGER_PIN1, 1)) {
        printf("gpio write/trigger err\n");
        exit(0);
    }
    usleep(10);
    GPIOWrite(TRIGGER_PIN1, 0);
    
    // Measure echo pulse duration
    while(GPIORead(ECHO_PIN1) == 0) {
        start_t = clock();
    }
    while(GPIORead(ECHO_PIN1) == 1) {
        end_t = clock();
    }
    
    // Calculate distance based on time
    time1 = (double)(end_t-start_t)/CLOCKS_PER_SEC;
    distance1 = time1/2*34000;
    
    return NULL;
}

/**
 * Thread function to measure distance using ultrasonic sensor 2
 */
void *ultrawave2_thd() {
    clock_t start2_t, end2_t;
    
    // Send trigger pulse
    if (-1 == GPIOWrite(TRIGGER_PIN2, 1)) {
        printf("gpio write/trigger err\n");
        exit(0);
    }
    usleep(10);
    GPIOWrite(TRIGGER_PIN2, 0);
    
    // Measure echo pulse duration
    while(GPIORead(ECHO_PIN2) == 0) {
        start2_t = clock();
    }
    while(GPIORead(ECHO_PIN2) == 1) {
        end2_t = clock();
    }
    
    // Calculate distance based on time
    time2 = (double)(end2_t-start2_t)/CLOCKS_PER_SEC;
    distance2 = time2/2*34000;
    
    return NULL;
}

int main(int argc, char *argv[]) {
    int dist1, dist2, dist_total;
    int serv_sock, clnt_sock = -1;
    char msg[5];
    
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    
    // Initialize GPIO pins
    if (-1 == GPIOExport(TRIGGER_PIN1) || -1 == GPIOExport(ECHO_PIN1))
        return 1;
    usleep(100000);
    
    if (-1 == GPIOExport(TRIGGER_PIN2) || -1 == GPIOExport(ECHO_PIN2))
        return 2;
    usleep(100000);
    
    if (-1 == GPIODirection(TRIGGER_PIN1, OUT) || -1 == GPIODirection(ECHO_PIN1, IN))
        return 3;
    
    if (-1 == GPIODirection(TRIGGER_PIN2, OUT) || -1 == GPIODirection(ECHO_PIN2, IN))
        return 4;
    
    if (-1 == GPIOWrite(TRIGGER_PIN1, 0))
        return 5;
    usleep(10000);
    
    if (-1 == GPIOWrite(TRIGGER_PIN2, 0))
        return 6;
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
        
        // Create threads for ultrasonic distance measurements
        thr_id = pthread_create(&p_thread[0], NULL, ultrawave1_thd, NULL);
        if (thr_id < 0) {
            perror("thread create error : ");
            exit(0);
        }
        
        thr_id = pthread_create(&p_thread[1], NULL, ultrawave2_thd, NULL);
        if (thr_id < 0) {
            perror("thread create error : ");
            exit(0);
        }
        
        // Wait for threads to complete
        pthread_join(p_thread[0], (void**)&status);
        pthread_join(p_thread[1], (void**)&status);
        
        // Prepare sensor data to send to client
        dist1 = (int)distance1;
        dist2 = (int)distance2;
        dist_total = dist1 * 100 + dist2;
        printf("dist1: %d   dist2: %d\n", dist1, dist2);
        
        // Send the combined distance value to the client
        snprintf(msg, 5, "%d", dist_total);
        write(clnt_sock, msg, sizeof(msg));
        
        usleep(500000);
    }
    
    // Cleanup
    close(clnt_sock);
    close(serv_sock);
    
    if (-1 == GPIOUnexport(TRIGGER_PIN1) || -1 == GPIOUnexport(ECHO_PIN1) ||
        -1 == GPIOUnexport(TRIGGER_PIN2) || -1 == GPIOUnexport(ECHO_PIN2))
        return 7;
    
    return 0;
}
