#ifndef VRBOX_JOYSTICK_H
#define VRBOX_JOYSTICK_H

#include <Arduino.h>

// Estrutura para dados do joystick VRBOX
struct JoystickData {
    int16_t analogX;        // Eixo X do stick analógico (-32768 a 32767)
    int16_t analogY;        // Eixo Y do stick analógico (-32768 a 32767)
    uint16_t buttons;       // Estado dos botões (bit mask)
    bool frontButton1;      // Botão frontal 1
    bool frontButton2;      // Botão frontal 2
};

// Constantes
#define STICK_CENTER 0
#define STICK_DEADZONE 1000
#define MAX_STICK_VALUE 32767

// Mapeamento de botões VRBOX (bit positions)
#define VRBOX_BTN_A 0
#define VRBOX_BTN_B 1
#define VRBOX_BTN_C 2
#define VRBOX_BTN_D 3
#define VRBOX_BTN_UP 4
#define VRBOX_BTN_DOWN 5
#define VRBOX_FRONT_BTN_1 6
#define VRBOX_FRONT_BTN_2 7

// Protótipos de funções
void initBluetooth();
void processHIDData(uint8_t* data, size_t length);
float mapSteering(int16_t analogX);
float mapAcceleration(int16_t analogY);
void processJoystickCommands();
void executeCommands();
void onBluetoothDataReceived();

#endif // VRBOX_JOYSTICK_H