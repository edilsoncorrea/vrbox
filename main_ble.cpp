#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include <HIDTypes.h>
#include <HIDKeyboardTypes.h>
#include "../include/vrbox_joystick.h"

// UUID para serviço HID
#define SERVICE_UUID        "1812"
#define CHARACTERISTIC_UUID "2A4D"

BLEHIDDevice* hid;
BLECharacteristic* inputCharacteristic;
bool deviceConnected = false;
JoystickData joystick;

// Variáveis para controle
float steering = 0.0;
float acceleration = 0.0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Cliente conectado via BLE");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Cliente desconectado");
        pServer->getAdvertising()->start(); // Reiniciar advertising
    }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        
        if (value.length() > 0) {
            uint8_t* data = (uint8_t*)value.data();
            size_t length = value.length();
            
            Serial.printf("Dados BLE recebidos: %d bytes\\n", length);
            
            // Debug raw data
            Serial.print("Raw data: ");
            for (size_t i = 0; i < length; i++) {
                Serial.printf("0x%02X ", data[i]);
            }
            Serial.println();
            
            processHIDData(data, length);
            processJoystickCommands();
        }
    }
};

void initBLE() {
    Serial.println("Inicializando BLE HID...");
    
    BLEDevice::init("ESP32-VRBOX-BLE");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Criar dispositivo HID
    hid = new BLEHIDDevice(pServer);
    inputCharacteristic = hid->inputReport(1); // Report ID 1
    inputCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    // Configurar HID info
    std::string name = "ESP32 VRBOX Receiver";
    hid->manufacturer()->setValue(name);
    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    hid->hidInfo(0x00, 0x02);

    // Report Map para joystick/gamepad
    const uint8_t reportMap[] = {
        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x05,        // Usage (Game Pad)
        0xA1, 0x01,        // Collection (Application)
        0x85, 0x01,        //   Report ID (1)
        0x09, 0x01,        //   Usage (Pointer)
        0xA1, 0x00,        //   Collection (Physical)
        0x09, 0x30,        //     Usage (X)
        0x09, 0x31,        //     Usage (Y)
        0x15, 0x00,        //     Logical Minimum (0)
        0x26, 0xFF, 0x00,  //     Logical Maximum (255)
        0x35, 0x00,        //     Physical Minimum (0)
        0x46, 0xFF, 0x00,  //     Physical Maximum (255)
        0x75, 0x08,        //     Report Size (8)
        0x95, 0x02,        //     Report Count (2)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              //   End Collection
        0x05, 0x09,        //   Usage Page (Button)
        0x19, 0x01,        //   Usage Minimum (0x01)
        0x29, 0x08,        //   Usage Maximum (0x08)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x95, 0x08,        //   Report Count (8)
        0x75, 0x01,        //   Report Size (1)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              // End Collection
    };
    
    hid->reportMap((uint8_t*)reportMap, sizeof(reportMap));
    hid->startServices();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->setAppearance(HID_GAMEPAD);
    pAdvertising->addServiceUUID(hid->hidService()->getUUID());
    pAdvertising->start();
    
    Serial.println("BLE HID iniciado. Aguardando conexão...");
}

void processHIDData(uint8_t* data, size_t length) {
    if (length < 3) return;
    
    // Formato simplificado para teste
    // Byte 0: X axis
    // Byte 1: Y axis  
    // Byte 2: Buttons
    
    joystick.analogX = (int16_t)((data[0] - 128) * 256); // Converter de 0-255 para -32768 a 32767
    joystick.analogY = (int16_t)((data[1] - 128) * 256);
    joystick.buttons = data[2];
    
    joystick.frontButton1 = (joystick.buttons & (1 << VRBOX_FRONT_BTN_1)) != 0;
    joystick.frontButton2 = (joystick.buttons & (1 << VRBOX_FRONT_BTN_2)) != 0;
}

float mapSteering(int16_t analogX) {
    if (abs(analogX) < STICK_DEADZONE) {
        return 0.0;
    }
    return (float)analogX / MAX_STICK_VALUE;
}

float mapAcceleration(int16_t analogY) {
    if (abs(analogY) < STICK_DEADZONE) {
        return 0.0;
    }
    return -(float)analogY / MAX_STICK_VALUE; // Inverter Y
}

void processJoystickCommands() {
    steering = mapSteering(joystick.analogX);
    acceleration = mapAcceleration(joystick.analogY);
    
    // Aplicar aceleração digital
    if (joystick.frontButton1) {
        acceleration = max(acceleration, 0.5);
    }
    if (joystick.frontButton2) {
        acceleration = 1.0;
    }
    
    Serial.printf("Dir: %.2f | Acel: %.2f | Btns: 0x%02X\\n", 
                  steering, acceleration, joystick.buttons);
    
    executeCommands();
}

void executeCommands() {
    if (abs(steering) > 0.1) {
        Serial.printf("Direção: %s (%.1f%%)\\n", 
                      steering > 0 ? "DIREITA" : "ESQUERDA", 
                      abs(steering) * 100);
    }
    
    if (abs(acceleration) > 0.1) {
        Serial.printf("Aceleração: %s (%.1f%%)\\n", 
                      acceleration > 0 ? "FRENTE" : "RÉ", 
                      abs(acceleration) * 100);
    }
    
    // Processar botões individuais
    for (int i = 0; i < 8; i++) {
        if (joystick.buttons & (1 << i)) {
            Serial.printf("Botão %d ativo\\n", i + 1);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\\n=== ESP32-C3 VRBOX BLE Receiver ===");
    
    initBLE();
    memset(&joystick, 0, sizeof(joystick));
    
    Serial.println("Sistema BLE pronto!");
}

void loop() {
    if (!deviceConnected) {
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 5000) {
            Serial.println("Aguardando conexão BLE...");
            lastPrint = millis();
        }
    }
    
    delay(10);
}