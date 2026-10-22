/*
 * Vinheria Agnello - Checkpoint 05 (Edge Computing & Computer Systems - FIAP)
 * ESP32 + DHT11 (temperatura/umidade) + LDR (luminosidade) -> MQTT (FIWARE IoT Agent MQTT / UltraLight 2.0)
 *
 * Bibliotecas (Library Manager da Arduino IDE):
 *   - PubSubClient (Nick O'Leary)
 *   - DHT sensor library (Adafruit) + Adafruit Unified Sensor
 */
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>

// ======================= CONFIGURAÇÕES (EDITE AQUI) =======================
const char* SSID     = "NOME_DA_REDE_WIFI";
const char* PASSWORD = "SENHA_DA_REDE_WIFI";
#define DHTTYPE DHT11

const char* BROKER_MQTT = "3.135.234.101";   // servidor do professor
const int   BROKER_PORT = 1883;
const char* MQTT_USER   = "";                        // deixe vazio se o broker não exigir
const char* MQTT_PASS   = "";

const char* TOPIC_PREFIX = "/TEF";                   // padrão FIWARE IoT Agent MQTT
const char* DEVICE_ID    = "wine001";                // ID do dispositivo
// Tópicos resultantes:
//   Publica atributos : /TEF/wine001/attrs   (e /attrs/t, /attrs/h, /attrs/l)
//   Recebe comandos   : /TEF/wine001/cmd
//   Responde comandos : /TEF/wine001/cmdexe
// ==========================================================================

#define PIN_DHT   4      // DHT11 DATA  -> D4
#define PIN_LDR   34     // LDR         -> D34 (entrada analógica)
#define PIN_LED   2      // LED1        -> D2

const unsigned long INTERVALO_MS = 5000;

DHT dht(PIN_DHT, DHTTYPE);
WiFiClient espClient;
PubSubClient mqtt(espClient);

String topicAttrs, topicCmd, topicCmdExe;
unsigned long ultimoEnvio = 0;
bool ledLigado = false;

// ---------- Utilitários ----------
String timestampISO() {
  struct tm t;
  if (!getLocalTime(&t, 2000)) return "1970-01-01T00:00:00Z";
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &t);   // UTC (configTime com offset 0)
  return String(buf);
}

void conectaWiFi() {
  Serial.printf("Conectando ao Wi-Fi %s", SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.printf("\nWi-Fi OK. IP: %s\n", WiFi.localIP().toString().c_str());
}

void conectaMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT... ");
    String clientId = String(DEVICE_ID) + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    bool ok = (strlen(MQTT_USER) > 0)
                ? mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)
                : mqtt.connect(clientId.c_str());
    if (ok) {
      Serial.println("conectado!");
      mqtt.subscribe(topicCmd.c_str());
      Serial.printf("Inscrito em %s\n", topicCmd.c_str());
    } else {
      Serial.printf("falhou (rc=%d). Nova tentativa em 3s\n", mqtt.state());
      delay(3000);
    }
  }
}

// ---------- Comandos recebidos (MyMQTT -> ESP32) ----------
// Payload UltraLight: "wine001@on|"  ou  "wine001@off|"
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.printf("[CMD] %s -> %s\n", topic, msg.c_str());

  String onCmd  = String(DEVICE_ID) + "@on|";
  String offCmd = String(DEVICE_ID) + "@off|";

  if (msg == onCmd) {
    ledLigado = true;  digitalWrite(PIN_LED, HIGH);
    mqtt.publish(topicCmdExe.c_str(), (String(DEVICE_ID) + "@on|LED ligado").c_str());
  } else if (msg == offCmd) {
    ledLigado = false; digitalWrite(PIN_LED, LOW);
    mqtt.publish(topicCmdExe.c_str(), (String(DEVICE_ID) + "@off|LED desligado").c_str());
  }
}

// ---------- Leitura e publicação ----------
void publicaLeituras() {
  float temp = dht.readTemperature();
  float umid = dht.readHumidity();
  int ldrRaw = analogRead(PIN_LDR);
  int lumin  = map(ldrRaw, 0, 4095, 0, 100);   // 0-100 % (ajuste/inverta conforme o divisor do LDR)

  if (isnan(temp) || isnan(umid)) {
    Serial.println("Falha na leitura do DHT11!");
    return;
  }
  String ts = timestampISO();

  // UltraLight 2.0: chave|valor|chave|valor  (mapeado no IoT Agent: t, h, l, ts)
  String payload = "t|" + String(temp, 1) + "|h|" + String(umid, 1) +
                   "|l|" + String(lumin) + "|ts|" + ts;
  mqtt.publish(topicAttrs.c_str(), payload.c_str());

  // Também publica cada atributo separado (facilita testar no MyMQTT)
  mqtt.publish((topicAttrs + "/t").c_str(), String(temp, 1).c_str());
  mqtt.publish((topicAttrs + "/h").c_str(), String(umid, 1).c_str());
  mqtt.publish((topicAttrs + "/l").c_str(), String(lumin).c_str());

  Serial.printf("[%s] T=%.1f C | U=%.1f %% | L=%d %% (raw=%d)\n",
                ts.c_str(), temp, umid, lumin, ldrRaw);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LDR, INPUT);
  dht.begin();

  topicAttrs  = String(TOPIC_PREFIX) + "/" + DEVICE_ID + "/attrs";
  topicCmd    = String(TOPIC_PREFIX) + "/" + DEVICE_ID + "/cmd";
  topicCmdExe = String(TOPIC_PREFIX) + "/" + DEVICE_ID + "/cmdexe";

  conectaWiFi();
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");   // timestamp em UTC via NTP
  for (int i = 0; i < 50 && time(nullptr) < 100000; i++) delay(200);   // aguarda sincronizar (máx. 10 s)

  mqtt.setServer(BROKER_MQTT, BROKER_PORT);
  mqtt.setCallback(callback);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) conectaWiFi();
  if (!mqtt.connected()) conectaMQTT();
  mqtt.loop();

  if (millis() - ultimoEnvio >= INTERVALO_MS) {
    ultimoEnvio = millis();
    publicaLeituras();
  }
}
