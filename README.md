# ESP32 VRBOX Joystick Receiver

> ✅ **Status**: Projeto compilado com sucesso e pronto para uso!

Este projeto implementa um receptor Bluetooth para o joystick VRBOX usando ESP32 e PlatformIO, permitindo controle remoto via HID Bluetooth.

## 📋 Descrição

O projeto permite que um ESP32 receba dados do joystick VRBOX via Bluetooth Classic, processando:
- **🕹️ Stick analógico**: Mapeado para direção (eixo X) e aceleração (eixo Y)
- **🔘 Botões frontais**: Usados para aceleração digital (50% e 100%)
- **⚡ Botões gerais**: Processamento de todos os botões do controle (A/B/C/D)
- **📡 Comunicação HID**: Decodificação completa dos dados do joystick
- **💡 Indicador LED**: Status visual da conexão Bluetooth

## 🔧 Hardware Necessário

- **ESP32** (DevKit, WROOM-32, ou similar) - ⚠️ Não ESP32-C3
- **Joystick VRBOX** (modelo VR Box Remote/Gamepad)
- **Cabo USB** para programação e debug
- **LED interno** no GPIO 2 (automático)

## 📁 Estrutura do Projeto

```
vrbox/
├── platformio.ini              # ✅ Configuração do PlatformIO (ESP32)
├── src/
│   └── main.cpp               # ✅ Código principal compilado
├── include/
│   └── vrbox_joystick.h       # 📚 Definições e protótipos
├── main_bluetooth.cpp          # 🔄 Backup do código original
├── main_ble.cpp               # 🔄 Versão BLE alternativa  
├── examples.cpp               # 📖 Exemplos de implementação
├── COMPILACAO_STATUS.md       # 📊 Status da compilação
└── README.md                  # 📖 Esta documentação
```

## 📊 Status da Compilação

- **✅ Compilação**: Bem-sucedida em ESP32
- **✅ Uso de RAM**: 12.2% (39,820 / 327,680 bytes)
- **✅ Uso de Flash**: 83.8% (1,098,113 / 1,310,720 bytes)
- **✅ Bibliotecas**: BluetoothSerial integrada
- **⚠️ Observação**: ESP32-C3 teve problemas de compatibilidade, usado ESP32 padrão

## ⚙️ Configuração e Instalação

### 1. Pré-requisitos

```bash
# Instalar PlatformIO (se ainda não tiver)
pip install platformio

# Verificar instalação
pio --version
```

### 2. Compilar o Projeto

```bash
# Navegar para o diretório do projeto
cd c:\\dsn\\arduino\\vrbox

# Compilar (já testado e funcionando)
pio run
```

**Resultado esperado**: ✅ `[SUCCESS] Took X.XX seconds`

### 3. Upload para ESP32

```bash
# Conectar ESP32 via USB e fazer upload
pio run --target upload

# Alternativa: especificar porta (se necessário)
pio run --target upload --upload-port COM3
```

### 4. Monitor Serial (Debug)

```bash
# Abrir monitor serial para debug
pio device monitor

# Ou especificar baudrate
pio device monitor --baud 115200
```

## 🎮 Uso do Joystick VRBOX

### Pareamento Bluetooth

1. **🔌 Conecte e faça upload** do código para o ESP32
2. **📱 Ligue o joystick VRBOX** (geralmente segurando o botão power por 3-5s)
3. **🔍 Procure dispositivos Bluetooth** no seu dispositivo
4. **🔗 Conecte ao dispositivo** "ESP32-VRBOX-Receiver"
5. **💻 Abra o monitor serial** (`pio device monitor`) para ver os dados

### Indicadores de Status (LED GPIO 2)

| Estado do LED | Significado |
|---------------|-------------|
| 💡 **Piscando lento** | Aguardando conexão Bluetooth |
| 🔴 **Aceso fixo** | Joystick conectado e funcionando |
| ⚡ **Piscadas rápidas** | Comandos de direção sendo executados |

### Controles e Mapeamentos

| Controle | Função | Saída no Serial | Range |
|----------|--------|-----------------|-------|
| **🕹️ Stick X** | Direção | `Direção: -0.50` | -1.0 (esquerda) a 1.0 (direita) |
| **🕹️ Stick Y** | Aceleração | `Aceleração: 0.75` | -1.0 (ré) a 1.0 (frente) |
| **🔘 Botão Frontal 1** | Aceleração mínima | `Botão frontal 1 ativo` | Força 50% de aceleração |
| **🔘 Botão Frontal 2** | Aceleração máxima | `Botão frontal 2 ativo` | Força 100% de aceleração |
| **⚫ Botões A/B/C/D** | Funções auxiliares | `Botão X pressionado` | Eventos digitais |

### Zona Morta Configurada

- **📏 Zona morta**: ±1000 no stick analógico
- **🎯 Propósito**: Evitar drift e inputs acidentais
- **⚙️ Configurável**: Altere `STICK_DEADZONE` no código

## 🔍 Debug e Troubleshooting

### Saída do Monitor Serial

Quando funcionando corretamente, você verá:

```
=== ESP32 VRBOX Joystick Receiver ===
Inicializando Bluetooth...
Bluetooth inicializado. Nome do dispositivo: ESP32-VRBOX-Receiver
Sistema pronto!
Para conectar o joystick VRBOX:
1. Ligue o joystick
2. Procure por 'ESP32-VRBOX-Receiver' na lista de dispositivos
3. Pareie o dispositivo
Aguardando conexão do joystick VRBOX...
Cliente conectado via Bluetooth
Dados recebidos: 6 bytes
Raw data: 0x01 0x80 0x7F 0x80 0x7F 0x00
Direção: 0.00 | Aceleração: 0.00 | Botões: 0x00
```

### Problemas Comuns e Soluções

#### ❌ **Erro de Compilação**
```bash
# Limpar e recompilar
pio run --target clean
pio run
```

#### ❌ **Joystick não conecta**
- ✅ Verifique se o joystick está em modo de pareamento (LED piscando)
- ✅ Tente resetar o Bluetooth: desligue/ligue o joystick
- ✅ Verifique se não há outros dispositivos conectados ao joystick
- ✅ Reinicie o ESP32 (`pio run --target upload`)

#### ❌ **Dados não chegam**
- ✅ Confirme conexão: LED do ESP32 deve estar aceso fixo
- ✅ Verifique monitor serial em 115200 baud
- ✅ Teste movendo o stick - deve aparecer "Raw data"
- ✅ Verifique se o joystick está enviando dados HID

#### ❌ **Valores erráticos**
- ✅ Ajuste zona morta: altere `STICK_DEADZONE` no código
- ✅ Verifique formato dos dados: analise "Raw data" no serial
- ✅ Calibre o joystick se possível

#### ❌ **ESP32-C3 não funciona**
- ⚠️ **Usar ESP32 padrão**: Projeto otimizado para ESP32 clássico
- ✅ Para ESP32-C3: usar versão BLE (`main_ble.cpp`)

### Versões Alternativas

Se o Bluetooth Classic não funcionar:

```bash
# Usar versão BLE
move src\main.cpp src\main_classic.cpp
move main_ble.cpp src\main.cpp
pio run
```

## 📊 Formato dos Dados

### Bluetooth Classic (main.cpp)
```
Byte 0: Report ID
Bytes 1-2: Eixo X (Little Endian, 16-bit)
Bytes 3-4: Eixo Y (Little Endian, 16-bit)  
Byte 5: Botões (8 bits)
```

### BLE (main_ble.cpp)
```
Byte 0: Eixo X (0-255, centro = 128)
Byte 1: Eixo Y (0-255, centro = 128)
Byte 2: Botões (8 bits)
```

## 🛠️ Personalização e Desenvolvimento

### Modificar Mapeamento dos Controles

Edite as funções em `src/main.cpp`:

```cpp
// Personalizar mapeamento de direção
float mapSteering(int16_t analogX) {
    if (abs(analogX) < STICK_DEADZONE) {
        return 0.0; // Zona morta
    }
    
    // Customizar sensibilidade: multiplicar por fator
    return (float)analogX / 32767.0 * SENSITIVITY_FACTOR;
}

// Personalizar mapeamento de aceleração
float mapAcceleration(int16_t analogY) {
    if (abs(analogY) < STICK_DEADZONE) {
        return 0.0;
    }
    
    // Inverter Y e aplicar curva personalizada
    float normalized = -(float)analogY / 32767.0;
    return pow(abs(normalized), ACCELERATION_CURVE) * (normalized >= 0 ? 1 : -1);
}

// Adicionar sua lógica de controle
void executeCommands() {
    // Exemplo: Controle de motores
    if (abs(steering) > 0.1) {
        // analogWrite(MOTOR_LEFT, baseSpeed + steering * maxSteer);
        // analogWrite(MOTOR_RIGHT, baseSpeed - steering * maxSteer);
    }
    
    // Exemplo: Controle de servos
    if (abs(acceleration) > 0.1) {
        // servo.write(90 + acceleration * 90);
    }
    
    // Exemplo: Ações por botões
    if (joystick.buttons & (1 << 0)) { // Botão A
        // digitalWrite(LED_PIN, HIGH);
    }
}
```

### Exemplos de Aplicação

O arquivo `examples.cpp` contém implementações para:

1. **🚗 Controle de Carro RC**: Servo para direção + motor para velocidade
2. **🚁 Controle de Drone**: Roll, pitch, throttle, yaw
3. **🤖 Robô Diferencial**: Tank drive com duas rodas
4. **📡 Transmissão via Rede**: UDP/TCP para controle remoto
5. **⚙️ Filtros de Input**: Suavização e anti-ruído

### Adicionar Bibliotecas

Para expandir funcionalidades, adicione ao `platformio.ini`:

```ini
lib_deps = 
    ESP32Servo                    # Controle de servos
    FastLED                       # LEDs RGB
    ArduinoJson                   # Comunicação JSON
    PubSubClient                  # MQTT
    AsyncTCP                      # Comunicação TCP assíncrona
    ESPAsyncWebServer            # Servidor web
```

### Configurações Avançadas

```ini
# Aumentar frequência do processador
board_build.f_cpu = 240000000L

# Habilitar debug via JTAG
debug_tool = esp-prog
debug_init_break = tbreak setup

# Particionamento personalizado
board_build.partitions = custom_partitions.csv

# Otimizações de compilação
build_flags = 
    -O3                          # Máxima otimização
    -DCORE_DEBUG_LEVEL=0         # Sem debug (mais rápido)
    -DARDUINO_USB_CDC_ON_BOOT=1  # USB CDC nativo
```

## 📡 Especificações Técnicas

### Hardware
- **🔧 Microcontrolador**: ESP32 (Dual Core, 240 MHz)
- **💾 RAM**: 320KB SRAM
- **💽 Flash**: 4MB (padrão)
- **📻 Conectividade**: WiFi 802.11 b/g/n + Bluetooth Classic 4.2
- **🔌 GPIO**: GPIO 2 usado para LED indicador

### Software
- **⚙️ Plataforma**: PlatformIO + Arduino Framework
- **📊 Baudrate Serial**: 115200 bps
- **🎯 Resolução Analógica**: 16-bit (-32768 a 32767)
- **⏱️ Frequência de Atualização**: ~100Hz (delay 10ms no loop)
- **📶 Bluetooth**: Classic Serial Profile (SPP)

### Consumo de Recursos (Compilado)
- **🐏 RAM Utilizada**: 39,820 bytes (12.2% de 327,680 bytes)
- **💿 Flash Utilizada**: 1,098,113 bytes (83.8% de 1,310,720 bytes)
- **📚 Bibliotecas**: BluetoothSerial (nativa do ESP32)

### Protocolo HID
```
Formato de dados esperado do VRBOX:
Byte 0: Report ID
Bytes 1-2: Eixo X (Little Endian, 16-bit signed)
Bytes 3-4: Eixo Y (Little Endian, 16-bit signed)  
Byte 5: Botões (8 bits, bit mask)
  - Bit 0-5: Botões A, B, C, D, Up, Down
  - Bit 6-7: Botões frontais 1 e 2
```

## 🔗 Referências e Links Úteis

### Documentação Oficial
- 📖 [ESP32 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)
- 🛠️ [PlatformIO ESP32](https://docs.platformio.org/en/latest/platforms/espressif32.html)
- 📋 [Arduino ESP32 Core](https://github.com/espressif/arduino-esp32)

### Bluetooth e HID
- 📡 [Bluetooth HID Profile](https://www.bluetooth.com/specifications/specs/human-interface-device-profile-1-1-1/)
- 🎮 [HID Usage Tables](https://usb.org/sites/default/files/hut1_4.pdf)
- � [ESP32 Bluetooth Classic](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/classic_bt.html)

### Joystick VRBOX
- 🕹️ [VR BOX Remote Gamepad](https://www.aliexpress.com/item/VR-BOX-Remote.html) - Exemplo de produto
- 📱 [Bluetooth Gamepad Apps](https://play.google.com/store/search?q=bluetooth%20gamepad) - Apps compatíveis
- 🔍 [HID Scanner Tools](https://github.com/NordicSemiconductor/Android-nRF-Connect) - Para debug Bluetooth

## 🤝 Contribuição e Suporte

### Como Contribuir
1. 🍴 **Fork** este repositório
2. 🔧 **Crie uma branch** para sua feature (`git checkout -b feature/MinhaFeature`)
3. 💾 **Commit** suas mudanças (`git commit -m 'Adiciona MinhaFeature'`)
4. 📤 **Push** para a branch (`git push origin feature/MinhaFeature`)
5. 🔀 **Abra um Pull Request**

### Reportar Problemas
- 🐛 **Issues**: Use o sistema de issues do GitHub
- 📧 **Contato**: Descreva o problema com detalhes
- 📊 **Logs**: Inclua saída do monitor serial se possível
- 🔧 **Hardware**: Especifique modelo do ESP32 e joystick

### Melhorias Futuras
- 🎮 **Mais joysticks**: Suporte para outros modelos
- 📱 **App companion**: Interface mobile para configuração
- 🤖 **Robôs específicos**: Templates para diferentes robôs
- 🌐 **WiFi integration**: Controle via rede local
- 📊 **Dashboard web**: Interface de monitoramento

## 📝 Changelog

### v1.0.0 (Atual)
- ✅ Implementação básica do Bluetooth Classic
- ✅ Mapeamento completo do joystick VRBOX
- ✅ Sistema de debug via Serial
- ✅ LED indicador de status
- ✅ Zona morta configurável
- ✅ Suporte para ESP32 padrão
- ✅ Documentação completa

### Planejado para v1.1.0
- 🔄 Suporte melhorado para ESP32-C3
- 📱 Versão BLE estável
- 🎛️ Configurações via web interface
- 📊 Telemetria avançada

## 📄 Licença

Este projeto é de **código aberto** sob a licença MIT. 

```
MIT License - Use, modifique e distribua livremente!
Veja o arquivo LICENSE para detalhes completos.
```

---

> 💡 **Dica**: Para dúvidas específicas, consulte os arquivos `examples.cpp` e `COMPILACAO_STATUS.md` no projeto!

**🚀 Bom desenvolvimento e divirta-se com seu projeto ESP32 + VRBOX!** 🎮