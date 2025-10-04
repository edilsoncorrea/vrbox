// Exemplos de configuração para diferentes usos do joystick VRBOX

// =============================================================================
// EXEMPLO 1: Controle de Carro RC
// =============================================================================

void carControlExample() {
    // Mapear direção para servo
    int servoAngle = map(steering * 1000, -1000, 1000, 0, 180);
    
    // Mapear aceleração para motor
    int motorSpeed = abs(acceleration) * 255;
    bool motorDirection = acceleration > 0; // true = frente, false = ré
    
    Serial.printf("Servo: %d° | Motor: %d (%s)\\n", 
                  servoAngle, motorSpeed, motorDirection ? "FRENTE" : "RÉ");
    
    // Implementar controle real aqui:
    // servo.write(servoAngle);
    // digitalWrite(motorDirPin, motorDirection);
    // analogWrite(motorSpeedPin, motorSpeed);
}

// =============================================================================
// EXEMPLO 2: Controle de Drone/Quadcopter
// =============================================================================

void droneControlExample() {
    // Mapear para comandos de drone
    float roll = steering;        // Inclinação lateral
    float pitch = acceleration;   // Inclinação frente/trás
    float throttle = 0.0;        // Controle de altitude
    float yaw = 0.0;             // Rotação
    
    // Usar botões para throttle e yaw
    if (joystick.frontButton1) throttle += 0.5;
    if (joystick.frontButton2) throttle = 1.0;
    
    // Outros botões para yaw
    if (joystick.buttons & (1 << VRBOX_BTN_A)) yaw = -0.5;
    if (joystick.buttons & (1 << VRBOX_BTN_B)) yaw = 0.5;
    
    Serial.printf("Roll:%.2f Pitch:%.2f Throttle:%.2f Yaw:%.2f\\n", 
                  roll, pitch, throttle, yaw);
}

// =============================================================================
// EXEMPLO 3: Controle de Robô Diferencial
// =============================================================================

void differentialRobotExample() {
    // Calcular velocidades das rodas (tank drive)
    float leftWheelSpeed = acceleration - steering;
    float rightWheelSpeed = acceleration + steering;
    
    // Normalizar para range [-1, 1]
    float maxSpeed = max(abs(leftWheelSpeed), abs(rightWheelSpeed));
    if (maxSpeed > 1.0) {
        leftWheelSpeed /= maxSpeed;
        rightWheelSpeed /= maxSpeed;
    }
    
    Serial.printf("Roda Esq: %.2f | Roda Dir: %.2f\\n", 
                  leftWheelSpeed, rightWheelSpeed);
    
    // Implementar controle de motores:
    // setMotorSpeed(LEFT_MOTOR, leftWheelSpeed * 255);
    // setMotorSpeed(RIGHT_MOTOR, rightWheelSpeed * 255);
}

// =============================================================================
// EXEMPLO 4: Controle via WiFi/Network
// =============================================================================

void networkControlExample() {
    // Criar pacote de dados para envio via rede
    struct NetworkPacket {
        float steering;
        float acceleration;
        uint8_t buttons;
        uint32_t timestamp;
    } packet;
    
    packet.steering = steering;
    packet.acceleration = acceleration;
    packet.buttons = joystick.buttons;
    packet.timestamp = millis();
    
    // Enviar via UDP ou TCP
    Serial.printf("Enviando pacote: S=%.2f A=%.2f B=0x%02X T=%u\\n",
                  packet.steering, packet.acceleration, 
                  packet.buttons, packet.timestamp);
    
    // Implementar envio de rede:
    // udp.broadcastTo((uint8_t*)&packet, sizeof(packet), UDP_PORT);
}

// =============================================================================
// EXEMPLO 5: Mapeamento Personalizado de Botões
// =============================================================================

void customButtonMapping() {
    // Mapear botões para funções específicas
    if (joystick.buttons & (1 << VRBOX_BTN_A)) {
        Serial.println("Ação: ACELERAR TURBO");
    }
    
    if (joystick.buttons & (1 << VRBOX_BTN_B)) {
        Serial.println("Ação: FREIO DE EMERGÊNCIA");
    }
    
    if (joystick.buttons & (1 << VRBOX_BTN_C)) {
        Serial.println("Ação: TROCAR MODO");
    }
    
    if (joystick.buttons & (1 << VRBOX_BTN_D)) {
        Serial.println("Ação: RESET SISTEMA");
    }
    
    // Combinações de botões
    if ((joystick.buttons & (1 << VRBOX_BTN_A)) && 
        (joystick.buttons & (1 << VRBOX_BTN_B))) {
        Serial.println("Ação: CALIBRAR");
    }
}

// =============================================================================
// EXEMPLO 6: Filtros e Suavização
// =============================================================================

class InputFilter {
private:
    float alpha = 0.1;  // Fator de suavização (0-1)
    float lastSteering = 0.0;
    float lastAcceleration = 0.0;
    
public:
    void updateFiltered(float& steering, float& acceleration) {
        // Filtro passa-baixa simples
        steering = alpha * steering + (1.0 - alpha) * lastSteering;
        acceleration = alpha * acceleration + (1.0 - alpha) * lastAcceleration;
        
        lastSteering = steering;
        lastAcceleration = acceleration;
    }
    
    void setFilterStrength(float strength) {
        alpha = constrain(strength, 0.01, 1.0);
    }
};

// Uso:
// InputFilter filter;
// filter.updateFiltered(steering, acceleration);