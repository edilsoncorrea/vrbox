# 📊 Sistema de Logging - Exemplos de Uso

## 🎯 Como Usar o Sistema de Logging

O sistema de logging implementado permite controlar a verbosidade dos debug prints através da constante `DEBUG_LEVEL` no arquivo `src/main.cpp`.

### 🔧 Configuração

```cpp
#define DEBUG_LEVEL 3  // ← ALTERE AQUI PARA CONTROLAR O NÍVEL DE DEBUG
```

### 📋 Níveis Disponíveis

| Nível | Nome | Emoji | Descrição | Quando Usar |
|-------|------|-------|-----------|-------------|
| 0 | NONE | - | Silencioso | Produção final |
| 1 | CRITICAL | 🔴 | Erros graves | Sempre importante |
| 2 | IMPORTANT | 🟡 | Eventos importantes | Conexões, inicializações |
| 3 | INFO | 🔵 | Status geral | **Padrão recomendado** |
| 4 | DEBUG | 🟢 | Debug detalhado | Desenvolvimento |
| 5 | VERBOSE | ⚪ | Todos os detalhes | Debug intensivo |

### 🏷️ Macros Especializadas (Nível 2+)

| Macro | Emoji | Uso |
|-------|-------|-----|
| `LOG_BLE()` | 📡 | Bluetooth Low Energy |
| `LOG_PWM()` | 🎛️ | Controle PWM |
| `LOG_JOYSTICK()` | 🕹️ | Dados do joystick |
| `LOG_RC()` | 🚗 | Controle RC |

## 📝 Exemplos Práticos

### Exemplo 1: DEBUG_LEVEL = 1 (Apenas Crítico)
```
🔴 Falha na conexão com o joystick
🔴 Erro na verificação de conexão
🔴 Conexão perdida, reconectando...
```

### Exemplo 2: DEBUG_LEVEL = 2 (Importante)
```
🔴 Erros críticos (como acima)
🟡 Conectado ao joystick VRBOX HID
📡 Cliente BLE conectado
🎛️ PWM configurado: GPIO4=Servo, GPIO5=ESC
🕹️ Dados do joystick atualizados
🚗 Controles RC atualizados
```

### Exemplo 3: DEBUG_LEVEL = 3 (Info - Padrão)
```
🔴 🟡 Tudo dos níveis anteriores
🔵 Reiniciando scan...
🔵 Status: X=0.50, Y=-0.25, Accel=0.75, Botões=0x0001
```

### Exemplo 4: DEBUG_LEVEL = 4 (Debug)
```
🔴 🟡 🔵 Tudo dos níveis anteriores
🟢 Executando comandos de ativação VRBOX...
🟢 Lendo Report Map para manter conexão...
🟢 Sem atividade por 2 minutos, verificando conexão...
```

### Exemplo 5: DEBUG_LEVEL = 5 (Verbose)
```
🔴 🟡 🔵 🟢 Tudo dos níveis anteriores
⚪ Conexão ativa [64 bytes]
⚪ Report Map: 156 bytes - Conexão ativa
⚪ Conexão OK [64 bytes]
```

## 🛠️ Como Alterar o Nível

1. **Abra** `src/main.cpp`
2. **Encontre** a linha: `#define DEBUG_LEVEL 3`
3. **Altere** o número para o nível desejado (0-5)
4. **Compile** o projeto: `pio run`
5. **Upload** para o ESP32: `pio run --target upload`

## 💡 Dicas de Uso

### Para Desenvolvimento
```cpp
#define DEBUG_LEVEL 4  // Mostra detalhes de desenvolvimento
```

### Para Produção
```cpp
#define DEBUG_LEVEL 1  // Apenas erros críticos
```

### Para Debug de Conexão BLE
```cpp
#define DEBUG_LEVEL 5  // Mostra todos os dados de rede
```

### Para Demo/Apresentação
```cpp
#define DEBUG_LEVEL 3  // Status limpo e informativo
```

## 🎯 Impacto na Performance

- **Nível 0-2**: Impacto mínimo na performance
- **Nível 3**: Recomendado para uso normal
- **Nível 4-5**: Use apenas para debug (mais lento)

## 🔄 Compilação

O sistema foi testado e compila com sucesso:
```
RAM:   [=         ]  11.9% (38876 bytes)
Flash: [========  ]  75.6% (990616 bytes)
========================= [SUCCESS] =========================
```

---
*Sistema implementado com sucesso em Janeiro 2025* ✅