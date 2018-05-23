#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_types.h>
#include "ssd1306.h"





portBASE_TYPE xStatusOLED;
xTaskHandle xDisplay_Handle;


void vDisplay(void *pvParameter)
{
	SSD1306_Init();
    
    while(1) {
		SSD1306_GotoXY(30, 4); // установить курсор в позицию 0 - горизонталь, 44 - вертикаль
		SSD1306_Puts("Hello, Zhan!", &Font_7x10, SSD1306_COLOR_WHITE); // шрифт Font_7x10, белым цветом
		SSD1306_DrawCircle(10, 33, 7, SSD1306_COLOR_WHITE); // рисуем белую окружность в позиции 10, 33 и радиус 7 пикселей
		SSD1306_GotoXY(50, 27);
		SSD1306_DrawFilledRectangle(50, 26, 27, 10, SSD1306_COLOR_WHITE); //draw white filled rectangle
		SSD1306_Puts("2018", &Font_7x10, SSD1306_COLOR_BLACK);
		SSD1306_UpdateScreen();
		vTaskDelay(10000 / portTICK_RATE_MS);
		SSD1306_Fill(SSD1306_COLOR_BLACK);
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


