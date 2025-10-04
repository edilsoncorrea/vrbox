#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Definir LED interno para ESP32-C3 (GPIO 8 - LED RGB)
#define LED_BUILTIN 8

// UUIDs para serviço Bluetooth LE HID
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-cba987654321"

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;

// Estrutura para armazenar dados do joystick
struct JoystickData {
    int16_t analogX;        // Eixo X do stick analógico (-32768 a 32767)
    int16_t analogY;        // Eixo Y do stick analógico (-32768 a 32767)
    uint16_t buttons;       // Estado dos botões (bit mask)
    bool frontButton1;      // Botão frontal 1 (aceleração digital)
    bool frontButton2;      // Botão frontal 2 (aceleração digital)
};

JoystickData joystick;

// Constantes para mapeamento do joystick
const int STICK_CENTER = 0;
const int STICK_DEADZONE = 1000;  // Zona morta para evitar drift

// Variáveis para controle de direção e aceleração
float steering = 0.0;      // -1.0 (esquerda) a 1.0 (direita)
float acceleration = 0.0;  // -1.0 (ré) a 1.0 (frente)

// Declaração de funções
void executeCommands();
void processJoystickCommands();
void processHIDData(uint8_t* data, size_t length);

// Callback para conexões BLE
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("Cliente BLE conectado!");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("Cliente BLE desconectado!");
        
        // Reiniciar advertising
        BLEDevice::startAdvertising();
        Serial.println("Advertising reiniciado");
    }
};

// Callback para recebimento de dados
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        
        if (rxValue.length() > 0) {
            Serial.print("Dados recebidos via BLE: ");
            for (int i = 0; i < rxValue.length(); i++) {
                Serial.print(rxValue[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
            
            // Processar dados HID do joystick
            processHIDData((uint8_t*)rxValue.c_str(), rxValue.length());
        }
    }
};

// Função para inicializar o Bluetooth LE
void initBLE() {
    Serial.println("Inicializando Bluetooth LE...");
    
    // Inicializar BLE Device
    BLEDevice::init("ESP32-C3-VRBOX-Receiver");
    
    // Criar servidor BLE
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // Criar serviço BLE
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    // Criar característica
    pCharacteristic = pService->createCharacteristic(
                         CHARACTERISTIC_UUID,
                         BLECharacteristic::PROPERTY_READ |
                         BLECharacteristic::PROPERTY_WRITE |
                         BLECharacteristic::PROPERTY_NOTIFY
                       );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());
    
    // Iniciar o serviço
    pService->start();
    
    // Iniciar advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
    
    Serial.println("Bluetooth LE inicializado. Nome do dispositivo: ESP32-C3-VRBOX-Receiver");
    Serial.println("Aguardando conexão do joystick VRBOX...");
}

// Função para processar dados HID recebidos
void processHIDData(uint8_t* data, size_t length) {
    if (length < 6) return; // Verificar se temos dados suficientes
    
    // Decodificar dados do joystick VRBOX
    // Formato típico: [X_low, X_high, Y_low, Y_high, buttons_low, buttons_high]
    
    joystick.analogX = (int16_t)((data[1] << 8) | data[0]);
    joystick.analogY = (int16_t)((data[3] << 8) | data[2]);
    joystick.buttons = (uint16_t)((data[5] << 8) | data[4]);
    
    // Extrair botões frontais dos bits dos botões
    joystick.frontButton1 = (joystick.buttons & 0x0001) != 0;
    joystick.frontButton2 = (joystick.buttons & 0x0002) != 0;
    
    // Processar comandos do joystick
    processJoystickCommands();
    
    // Debug: imprimir valores recebidos
    Serial.printf("X: %d, Y: %d, Botões: 0x%04X, FB1: %s, FB2: %s\n", 
                  joystick.analogX, joystick.analogY, joystick.buttons,
                  joystick.frontButton1 ? "ON" : "OFF",
                  joystick.frontButton2 ? "ON" : "OFF");
}

// Função para processar comandos do joystick
void processJoystickCommands() {
    // Mapear eixo X para direção (-1.0 a 1.0)
    if (abs(joystick.analogX) > STICK_DEADZONE) {
        steering = (float)joystick.analogX / 32767.0;
        steering = constrain(steering, -1.0, 1.0);
    } else {
        steering = 0.0;
    }
    
    // Mapear eixo Y para aceleração (-1.0 a 1.0)
    if (abs(joystick.analogY) > STICK_DEADZONE) {
        acceleration = -(float)joystick.analogY / 32767.0; // Inverter Y
        acceleration = constrain(acceleration, -1.0, 1.0);
    } else {
        acceleration = 0.0;
    }
    
    // Aplicar aceleração digital dos botões frontais
    if (joystick.frontButton1) {
        acceleration = 1.0; // Aceleração máxima para frente
    }
    if (joystick.frontButton2) {
        acceleration = -1.0; // Aceleração máxima para trás
    }
    
    // Executar comandos baseados nos valores processados
    executeCommands();
}

// Função para executar comandos baseados nos dados do joystick
void executeCommands() {
    // Aqui você pode implementar a lógica específica do seu projeto
    // Por exemplo: controlar motores, servos, etc.
    
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 100) { // Imprimir a cada 100ms
        Serial.printf("Direção: %.2f, Aceleração: %.2f\n", steering, acceleration);
        lastPrint = millis();
    }
    
    // Exemplo de controle de LED baseado na aceleração
    if (acceleration > 0.5) {
        digitalWrite(LED_BUILTIN, HIGH); // LED ligado para aceleração alta
    } else {
        digitalWrite(LED_BUILTIN, LOW);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32-C3 VRBOX Receiver iniciando...");
    
    // Configurar LED interno
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    // Inicializar valores do joystick
    joystick.analogX = 0;
    joystick.analogY = 0;
    joystick.buttons = 0;
    joystick.frontButton1 = false;
    joystick.frontButton2 = false;
    
    // Inicializar Bluetooth LE
    initBLE();
    
    Serial.println("Sistema pronto para receber comandos do joystick VRBOX!");
}

void loop() {
    // Verificar conexão BLE
    if (!deviceConnected) {
        // Piscar LED quando não conectado
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 500) {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            lastBlink = millis();
        }
    }
    
    delay(20); // Pequeno delay para não sobrecarregar o sistema
}