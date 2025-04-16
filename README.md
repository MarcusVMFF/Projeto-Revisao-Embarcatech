# 📊 **Projeto-Revisao-Embarcatech**

Projeto de revisão sobre a capacitação em Sistemas Embarcados TIC 37 - Embarcatech. Este projeto consiste em um sistema de visualização de cor RGB utilizando joystick analógico, LEDs RGB por PWM, matriz 5x5 controlada por PIO, display OLED e comunicação via I2C e UART, todos integrados na placa BitDogLab.

---

## 🔎 **Objetivos**

O objetivo principal é revisar todos os conteúdos aprendidos durante a capacitação, criando um projeto que consiga incluir todos, sendo feito usando unicamente a placa BitDogLab.
Este projeto utiliza os assuntos aprendidos durante a capacitação: manipulação de sinais analógicos via ADC, controle de LEDs com PWM, utilização de joystick, exibição gráfica em display OLED via I2C, controle de matriz de LEDs 5x5 utilizando PIO (Programmable IO), e estruturação de código em C usando CMake.

---

## 🎥 **Demonstração**

[Ver Vídeo do Projeto](https://drive.google.com/file/d/1PNSEIfgEr9aK2G-BgxIsujloKBIfRQ7C/view?usp=drive_link)

---

## 🛠️ **Tecnologias Utilizadas**

- **Linguagem de Programação:** C / CMake
- **Placas Microcontroladoras:**
  - BitDogLab
  - Pico W
---

## 📖 **Como Utilizar**

- Controla a intensidade e a cor dos LED's PWM vermelho e verde movendo o joystick
- Um quadrado de 8x8 pixels, inicialmente centralizado, se move na tela proporcionalmente aos valores do joystick
- O botão A troca a cor da matriz PIO e toca um curto barulho com o buzzer, cuja frequencia altera para cada cor
- O botão B entra em modo BOOTSEEL
- Pode-se acompanhar as coordenadas X e Y, junto ao duty cicle pelo Serial Monitor
---
