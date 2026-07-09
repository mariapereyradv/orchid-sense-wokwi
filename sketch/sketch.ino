// ===============================
// TP2 - ESP32
// Autor: Maria Pereyra
// [26-1][ACN5AV] PP: Integración Tecnológica
// Profesor: Gabriel Bragoli
//
// Funciones:
// - P1 enciende LED1 durante 3 segundos
// - P2 enciende y apaga LED2
// - DHT22 mide temperatura y humedad
// - BMP180 mide presión atmosférica
// - OLED muestra los datos en pantalla
// - Monitor Serie muestra los datos por consola
// ===============================

#include <HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---------- LEDs ----------
#define LED1 13
#define LED2 32

// ---------- Pulsadores ----------
#define BOTON1 15
#define BOTON2 27

// ---------- Sensores  ----------
#define DHTPIN 4
#define DHTTYPE DHT22

// ---------- Pantalla OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// ----------TOKEN -------------------
#define UBIDOTS_TOKEN "BBUS-bDjtugM6zmFaDtyvb2Fza1ap26biRW"
#define DEVICE_LABEL "esp32_tp2"
#define MQTT_BROKER "industrial.api.ubidots.com"
#define MQTT_PORT 1883

// --------wifi ----------------------
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""

//#define SERVER_URL "http://TU_IP_LOCAL:3000/api/sensores/datos"
#define SERVER_URL "http://192.168.100.17:3000/api/sensores/datos"

// Crear objeto para manejar la pantalla
Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET);

// Crear objetos para los sensores
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

// Variable para recordar el estado del LED2
bool estadoLED2 = false;

// Variable para detectar cuándo se presiona el botón
bool ultimoEstadoBoton2 = HIGH;

// Variable para controlar cada cuánto se actualizan los sensores
unsigned long ultimoEnvio = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void conectarWiFi()
{
  Serial.print("Conectando WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void conectarMQTT()
{
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  String clientId = "esp32_tp2_" + String(random(0xffff), HEX);
  while (!mqttClient.connected())
  {
    Serial.print("Conectando a Ubidots...");
    if (mqttClient.connect(clientId.c_str(), UBIDOTS_TOKEN, UBIDOTS_TOKEN))
    {
      Serial.println("conectado!");
    }
    else
    {
      delay(2000);
    }
  }
}

void publicarUbidots(float temp, float hum, float pres, int led2val)
{
  if (!mqttClient.connected())
    conectarMQTT();
  String topic = "/v1.6/devices/" + String(DEVICE_LABEL);
  String payload = "{\"temperatura\":" + String(temp, 1) +
                   ",\"humedad\":" + String(hum, 1) +
                   ",\"presion\":" + String(pres, 1) +
                   ",\"led2\":" + String(led2val) + "}";
  mqttClient.publish(topic.c_str(), payload.c_str());
  Serial.println("Enviado a Ubidots: " + payload);
}

void enviarAlServidor(float temp, float hum, float pres)
{
  HTTPClient http;
  http.begin("http://host.wokwi.internal:3000/api/sensores/datos");
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"temperatura\":" + String(temp, 1) +
                   ",\"humedad\":" + String(hum, 1) +
                   ",\"presion\":" + String(pres, 1) + "}";
  int httpCode = http.POST(payload);
  Serial.print("HTTP Response: ");
  Serial.println(httpCode);
  http.end();
}

void setup()
{

  // Iniciar monitor serie
  Serial.begin(115200);

  // Configurar LEDs como salida
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  // Configurar botones como entrada
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);

  // Iniciar sensor DHT22
  dht.begin();

  // Iniciar sensor BMP180
  if (!bmp.begin())
  {

    Serial.println("Error al iniciar BMP180");

    while (true)
      ;
  }

  // Iniciar pantalla OLED
  if (!display.begin(
          SSD1306_SWITCHCAPVCC,
          0x3C))
  {

    Serial.println("Error al iniciar OLED");

    while (true)
      ;
  }

  // Mensaje inicial en la pantalla
  display.clearDisplay();

  display.setTextSize(1);

  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);

  display.println("Sistema iniciado");

  display.println("TP2 ESP32");

  display.display();
  conectarWiFi();
  conectarMQTT();

  Serial.println("Sistema iniciado correctamente");
}

void loop()
{

  // ==================================================
  // BOTON 1
  // Enciende LED1 durante 3 segundos
  // ==================================================

  if (digitalRead(BOTON1) == LOW)
  {

    digitalWrite(LED1, HIGH);

    delay(3000);

    digitalWrite(LED1, LOW);
  }

  // ==================================================
  // BOTON 2
  // Enciende y apaga LED2 alternadamente
  // ==================================================

  bool lecturaBoton2 = digitalRead(BOTON2);

  if (lecturaBoton2 == LOW &&
      ultimoEstadoBoton2 == HIGH)
  {

    estadoLED2 = !estadoLED2;

    digitalWrite(LED2, estadoLED2);

    delay(200); // evita múltiples pulsaciones
  }

  ultimoEstadoBoton2 = lecturaBoton2;

  // ==================================================
  // LECTURA DE SENSORES CADA 15 SEGUNDOS
  // ==================================================

  if (millis() - ultimoEnvio >= 15000)
  {

    ultimoEnvio = millis();

    // Leer temperatura
    float temperatura =
        dht.readTemperature();

    // Leer humedad
    float humedad =
        dht.readHumidity();

    // Leer presión atmosférica
    float presion =
        bmp.readPressure() / 100.0;

    // ==========================================
    // Mostrar datos en Monitor Serie
    // ==========================================

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" C");

    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.println(" %");

    Serial.print("Presion: ");
    Serial.print(presion);
    Serial.println(" hPa");

    Serial.println("---------------------");

    // ==========================================
    // Mostrar datos en la pantalla OLED
    // ==========================================

    display.clearDisplay();

    display.setCursor(0, 0);

    display.print("Temp: ");
    display.print(temperatura);
    display.println(" C");

    display.print("Hum: ");
    display.print(humedad);
    display.println(" %");

    display.print("Pres: ");
    display.print(presion);
    display.println(" hPa");

    display.println();

    display.print("LED1: ");

    if (digitalRead(LED1))
    {
      display.println("ON");
    }
    else
    {
      display.println("OFF");
    }

    display.print("LED2: ");

    if (estadoLED2)
    {
      display.println("ON");
    }
    else
    {
      display.println("OFF");
    }

    display.display();
    if (WiFi.status() == WL_CONNECTED)
    {
      mqttClient.loop();
      publicarUbidots(temperatura, humedad, presion, estadoLED2 ? 1 : 0);
      enviarAlServidor(temperatura, humedad, presion);
    }

    mqttClient.loop();
  }
}
