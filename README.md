Project name: esp32_ssd1306.
This project is based on Espressif IoT Development Framework (esp-idf).

I modified this project for ESP32. The original project for STM32.
* original author:  Tilen Majerle<tilen@majerle.eu>
* modification for STM32f10x: Alexander Lutsai<s.lyra@ya.ru>
* modification for ESP32: Zhan Beshchanov<zip1982b@gmail.com>


![alt text](img/oled128x64.jpg "my first termostat :)")
Hardware:
* esp32
* OLED ssd1306 (128x64)
* Rotary encoder
* 2 Relay NO


Connections:
* Relay 1,2 - GPIO14, GPIO12
* encoder clk - GPIO33, DT - GPIO25, SW - GPIO26
* SSD1306 VCC - 3.3v 
* SSD1306 GND
* SSD1306 SCL - GPIO19
* SSD1306 SDA - GPIO18






