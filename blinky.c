/*
 * blinky.c -- working with Serial I/O and GPIO
 *
 * Assumes the LED's are connected to AXI_GPIO_0, on channel 1
 *
 * Terminal Settings:
 *  -Baud: 115200
 *  -Data bits: 8
 *  -Parity: no
 *  -Stop bits: 1
 */
#include <stdio.h>							/* printf(), getchar() */
#include "xil_types.h"					/* u32, u16 etc */
#include "platform.h"						/* ZYBOboard interface */
#include <xgpio.h>							/* Xilinx GPIO functions */
#include "xparameters.h"				/* constants used by the hardware */
#include <string.h>
#include <stdlib.h>
#include "led.h"
#include "gic.h"		/* interrupt controller interface */
#include "io.h"
#include "xttcps.h"
#include "ttc.h"
#include "xtmrctr.h"
#include "servo.h"
#include "adc.h"
#include <math.h>

#define LOW 4.25
#define HIGH 8.75

static double dutycycle;

void getLine (char *str);
void callback(u32 led_num){
	led_toggle(led_num);

	if (led_num == 0){
		 float temperature = adc_get_temp();

		 printf("[Temp = %.2fc]\n\r>", temperature);
		 fflush(stdout);
	} else if (led_num == 1){
		 float voltage = adc_get_vccint();

		 printf("[VccInt = %.2fv]\n\r>", voltage);
		 fflush(stdout);
	} else if (led_num == 2){
		 float voltage = adc_get_pot();

		 printf("[Pot = %.2fv]\n\r>", voltage);
		 fflush(stdout);
	} else if (led_num == 3){
		// Potentiometer and servo
		 float voltage = adc_get_pot();
		 dutycycle = (voltage * 4.5) + 4.25;
		 printf("[%.2f]\n\r>", dutycycle);
		 fflush(stdout);
		 servo_set(dutycycle);
	}
}

void ttc_callback(void){
	led_toggle(4);
}


int main() {
	led_init();
	s32 answer = gic_init(); /* initialize the gic (c.f. gic.h) */

	if (answer == 0){
		io_btn_init(callback);
		io_sw_init(callback);
		u32 freq = 1;
		ttc_init(freq, ttc_callback);
	}

	ttc_start();

	servo_init();
	dutycycle = 6.5;
	servo_set(dutycycle);

	adc_init();

   int buff = 80;
   char str[buff];

	 /*
		* set stdin unbuffered, forcing getchar to return immediately when
		* a character is typed.
		*/
	 setvbuf(stdin,NULL,_IONBF,0);

	 printf("[Hello]\n");
	 led_set(4, LED_ON, 0);

	 led_set(0, LED_ON, 0);


	 int value = 0;
	 bool ans;

	 do {
		 if (strcmp(str, "0") == 0 || strcmp(str, "1") == 0 || strcmp(str, "2") == 0 || strcmp(str, "3") == 0){
			 if (strcmp(str, "0") == 0){
				 led_toggle(0);
				 ans = led_get(0);
			 } else if (strcmp(str, "1") == 0){
				 led_toggle(1);
				 ans = led_get(1);
			 } else if (strcmp(str, "2") == 0){
				 led_toggle(2);
				 ans = led_get(2);
			 } else if (strcmp(str, "3") == 0){
				 led_toggle(3);
				 ans = led_get(3);
			 }

			 if (ans == 1){
				 printf("[%s on]\n\r", str);
			 } else printf("[%s off]\n\r", str);

			 fflush(stdout);

		 }

		 if (strcmp(str, "r") == 0 || strcmp(str, "b") == 0 || strcmp(str, "g") == 0 || strcmp(str, "y") == 0){
			 if (strcmp(str, "r") == 0){
				 led_set(6, LED_OFF, 4);
			 } else if (strcmp(str, "b") == 0){
				 led_set(6, LED_OFF, 1);
			 } else if (strcmp(str, "g") == 0){
				 led_set(6, LED_OFF, 2);
			 } else if (strcmp(str, "y") == 0){
				 led_set(6, LED_OFF, 6);
			 }
		 }

		 if (strcmp(str, "a") == 0 || strcmp(str, "s") == 0){
			 if ((strcmp(str, "a") == 0) && ((dutycycle + 0.25) <= HIGH)){
				 dutycycle += 0.25;
			 } else if ((strcmp(str, "s") == 0) && ((dutycycle - 0.25) >= LOW)){
				 dutycycle -= 0.25;
			 }
			 printf("[%.2f]\n\r", dutycycle);
			 fflush(stdout);
			 servo_set(dutycycle);
		 }

		 if (strcmp(str, "low") == 0){
			 dutycycle =  LOW;
			 servo_set(dutycycle);
			 printf("[%.2f]\n\r", dutycycle);
			 fflush(stdout);

		 }

		 if (strcmp(str, "high") == 0){
			 dutycycle =  HIGH;
			 printf("[%.2f]\n\r", dutycycle);
			 fflush(stdout);
			 servo_set(dutycycle);

		 }

		 printf(">");
		 fflush(stdout);
		 getLine(str);
		 printf("\n");
		 fflush(stdout);
		 value = strcmp(str, "q");
	 } while (value != 0);

	 led_set(0, LED_OFF, 0);
	 led_set(4, LED_OFF, 0);
	 led_set(6, LED_OFF, 0);
	 led_set(ALL, LED_OFF, 0);

	  io_btn_close();
	  io_sw_close();
	  ttc_stop();
	  ttc_close();
	  gic_close();


   cleanup_platform();					/* cleanup the hardware platform */
   return 0;
}

void getLine(char * str){
	char c;
	int i = 0;

	c = getchar();
	printf("%c", c);
	fflush(stdout);

	while ( c != 13 ){
		str[i] = c;
		i++;
		c = getchar();
		printf("%c", c);
		fflush(stdout);
	}
	str[i] = '\0';
}
