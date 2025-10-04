/*
 * = * Funcionalidades:
 * - ✅ Conexão BLE automática com VRBOX
 * - ✅ Parsing específico dos protocolos VRBOX HID
 * - ✅ Recepção de dados de joystick X/Y (range real: -36 a +36)
 * - ✅ Detecção de triggers e botões A/B/C/D
 * - ✅ Comandos de ativação baseados no padrão PS3/BigJBehr
 * - ✅ Filtros para evitar conexões incorretas
 * - ✅ Reconexão automática e polling ativo
 * - ✅ Controle PWM completo para servo e ESC de carro RC
 * - ✅ Mapeamento otimizado para range real do VRBOX
 * - ✅ Zona morta e filtros de suavização==============================================================
 * VRBOX ESP32-C3 BLE Client - Projeto COMPLETO e FUNCIONAL
 * ========================================================================
 * 
 * Este código implementa um cliente BLE para conectar ESP32-C3 ao joystick
 * VRBOX, recebendo dados de movimento, triggers e botões em tempo real.
 * 
 * Funcionalidades:
 * - ✅ Conexão BLE automática com VRBOX
 * - ✅ Parsing específico dos protocolos VRBOX
 * - ✅ Recepção de dados de joystick X/Y (-127 a +127)
 * - ✅ Detecção de triggers e botões A/B/C/D
 * - ✅ Comandos de ativação baseados no padrão PS3/BigJBehr
 * - ✅ Filtros para evitar conexões incorretas
 * - ✅ Reconexão automática e polling ativo
 * - ✅ Controle PWM para servo e ESC de carro RC
 * 
 * Hardware: ESP32-C3-DevKitM-1
 * Joystick: VRBOX (nome BLE: "VR BOX")
 * Protocolo: HID over GATT (UUID 1812)
 * 
 * Autor: Edilson Correa
 * Status: FUNCIONANDO PERFEITAMENTE - Sistema completo de controle RC
 * Versão: 2.1 - PWM implementado com mapeamento otimizado
 * Data: Dezembro 2024
 * ========================================================================
 */

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// ========================================================================
// UUIDs PADRÃO PARA HID DEVICE (Bluetooth SIG)
// ========================================================================
static BLEUUID HID_SERVICE_UUID("1812");        // HID Service - Serviço principal para dispositivos HID
static BLEUUID INPUT_REPORT_UUID("2a4d");       // HID Input Report - Dados do joystick e botões
static BLEUUID REPORT_MAP_UUID("2a4b");         // HID Report Map - Estrutura dos dados HID
static BLEUUID HID_INFO_UUID("2a4a");           // HID Information - Informações do dispositivo
static BLEUUID HID_CONTROL_UUID("2a4c");        // HID Control Point - Comandos de ativação
static BLEUUID BATTERY_SERVICE_UUID("180f");    // Battery Service - Serviço de bateria
static BLEUUID BATTERY_LEVEL_UUID("2a19");      // Battery Level - Nível da bateria

// Descriptor para habilitar notificações BLE
static BLEUUID DESCRIPTOR_UUID("2902");

// ========================================================================
// CONFIGURAÇÕES PWM PARA CONTROLE DE CARRO RC
// ========================================================================
// Pinos do ESP32-C3 para saídas PWM - PINOS SEGUROS
#define SERVO_PIN 4     // GPIO4 - Controle do servo (direção)
#define ESC_PIN 5       // GPIO5 - Controle do ESC (motor)

// Configurações PWM - Ajustado para ESP32-C3
#define PWM_FREQ 50     // 50Hz para servo/ESC padrão
#define PWM_RES 12      // Resolução de 12 bits (0-4095) - mais compatível

// Canais PWM (ESP32-C3 tem 6 canais PWM)
#define SERVO_CHANNEL 0
#define ESC_CHANNEL 1

// Valores PWM para controle RC (em microssegundos)
#define PWM_MIN 1000    // 1ms - Posição mínima
#define PWM_CENTER 1500 // 1.5ms - Posição central/neutro
#define PWM_MAX 2000    // 2ms - Posição máxima

// Zona morta para evitar jitter (±10% do range real do VRBOX)
#define DEADBAND 4      // ±4 de 36 = ~10% (ajustado para range real -36~+36)

// Filtro de suavização (0.0 = sem filtro, 0.9 = muito suave)
#define SMOOTHING_FACTOR 0.3f

// Valores PWM atuais (para suavização)
float currentServoPWM = PWM_CENTER;
float currentESCPWM = PWM_CENTER;

// ========================================================================
// VARIÁVEIS GLOBAIS DE CONTROLE BLE
// ========================================================================
bool deviceConnected = false;               // Flag de conexão ativa
bool doConnect = false;                     // Flag para iniciar conexão
bool doScan = false;                        // Flag para reiniciar scan
BLEAdvertisedDevice* myDevice;              // Dispositivo VRBOX encontrado
BLEClient* pClient = nullptr;               // Cliente BLE principal
BLERemoteService* pRemoteService = nullptr; // Serviço remoto HID
BLERemoteCharacteristic* pInputReportChar = nullptr;  // Característica de input para polling

// Timing para polling e verificações
unsigned long lastPoll = 0;
unsigned long lastActivation = 0;

// ========================================================================
// ESTRUTURA DE DADOS DO JOYSTICK VRBOX
// ========================================================================
// Estrutura normalizada para compatibilidade com diferentes aplicações
struct JoystickData {
  float directionX = 0.0;      // Eixo X normalizado: -1.0 (esquerda) a +1.0 (direita)
  float directionY = 0.0;      // Eixo Y normalizado: -1.0 (baixo) a +1.0 (cima)
  float acceleration = 0.0;    // Reservado para expansão futura
  uint16_t buttons = 0;        // Estado dos botões (bitmask)
};

JoystickData joystickData;  // Instância global dos dados do joystick

// ========================================================================
// CALLBACK DE NOTIFICAÇÕES BLE - PARSING ESPECÍFICO VRBOX
// ========================================================================
/*
 * Esta função é chamada automaticamente quando o VRBOX envia dados.
 * Implementa parsing específico baseado na análise do repositório BigJBehr:
 * https://github.com/BigJBehr/ESP32-Bluetooth-BLE-Remote-Control
 * 
 * Formatos suportados:
 * - 4 bytes: Joystick/Triggers (modo Mouse ativado com @ + D)
 * - 2 bytes: Botões A/B/C/D com diferentes comportamentos
 */
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                          uint8_t* pData, size_t length, bool isNotify) {
  Serial.printf("📨 Dados recebidos [%d bytes]: ", length);
  
  // Exibe dados em hexadecimal
  for (int i = 0; i < length; i++) {
    Serial.printf("%02X ", pData[i]);
  }
  Serial.println();
  
  // ============================================================
  // PARSING ESPECÍFICO VRBOX baseado no repositório BigJBehr
  // ============================================================
  
  if (length == 4) {
    // ✅ FORMATO DE 4 BYTES: Joystick/Triggers (modo Mouse @ + D)
    // Byte 0: Trigger buttons (bit 0=lower trigger, bit 1=upper trigger)
    // Byte 1: Joystick X (signed 8-bit: -127 a +127)
    // Byte 2: Joystick Y (signed 8-bit: -127 a +127) 
    // Byte 3: 0x00 (sempre zero no modo mouse)
    
    uint8_t triggers = pData[0];
    int8_t joyX = (int8_t)pData[1];  // Signed
    int8_t joyY = (int8_t)pData[2];  // Signed
    
    // Interpretar triggers
    bool lowerTrigger = (triggers & 0x01) != 0;
    bool upperTrigger = (triggers & 0x02) != 0;
    
    // Converter para nosso formato (manter compatibilidade)
    joystickData.directionX = joyX / 127.0f;  // Normalizar para -1.0 a +1.0
    joystickData.directionY = joyY / 127.0f;
    joystickData.buttons = triggers;  // Triggers como botões
    
    Serial.printf("🕹️  JOYSTICK - X: %d (%.2f), Y: %d (%.2f), Triggers: 0x%02X [L:%s, U:%s]\n",
                  joyX, joystickData.directionX, 
                  joyY, joystickData.directionY,
                  triggers, 
                  lowerTrigger ? "ON" : "OFF",
                  upperTrigger ? "ON" : "OFF");
    
  } else if (length == 2) {
    // ✅ FORMATO DE 2 BYTES: Botões A/B ou C/D
    uint8_t buttonData = pData[0];
    uint8_t lowNibble = buttonData & 0x0F;
    
    if (lowNibble == 0x05) {
      // BOTÕES A/B (auto-repeat quando segurado)
      uint8_t buttons = buttonData & 0xF0;
      bool buttonA = (buttons & 0x10) != 0;
      bool buttonB = (buttons & 0x20) != 0;
      
      Serial.printf("🎮 BOTÕES A/B: 0x%02X [A:%s, B:%s] - Auto-repeat\n",
                    buttons, 
                    buttonA ? "ON" : "OFF",
                    buttonB ? "ON" : "OFF");
      
      // Atualizar estrutura
      joystickData.buttons |= buttons;
      
    } else {
      // BOTÕES C/D (sem auto-repeat)
      bool buttonC = (buttonData & 0x01) != 0;
      bool buttonD = (buttonData & 0x02) != 0;
      
      Serial.printf("🎮 BOTÕES C/D: 0x%02X [C:%s, D:%s] - Single press\n",
                    buttonData,
                    buttonC ? "ON" : "OFF", 
                    buttonD ? "ON" : "OFF");
      
      // Atualizar estrutura  
      joystickData.buttons |= (buttonData << 8);  // Shift para posição alta
    }
    
  } else {
    // ❓ FORMATO DESCONHECIDO
    Serial.printf("❓ Formato desconhecido (%d bytes), exibindo raw:\n", length);
    for (int i = 0; i < length; i++) {
      Serial.printf("  [%d]: 0x%02X (%d)\n", i, pData[i], pData[i]);
    }
  }
}

// Função para ler Report Map e entender o formato dos dados
void readReportMap(BLERemoteService* pRemoteService) {
  Serial.println("Tentando ler Report Map...");
  
  BLERemoteCharacteristic* pReportMapChar = pRemoteService->getCharacteristic(REPORT_MAP_UUID);
  if (pReportMapChar != nullptr) {
    if (pReportMapChar->canRead()) {
      std::string reportMap = pReportMapChar->readValue();
      Serial.printf("Report Map lido [%d bytes]: ", reportMap.length());
      
      for (int i = 0; i < reportMap.length(); i++) {
        Serial.printf("%02X ", (uint8_t)reportMap[i]);
        if ((i + 1) % 16 == 0) Serial.println();
      }
      Serial.println();
    }
  } else {
    Serial.println("Report Map characteristic não encontrada");
  }
}

// Função para ler HID Information
void readHIDInfo(BLERemoteService* pRemoteService) {
  Serial.println("Tentando ler HID Info...");
  
  BLERemoteCharacteristic* pHIDInfoChar = pRemoteService->getCharacteristic(HID_INFO_UUID);
  if (pHIDInfoChar != nullptr) {
    if (pHIDInfoChar->canRead()) {
      std::string hidInfo = pHIDInfoChar->readValue();
      Serial.printf("HID Info [%d bytes]: ", hidInfo.length());
      
      for (int i = 0; i < hidInfo.length(); i++) {
        Serial.printf("%02X ", (uint8_t)hidInfo[i]);
      }
      Serial.println();
    }
  }
}

bool connectToServer() {
  Serial.print("Conectando ao dispositivo: ");
  Serial.println(myDevice->getAddress().toString().c_str());
  
  pClient = BLEDevice::createClient();
  Serial.println("Cliente BLE criado");

  // Conectar ao servidor remoto
  if (!pClient->connect(myDevice)) {
    Serial.println("Falha na conexão");
    return false;
  }
  Serial.println("Conectado ao servidor");

  // Descobrir TODOS os serviços disponíveis
  Serial.println("Descobrindo todos os serviços...");
  std::map<std::string, BLERemoteService*>* services = pClient->getServices();
  
  Serial.printf("Encontrados %d serviços:\n", services->size());
  
  bool foundInputService = false;
  
  for (auto& servicePair : *services) {
    BLERemoteService* pService = servicePair.second;
    Serial.printf("\n=== Serviço UUID: %s ===\n", pService->getUUID().toString().c_str());
    
    // Listar todas as características deste serviço
    std::map<std::string, BLERemoteCharacteristic*>* characteristics = pService->getCharacteristics();
    Serial.printf("  Características encontradas: %d\n", characteristics->size());
    
    for (auto& charPair : *characteristics) {
      BLERemoteCharacteristic* pChar = charPair.second;
      Serial.printf("    Característica UUID: %s\n", pChar->getUUID().toString().c_str());
      Serial.printf("      Propriedades: ");
      
      if (pChar->canRead()) Serial.print("READ ");
      if (pChar->canWrite()) Serial.print("WRITE ");
      if (pChar->canNotify()) Serial.print("NOTIFY ");
      if (pChar->canIndicate()) Serial.print("INDICATE ");
      Serial.println();
      
      // Se pode notificar, registrar callback
      if (pChar->canNotify()) {
        Serial.printf("      Registrando callback para notificações\n");
        pChar->registerForNotify(notifyCallback);
        foundInputService = true;
        
        // Habilitar notificações via descriptor
        BLERemoteDescriptor* pDescriptor = pChar->getDescriptor(DESCRIPTOR_UUID);
        if (pDescriptor != nullptr) {
          uint8_t notifyValue[] = {0x01, 0x00};
          pDescriptor->writeValue(notifyValue, 2, true);
          Serial.println("      Notificações habilitadas via descriptor");
        }
        
        // Se for a característica INPUT_REPORT, guardar referência para polling
        if (pService->getUUID().equals(HID_SERVICE_UUID) && 
            pChar->getUUID().equals(INPUT_REPORT_UUID)) {
          pInputReportChar = pChar;
          Serial.println("      📍 Característica INPUT_REPORT salva para polling");
        }
      }
      
      // Tentar ativar HID Control Point se for o serviço HID  
      if (pService->getUUID().equals(HID_SERVICE_UUID) && 
          pChar->getUUID().equals(HID_CONTROL_UUID)) {
        Serial.println("      🎛️  Ativando HID Control Point (comando único)...");
        if (pChar->canWrite()) {
          // Comando simples de ativação (sem sequências complexas)
          uint8_t activateCmd[] = {0x00};  // Exit suspend mode
          pChar->writeValue(activateCmd, 1, true);
          Serial.println("      ✅ HID ativado");
          
          delay(100);  // Pequena pausa
        }
      }
      
      // Se pode ler, tentar ler valor inicial
      if (pChar->canRead()) {
        Serial.println("      Lendo valor inicial...");
        try {
          std::string value = pChar->readValue();
          if (value.length() > 0) {
            Serial.printf("      Valor inicial [%d bytes]: ", value.length());
            for (int i = 0; i < value.length(); i++) {
              Serial.printf("%02X ", (uint8_t)value[i]);
            }
            Serial.println();
          }
        } catch (...) {
          Serial.println("      Erro ao ler valor inicial");
        }
      }
    }
  }

  if (foundInputService) {
    Serial.println("\n✅ Encontrou pelo menos um serviço com notificações!");
    deviceConnected = true;
    return true;
  } else {
    Serial.println("\n❌ Nenhum serviço com notificações encontrado");
    pClient->disconnect();
    return false;
  }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    
    // Conectar ESPECIFICAMENTE ao "VR BOX" (nome exato conforme BigJBehr)
    bool shouldConnect = false;
    
    if (advertisedDevice.haveName()) {
      std::string name = advertisedDevice.getName();
      Serial.printf("🔍 Dispositivo encontrado: '%s'\n", name.c_str());
      
      // Nome exato conforme documentação BigJBehr
      if (name == "VR BOX") {
        shouldConnect = true;
        Serial.println("✅ VRBOX encontrado!");
      } else if (name.find("VR") != std::string::npos || 
                 name.find("vrbox") != std::string::npos ||
                 name.find("VRBOX") != std::string::npos) {
        shouldConnect = true;
        Serial.println("✅ Dispositivo VRBOX encontrado!");
      }
    }
    
    // Se não encontrou pelo nome, verificar se é um dispositivo HID ESPECIFICAMENTE
    if (!shouldConnect && advertisedDevice.haveServiceUUID() && 
        advertisedDevice.isAdvertisingService(HID_SERVICE_UUID)) {
      shouldConnect = true;
      Serial.println("✅ Dispositivo HID encontrado - pode ser VRBOX!");
    }
    
    if (shouldConnect) {
      Serial.println("🎯 Conectando ao VRBOX...");
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    }
  }
};

// ========================================================================
// FUNÇÕES PWM PARA CONTROLE DE CARRO RC
// ========================================================================

// Função para configurar os canais PWM
void setupPWM() {
  Serial.println("🚗 Configurando PWM para controle de carro RC...");
  
  // Configurar canal PWM para servo (direção) - ESP32-C3 específico
  if (!ledcSetup(SERVO_CHANNEL, PWM_FREQ, PWM_RES)) {
    Serial.println("❌ Erro ao configurar canal PWM do servo");
    return;
  }
  ledcAttachPin(SERVO_PIN, SERVO_CHANNEL);
  
  // Configurar canal PWM para ESC (motor) - ESP32-C3 específico
  if (!ledcSetup(ESC_CHANNEL, PWM_FREQ, PWM_RES)) {
    Serial.println("❌ Erro ao configurar canal PWM do ESC");
    return;
  }
  ledcAttachPin(ESC_PIN, ESC_CHANNEL);
  
  // Calcular valor PWM central para 12 bits
  uint32_t centerValue = (PWM_CENTER * 4095UL) / 20000UL;  // 1500μs -> valor PWM
  
  // Inicializar ambos na posição central/neutro
  ledcWrite(SERVO_CHANNEL, centerValue);
  ledcWrite(ESC_CHANNEL, centerValue);
  
  Serial.printf("   🎛️  Servo (direção): GPIO%d, Canal %d\n", SERVO_PIN, SERVO_CHANNEL);
  Serial.printf("   ⚡ ESC (motor): GPIO%d, Canal %d\n", ESC_PIN, ESC_CHANNEL);
  Serial.printf("   📊 Resolução: %d bits, Freq: %dHz\n", PWM_RES, PWM_FREQ);
  Serial.printf("   🎯 Valor central: %d (1500μs)\n", centerValue);
  Serial.println("   ✅ PWM configurado - Posição central/neutro");
}

// Função para converter microssegundos para valor PWM
uint32_t microsToPWMValue(uint16_t micros) {
  // Calcular duty cycle para 50Hz com resolução de 12 bits
  // Period = 1/50 = 20ms = 20000μs
  // PWM Value = (micros / 20000) * 4095 (para 12 bits)
  uint32_t pwmValue = (uint32_t)((micros * 4095UL) / 20000UL);
  
  return pwmValue;
}

// Função para definir PWM em microssegundos
void setPWM(uint8_t channel, uint16_t micros) {
  // Limitar valores dentro do range seguro
  micros = constrain(micros, PWM_MIN, PWM_MAX);
  
  uint32_t pwmValue = microsToPWMValue(micros);
  ledcWrite(channel, pwmValue);
  
  // Debug: mostrar valores PWM calculados
  Serial.printf("🎛️  Canal %d: %dμs -> PWM: %d (%.1f%%)\n", 
                channel, micros, pwmValue, (pwmValue * 100.0) / 4095.0);
}

// Função para mapear valor do joystick (range real do VRBOX) para PWM (1000-2000μs)
uint16_t mapJoystickToPWM(int8_t joystickValue) {
  // Aplicar zona morta
  if (abs(joystickValue) < DEADBAND) {
    return PWM_CENTER;
  }
  
  // AJUSTE: Mapear range real do VRBOX (-36~+36) para 1000~2000μs
  // Baseado nos valores observados: X: 0~36, Y: -36~+36
  return map(joystickValue, -36, 36, PWM_MIN, PWM_MAX);
}

// Função para aplicar filtro de suavização
float smoothPWM(float current, float target, float factor) {
  return current + (target - current) * factor;
}

// Função principal para atualizar controles do carro RC
void updateRCControls() {
  // Obter valores atuais do joystick
  int8_t rawX = (int8_t)(joystickData.directionX * 127);
  int8_t rawY = (int8_t)(joystickData.directionY * 127);
  
  // Mapear para valores PWM
  uint16_t targetServoPWM = mapJoystickToPWM(rawX);    // X = direção
  uint16_t targetESCPWM = mapJoystickToPWM(rawY);      // Y = motor
  
  // Aplicar suavização
  currentServoPWM = smoothPWM(currentServoPWM, targetServoPWM, SMOOTHING_FACTOR);
  currentESCPWM = smoothPWM(currentESCPWM, targetESCPWM, SMOOTHING_FACTOR);
  
  // Atualizar saídas PWM
  setPWM(SERVO_CHANNEL, (uint16_t)currentServoPWM);
  setPWM(ESC_CHANNEL, (uint16_t)currentESCPWM);
  
  // Debug específico do RC Control
  Serial.printf("🚗 RC Control - X:%d->%dμs, Y:%d->%dμs\n", 
                rawX, (uint16_t)currentServoPWM, 
                rawY, (uint16_t)currentESCPWM);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando cliente BLE para joystick VRBOX (HID)...");

  // Configurar PWM para controle de carro RC
  setupPWM();

  BLEDevice::init("");
  
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void loop() {
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("Conectado ao joystick VRBOX HID");
    } else {
      Serial.println("Falha na conexão com o joystick");
    }
    doConnect = false;
  }

  if (!deviceConnected && doScan) {
    Serial.println("Reiniciando scan...");
    BLEDevice::getScan()->start(0);
    doScan = false;
  }

  if (deviceConnected) {
    // Status básico a cada 10 segundos
    static unsigned long lastUpdate = 0;
    static unsigned long connectionTime = millis();
    
    // Polling simplificado apenas para verificar conexão
    if (pInputReportChar != nullptr && millis() - lastPoll > 2000) { // A cada 2 segundos apenas
      try {
        std::string value = pInputReportChar->readValue();
        if (value.length() > 0) {
          // Apenas verificar se a conexão está ativa, sem processar dados
          // (dados já são processados via notificações)
          Serial.printf("� Conexão ativa [%d bytes]\n", value.length());
        }
      } catch (...) {
        Serial.println("❌ Erro na verificação de conexão");
      }
      lastPoll = millis();
    }
    
    // Tentar comandos de ativação VRBOX apenas NA CONEXÃO INICIAL
    if (millis() - lastActivation > 60000) { // Reduzido para 60 segundos
      Serial.println("🔄 Executando comandos de ativação VRBOX (modo minimal)...");
      
      // Buscar serviço HID apenas para manter ativo
      BLERemoteService* pHIDService = pClient->getService(HID_SERVICE_UUID);
      if (pHIDService != nullptr) {
        // Apenas uma leitura simples do Report Map para manter conexão
        BLERemoteCharacteristic* pReportMapChar = pHIDService->getCharacteristic(REPORT_MAP_UUID);
        if (pReportMapChar != nullptr && pReportMapChar->canRead()) {
          Serial.println("   📋 Lendo Report Map para manter conexão...");
          try {
            std::string reportMap = pReportMapChar->readValue();
            Serial.printf("   📋 Report Map: %d bytes - Conexão ativa\n", reportMap.length());
          } catch (...) {
            Serial.println("   ❌ Erro ao ler Report Map");
          }
        }
      }
      
      lastActivation = millis();
    }
    
    if (millis() - lastUpdate > 5000) {  // Status a cada 5 segundos
      Serial.printf("Status: X=%.2f, Y=%.2f, Accel=%.2f, Botões=0x%04X\n",
                    joystickData.directionX, joystickData.directionY,
                    joystickData.acceleration, joystickData.buttons);
      lastUpdate = millis();
    }
    
    // Atualizar controles do carro RC em tempo real
    updateRCControls();
    
    // Reconexão apenas em caso de problemas graves (2 minutos sem atividade)
    if (millis() - connectionTime > 120000 && 
        joystickData.directionX == 0.0 && joystickData.directionY == 0.0 && 
        joystickData.acceleration == 0.0 && joystickData.buttons == 0) {
      Serial.println("🔄 Sem atividade por 2 minutos, verificando conexão...");
        
        // Tentar uma leitura simples antes de desconectar
        if (pInputReportChar != nullptr) {
          try {
            std::string testRead = pInputReportChar->readValue();
            Serial.printf("✅ Conexão OK [%d bytes]\n", testRead.length());
            connectionTime = millis(); // Reset timer se conexão OK
          } catch (...) {
            Serial.println("❌ Conexão perdida, reconectando...");
            if (pClient != nullptr) {
              pClient->disconnect();
              delete pClient;
              pClient = nullptr;
            }
            pInputReportChar = nullptr;
            deviceConnected = false;
            doScan = true;
            connectionTime = millis();
          }
        }
    }
  }

  delay(1000);
}