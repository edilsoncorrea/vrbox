# 🚀 Quick Start Guide - ESP32 VRBOX

> **Status**: ✅ Projeto compilado e pronto para uso!

## ⚡ Início Rápido (5 minutos)

### 1. 📋 Pré-requisitos
- ESP32 DevKit (não ESP32-C3)
- Joystick VRBOX
- Cabo USB
- PlatformIO instalado

### 2. 🔧 Compilar e Upload
```bash
cd c:\dsn\arduino\vrbox
pio run --target upload
```

### 3. 🖥️ Monitorar
```bash
pio device monitor
```

### 4. 🎮 Conectar Joystick
1. Ligar joystick VRBOX (botão power 3-5s)
2. Procurar "ESP32-VRBOX-Receiver" no Bluetooth
3. Parear dispositivo
4. Ver dados no monitor serial!

---

## 🎯 O que você verá funcionando:

### LED do ESP32
- 💡 **Piscando**: Aguardando conexão
- 🔴 **Aceso**: Joystick conectado
- ⚡ **Piscadas rápidas**: Comandos ativos

### Monitor Serial
```
=== ESP32 VRBOX Joystick Receiver ===
Bluetooth inicializado. Nome: ESP32-VRBOX-Receiver
Cliente conectado via Bluetooth
Dados recebidos: 6 bytes
Raw data: 0x01 0x80 0x7F 0x80 0x7F 0x00
Direção: 0.00 | Aceleração: 0.00 | Botões: 0x00
Virando à DIREITA
Acelerando: 75%
Botão 1 pressionado
```

### Controles
- **🕹️ Stick X/Y**: Direção e aceleração (-1.0 a 1.0)
- **🔘 Botões frontais**: Aceleração digital (50% e 100%)
- **⚫ Botões A/B/C/D**: Funções personalizáveis

---

## 🔧 Personalização Rápida

Edite `src/main.cpp` na função `executeCommands()`:

```cpp
void executeCommands() {
    // Seu código aqui!
    
    // Exemplo: Controlar servos
    if (abs(steering) > 0.1) {
        // servo.write(90 + steering * 45);
    }
    
    // Exemplo: Controlar motores
    if (abs(acceleration) > 0.1) {
        // analogWrite(MOTOR_PIN, acceleration * 255);
    }
    
    // Exemplo: LEDs por botões
    if (joystick.buttons & (1 << 0)) { // Botão A
        // digitalWrite(LED_PIN, HIGH);
    }
}
```

---

## ❓ Problemas?

### Não compila?
```bash
pio run --target clean
pio run
```

### Joystick não conecta?
- ✅ LED do joystick piscando?
- ✅ ESP32 padrão (não C3)?
- ✅ Monitor serial aberto?

### Ver mais detalhes?
- 📖 `README.md` - Documentação completa
- 🔍 `CHANGELOG.md` - Histórico de mudanças
- 📊 `COMPILACAO_STATUS.md` - Status técnico
- 📝 `examples.cpp` - Exemplos de código

---

## 🎉 Pronto!

Seu ESP32 agora pode receber comandos do joystick VRBOX via Bluetooth!

**🚀 Próximo passo**: Adicione sua lógica de controle na função `executeCommands()`