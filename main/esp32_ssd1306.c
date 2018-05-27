#include <stdio.h>
//#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_types.h>
#include "ssd1306.h"





portBASE_TYPE xStatusOLED;
xTaskHandle xDisplay_Handle;


void vDisplay(void *pvParameter)
{
	SSD1306_Init();
	
	SSD1306_DrawCircle(10, 33, 7, SSD1306_COLOR_WHITE); // рисуем белую окружность в позиции 10, 33 и радиус 7 пикселей
	SSD1306_GotoXY(50, 27);
	SSD1306_DrawFilledRectangle(50, 26, 27, 10, SSD1306_COLOR_WHITE); //draw white filled rectangle
	SSD1306_Puts("2018", &Font_7x10, SSD1306_COLOR_BLACK);
	SSD1306_UpdateScreen();
	vTaskDelay(1000 / portTICK_RATE_MS);
	
	SSD1306_UpdateScreen();
	
    
    while(1) {
		char S[13] = "Hello Zhan!*";
		char *s;
		s = &S;
		
		SSD1306_GotoXY(0, 4); // установить курсор в позицию 0 - горизонталь, 44 - вертикаль
		SSD1306_Puts(s, &Font_7x10, SSD1306_COLOR_WHITE); // шрифт Font_7x10, белым цветом
		SSD1306_UpdateScreen();
		vTaskDelay(5000 / portTICK_RATE_MS);
		
		/*
		int len = strlen(S);
		printf("len = %d\n", strlen(S));
		int i;
		for(i = 0; i <= len; i++)
		{
			S[i] = ' ';
			printf("%c\n", S[i]);	
		}
		printf("S = %s\n", S);
		*/
		Del_str(s);
		printf("S = %s\n", S);
		SSD1306_GotoXY(0, 4); // установить курсор в позицию 0 - горизонталь, 44 - вертикаль
		SSD1306_Puts(s, &Font_7x10, SSD1306_COLOR_WHITE);
		SSD1306_UpdateScreen();
		vTaskDelay(5000 / portTICK_RATE_MS);
		
		
		
		
    }
	vTaskDelete(NULL);
}


void app_main()
{
	
	
	xStatusOLED = xTaskCreate(vDisplay, "vDisplay", 1024 * 2, NULL, 10, &xDisplay_Handle);
		if(xStatusOLED == pdPASS)
			printf("Task vDisplay is created!\n");
		else
			printf("Task vDisplay is not created\n");
}


