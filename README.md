# 🌸 Orchid Sense - Simulador ESP32

Simulador del ESP32 para el sistema de monitoreo de invernadero de orquídeas.

## Autor
María Pereyra | Final ACN5AV | Integración Tecnológica  
Profesor: Gabriel Angel Bragoli

## Descripción
Proyecto Wokwi para VS Code que simula el ESP32 con sensores ambientales.
Envía datos por MQTT a Ubidots y por HTTP al servidor Node.js.

## Componentes del circuito
- ESP32 DevKit C V4
- DHT22 → Temperatura y Humedad (GPIO 4)
- BMP180 → Presión atmosférica (I2C: SDA 21, SCL 22)
- OLED SSD1306 → Pantalla (I2C: SDA 21, SCL 22)
- LED Verde → GPIO 13
- LED Azul → GPIO 32
- Botón P1 → GPIO 15
- Botón P2 → GPIO 27

## Rangos óptimos para orquídeas
- Temperatura: 18°C a 24°C
- Humedad: 60% a 80%

## Links
- Servidor Node.js: https://github.com/mariapereyradv/orchid-sense
- Dashboard Ubidots: [pegá el link de Ubidots]