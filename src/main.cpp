#include <Arduino.h>
#include "BluetoothSerial.h"

// Definir LED interno para ESP32 (GPIO 2)
#define LED_BUILTIN 2

BluetoothSerial SerialBT;

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

// Função para inicializar o Bluetooth
void initBluetooth() {
    Serial.println("Inicializando Bluetooth...");
    
    if (!SerialBT.begin("ESP32-VRBOX-Receiver")) {
        Serial.println("Erro ao inicializar Bluetooth Serial");
        return;
    }
    
    Serial.println("Bluetooth inicializado. Nome do dispositivo: ESP32-VRBOX-Receiver");
    Serial.println("Aguardando conexão do joystick VRBOX...");
}

// Função para processar dados HID recebidos
void processHIDData(uint8_t* data, size_t length) {
    if (length < 6) return; // Verificar se temos dados suficientes
    
    // Decodificar dados do joystick VRBOX
    // Formato típico de HID para joystick:
    // Byte 0: Report ID
    // Bytes 1-2: Eixo X (Little Endian)
    // Bytes 3-4: Eixo Y (Little Endian)
    // Byte 5: Botões (bit mask)
    
    joystick.analogX = (int16_t)((data[2] << 8) | data[1]);
    joystick.analogY = (int16_t)((data[4] << 8) | data[3]);
    joystick.buttons = data[5];
    
    // Extrair botões frontais específicos (assumindo bits 6 e 7)
    joystick.frontButton1 = (joystick.buttons & 0x40) != 0;
    joystick.frontButton2 = (joystick.buttons & 0x80) != 0;
}

// Função para mapear stick analógico para valores de direção
float mapSteering(int16_t analogX) {
    if (abs(analogX) < STICK_DEADZONE) {
        return 0.0; // Zona morta
    }
    
    return (float)analogX / 32767.0; // Normalizar para -1.0 a 1.0
}

// Função para mapear stick analógico para valores de aceleração
float mapAcceleration(int16_t analogY) {
    if (abs(analogY) < STICK_DEADZONE) {
        return 0.0; // Zona morta
    }
    
    // Inverter Y (para frente ser positivo)
    return -(float)analogY / 32767.0; // Normalizar para -1.0 a 1.0
}

// Função para processar comandos do joystick
void processJoystickCommands() {
    // Mapear eixos analógicos
    steering = mapSteering(joystick.analogX);
    acceleration = mapAcceleration(joystick.analogY);
    
    // Aplicar aceleração digital dos botões frontais se necessário
    if (joystick.frontButton1) {
        acceleration = max(acceleration, 0.5f); // Aceleração mínima
    }
    if (joystick.frontButton2) {
        acceleration = 1.0f; // Aceleração máxima
    }
    
    // Debug: imprimir valores processados
    Serial.printf("Direção: %.2f | Aceleração: %.2f | Botões: 0x%02X\\n", 
                  steering, acceleration, joystick.buttons);
    
    // Aqui você pode adicionar sua lógica específica
    // Por exemplo, controlar motores, enviar comandos, etc.
    executeCommands();
}

// Função para executar comandos baseados nos inputs do joystick
void executeCommands() {
    // Exemplo de lógica de controle
    
    if (abs(steering) > 0.1) {
        if (steering > 0) {
            Serial.println("Virando à DIREITA");
            // Piscar LED mais rápido quando virando
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
        } else {
            Serial.println("Virando à ESQUERDA");
            // Piscar LED mais rápido quando virando
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
        }
    }
    
    if (abs(acceleration) > 0.1) {
        if (acceleration > 0) {
            Serial.printf("Acelerando: %.0f%%\\n", acceleration * 100);
        } else {
            Serial.printf("Freando/Ré: %.0f%%\\n", abs(acceleration) * 100);
        }
    }
    
    // Processar outros botões
    for (int i = 0; i < 6; i++) {
        if (joystick.buttons & (1 << i)) {
            Serial.printf("Botão %d pressionado\\n", i + 1);
        }
    }
    
    if (joystick.frontButton1) {
        Serial.println("Botão frontal 1 ativo");
    }
    if (joystick.frontButton2) {
        Serial.println("Botão frontal 2 ativo");
    }
}

// Callback para quando dados são recebidos via Bluetooth
void onBluetoothDataReceived() {
    if (SerialBT.available()) {
        uint8_t buffer[32];
        size_t bytesRead = SerialBT.readBytes(buffer, sizeof(buffer));
        
        if (bytesRead > 0) {
            Serial.printf("Dados recebidos: %d bytes\\n", bytesRead);
            
            // Debug: imprimir dados raw
            Serial.print("Raw data: ");
            for (size_t i = 0; i < bytesRead; i++) {
                Serial.printf("0x%02X ", buffer[i]);
            }
            Serial.println();
            
            // Processar dados HID
            processHIDData(buffer, bytesRead);
            processJoystickCommands();
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\\n=== ESP32 VRBOX Joystick Receiver ===");
    
    // Configurar LED interno
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Inicializar Bluetooth
    initBluetooth();
    
    // Inicializar estrutura do joystick
    memset(&joystick, 0, sizeof(joystick));
    
    Serial.println("Sistema pronto!");
    Serial.println("Para conectar o joystick VRBOX:");
    Serial.println("1. Ligue o joystick");
    Serial.println("2. Procure por 'ESP32-VRBOX-Receiver' na lista de dispositivos");
    Serial.println("3. Pareie o dispositivo");
}

void loop() {
    // Verificar se há conexão Bluetooth
    if (SerialBT.hasClient()) {
        // Processar dados recebidos
        onBluetoothDataReceived();
        
        // LED aceso quando conectado
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        // Indicar que está aguardando conexão
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 5000) {
            Serial.println("Aguardando conexão do joystick VRBOX...");
            lastPrint = millis();
        }
        
        // LED piscando quando aguardando conexão
        digitalWrite(LED_BUILTIN, (millis() / 500) % 2);
    }
    
    delay(10); // Pequeno delay para não sobrecarregar o processador
}