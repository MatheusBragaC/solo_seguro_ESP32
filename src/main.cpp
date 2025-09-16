/*
  Solo Seguro - Sistema de Monitoramento de Umidade, Temperatura e Gás do Solo
  Desenvolvido para ESP32

  Conexões:
  - Sensor de umidade (analógico): GPIO 34
  - Sensor de umidade (digital):  GPIO 26
  - Sensor de temperatura (DS18B20): GPIO 4
  - Sensor de gás (MQ-135):     GPIO 35
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <OneWire.h>           // NOVO: Biblioteca para o sensor DS18B20
#include <DallasTemperature.h> // NOVO: Biblioteca para o sensor DS18B20

// Definições dos pinos
#define SOIL_MOISTURE_ANALOG_PIN 34
#define SOIL_MOISTURE_DIGITAL_PIN 26
#define ONE_WIRE_BUS 4           // NOVO: Pino para o sensor de temperatura DS18B20
#define GAS_SENSOR_ANALOG_PIN 35 // NOVO: Pino para o sensor de gás MQ-135

// Configurações da rede Wi-Fi
const char *ssid = "Solo Seguro";
const char *password = "soloseguro123";

// Valores de calibração do sensor de umidade
const int AIR_VALUE = 3200;
const int WATER_VALUE = 1400;

// Servidor web na porta 80
WebServer server(80);

void handleRoot();
void handleData();
void handleNotFound();
// NOVO: Configuração para o sensor de temperatura
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup()
{
  Serial.begin(115200);

  pinMode(SOIL_MOISTURE_DIGITAL_PIN, INPUT);
  pinMode(SOIL_MOISTURE_ANALOG_PIN, INPUT);
  pinMode(GAS_SENSOR_ANALOG_PIN, INPUT); // NOVO: Configura pino do sensor de gás

  sensors.begin(); // NOVO: Inicia o sensor de temperatura
  Serial.println();
  Serial.print("Localizando sensores DS18B20...");
  Serial.print("Encontrados: ");
  Serial.println(sensors.getDeviceCount(), DEC);
  Serial.println();
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Endereço IP do AP: ");
  Serial.println(IP);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop()
{
  server.handleClient();
  delay(10);
}

// Função para ler a umidade em porcentagem
int getSoilMoisturePercent()
{
  int soilMoistureValue = analogRead(SOIL_MOISTURE_ANALOG_PIN);
  soilMoistureValue = constrain(soilMoistureValue, WATER_VALUE, AIR_VALUE);
  int soilMoisturePercent = map(soilMoistureValue, AIR_VALUE, WATER_VALUE, 0, 100);
  return soilMoisturePercent;
}

// NOVO: Função para ler a temperatura em Celsius
float getTemperature()
{
  sensors.requestTemperatures();

  float temperatureC = sensors.getTempCByIndex(0);
  // Checa se o valor é válido. -127 é um valor de erro comum para este sensor.
  if (temperatureC == -127.00)
  {
    Serial.println("Erro ao ler temperatura");
    return 0.0; // Retorna 0 em caso de erro para não quebrar o JSON
  }
  return temperatureC;
}

// NOVO: Função para ler o nível de gás (valor analógico bruto)
int getGasLevel()
{
  return analogRead(GAS_SENSOR_ANALOG_PIN);
}

// MODIFICADO: Função para lidar com a página raiz (HTML)
void handleRoot()
{
  String html = "<!DOCTYPE html>"
                "<html lang='pt-BR'>"
                "<head>"
                "  <meta charset='UTF-8'>"
                "  <meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                "  <title>Solo Seguro - Dashboard</title>"
                "  <style>"
                "    body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; background-color: #f4f7f6; }"
                "    h1 { color: #2c3e50; }"
                "    .dashboard { display: flex; flex-wrap: wrap; justify-content: center; gap: 20px; }"
                "    .card { background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); width: 280px; text-align: left; }"
                "    .card h2 { margin-top: 0; text-align: center; }"
                "    .status { font-size: 24px; font-weight: bold; margin: 10px 0; text-align: center; }"
                "    .wet { color: #3498db; } .dry { color: #e74c3c; } .hot { color: #e74c3c; } .cold { color: #3498db; } .ok { color: #2ecc71; } .critical { color: #e74c3c; }"
                "    .progress-container { width: 100%; background-color: #f3f3f3; border-radius: 10px; margin: 15px 0; }"
                "    .progress-bar { height: 30px; border-radius: 10px; text-align: center; line-height: 30px; color: white; transition: width 0.5s; }"
                "    .data-row { display: flex; justify-content: space-between; margin: 8px 0; font-size: 14px; }"
                "    .data-label { font-weight: bold; }"
                "    .update-status { font-size: 12px; color: #7f8c8d; margin-top: 20px; text-align: center; }"
                "    button { background-color: #3498db; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; margin-top: 10px; }"
                "  </style>"
                "</head>"
                "<body>"
                "  <h1>Solo Seguro - Dashboard de Monitoramento</h1>"
                "  <div class='dashboard'>"
                "    "
                "    <div class='card'>"
                "      <h2>💧 Umidade do Solo</h2>"
                "      <div id='moisture-status' class='status'>...</div>"
                "      <div class='progress-container'>"
                "        <div id='moisture-bar' class='progress-bar' style='width: 0%; background-color: #3498db;'>0%</div>"
                "      </div>"
                "      <div class='data-row'><span class='data-label'>Sensor Digital:</span><span id='digital-value'>...</span></div>"
                "      <div class='data-row'><span class='data-label'>Valor Analógico:</span><span id='analog-value'>...</span></div>"
                "    </div>"
                "    "
                "    <div class='card'>"
                "      <h2>🌡️ Temperatura do Solo</h2>"
                "      <div id='temp-status' class='status'>...</div>"
                "      <div class='data-row'><span class='data-label'>Temperatura:</span><span id='temp-value'>...</span></div>"
                "    </div>"
                "    "
                "    <div class='card'>"
                "      <h2>💨 Nível de Gás</h2>"
                "      <div id='gas-status' class='status'>...</div>"
                "      <div class='data-row'><span class='data-label'>Valor Analógico:</span><span id='gas-value'>...</span></div>"
                "    </div>"
                "  </div>"
                "  <button id='toggle-updates'>Pausar Atualizações</button>"
                "  <div id='update-status' class='update-status'>Última atualização: Nunca</div>"
                "  <script>"
                "    let updating = true;"
                "    const toggleBtn = document.getElementById('toggle-updates');"
                "    toggleBtn.addEventListener('click', () => { updating = !updating; this.textContent = updating ? 'Pausar' : 'Retomar'; if(updating) fetchData(); });"
                "    function updateUI(data) {"
                "      const updateStatus = document.getElementById('update-status');"
                "      updateStatus.textContent = 'Última atualização: ' + new Date().toLocaleTimeString();"
                "      "
                "      /* --- UMIDADE --- */"
                "      document.getElementById('digital-value').textContent = data.digital === 1 ? 'SECO' : 'ÚMIDO';"
                "      document.getElementById('analog-value').textContent = data.analog;"
                "      const moistureBar = document.getElementById('moisture-bar');"
                "      moistureBar.style.width = data.percent + '%'; moistureBar.textContent = data.percent + '%';"
                "      const moistureStatus = document.getElementById('moisture-status');"
                "      if (data.percent < 30) { moistureStatus.textContent = 'SECO'; moistureStatus.className = 'status dry'; moistureBar.style.backgroundColor = '#e74c3c'; }"
                "      else if (data.percent < 70) { moistureStatus.textContent = 'MODERADO'; moistureStatus.className = 'status ok'; moistureBar.style.backgroundColor = '#f39c12'; }"
                "      else { moistureStatus.textContent = 'ÚMIDO'; moistureStatus.className = 'status wet'; moistureBar.style.backgroundColor = '#3498db'; }"
                "      "
                "      /* --- TEMPERATURA --- */"
                "      document.getElementById('temp-value').textContent = data.temperature.toFixed(2) + ' °C';"
                "      const tempStatus = document.getElementById('temp-status');"
                "      if(data.temperature > 30) { tempStatus.textContent = 'ALTA'; tempStatus.className = 'status hot'; }"
                "      else if(data.temperature < 15) { tempStatus.textContent = 'BAIXA'; tempStatus.className = 'status cold'; }"
                "      else { tempStatus.textContent = 'IDEAL'; tempStatus.className = 'status ok'; }"
                "      "
                "      /* --- GÁS --- */"
                "      document.getElementById('gas-value').textContent = data.gas;"
                "      const gasStatus = document.getElementById('gas-status');"
                "      if(data.gas > 700) { gasStatus.textContent = 'CRÍTICO'; gasStatus.className = 'status critical'; }"
                "      else { gasStatus.textContent = 'NORMAL'; gasStatus.className = 'status ok'; }"
                "    }"
                "    function fetchData() {"
                "      if (!updating) return;"
                "      fetch('/data').then(response => response.json()).then(data => {"
                "        updateUI(data);"
                "        setTimeout(fetchData, 2000);"
                "      }).catch(error => { console.error('Erro:', error); setTimeout(fetchData, 5000); });"
                "    }"
                "    fetchData();"
                "  </script>"
                "</body>"
                "</html>";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", html);
}

// MODIFICADO: Função para fornecer todos os dados em JSON
void handleData()
{
  // Cria o objeto JSON
  StaticJsonDocument<256> doc; // Aumentado para comportar mais dados

  // Lê e adiciona os dados de umidade
  doc["digital"] = digitalRead(SOIL_MOISTURE_DIGITAL_PIN);
  doc["analog"] = analogRead(SOIL_MOISTURE_ANALOG_PIN);
  doc["percent"] = getSoilMoisturePercent();

  // NOVO: Lê e adiciona os dados de temperatura e gás
  doc["temperature"] = getTemperature();
  doc["gas"] = getGasLevel();

  // Serializa e envia a resposta
  String jsonString;
  serializeJson(doc, jsonString);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", jsonString);
}

// Função para lidar com páginas não encontradas
void handleNotFound()
{
  server.send(404, "text/plain", "Página não encontrada");
}