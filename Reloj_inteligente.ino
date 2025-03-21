#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

WiFiManager wifiManager;


// API de WorldTimeAPI
const char* server = "worldtimeapi.org";
const char* endpoint = "/api/ip";

//API OpenWeather
const char* host = "api.openweathermap.org";
const char* apiKey = "";
const char* city = "";
const char* url = "/data/2.5/weather?q=";

//Datos
String hora = "";
String clima = "";
String temperatura = "";

int segs = 0;
int horas = 0;
int mins = 0;

void setup() {
  Serial.begin(115200);
  delay(10);

  wifiManager.startConfigPortal("Reloj Arduino");
  
  Serial.println("Conectado al WiFi");
  delay(100);
  Serial.println("Solicitando datos...");
  hora = ObtenerHora();
  clima = String(ObtenerClima());
  temperatura = String(ObtenerTemperatura());
  segs = (hora.substring(6,8)).toInt();
  horas = (hora.substring(0, 2)).toInt();
  mins = (hora.substring(3,6)).toInt();
}

void loop() {
  String Datos = "=============================== \n Clima: " + clima + "\r\n Temperatura: " + temperatura + "\r\n Hora: " + (String(horas) + ":" + String(mins) + ":" + String(segs)) + "\n=============================== \n";
  Serial.println(Datos);
  
  delay(1000);
  
  segs += 1;
  
  // En una hora verificamos la informacion
    if (segs == 60){
      segs = 0;
      mins +=1;
      if (mins == 60){
         horas = (horas + 1) % 24;
         mins = 0;
         segs = 0;
        
         clima = String(ObtenerClima());
         temperatura = String(ObtenerTemperatura());
      
         hora = ObtenerHora();
         horas = (hora.substring(0,2)).toInt();
         mins = (hora.substring(3,6)).toInt();
         segs = (hora.substring(6, 8)).toInt();
      }
    }
} 

String ObtenerHora() {
  
  WiFiClient client;

  if (!client.connect(server, 80)) {
    Serial.println("Fallo de conexion con la api de WorldTimeApi");
    return ("Error al obtener la hora");
  }
    // Realizar solicitud HTTP GET a la API
    client.print(String("GET ") + endpoint + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");

    String response = "";
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        break;
      }
    }

    while (client.available()) {
      String line = client.readStringUntil('\n');
      response += line;
    }

    client.stop();

    const size_t bufferSize = JSON_OBJECT_SIZE(4) + 160;
    DynamicJsonDocument doc(bufferSize);
    deserializeJson(doc, response);

    const char* datetime = doc["datetime"];
    String solohora = String(datetime);
    return (solohora.substring(solohora.indexOf('T') + 1, solohora.indexOf('T') + 9));
}

float ObtenerTemperatura() {
  WiFiClient client;
  if (!client.connect(host, 80)) {
    Serial.print("Error al obtener la temperatura");
    return 0;
  }

  String requestUrl = String(url) + String(city) + "&appid=" + String(apiKey) + "&units=metric";

  client.print(String("GET ") + requestUrl + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  String response = "";
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  while (client.available()) {
    String line = client.readStringUntil('\n');
    response += line;
  }

  client.stop();

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, response);

  JsonObject main = doc["main"];
  float temperature = main["temp"];
  return temperature;
}

String ObtenerClima() {
  // Peticion y conexion a OpenWeather
  WiFiClient client;

  if (!client.connect(host, 80)) {
    Serial.println("Conexion fallida");
    return "NoObtenido";
  }
  float Lat = -34.8957;
  float Lon = -60.0167;
  String PeticionURL = "/data/2.5/weather?lat=" + String(Lat) + "&lon=" + String(Lon) + "&appid=" + String(apiKey) + "&lang=es";
  client.print(String("GET ") + PeticionURL + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  //Respuesta a la peticion
  String Respuesta = "";
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  while (client.available()) {
    String line = client.readStringUntil('\n');
    Respuesta += line;
  }

  //Lee el archivo .json
  client.stop();
  DynamicJsonDocument documento(1024);
  deserializeJson(documento, Respuesta);

  JsonObject weather = documento["weather"][0];
  String Clima = weather["description"];
  return Clima;
}