# 🎮 VRBOX - Guia de Pareamento ESP32-C3

## ✅ Status Atual
- **ESP32-C3:** ✅ Conectado em COM5 e funcionando
- **Firmware:** ✅ BLE carregado com sucesso  
- **Joystick:** 🔄 Aguardando pareamento

## 📱 Próximos Passos

### 1. Configurações de Bluetooth (Windows)
```
Configurações > Dispositivos > Bluetooth e outros dispositivos
```

### 2. Adicionar Dispositivo
- Clique em "Adicionar Bluetooth ou outro dispositivo"
- Selecione "Bluetooth"
- Aguarde detectar o joystick VRBOX

### 3. Verificar Conexão
Execute no terminal para monitorar:
```cmd
pio device monitor --port COM5
```

### 4. Dispositivos Esperados
- **ESP32-C3:** `ESP32-C3-VRBOX-Receiver`
- **Joystick:** `VRBOX` ou nome similar

## 🔧 Troubleshooting

### Joystick não aparece:
1. Verifique se está ligado
2. Pressione botão de pareamento (3-5 segundos)
3. Aguarde LED piscar rapidamente

### ESP32-C3 não recebe dados:
1. Verifique se joystick está pareado com Windows
2. Execute monitor serial: `pio device monitor --port COM5`
3. Teste movimentar joystick

## 🎯 Indicadores de Sucesso
- ✅ Joystick aparecer na lista Bluetooth
- ✅ Status "Conectado" no Windows
- ✅ LED ESP32-C3 parar de piscar (conexão ativa)
- ✅ Dados aparecerem no monitor serial