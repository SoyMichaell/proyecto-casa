#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include "ESP32_MailClient.h"
#include "ThingSpeak.h"

//credenciales de red
const char* ssid = "RED O";
const char* pass = "267519XYZ";

//puerto
WiFiServer server(80);

//respuestas
String header;

String estadoLed1 = "off";
String estadoLed2 = "off";
String message;

//detalles correo
const char* host = "smtp.gmail.com";
const char* loginEmail = "maicolh474@gmail.com";
const char* loginPassword = "maROHer142020";
const int port = 465;
const char* accountEmail = "maicolh474@gmail.com";

//detalles thingspeak
unsigned long channelID = 1580456;
const char* WriteAPIKey = "14O6KM1OFX4KWQB4";

//Asignacion de pines
const int led1 = 4;
const int dhtPin = 18;
const int rele = 19;
//const int suelo = 23;

float t,h;

unsigned long horaActual = millis();
unsigned long horaAnterior = 0;
const long tiempoFuera = 2000;

SMTPData datosSMTP; 
DHT dht(dhtPin, DHT11);

WiFiClient client;

void sendEmail(String message){
  datosSMTP.setLogin(host,port,loginEmail,loginPassword);
  datosSMTP.setSender("CASA /ESP32 ", loginEmail);
  datosSMTP.setPriority("Normal");
  datosSMTP.setSubject("Estado AIRE ACONDICIONADO");
  datosSMTP.setMessage("Hola soy ESP32/ CASA, envio estado del aire acondicionado" + message, false);
  datosSMTP.addRecipient(accountEmail);
  if(!MailClient.sendMail(datosSMTP)){
    Serial.println("Error enviando el correo, " + MailClient.smtpErrorReason());
    datosSMTP.empty();
    delay(10000);
  }
}

void setup(){
    Serial.begin(115200);
    //estableciendo como salidas
    pinMode(led1, OUTPUT);
    pinMode(rele, OUTPUT);
    //pinMode(suelo, INPUT);
    //escritura estado
    digitalWrite(led1, LOW);
    digitalWrite(rele, HIGH);

    Serial.print("Conectando a ");
    Serial.println(ssid);
    WiFi.begin(ssid,pass);
    while(WiFi.status() != WL_CONNECTED ){
        delay(500);
        Serial.print(".");
    }
    //Ip
    Serial.println("");
    Serial.println("Conectado a la red");
    Serial.println("Direccio IP ");
    Serial.println(WiFi.localIP());
    dht.begin();
    server.begin();
    ThingSpeak.begin(client);

}

void loop(){
  delay(5000);
     client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    horaActual = millis();
    horaAnterior = horaActual;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && horaActual - horaAnterior <= tiempoFuera) {  // loop while the client's connected
      horaActual = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    
        header += c;
        if (c == '\n') {                    
          
          if (currentLine.length() == 0) {
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            //temperatura y humedad
            t = dht.readTemperature();
            Serial.print("Temperatura ambiente -> ");
            Serial.println(t);
            ThingSpeak.setField(1,t);
            h = dht.readHumidity();
            Serial.print("Humedad -> ");
            Serial.print(h);
            Serial.println("%");
            ThingSpeak.setField(2,h);
            
            //humedad de suelo
            //int humedad = digitalRead(suelo);
            int humedad = analogRead(A4);
            int hump = map();
            Serial.print("Humedad del suelo -> ");
            Serial.println(humedad);
          
            // HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<link href='https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css' rel='stylesheet' integrity='sha384-EVSTQN3/azprG1Anm3QDgpJLIm9Nao0Yz1ztcQTwFspd3yD65VohhpuuCOmLASjC' crossorigin='anonymous'>");
            client.println("<link href='https://fonts.googleapis.com/css2?family=Noto+Sans:wght@400;700&display=swap' rel='stylesheet'>");
            client.println("<style>html { font-family: 'Noto Sans', sans-serif !important; display: inline-block; margin: 0px auto;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            
            client.println("<body>");
              client.println("<div class=\"container\">");
                client.println("<h1 class=\"display-1\" style=\"font-weight: 700; margin-top: 5%;\">Plataforma movil</h1>");
                client.println("<p class=\"lead\">Toma de parametros agroambientales en un cultivo</p>");
                client.println("<hr>");
                client.println("<a class=\"btn btn-primary btn-sm\" href=\"./\"><i class=\"fas fa-sync-alt\"></i> Recargar</a>");
                client.println("<div class=\"row mt-3\">");
                  //tarjet de temperatura
                  client.println("<div class=\"col-md-6\">");
                    client.println("<div class=\"card mt-2 shadow-sm\">");
                      client.println("<div class=\"card-header\"><i class=\"fas fa-thermometer-empty\"></i> Temperatura ambiente</div>");
                      client.println("<div class=\"card-body p-3\">");
                      client.println("<h2>");client.println(t);client.println(" C</h2>");
                      //control de temperatura
                      if(t >= 28.2){
                        digitalWrite(rele, LOW);
                        Serial.print("Sistema de ventilacin encendido");
                        message = " Sistema de ventilacion encendido";
                        sendEmail(message);
                        client.println("<div class=\"alert alert-success\">Sistema de ventilacion encendido</div>");
                      }else{
                        digitalWrite(rele, HIGH);
                        Serial.print("Sistema de ventilacion apagado");
                        message = " Sistema de ventilacion apagado";
                        sendEmail(message);
                        client.println("<div class=\"alert alert-success\">Sistema de ventilacion apagado</div>");
                      }
                      client.println("</div>");
                    client.println("</div>");
                  client.println("</div>");
                  //tarjeta de humedad
                  client.println("<div class=\"col-md-6\">");
                    client.println("<div class=\"card mt-2 shadow-sm\">");
                      client.println("<div class=\"card-header\"><i class=\"fas fa-tint\"></i> Humedad</div>");
                      client.println("<div class=\"card-body p-3\">");
                      client.println("<h2>");client.println(h);client.println("%</h2>");
                      client.println("</div>");
                    client.println("</div>");
                  client.println("</div>");
                  //tarjeta de humedad suelo
                  client.println("<div class=\"col-md-6\">");
                    client.println("<div class=\"card mt-2 shadow-sm\">");
                      client.println("<div class=\"card-header\"><i class=\"fas fa-tint\"></i> Humedad suelo</div>");
                      client.println("<div class=\"card-body p-3\">");
                      client.println("<h2>");client.println(humedad);client.println("%</h2>");
                      if( (humedad >= 1) && (humedad <= 300)){
                        digitalWrite(led1, HIGH);
                        Serial.println("Humedad baja, sistema de riego activado");
                        client.println("<div class=\"alert alert-warning\">Suelo seco, sistema de riego activado</div>");
                      }else if((humedad > 300) && (humedad <= 700)){
                        digitalWrite(led1, LOW);
                        Serial.println("Humedad baja, sistema de riego apagado");
                        client.println("<div class=\"alert alert-success\">Suelo humedo, sistema de riego apagado</div>");  
                      }
                      client.println("</div>");
                    client.println("</div>");
                  client.println("</div>");
                client.println("</div>");
              client.println("</div>");  
              client.println("<script src='https://kit.fontawesome.com/8e596f10f3.js' crossorigin='anonymous'></script>");
            client.println("</body></html>");
            
            ThingSpeak.writeFields(channelID,WriteAPIKey);
            Serial.println("Datos enviados a ThingSeak");

            client.println();
            // Break out of the while loop
            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') { 
          currentLine += c;     
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    Serial.println("");
  }
}

