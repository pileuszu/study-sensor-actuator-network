#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "gpio_utils.h"
#include "pwm_utils.h"
#include "network_utils.h"

#define BUTTON_PIN 20
#define MOTOR_PIN 21

int state = 1, prev_state = 1;
int button_on = 1;
int button_stop = 0;
char msg[5];

/**
 * Thread function to handle button press
 */
void *button_thd() {
	button_stop = 0;
	//button_stop = 0 while motor_thd sends 1
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
 * Thread function to control motor based on sensor data
 */
void *motor_thd() {
	int motor_on = 0;
	int dist_total = atoi(msg);
	int dist1 = dist_total / 100;
	int dist2 = dist_total % 100;
	
	printf("Distance 1: %d cm, Distance 2: %d cm\n", dist1, dist2);
	
	// Check button state
	if (button_on == 0) {
		// If button is off, turn motor off
		if (motor_on == 1) {
			PWMUnable(PWM);
			motor_on = 0;
			printf("Motor stopped (button)\n");
		}
	} else {
		// If button is on, control motor based on sensor data
		if (dist1 <= 30 || dist2 <= 30) {
			if (motor_on == 0) {
				PWMEnable(PWM);
				motor_on = 1;
				printf("Motor started (sensor)\n");
			}
		} else {
			if (motor_on == 1) {
				PWMUnable(PWM);
				motor_on = 0;
				printf("Motor stopped (sensor)\n");
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
	
	// Setup GPIO and PWM
	if (-1 == GPIOExport(BUTTON_PIN))
		return 1;
	usleep(100000);
	
	if (-1 == GPIODirection(BUTTON_PIN, IN))
		return 2;
	usleep(10000);
	
	if (-1 == PWMExport(PWM))
		return 3;
	usleep(10000);
	
	if (-1 == PWMWritePeriod(PWM, 20000000))
		return 4;
	usleep(10000);
	
	if (-1 == PWMWriteDutyCycle(PWM, 1500000))
		return 5;
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
		
		// Create thread for motor control
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
	
	if (-1 == PWMUnable(PWM))
		return 6;
	usleep(10000);
	
	if (-1 == PWMUnexport(PWM))
		return 7;
	usleep(10000);
	
	if (-1 == GPIOUnexport(BUTTON_PIN))
		return 8;
	
	return 0;
}
