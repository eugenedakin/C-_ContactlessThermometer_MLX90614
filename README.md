# C++_ContactlessThermometer_MLX90614
A non-contact thermometer using GY-906 sensor on a MLX90614 board using C++, wxwidgets and the Raspberry Pi GPIO I2C pins.

This C++ program for the Raspberry Pi uses the GY-906 sensor on an MLX90614 module board to detect the object and ambient temperature of an object. This is useful for temperature monitoring and other practical tasks. This was created on 19 Mar 2026. 

Ensure the I2C communication protocol is enabled on the Raspberry Pi. The below video shows: 1) compiling, 2) running, and 3) retrieving temperature data. 

![](https://github.com/eugenedakin/C-_ContactlessThermometer_MLX90614/blob/main/MLX90614.gif)


Installation instructions:
1) Install Raspberry Pi OS (64-bit)
2) Open a terminal and type the following commands:
3) 


Use I2Cdetect -y 1 to confirm the MLX90614 is available at I2C hexadecimal address 5A.
![](https://github.com/eugenedakin/CPP_ContactlessThermometer_MLX90614/blob/main/I2cdetect.png)

Below is the breadboard layout with the Raspberry Pi 4 connecting with I2C to the MLX90614.
![](https://github.com/eugenedakin/CPP_ContactlessThermometer_MLX90614/blob/main/BreadBoard.png)

