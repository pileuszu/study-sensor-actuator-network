#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "gpio_utils.h"
#include "pwm_utils.h"
#include "network_utils.h"

#define BUTTON_PIN 20
#define PUMP_PIN 21
#define LED_PIN 24

int state = 1, prev_state = 1;
int motion_state = 0, motion_prev_state = 0;
int button_on = 1;
int button_stop = 0;
char msg[3];

/**
 * Thread function to handle button press
 */
void *button_thd() {
	button_stop = 0;
	// button_stop = 0 while motor_thd sends 1
	while (button_stop == 0) {
		state = GPIORead(BUTTON_PIN);
		if (prev_state == 0 && state == 1) {
			button_on = (button_on+1)%2;
			printf("Button pressed! button_on = %d\n", button_on);
		}
		prev_state = state;
		usleep(50000);
	}
	return NULL;
}

/**
 * Thread function to control water pump based on sensor data
 */
void *motor_thd() {
	int motor_on = 0;
	int sensor_state = atoi(msg);
	int water_state = sensor_state / 10;
	int motion_state = sensor_state % 10;
	
	printf("Water: %d   Motion: %d   Button: %d\n", water_state, motion_state, button_on);
	
	// Check button state
	if (button_on == 0) {
		// If button is off, turn pump off
		if (motor_on == 1) {
			GPIOWrite(PUMP_PIN, 0);
			GPIOWrite(LED_PIN, 0);
			motor_on = 0;
			printf("Pump stopped (button)\n");
		}
	} else {
		// If button is on, control pump based on sensor data
		if (water_state == 1 && motion_state == 1) {
			// Water detected and motion detected - turn on pump
			if (motor_on == 0) {
				GPIOWrite(PUMP_PIN, 1);
				GPIOWrite(LED_PIN, 1);
				motor_on = 1;
				printf("Pump started (water + motion)\n");
			}
		} else {
			// Otherwise, turn off pump
			if (motor_on == 1) {
				GPIOWrite(PUMP_PIN, 0);
				GPIOWrite(LED_PIN, 0);
				motor_on = 0;
				printf("Pump stopped (no water or no motion)\n");
			}
		}
	}
	
	button_stop = 1;
	return NULL;
}

int main(int argc, char *argv[]) {
	int sock;
	pthread_t p_thread[2];
	int thr_id;
	int status;
	
	if (argc != 3) {
		printf("Usage: %s <IP> <port>\n", argv[0]);
		return 1;
	}
	
	// Setup GPIO
	if (-1 == GPIOExport(BUTTON_PIN) || -1 == GPIOExport(PUMP_PIN) || -1 == GPIOExport(LED_PIN))
		return 1;
	usleep(100000);
	
	if (-1 == GPIODirection(BUTTON_PIN, IN) || -1 == GPIODirection(PUMP_PIN, OUT) || -1 == GPIODirection(LED_PIN, OUT))
		return 2;
	usleep(10000);
	
	if (-1 == GPIOWrite(PUMP_PIN, 0) || -1 == GPIOWrite(LED_PIN, 0))
		return 3;
	usleep(10000);
	
	// Connect to server
	sock = connect_to_server(argv[1], atoi(argv[2]));
	
	// Main loop
	while (1) {
		int str_len = read(sock, msg, sizeof(msg));
		if (str_len <= 0) {
			break;
		}
		
		// Create thread for button monitoring
		thr_id = pthread_create(&p_thread[0], NULL, button_thd, NULL);
		if (thr_id < 0) {
			perror("thread create error : ");
			exit(0);
		}
		
		// Create thread for pump control
		thr_id = pthread_create(&p_thread[1], NULL, motor_thd, NULL);
		if (thr_id < 0) {
			perror("thread create error : ");
			exit(0);
		}
		
		// Wait for threads to complete
		pthread_join(p_thread[0], (void**)&status);
		pthread_join(p_thread[1], (void**)&status);
		
		// Short delay
		usleep(100000);
	}
	
	// Cleanup
	close(sock);
	
	// Turn off pump and LED
	GPIOWrite(PUMP_PIN, 0);
	GPIOWrite(LED_PIN, 0);
	
	if (-1 == GPIOUnexport(BUTTON_PIN) || -1 == GPIOUnexport(PUMP_PIN) || -1 == GPIOUnexport(LED_PIN))
		return 4;
	
	return 0;
}

