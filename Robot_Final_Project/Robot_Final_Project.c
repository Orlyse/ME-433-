#include <stdio.h>
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "cam.h"

#define PWMR 19 
#define PWML 20
#define PHASER 17
#define PHASEL 16
#define BASE_SPEED 25

void pin_init(){
    gpio_init(PHASER);
    gpio_init(PHASEL);

    gpio_set_dir(PHASEL, GPIO_OUT);
    gpio_set_dir(PHASER, GPIO_OUT);
}

void set_pwm(int pin, uint16_t wrap, float divider, float level){
    // freq = 150M / Divider*wrap
    // Duty cyce = Level/wrap

    gpio_set_function(pin, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(pin); // Get PWM slice number
    //float div = 3; // must be between 1-255
    pwm_set_clkdiv(slice_num, divider); // divider
    //uint16_t wrap = 50000; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM
    pwm_set_gpio_level(pin, level); // 0 < level < wrap
}

void read_camera(int *cm, int *srt, int *stp, int *cm1, int *srt1, int *stp1){
    setSaveImage(1);
    while(getSaveImage()==1){}
    convertImage();
    //printImage();

    int s, e, s1, e1;
    
    int com = findLine(10, &s, &e); // calculate the position of the center of the ine
    printf("Com = %d\r\n", com); // comment this when testing with python

    int com1 = findLine(40, &s1, &e);
    printf("Com1 = %d\r\n",com1); // comment this when testing with python

    *cm = com;
    *cm1 = com1;

    *srt = s;
    *stp = e;

    *srt1 = s1;
    *stp1 = e1;
        
    //  setPixel(IMAGESIZEY/2,com,0,255,0); // draw the center so you can see it in python
    //  printImage(); 

    // return 0;
}


float clamp(float val, float min, float max){
    if (val < min){val = min;}
    if (val > max){val = max;}

    return val;
}

void wait(int prevr, int prevl){
    float sr = clamp((float) prevr*0.6, 20.0, 30.0);
    float sl = clamp((float) prevl*0.6, 20.0, 30.0);

    set_pwm(PWMR, 100, 15, (int) sr);   
    set_pwm(PWML, 100, 15, (int) sl); 
    sleep_ms(300);  // 850 good 
    //  set_pwm(PWMR, 100, 15, BASE_SPEED);   
    //  set_pwm(PWML, 100, 15, BASE_SPEED); 
}


void sharp_turn(int dir){
    // 1 = left
    // 0 = right
    /*
    if (dir == 0){ 
        // right gets a -ve pwm
        printf("Here r\r\n");
        gpio_put(PWMR, false);

        set_pwm(PWML, 100, 15, 100);
        set_pwm(PWMR, 100, 15, 70);
        //  sleep_ms(100);
        gpio_put(PWMR, true);
    }
    else{
        printf("Here l\r\n");
        gpio_put(PWML, false);

        set_pwm(PWML, 100, 15, 70);
        set_pwm(PWMR, 100, 15, 100);
        //  sleep_ms(100);
        gpio_put(PWML, true);
    }
    */
   printf("here sharp\r\n");
   if (dir == 0){
        set_pwm(PWML, 100, 15, 50);
        set_pwm(PWMR, 100, 15, 0);
        sleep_ms(250);
    }
    else{
        set_pwm(PWML, 100, 15, 0);
        set_pwm(PWMR, 100, 15, 50);
        sleep_ms(250);
    }
}
static volatile float current_pwm = 0;
static volatile float curr_pr = 0;
static volatile float curr_pl = 0;
static volatile float kp = 0.4; // 0.7 good
static volatile float kd = 0.75;    // 0.65
static volatile float ft = 0.65;
static volatile float error_prev = 0; 


int pd_control_01(int com, int start, int stop, int com1, int start1, int stop1){

    float error = com1-40;
    float deriv = error - error_prev;
    float future = 0;

    float corr_pwm = kp*error + kd*deriv + future;
    int com_diff = com-com1;

    if (start1 == -1 && stop1 == -1){
        printf("Sharp right turn \r\n");
        // sharp_turn(0);

        if (com>72){
            sharp_turn(0);
        }
        else if(com>0 && com<18){
            sharp_turn(1);
        }
        
        if (com < com1){
            //  future = 0;
            printf("Sharp right turn \r\n");
            future = error*ft;
            //  sharp_turn(0);
        }
        else{
            //  future = 0;
            future = -error*ft;
            //  sharp_turn(1);
        }
        

        //  error = 1000;
        //  return 0;

    }
    else{
        future = (com-com1)*ft;
    }

    //  corr_pwm += future;

    printf("Future = %.2f\r\n", future);
    printf("Error = %.2f, derivative = %.2f, corr = %.2f\r\n", error, deriv, corr_pwm);
    /*
    float left_fast = clamp(BASE_SPEED+corr_pwm, BASE_SPEED, 70.0);
    float left_slow = clamp(BASE_SPEED-corr_pwm, BASE_SPEED, 70.0);

    float right_fast = clamp(BASE_SPEED+corr_pwm, BASE_SPEED, 70.0);
    float right_slow = clamp(BASE_SPEED-corr_pwm, BASE_SPEED, 70.0);
    */
    /*
    if (error == 1000){ // error is set to 0 after a sharp turn
        set_pwm(PWML, 100, 15, 40);
        set_pwm(PWMR, 100, 15, 40);
        sleep_ms(100);
        printf("Going straight after sharp turn\r\n");
    }
    */

    if (com_diff<3 && com_diff>-3){
        set_pwm(PWML, 100, 15, 30);
        set_pwm(PWMR, 100, 15, 30);

        curr_pl = 30;
        curr_pr = 30;

        sleep_ms(200);

        wait(curr_pr, curr_pl);

        printf("Going straight\r\n");
    }

    else if (com1>40){   // turn right
        float l;
        float r;
        if (corr_pwm < 0){
            l = clamp(BASE_SPEED-corr_pwm + abs(future), BASE_SPEED, 50.0);
            r = clamp(BASE_SPEED+corr_pwm - abs(future), BASE_SPEED, 50.0);
        }
        else{
            l = clamp(BASE_SPEED+corr_pwm + abs(future), BASE_SPEED, 50.0);
            r = clamp(BASE_SPEED-corr_pwm - abs(future), BASE_SPEED, 50.0);
        }

        set_pwm(PWMR, 100, 15, (int) r);   
        set_pwm(PWML, 100, 15, (int) l);

        curr_pl = (int) l;
        curr_pr = (int) r;

        sleep_ms(250);

        wait(curr_pr, curr_pl);

        printf("Turning right: R = %.2f, L = %.2f\n", r, l);
    }
    else{   // turn left
        float l;
        float r;
        if (corr_pwm<0){
            l = clamp(BASE_SPEED+corr_pwm-abs(future), BASE_SPEED, 50.0);
            r = clamp(BASE_SPEED-corr_pwm+abs(future), BASE_SPEED, 50.0);
        }
        else{
            l = clamp(BASE_SPEED-corr_pwm-abs(future), BASE_SPEED, 50.0);
            r = clamp(BASE_SPEED+corr_pwm+abs(future), BASE_SPEED, 50.0);
        }

        set_pwm(PWMR, 100, 15, (int) r);   
        set_pwm(PWML, 100, 15, (int) l);

        sleep_ms(250);

        curr_pl = (int) l;
        curr_pr = (int) r;

        wait(curr_pr, curr_pl);
        
        printf("Turning left: R = %.2f, L = %.2f\n", r, l);
    }
    error_prev = error;

    return 0;
}

int main(){
    stdio_init_all();
    pin_init();
    init_camera_pins();

    int com, srt, stp, com1, srt1, stp1;

    gpio_put(PHASEL, false);
    gpio_put(PHASER, false);
    //  set_pwm(PWMR, 100, 15, -60);

    
    while(1){
        read_camera(&com, &srt, &stp, &com1, &srt1, &stp1);
        //  sleep_ms(100);
        pd_control_01(com, srt, stp, com1, srt1, stp1);
        
    }
    
}
