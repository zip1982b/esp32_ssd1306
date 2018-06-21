Project name: esp32_ssd1306.
This project is based on Espressif IoT Development Framework (esp-idf).



![alt text](img/oled128x64.png "my first termostat :)")
Hardware:
* esp32
* OLED ssd1306 (128x64)
* Rotary encoder
* 2 Relay NO


Connections:
* Relay 1,2 - GPIO14, GPIO12
* encoder clk - GPIO33, DT - GPIO25, SW - GPIO26
* OLED DISPLAY ------------------------------
   SSD1306    |ESP32        |DESCRIPTION

   VCC        |3.3V         |
   GND        |GND          |
   SCL        |19           |Serial clock line
   SDA        |18           |Serial data line
----------------------------------------------






