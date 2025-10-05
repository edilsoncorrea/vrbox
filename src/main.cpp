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
 * - ✅ Zona morta e filtros de suavização=====================================          LOG_VERBOSE("🔗 Conexão ativa [%d bytes]\n", value.length());         LOG_VERBOSE("🔗 Conexão ativa [%d bytes]\n", value.length());========================
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
// SISTEMA DE LOGGING COM NÍVEIS DE PRIORIDADE
// ========================================================================
// Níveis de debug:
// 0 = NENHUM LOG (apenas erros críticos)
// 1 = CRÍTICO (conexões, falhas importantes)
// 2 = IMPORTANTE (dados do joystick, PWM)
// 3 = INFORMATIVO (descoberta de serviços, ativações)
// 4 = DEBUG (polling, verificações)
// 5 = VERBOSE (todos os detalhes, hex dumps)

/*
 * Sistema de Logging com Níveis de Prioridade
 * ===========================================
 * 
 * O sistema permite controlar a verbosidade dos debug prints através da constante DEBUG_LEVEL.
 * 
 * Níveis disponíveis:
 * 0 = Nenhum debug (silencioso)
 * 1 = Apenas mensagens CRÍTICAS (erros graves) 🔴
 * 2 = IMPORTANTES (conexões BLE, PWM, Joystick, RC) 🟡📡🎛️🕹️🚗
 * 3 = INFORMAÇÕES (status geral, padrão recomendado) 🔵
 * 4 = DEBUG (informações detalhadas para desenvolvimento) 🟢
 * 5 = VERBOSE (todos os detalhes, dados de rede) ⚪
 * 
 * Macros disponíveis:
 * - LOG_CRITICAL() : Erros críticos (sempre importantes)
 * - LOG_IMPORTANT(): Eventos importantes (conexões, inicializações)
 * - LOG_INFO()     : Informações gerais (status, estado atual) 
 * - LOG_DEBUG()    : Debug detalhado (desenvolvimento)
 * - LOG_VERBOSE()  : Informações muito detalhadas
 * 
 * Macros especializadas (nível 2+):
 * - LOG_BLE()      : Específico para Bluetooth Low Energy
 * - LOG_PWM()      : Específico para controle PWM
 * - LOG_JOYSTICK() : Específico para dados do joystick
 * - LOG_RC()       : Específico para controle RC
 */
#define DEBUG_LEVEL 3  // ← ALTERE AQUI PARA CONTROLAR O NÍVEL DE DEBUG

// Macros para diferentes níveis de log
#define LOG_CRITICAL(fmt, ...) if(DEBUG_LEVEL >= 1) Serial.printf("🔴 " fmt, ##__VA_ARGS__)
#define LOG_IMPORTANT(fmt, ...) if(DEBUG_LEVEL >= 2) Serial.printf("🟡 " fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) if(DEBUG_LEVEL >= 3) Serial.printf("🔵 " fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) if(DEBUG_LEVEL >= 4) Serial.printf("🟢 " fmt, ##__VA_ARGS__)
#define LOG_VERBOSE(fmt, ...) if(DEBUG_LEVEL >= 5) Serial.printf("⚪ " fmt, ##__VA_ARGS__)

// Macros especiais para diferentes categorias
#define LOG_BLE(fmt, ...) if(DEBUG_LEVEL >= 2) Serial.printf("📡 " fmt, ##__VA_ARGS__)
#define LOG_PWM(fmt, ...) if(DEBUG_LEVEL >= 2) Serial.printf("🎛️ " fmt, ##__VA_ARGS__)
#define LOG_JOYSTICK(fmt, ...) if(DEBUG_LEVEL >= 2) Serial.printf("🕹️ " fmt, ##__VA_ARGS__)
#define LOG_RC(fmt, ...) if(DEBUG_LEVEL >= 2) Serial.printf("🚗 " fmt, ##__VA_ARGS__)

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
  LOG_VERBOSE("📨 Dados recebidos [%d bytes]: ", length);
  
  // Exibe dados em hexadecimal
  if(DEBUG_LEVEL >= 5) {
    for (int i = 0; i < length; i++) {
      Serial.printf("%02X ", pData[i]);
    }
    Serial.println();
  }
  
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
    
    LOG_JOYSTICK("JOYSTICK - X: %d (%.2f), Y: %d (%.2f), Triggers: 0x%02X [L:%s, U:%s]\n",
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
      
      LOG_INFO("🎮 BOTÕES A/B: 0x%02X [A:%s, B:%s] - Auto-repeat\n",
                    buttons, 
                    buttonA ? "ON" : "OFF",
                    buttonB ? "ON" : "OFF");
      
      // Atualizar estrutura
      joystickData.buttons |= buttons;
      
    } else {
      // BOTÕES C/D (sem auto-repeat)
      bool buttonC = (buttonData & 0x01) != 0;
      bool buttonD = (buttonData & 0x02) != 0;
      
      LOG_INFO("🎮 BOTÕES C/D: 0x%02X [C:%s, D:%s] - Single press\n",
                    buttonData,
                    buttonC ? "ON" : "OFF", 
                    buttonD ? "ON" : "OFF");
      
      // Atualizar estrutura  
      joystickData.buttons |= (buttonData << 8);  // Shift para posição alta
    }
    
  } else {
    // ❓ FORMATO DESCONHECIDO
    LOG_CRITICAL("❓ Formato desconhecido (%d bytes), exibindo raw:\n", length);
    if(DEBUG_LEVEL >= 1) {
      for (int i = 0; i < length; i++) {
        Serial.printf("  [%d]: 0x%02X (%d)\n", i, pData[i], pData[i]);
      }
    }
  }
}

// Função para ler Report Map e entender o formato dos dados
void readReportMap(BLERemoteService* pRemoteService) {
  LOG_DEBUG("Tentando ler Report Map...\n");
  
  BLERemoteCharacteristic* pReportMapChar = pRemoteService->getCharacteristic(REPORT_MAP_UUID);
  if (pReportMapChar != nullptr) {
    if (pReportMapChar->canRead()) {
      std::string reportMap = pReportMapChar->readValue();
      LOG_VERBOSE("Report Map lido [%d bytes]: ", reportMap.length());
      
      if(DEBUG_LEVEL >= 5) {
        for (int i = 0; i < reportMap.length(); i++) {
          Serial.printf("%02X ", (uint8_t)reportMap[i]);
          if ((i + 1) % 16 == 0) Serial.println();
        }
        Serial.println();
      }
    }
  } else {
    LOG_DEBUG("Report Map characteristic não encontrada\n");
  }
}

// Função para ler HID Information
void readHIDInfo(BLERemoteService* pRemoteService) {
  LOG_DEBUG("Tentando ler HID Info...\n");
  
  BLERemoteCharacteristic* pHIDInfoChar = pRemoteService->getCharacteristic(HID_INFO_UUID);
  if (pHIDInfoChar != nullptr) {
    if (pHIDInfoChar->canRead()) {
      std::string hidInfo = pHIDInfoChar->readValue();
      LOG_VERBOSE("HID Info [%d bytes]: ", hidInfo.length());
      
      if(DEBUG_LEVEL >= 5) {
        for (int i = 0; i < hidInfo.length(); i++) {
          Serial.printf("%02X ", (uint8_t)hidInfo[i]);
        }
        Serial.println();
      }
    }
  }
}

bool connectToServer() {
  LOG_IMPORTANT("Conectando ao dispositivo: %s\n", myDevice->getAddress().toString().c_str());
  
  pClient = BLEDevice::createClient();
  LOG_INFO("Cliente BLE criado\n");

  // Conectar ao servidor remoto
  if (!pClient->connect(myDevice)) {
    LOG_CRITICAL("Falha na conexão\n");
    return false;
  }
  LOG_IMPORTANT("Conectado ao servidor\n");

  // Descobrir TODOS os serviços disponíveis
  LOG_INFO("Descobrindo todos os serviços...\n");
  std::map<std::string, BLERemoteService*>* services = pClient->getServices();
  
  LOG_INFO("Encontrados %d serviços:\n", services->size());
  
  bool foundInputService = false;
  
  for (auto& servicePair : *services) {
    BLERemoteService* pService = servicePair.second;
    LOG_VERBOSE("\n=== Serviço UUID: %s ===\n", pService->getUUID().toString().c_str());
    
    // Listar todas as características deste serviço
    std::map<std::string, BLERemoteCharacteristic*>* characteristics = pService->getCharacteristics();
    LOG_VERBOSE("  Características encontradas: %d\n", characteristics->size());
    
    for (auto& charPair : *characteristics) {
      BLERemoteCharacteristic* pChar = charPair.second;
      LOG_VERBOSE("    Característica UUID: %s\n", pChar->getUUID().toString().c_str());
      
      if(DEBUG_LEVEL >= 5) {
        Serial.printf("      Propriedades: ");
        if (pChar->canRead()) Serial.print("READ ");
        if (pChar->canWrite()) Serial.print("WRITE ");
        if (pChar->canNotify()) Serial.print("NOTIFY ");
        if (pChar->canIndicate()) Serial.print("INDICATE ");
        Serial.println();
      }
      
      // Se pode notificar, registrar callback
      if (pChar->canNotify()) {
        LOG_INFO("      Registrando callback para notificações\n");
        pChar->registerForNotify(notifyCallback);
        foundInputService = true;
        
        // Habilitar notificações via descriptor
        BLERemoteDescriptor* pDescriptor = pChar->getDescriptor(DESCRIPTOR_UUID);
        if (pDescriptor != nullptr) {
          uint8_t notifyValue[] = {0x01, 0x00};
          pDescriptor->writeValue(notifyValue, 2, true);
          LOG_INFO("      Notificações habilitadas via descriptor\n");
        }
        
        // Se for a característica INPUT_REPORT, guardar referência para polling
        if (pService->getUUID().equals(HID_SERVICE_UUID) && 
            pChar->getUUID().equals(INPUT_REPORT_UUID)) {
          pInputReportChar = pChar;
          LOG_INFO("      📍 Característica INPUT_REPORT salva para polling\n");
        }
      }
      
      // Tentar ativar HID Control Point se for o serviço HID  
      if (pService->getUUID().equals(HID_SERVICE_UUID) && 
          pChar->getUUID().equals(HID_CONTROL_UUID)) {
        LOG_INFO("      🎛️  Ativando HID Control Point (comando único)...\n");
        if (pChar->canWrite()) {
          // Comando simples de ativação (sem sequências complexas)
          uint8_t activateCmd[] = {0x00};  // Exit suspend mode
          pChar->writeValue(activateCmd, 1, true);
          LOG_INFO("      ✅ HID ativado\n");
          
          delay(100);  // Pequena pausa
        }
      }
      
      // Se pode ler, tentar ler valor inicial
      if (pChar->canRead()) {
        LOG_VERBOSE("      Lendo valor inicial...\n");
        try {
          std::string value = pChar->readValue();
          if (value.length() > 0) {
            LOG_VERBOSE("      Valor inicial [%d bytes]: ", value.length());
            if(DEBUG_LEVEL >= 5) {
              for (int i = 0; i < value.length(); i++) {
                Serial.printf("%02X ", (uint8_t)value[i]);
              }
              Serial.println();
            }
          }
        } catch (...) {
          LOG_DEBUG("      Erro ao ler valor inicial\n");
        }
      }
    }
  }

  if (foundInputService) {
    LOG_IMPORTANT("\n✅ Encontrou pelo menos um serviço com notificações!\n");
    deviceConnected = true;
    return true;
  } else {
    LOG_CRITICAL("\n❌ Nenhum serviço com notificações encontrado\n");
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
      LOG_DEBUG("🔍 Dispositivo encontrado: '%s'\n", name.c_str());
      
      // Nome exato conforme documentação BigJBehr
      if (name == "VR BOX") {
        shouldConnect = true;
        LOG_IMPORTANT("✅ VRBOX encontrado!\n");
      } else if (name.find("VR") != std::string::npos || 
                 name.find("vrbox") != std::string::npos ||
                 name.find("VRBOX") != std::string::npos) {
        shouldConnect = true;
        LOG_IMPORTANT("✅ Dispositivo VRBOX encontrado!\n");
      }
    }
    
    // Se não encontrou pelo nome, verificar se é um dispositivo HID ESPECIFICAMENTE
    if (!shouldConnect && advertisedDevice.haveServiceUUID() && 
        advertisedDevice.isAdvertisingService(HID_SERVICE_UUID)) {
      shouldConnect = true;
      LOG_INFO("✅ Dispositivo HID encontrado - pode ser VRBOX!\n");
    }
    
    if (shouldConnect) {
      LOG_IMPORTANT("🎯 Conectando ao VRBOX...\n");
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
  LOG_IMPORTANT("🚗 Configurando PWM para controle de carro RC...\n");
  
  // Configurar canal PWM para servo (direção) - ESP32-C3 específico
  if (!ledcSetup(SERVO_CHANNEL, PWM_FREQ, PWM_RES)) {
    LOG_CRITICAL("❌ Erro ao configurar canal PWM do servo\n");
    return;
  }
  ledcAttachPin(SERVO_PIN, SERVO_CHANNEL);
  
  // Configurar canal PWM para ESC (motor) - ESP32-C3 específico
  if (!ledcSetup(ESC_CHANNEL, PWM_FREQ, PWM_RES)) {
    LOG_CRITICAL("❌ Erro ao configurar canal PWM do ESC\n");
    return;
  }
  ledcAttachPin(ESC_PIN, ESC_CHANNEL);
  
  // Calcular valor PWM central para 12 bits
  uint32_t centerValue = (PWM_CENTER * 4095UL) / 20000UL;  // 1500μs -> valor PWM
  
  // Inicializar ambos na posição central/neutro
  ledcWrite(SERVO_CHANNEL, centerValue);
  ledcWrite(ESC_CHANNEL, centerValue);
  
  LOG_INFO("   🎛️  Servo (direção): GPIO%d, Canal %d\n", SERVO_PIN, SERVO_CHANNEL);
  LOG_INFO("   ⚡ ESC (motor): GPIO%d, Canal %d\n", ESC_PIN, ESC_CHANNEL);
  LOG_INFO("   📊 Resolução: %d bits, Freq: %dHz\n", PWM_RES, PWM_FREQ);
  LOG_INFO("   🎯 Valor central: %d (1500μs)\n", centerValue);
  LOG_IMPORTANT("   ✅ PWM configurado - Posição central/neutro\n");
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
  LOG_PWM("Canal %d: %dμs -> PWM: %d (%.1f%%)\n", 
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
  LOG_RC("RC Control - X:%d->%dμs, Y:%d->%dμs\n", 
                rawX, (uint16_t)currentServoPWM, 
                rawY, (uint16_t)currentESCPWM);
}

void setup() {
  Serial.begin(115200);
  LOG_CRITICAL("Iniciando cliente BLE para joystick VRBOX (HID)...\n");

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
      LOG_IMPORTANT("Conectado ao joystick VRBOX HID\n");
    } else {
      LOG_CRITICAL("Falha na conexão com o joystick\n");
    }
    doConnect = false;
  }

  if (!deviceConnected && doScan) {
    LOG_INFO("Reiniciando scan...\n");
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
        LOG_CRITICAL("❌ Erro na verificação de conexão\n");
      }
      lastPoll = millis();
    }
    
    // Tentar comandos de ativação VRBOX apenas NA CONEXÃO INICIAL
    if (millis() - lastActivation > 60000) { // Reduzido para 60 segundos
      LOG_DEBUG("🔄 Executando comandos de ativação VRBOX (modo minimal)...\n");
      
      // Buscar serviço HID apenas para manter ativo
      BLERemoteService* pHIDService = pClient->getService(HID_SERVICE_UUID);
      if (pHIDService != nullptr) {
        // Apenas uma leitura simples do Report Map para manter conexão
        BLERemoteCharacteristic* pReportMapChar = pHIDService->getCharacteristic(REPORT_MAP_UUID);
        if (pReportMapChar != nullptr && pReportMapChar->canRead()) {
          LOG_DEBUG("   📋 Lendo Report Map para manter conexão...\n");
          try {
            std::string reportMap = pReportMapChar->readValue();
            LOG_VERBOSE("   📋 Report Map: %d bytes - Conexão ativa\n", reportMap.length());
          } catch (...) {
            LOG_CRITICAL("   ❌ Erro ao ler Report Map\n");
          }
        }
      }
      
      lastActivation = millis();
    }
    
    if (millis() - lastUpdate > 5000) {  // Status a cada 5 segundos
      LOG_INFO("Status: X=%.2f, Y=%.2f, Accel=%.2f, Botões=0x%04X\n",
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
      LOG_DEBUG("🔄 Sem atividade por 2 minutos, verificando conexão...\n");
        
        // Tentar uma leitura simples antes de desconectar
        if (pInputReportChar != nullptr) {
          try {
            std::string testRead = pInputReportChar->readValue();
            LOG_VERBOSE("✅ Conexão OK [%d bytes]\n", testRead.length());
            connectionTime = millis(); // Reset timer se conexão OK
          } catch (...) {
            LOG_CRITICAL("❌ Conexão perdida, reconectando...\n");
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