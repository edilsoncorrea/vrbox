# 📋 CHANGELOG - ESP32 VRBOX Joystick Receiver

Todas as mudanças notáveis neste projeto serão documentadas neste arquivo.

## [1.0.0] - 2025-10-04 ✅ ATUAL

### ✨ Adicionado
- **Implementação completa** do receptor Bluetooth para joystick VRBOX
- **Suporte ESP32** padrão (migrado de ESP32-C3 por compatibilidade)
- **BluetoothSerial** nativo para comunicação estável
- **Mapeamento analógico** completo do stick (X=direção, Y=aceleração)
- **Sistema de zona morta** configurável (±1000) para evitar drift
- **LED indicador** de status da conexão (GPIO 2)
- **Debug serial** completo com dados HID raw em hexadecimal
- **Processamento de botões** frontais para aceleração digital
- **Estrutura modular** com funções separadas para cada funcionalidade

### 🔧 Configuração Técnica
- **Platform**: espressif32 @ 6.12.0
- **Board**: esp32dev (ESP32 DevKit)
- **Framework**: Arduino
- **Baudrate**: 115200 bps
- **Bluetooth**: Classic Serial Profile (SPP)

### 📊 Recursos Utilizados
- **RAM**: 39,820 bytes (12.2% de 327,680 bytes)
- **Flash**: 1,098,113 bytes (83.8% de 1,310,720 bytes)
- **Bibliotecas**: BluetoothSerial @ 2.0.0

### 🎯 Funcionalidades Implementadas
- [x] Pareamento automático com joystick VRBOX
- [x] Decodificação de dados HID (6 bytes padrão)
- [x] Mapeamento de stick analógico para float (-1.0 a 1.0)
- [x] Processamento de 8 botões (A/B/C/D + frontais + direcionais)
- [x] Aceleração digital via botões frontais (50% e 100%)
- [x] Indicação visual via LED (piscando=aguardando, aceso=conectado)
- [x] Debug em tempo real via Serial Monitor

### 📁 Arquivos Criados
- `src/main.cpp` - Código principal funcional
- `include/vrbox_joystick.h` - Definições e protótipos
- `platformio.ini` - Configuração do projeto
- `README.md` - Documentação completa
- `COMPILACAO_STATUS.md` - Status detalhado da compilação
- `examples.cpp` - Exemplos de implementação (backup)
- `main_bluetooth.cpp` - Backup do código original
- `main_ble.cpp` - Versão BLE alternativa

### 🔄 Migração ESP32-C3 → ESP32
- **Problema**: ESP32-C3 com erro `pins_arduino.h: No such file or directory`
- **Solução**: Migração para ESP32 padrão com melhor suporte
- **Resultado**: Compilação 100% bem-sucedida

---

## [0.3.0] - 2025-10-04 (Desenvolvimento)

### 🔧 Tentativas de Correção ESP32-C3
- Testado diferentes boards: `esp32-c3-devkitm-1`, `esp32-c3-devkitc-02`
- Tentativa de versão específica do framework Arduino
- Limpeza e reinstalação da plataforma
- **Resultado**: Incompatibilidade persistente

### ⚠️ Problemas Identificados
- Framework Arduino ESP32 v3.x com incompatibilidade em ESP32-C3
- Arquivo `pins_arduino.h` ausente em todas as tentativas
- Conflitos de configuração do Bluetooth Classic

---

## [0.2.0] - 2025-10-04 (Desenvolvimento)

### ✨ Adicionado
- Implementação inicial do código ESP32-C3
- Configuração do PlatformIO para ESP32-C3
- Estrutura de projeto base
- Código de exemplo com LEDs
- Configurações de Bluetooth Classic e BLE

### ❌ Problemas Encontrados
- Erro de compilação com `pins_arduino.h`
- Biblioteca BLE conflitando
- Framework Arduino ESP32 instável

---

## [0.1.0] - 2025-10-04 (Inicial)

### 🎯 Planejamento Inicial
- Criação da estrutura de diretórios
- Definição dos requisitos do projeto
- Pesquisa sobre joystick VRBOX
- Configuração inicial do PlatformIO
- Todo list para desenvolvimento

### 📋 Objetivos Definidos
- Comunicação Bluetooth com joystick VRBOX
- Mapeamento de stick analógico para direção/aceleração
- Processamento de botões frontais
- Sistema de debug e monitoramento

---

## 🔮 Roadmap Futuro

### [1.1.0] - Planejado
- [ ] **ESP32-C3 Support**: Resolver incompatibilidades
- [ ] **BLE Stable**: Versão BLE totalmente funcional
- [ ] **Web Interface**: Configuração via browser
- [ ] **Multiple Joysticks**: Suporte para diferentes modelos
- [ ] **Motor Control**: Templates para controle de motores

### [1.2.0] - Planejado
- [ ] **WiFi Integration**: Controle via rede
- [ ] **MQTT Support**: Integração com IoT
- [ ] **Mobile App**: Aplicativo companion
- [ ] **OTA Updates**: Atualizações via WiFi
- [ ] **Telemetry**: Dashboard de monitoramento

### [2.0.0] - Futuro
- [ ] **Multi-Protocol**: Suporte WiFi Direct, LoRa
- [ ] **AI Integration**: Controle assistido por IA
- [ ] **VR Support**: Integração com headsets VR
- [ ] **Robot Templates**: Bibliotecas para robôs específicos

---

## 🐛 Issues Conhecidos

### Críticos
- **ESP32-C3**: Incompatibilidade com framework atual
- **Pins Arduino**: Arquivo não encontrado na compilação

### Menores  
- **Warnings**: Redefinição de `CONFIG_BLUEDROID_ENABLED` (não afeta funcionamento)
- **Flash Usage**: 83.8% pode ser otimizado

### Contornos Implementados
- ✅ **ESP32 padrão**: Funciona perfeitamente
- ✅ **BLE alternativo**: Código backup disponível
- ✅ **Debug extensivo**: Monitor serial detalhado

---

## 🔧 Notas Técnicas

### Compilação
```bash
# Última compilação bem-sucedida
Platform: Espressif 32 (6.12.0) > Espressif ESP32 Dev Module
Framework: Arduino
Toolchain: xtensa-esp32 @ 8.4.0+2021r2-patch5
Result: [SUCCESS] Took 15.52 seconds
```

### Dependências
```ini
lib_deps = BluetoothSerial @ 2.0.0 (nativa)
build_flags = -DCONFIG_BT_ENABLED=1 -DCONFIG_BLUEDROID_ENABLED=1
```

### Ambiente de Desenvolvimento
- **OS**: Windows 11
- **PlatformIO**: Core 6.1.18
- **Python**: 3.13
- **Terminal**: PowerShell 7.x

---

*📝 Este changelog segue o formato [Keep a Changelog](https://keepachangelog.com/) e usa [Versionamento Semântico](https://semver.org/).*