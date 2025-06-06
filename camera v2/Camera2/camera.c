#include <stdio.h>
#include "pico/stdlib.h"
#include "cam.h"

int main()
{
    stdio_init_all();

    //while (!stdio_usb_connected()) {
    //    sleep_ms(100);
    //}
    //printf("Hello, camera!\n");

    init_camera_pins();
 
    while (true) {
        // uncomment these and printImage() when testing with python 
        char m[10];
        scanf("%s",m);

        setSaveImage(1);
        while(getSaveImage()==1){}
        convertImage();
        //  printImage();

        int com = findLine(10); // front
        //  printf("Com = %d\r\n", com);

        int com1 = findLine(40);    // back
        //  printf("Com1 = %d\r\n", com1);

        setPixel(10,com,0,255,0); // draw the center so you can see it in python
        setPixel(40, com1, 0, 255, 0);
        printImage();
        

        //  printf("%d\r\n",com); // comment this when testing with python
    }
}


