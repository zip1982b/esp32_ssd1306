#include <stdio.h>
#include "driver/gpio.h"
//#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_types.h>
#include "ssd1306.h"
#include "freertos/queue.h"

/*
 * GPIO status:
 *	GPIO34: output (relay1)
 *	GPIO35: output (relay2)
 *	GPIO33:(clk - encoder) input
 *	GPIO25:(dt - encoder) input
 *	GPIO26:(sw - encoder) input
 *
*/

#define GPIO_RELAY1    14
#define GPIO_RELAY2    12
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_RELAY1) | (1ULL<<GPIO_RELAY2))
#define GPIO_ENC_CLK     33
#define GPIO_ENC_DT		 25
#define GPIO_ENC_SW		 26
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_ENC_CLK) | (1ULL<<GPIO_ENC_DT) | (1ULL<<GPIO_ENC_SW)) 
#define ESP_INTR_FLAG_DEFAULT 0


/* 	
	cr	- clockwise rotation
	ccr	- counter clockwise rotation
	bp	- button pressed
*/
enum action{cr, ccr, bp}; 

portBASE_TYPE xStatusOLED;
xTaskHandle xDisplay_Handle;

static xQueueHandle gpio_evt_queue = NULL;
static xQueueHandle ENC_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
	
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}



static void ENC(void* arg)
{
	
	enum action rotate;
	portBASE_TYPE xStatus;
    uint32_t io_num;
    for(;;) {
		xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY);
		if(io_num == GPIO_ENC_CLK)
		{
			//vTaskDelay(10 / portTICK_RATE_MS);
			xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY);
			if (io_num == GPIO_ENC_DT)
			{
				rotate = cr;
				//printf("[ENC]clockwise rotation\n");
				xStatus = xQueueSendToBack(ENC_queue, &rotate, 0);
			}
				
		}	
		else if(io_num == GPIO_ENC_DT)
		{
			//vTaskDelay(10 / portTICK_RATE_MS);
			xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY);
			if (io_num == GPIO_ENC_CLK)
			{
				rotate = ccr;
				//printf("[ENC]rotate = %d\n", rotate);
				//printf("[ENC]counter clockwise rotation\n");
				xStatus = xQueueSendToBack(ENC_queue, &rotate, 0);
			}
				
		}
		else if(io_num == GPIO_ENC_SW)
		{
			rotate = bp;
			//printf("[ENC]rotate = %d\n", rotate);
			//printf("[ENC]Button is pressed\n");
			xStatus = xQueueSendToBack(ENC_queue, &rotate, 0);
		}
    }
}



void vDisplay(void *pvParameter)
{
	portBASE_TYPE xStatus;
	enum action rotate;
	
	SSD1306_Init();
    while(1) {
		vDrawMenu();
		xStatus = xQueueReceive(ENC_queue, &rotate, 0); // portMAX_DELAY - сколь угодно долго
		if(xStatus)
		{
			printf("[vDisplay]rotate = %d\n", rotate);
		}
		
		
		/*
		
		if (up && page == 1) {
			up = false;
			if(menuitem == 2 && frame == 2)
			{
				frame--;
			}
			if(menuitem == 4 && frame == 4)
			{
				frame--;
			}
			if(menuitem == 3 && frame == 3)
			{
				frame--;
			}
			lastMenuItem = menuitem;
			menuitem--;
			if (menuitem == 0)
			{
				menuitem = 1;
			}
		}
		else if (up && page == 2 && menuitem == 1) {
			up = false;
			contrast--;
			//setContrast();
		}
		else if(up && page == 2 && menuitem == 2)
		{
			up = false;
			volume--;
		}
		else if(up && page == 2 && menuitem == 3)
		{
			up = false;
			selectedLanguage--;
			if(selectedLanguage == -1)
			{
				selectedLanguage = 2;
			}
		}
		else if(up && page == 2 && menuitem == 4)
		{
			up = false;
			selectedDifficulty--;
			if(selectedDifficulty == -1)
			{
				selectedDifficulty = 1;
			}
		}
		
		
		//We have turned the Rotary Encoder Clockwise
		if(down && page == 1)
		{
			down = false;
			if(menuitem == 3 && lastMenuItem == 2)
			{
				frame ++;
			}
			else if(menuitem == 4 && lastMenuItem == 3)
			{
				frame ++;
			}
			else if(menuitem == 5 && lastMenuItem == 4 && frame != 4)
			{
				frame ++;
			}
			lastMenuItem = menuitem;
			menuitem++;  
			if(menuitem==7) 
			{
				menuitem--;
			}
		}
		else if(down && page == 2 && menuitem == 1)
		{
			down = false;
			contrast++;
			setContrast();
		}
		else if(down && page == 2 && menuitem == 2)
		{
			down = false;
			volume++;
		}
		else if(down && page == 2 && menuitem == 3)
		{
			down = false;
			selectedLanguage++;
			if(selectedLanguage == 3)
			{
				selectedLanguage = 0;
			}
		}
		else if(down && page == 2 && menuitem == 4)
		{
			down = false;
			selectedDifficulty++;
			if(selectedDifficulty == 2)
			{
				selectedDifficulty = 0;
			}
		}
		
		
		//Middle Button is Pressed
		if(middle)
		{
			//Middle Button is Pressed
			middle = false;
			if (page == 1 && menuitem == 5) // Backlight Control 
			{
				
				if (backlight) 
				{
					backlight = false;
					menuItem5 = "Light: OFF";
					turnBacklightOff();
				}
				else 
				{
					backlight = true; 
					menuItem5 = "Light: ON";
					turnBacklightOn();
				}
			}
			if(page == 1 && menuitem == 6)// Reset
			{
				resetDefaults();
			}
			
			else if(page == 1 && menuitem <= 4)
			{
				page = 2;
			}
			else if(page == 2)
			{
				page = 1;
			}
		}
		*/
		/*
		char S[13] = "Hello Zhan!*";
		char *s;
		s = &S;
		
		SSD1306_GotoXY(0, 4); // установить курсор в позицию 0 - горизонталь, 44 - вертикаль
		SSD1306_Puts(s, &Font_7x10, SSD1306_COLOR_WHITE); // шрифт Font_7x10, белым цветом
		SSD1306_UpdateScreen();
		vTaskDelay(5000 / portTICK_RATE_MS);
		
		
		Del_str(s);
		printf("S = %s\n", S);
		SSD1306_GotoXY(0, 4); // установить курсор в позицию 0 - горизонталь, 44 - вертикаль
		SSD1306_Puts(s, &Font_7x10, SSD1306_COLOR_WHITE);
		SSD1306_UpdateScreen();
		vTaskDelay(5000 / portTICK_RATE_MS);
		*/
		
		
		
    }
	vTaskDelete(NULL);
}


void app_main()
{
	gpio_config_t io_conf;
	//Настройки GPIO для релейного ВЫХОДа
    //disable interrupt - отключитли прерывания
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	
    //set as output mode - установка в режим Выход
    io_conf.mode = GPIO_MODE_OUTPUT;
	
    //bit mask of the pins that you want to set,e.g.GPIO14/12
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
	
    //disable pull-down mode - отключитли подтяжку к земле
    io_conf.pull_down_en = 0;
	
    //disable pull-up mode - отключили подтяжку к питанию
    io_conf.pull_up_en = 0;
	
    //configure GPIO with the given settings - конфигурирование портов с данными настройками (Выход)
    gpio_config(&io_conf);
	
	//Настройки GPIO Энкодера на ВХОД
	//interrupt of falling edge - прерывание по спадающему фронту и по возрастающему фронту
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    //bit mask of the pins, use GPIO33/25/26 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    
	//enable pull-up mode
    //io_conf.pull_up_en = 1;
	
    gpio_config(&io_conf);
	
	//change gpio interrupt type for one pin
    gpio_set_intr_type(GPIO_ENC_SW, GPIO_INTR_NEGEDGE); //Button pressed in gpio26
	
	
	//create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(ENC, "ENC", 2048, NULL, 10, NULL);
	
	ENC_queue = xQueueCreate(10, sizeof(uint32_t));
	
	//install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_ENC_CLK, gpio_isr_handler, (void*) GPIO_ENC_CLK);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_ENC_DT, gpio_isr_handler, (void*) GPIO_ENC_DT);
	//hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_ENC_SW, gpio_isr_handler, (void*) GPIO_ENC_SW);
	
	
	xStatusOLED = xTaskCreate(vDisplay, "vDisplay", 1024 * 2, NULL, 10, &xDisplay_Handle);
		if(xStatusOLED == pdPASS)
			printf("Task vDisplay is created!\n");
		else
			printf("Task vDisplay is not created\n");
}


