#include <stdio.h>
#include <stdlib.h>
#include "driver/gpio.h"
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

/* Global */
uint8_t menuitem = 1;
uint8_t frame = 1;
uint8_t page = 1;
uint8_t lastMenuItem = 1;


char menuItem1[] = "Contrast";
char menuItem2[] = "Volume";
char menuItem3[] = "Language";
char menuItem4[] = "Difficulty";
char menuItem5[] = "Relay1";
char menuItem6[] = "Reset";


uint8_t contrast = 100;
uint8_t volume = 50;


char *language[3] = { "EN", "ES", "EL" };
int selectedLanguage = 0;

char *difficulty[2] = { "EASY", "HARD" };
int selectedDifficulty = 0;


char *Relay1[2] = { "OFF", "ON" };
int selectedRelay1 = 0;


uint8_t up = 0;
uint8_t down = 0;
uint8_t middle = 0;

int8_t last, value;

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

void resetDefaults(void);
void vDisplayMenuItem(char *item, uint8_t position, uint8_t selected);
void vDisplayMenuPage(char *menuItem, uint8_t *value);
void vDisplayCharMenuPage(char *menuitem, char *value);

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
	switch(gpio_num){
		case GPIO_ENC_CLK:
				gpio_set_intr_type(GPIO_ENC_CLK, GPIO_INTR_DISABLE);
				break;
		case GPIO_ENC_DT:
				gpio_set_intr_type(GPIO_ENC_DT, GPIO_INTR_DISABLE);
				break;
		case GPIO_ENC_SW:
				gpio_set_intr_type(GPIO_ENC_SW, GPIO_INTR_DISABLE);
				break;
	}
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, 0);
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
			xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY);
			gpio_set_intr_type(GPIO_ENC_CLK, GPIO_INTR_ANYEDGE); //enable
			
			if (io_num == GPIO_ENC_DT)
			{
				rotate = cr;
				printf("[ENC]clockwise rotation\n");
				xStatus = xQueueSendToBack(ENC_queue, &rotate, 100/portTICK_RATE_MS);
				gpio_set_intr_type(GPIO_ENC_DT, GPIO_INTR_ANYEDGE);//enable
				
			}
				
		}	
		else if(io_num == GPIO_ENC_DT)
		{
			//vTaskDelay(10 / portTICK_RATE_MS);
			xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY);
			gpio_set_intr_type(GPIO_ENC_DT, GPIO_INTR_ANYEDGE);
			
			if (io_num == GPIO_ENC_CLK)
			{
				rotate = ccr;
				//printf("[ENC]rotate = %d\n", rotate);
				printf("[ENC]counter clockwise rotation\n");
				xStatus = xQueueSendToBack(ENC_queue, &rotate, 100/portTICK_RATE_MS);
				gpio_set_intr_type(GPIO_ENC_CLK, GPIO_INTR_ANYEDGE);
			}
				
		}
		else if(io_num == GPIO_ENC_SW)
		{
			rotate = bp;
			//printf("[ENC]rotate = %d\n", rotate);
			printf("[ENC]Button is pressed\n");
			vTaskDelay(500 / portTICK_RATE_MS);
			xStatus = xQueueSendToBack(ENC_queue, &rotate, 100/portTICK_RATE_MS);
			gpio_set_intr_type(GPIO_ENC_SW, GPIO_INTR_NEGEDGE);//enable
			
		}
    }
}



void vDisplay(void *pvParameter)
{
	portBASE_TYPE xStatus;
	enum action rotate;
	uint8_t change = 1;
	SSD1306_Init();
    while(1) {
		if(change)
		{
			vDrawMenu();
			change = 0;
		}
	/***** Read Encoder ***********************/
		xStatus = xQueueReceive(ENC_queue, &rotate, 0); // portMAX_DELAY - сколь угодно долго
		if(xStatus == pdPASS)
		{
			change = 1;
			//printf("[vDisplay]rotate = %d\n", rotate);
			switch(rotate){
				case 0: //down
					down = 1;
					break;
				case 1: //up
					up = 1;
					break;
				case 2: //middle - button pressed
					middle = 1;
					break;
			}
		}
		if (up && page == 1) {
			up = 0;
			if(menuitem == 2 && frame == 2)
			{
				frame--;
			}
			
			if(menuitem == 3 && frame == 3)
			{
				frame--;
			}
			
			if(menuitem == 4 && frame == 4)
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
			up = 0;
			contrast--;
			vSetContrast(contrast);
			//setContrast();
		}
		else if(up && page == 2 && menuitem == 2)
		{
			up = 0;
			volume--;
		}
		else if(up && page == 2 && menuitem == 3)
		{
			up = 0;
			selectedLanguage--;
			if(selectedLanguage == -1)
			{
				selectedLanguage = 2;
			}
		}
		else if(up && page == 2 && menuitem == 4)
		{
			up = 0;
			selectedDifficulty--;
			if(selectedDifficulty == -1)
			{
				selectedDifficulty = 1;
			}
		}
		else if(up && page == 2 && menuitem == 5)
		{
			up = 0;
			selectedRelay1--;
			if(selectedRelay1 <= -1)
			{
				selectedRelay1 = 1;
			}
		}
		
		
		
		//We have turned the Rotary Encoder Clockwise
		if(down && page == 1)
		{
			down = 0;
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
			if(menuitem == 7) 
			{
				menuitem--;
			}
		}
		
		else if(down && page == 2 && menuitem == 1)
		{
			down = 0;
			contrast++;
			vSetContrast(contrast);
		}
		else if(down && page == 2 && menuitem == 2)
		{
			down = 0;
			volume++;
		}
		else if(down && page == 2 && menuitem == 3)
		{
			down = 0;
			selectedLanguage++;
			if(selectedLanguage == 3)
			{
				selectedLanguage = 0;
			}
		}
		else if(down && page == 2 && menuitem == 4)
		{
			down = 0;
			selectedDifficulty++;
			if(selectedDifficulty == 2)
			{
				selectedDifficulty = 0;
			}
		}
		else if(down && page == 2 && menuitem == 5)
		{
			down = 0;
			selectedRelay1++;
			if(selectedRelay1 >= 2)
			{
				selectedRelay1 = 0;
			} 
		}
		//Middle Button is Pressed
		if(middle)
		{
			//Middle Button is Pressed
			middle = 0;
			if(page == 1 && menuitem == 6)// Reset
			{
				resetDefaults();
			}
			else if(page == 1 && menuitem <= 5) //submenu
			{
				page = 2;
			}
			else if(page == 2)
			{
				page = 1;
			}
		}
    }
	vTaskDelete(NULL);
}




void resetDefaults(void)
  {
    contrast = 100;
    volume = 50;
    selectedLanguage = 0;
    selectedDifficulty = 0;
    vSetContrast(contrast);
	selectedRelay1 = 0;
    turnRelay1_Off();
  }



void vDrawMenu(void)
{
	if(page == 1)
	{
		SSD1306_Fill(SSD1306_COLOR_BLACK);
		SSD1306_GotoXY(25, 0); // установить курсор в позицию 15 - горизонталь, 0 - вертикаль
		SSD1306_Puts("MAIN MENU", &Font_7x10, SSD1306_COLOR_WHITE); // шрифт Font_7x10, белым цветом
		SSD1306_DrawLine(10, 12, 110, 12, SSD1306_COLOR_WHITE);
		//SSD1306_UpdateScreen();
	
	/*************** Frame 1 ******************************/
		if(menuitem == 1 && frame == 1)
		{
			vDisplayMenuItem(menuItem1, 15, 1);
			vDisplayMenuItem(menuItem2, 25, 0);
			vDisplayMenuItem(menuItem3, 35, 0);
		}
		else if(menuitem == 2 && frame == 1)
		{
			vDisplayMenuItem(menuItem1, 15, 0);
			vDisplayMenuItem(menuItem2, 25, 1);
			vDisplayMenuItem(menuItem3, 35, 0);
		}
		else if(menuitem == 3 && frame == 1)
		{
			vDisplayMenuItem(menuItem1, 15, 0);
			vDisplayMenuItem(menuItem2, 25, 0);
			vDisplayMenuItem(menuItem3, 35, 1);
		}
	/************ Frame 2 **********************************/
		else if(menuitem == 2 && frame == 2)
		{
			vDisplayMenuItem(menuItem2, 15, 1);
			vDisplayMenuItem(menuItem3, 25, 0);
			vDisplayMenuItem(menuItem4, 35, 0);
		}
		else if(menuitem == 3 && frame == 2)
		{
			vDisplayMenuItem(menuItem2, 15, 0);
			vDisplayMenuItem(menuItem3, 25, 1);
			vDisplayMenuItem(menuItem4, 35, 0);
		}
		else if(menuitem == 4 && frame == 2)
		{
			vDisplayMenuItem(menuItem2, 15, 0);
			vDisplayMenuItem(menuItem3, 25, 0);
			vDisplayMenuItem(menuItem4, 35, 1);
		}
	/************* Frame 3 *********************************/
		
		else if(menuitem == 3 && frame == 3)
		{
			vDisplayMenuItem(menuItem3, 15, 1);
			vDisplayMenuItem(menuItem4, 25, 0);
			vDisplayMenuItem(menuItem5, 35, 0);
		}
		else if(menuitem == 4 && frame == 3)
		{
			vDisplayMenuItem(menuItem3, 15, 0);
			vDisplayMenuItem(menuItem4, 25, 1);
			vDisplayMenuItem(menuItem5, 35, 0);
		}
		else if(menuitem == 5 && frame == 3)
		{
			vDisplayMenuItem(menuItem3, 15, 0);
			vDisplayMenuItem(menuItem4, 25, 0);
			vDisplayMenuItem(menuItem5, 35, 1);
		}
	/*************** Frame 4 *******************************/
		else if(menuitem == 4 && frame == 4)
		{
			vDisplayMenuItem(menuItem4, 15, 1);
			vDisplayMenuItem(menuItem5, 25, 0);
			vDisplayMenuItem(menuItem6, 35, 0);
		}
		else if(menuitem == 5 && frame == 4)
		{
			vDisplayMenuItem(menuItem4, 15, 0);
			vDisplayMenuItem(menuItem5, 25, 1);
			vDisplayMenuItem(menuItem6, 35, 0);
		}
		else if(menuitem == 6 && frame == 4)
		{
			vDisplayMenuItem(menuItem4, 15, 0);
			vDisplayMenuItem(menuItem5, 25, 0);
			vDisplayMenuItem(menuItem6, 35, 1);
		}
	/***************** End Frame *****************************/
	}
	/***************** Page 1 end ***************************/
	
	/*****************  Page 2 ******************************/
	else if(page==2 && menuitem == 1)
		vDisplayMenuPage(menuItem1, &contrast);
	
	
	else if(page==2 && menuitem == 2)
		vDisplayMenuPage(menuItem2, &volume);
	
	
	else if(page==2 && menuitem == 3)
		vDisplayCharMenuPage(menuItem3, language[selectedLanguage]);
	
	
	else if(page==2 && menuitem == 4)
		vDisplayCharMenuPage(menuItem4, difficulty[selectedDifficulty]);
	
	
	else if(page==2 && menuitem == 5)
		vDisplayCharMenuPage(menuItem5, Relay1[selectedRelay1]);
		if(selectedRelay1 == 1)
			turnRelay1_On();
		else if(selectedRelay1 == 0)
			turnRelay1_Off();
			
}


void vDisplayMenuItem(char *item, uint8_t position, uint8_t selected)
{
	if(selected)
	{
		SSD1306_GotoXY(0, position);
		SSD1306_Puts(">", &Font_7x10, SSD1306_COLOR_BLACK); // шрифт Font_7x10, цвет чёрным
		SSD1306_Puts(item, &Font_7x10, SSD1306_COLOR_BLACK); // шрифт Font_7x10, цвет чёрным
		//SSD1306_UpdateScreen();
	}
	else
	{
		SSD1306_GotoXY(0, position);
		SSD1306_Puts(">", &Font_7x10, SSD1306_COLOR_WHITE); // шрифт Font_7x10, цвет белым
		SSD1306_Puts(item, &Font_7x10, SSD1306_COLOR_WHITE); // шрифт Font_7x10, цвет белым
		//SSD1306_UpdateScreen();
	}
	SSD1306_UpdateScreen();
}



void vDisplayMenuPage(char *menuitem, uint8_t *value)
{
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	//SSD1306_UpdateScreen();
	SSD1306_GotoXY(25, 0); // установить курсор в позицию 15 - горизонталь, 0 - вертикаль
	SSD1306_Puts(menuitem, &Font_7x10, SSD1306_COLOR_WHITE); // шрифт Font_7x10, белым цветом
	SSD1306_DrawLine(10, 12, 110, 12, SSD1306_COLOR_WHITE); // draw line
	SSD1306_GotoXY(5, 15);
	SSD1306_Puts("Value", &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(5, 25);
	char v[24];
	itoa(*value, v, 10);
	//printf("v = %s\n", v);
	SSD1306_Puts(v, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_UpdateScreen();
}




void vDisplayCharMenuPage(char *menuitem, char *value)
{
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	//SSD1306_UpdateScreen();
	SSD1306_GotoXY(25, 0); // установить курсор в позицию 15 - горизонталь, 0 - вертикаль
	SSD1306_Puts(menuitem, &Font_7x10, SSD1306_COLOR_WHITE); // шрифт Font_7x10, белым цветом
	SSD1306_DrawLine(10, 12, 110, 12, SSD1306_COLOR_WHITE); // draw line
	SSD1306_GotoXY(5, 15);
	SSD1306_Puts("Value", &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(5, 25);
	SSD1306_Puts(value, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_UpdateScreen();
}


void turnRelay1_Off(void)
{
	gpio_set_level(GPIO_RELAY1, 0);
}

void turnRelay1_On(void)
{
	gpio_set_level(GPIO_RELAY1, 1);
}


void app_main()
{
	
	
	gpio_config_t io_conf;
	//Настройки GPIO для релейного ВЫХОДа
    //disable interrupt - отключитли прерывания
    io_conf.intr_type = GPIO_INTR_DISABLE;
	
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
    gpio_evt_queue = xQueueCreate(5, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(ENC, "ENC", 1548, NULL, 10, NULL);
	
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


