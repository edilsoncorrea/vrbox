# ESP32 VRBOX Joystick Receiver

[![PlatformIO CI](https://github.com/edilsoncorrea/vrbox/actions/workflows/ci.yml/badge.svg)](https://github.com/edilsoncorrea/vrbox/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/github/v/release/edilsoncorrea/vrbox)](https://github.com/edilsoncorrea/vrbox/releases)
[![ESP32](https://img.shields.io/badge/ESP32-Compatible-green.svg)](https://www.espressif.com/en/products/socs/esp32)

> ✅ **Status**: Projeto compilado com sucesso e pronto para uso!

Receptor Bluetooth para joystick VRBOX usando ESP32 e PlatformIO. Permite controle remoto via HID Bluetooth com mapeamento completo de stick analógico e botões.

## 🚀 Quick Start

```bash
# Clone o repositório
git clone https://github.com/edilsoncorrea/vrbox.git
cd vrbox

# Compile e faça upload
pio run --target upload

# Monitor serial
pio device monitor
```

## 📋 Recursos Principais

- **🎮 Joystick VRBOX**: Comunicação Bluetooth Classic completa
- **🕹️ Stick Analógico**: Mapeamento para direção e aceleração (-1.0 a 1.0)
- **🔘 Botões Digitais**: Processamento de todos os botões incluindo frontais
- **💡 LED Indicador**: Status visual da conexão (GPIO 2)
- **🔧 Zona Morta**: Configurável para evitar drift do stick
- **📊 Debug Completo**: Monitor serial com dados HID em tempo real

## 📊 Especificações

- **Plataforma**: ESP32 (240MHz, dual core)
- **RAM**: 12.2% utilizada (39,820 bytes)
- **Flash**: 83.8% utilizada (1,098,113 bytes)
- **Comunicação**: Bluetooth Classic SPP
- **Framework**: Arduino + PlatformIO

## 📖 Documentação

- 📚 [**README Completo**](README.md) - Documentação detalhada
- ⚡ [**Quick Start**](QUICK_START.md) - Guia de início rápido
- 📅 [**Changelog**](CHANGELOG.md) - Histórico de versões
- 📖 [**Exemplos**](examples.cpp) - Implementações de referência

## 🎯 Compatibilidade

- ✅ **ESP32 DevKit** (recomendado)
- ✅ **ESP32 WROOM-32**
- ⚠️ **ESP32-C3** (use versão BLE alternativa)
- 🎮 **VRBOX Gamepad** (HID via Bluetooth)

## 🤝 Contribuições

Contribuições são bem-vindas! Veja o [guia de contribuição](README.md#-contribuição-e-suporte) para detalhes.

## 📄 Licença

Este projeto está licenciado sob a [Licença MIT](LICENSE).

---

⭐ **Se este projeto foi útil, considere dar uma estrela no GitHub!**