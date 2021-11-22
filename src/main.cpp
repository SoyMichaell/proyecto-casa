#include <Arduino.h>
#include "DHT.h"
#include "WiFi.h"
#include "ESP32_MailClient.h"

const char* ssid = "nombre_red";
const char* pass = "clave";
const int dht11 = 8;

//port serve
WiFiServer server(80);

//Objects
SMTPData datosSMTP;

//Details SMTP
const char* host = "smtp.gmail.com";
const char* loginEmail = "maicolh474@gmail.com";
const char* loginPassword = "maROHer142020";
const int port = 465;
const char* accountEmail = "soymichaelfb2@gmail.com";

//Http request
String header;

//Auxiliar
String led1out = "off";
String led2out =  "off";
String message;

//Outputs pins
const int led1 = 4; //room
const int led2 = 5; //eat

//dht11
double tmp,hm;

//rele
const int rele = 10;

DHT dht(dht11, DHT11);

void setup() {
  Serial.begin(115200);
  delay(10);

  dht.begin();
  Serial.println("\n");

  initialize();
  connect_wifi();
}

void loop() {

  //Listen
  WiFiClient client = server.available();

  if(client){
    delay(500);
    Serial.println("New client");
    String currentLine = "";
    while(client.connected() && 500 <= 2000){
      if(client.available()){
        char c = client.read();
        Serial.write(c); //Write serial monitor
        header += c;
        if(c == '\n'){
          if(currentLine.length() == 0){
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type: text/html");
            client.println("Connection: close");
            client.println();

            if(header.indexOf("GET /led1/on") >= 0){
              Serial.println("Led 1 on");
              led1out = "on";
              digitalWrite(led1, HIGH);
            }else if(header.indexOf("GET /led1/off") >= 0){
              Serial.println("Led 1 off");
              led1out = "off";
              digitalWrite(led1, LOW);
            }else if(header.indexOf("GET /led2/on") >= 0){
              Serial.println("Led 2 on");
              led2out = "on";
              digitalWrite(led2, HIGH);
            }else if(header.indexOf("GET /led2/off") >= 0){
              Serial.println("Led 2 off");
              led2out = "off";
              digitalWrite(led2, LOW);
            }

            

            float t = dht.readTemperature();
            Serial.println("Temperatura -> " + int(t));
            float h = dht.readHumidity();
            Serial.println("Humedad ->" + int(h));

            control_temp(t, rele, message);
            
            //Page htlm
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<link href='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css' rel='stylesheet' integrity='sha384-1BmE4kWBq78iYhFldvKuhfTAU6auU8tT94WrHftjDbrCEXSU1oBoqyl2QvZ6jIW3' crossorigin='anonymous'>");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".slot{ margin-top: 5%; }");
            client.println(".card{ width: 18rem !important; }");
            client.println(".button2 {background-color: #555555;}</style></head>");

            //Page heading
            client.println("<body><h1>My home</h1>");

            //room
            client.println("<p>Room</p>");
            if(led1out == "off"){
              client.println("<p><a href=\"/led1/on\"><button class=\"button\">ON</button></a></p>");
            }else{
              client.println("<p><a href=\"/led1/off\"><button class=\"button2\">OFF</button></a></p>");
            }

            //living room
            client.println("<p>Living room ON</p>");
            if(led2out == "off"){
              client.println("<p><a href=\"/led2/on\"><button class=\"button\">ON</button></a></p>");
            }else{
              client.println("<p><a href=\"/led2/off\"><button class=\"button2\">OFF</button></a></p>");
            }

            client.println("<div class=\"slot\"></div>");
          
            if(header.indexOf("GET /cnt/") >=0){
              
            }

            client.println("<div class=\"row\">");
              client.println("<div class=\"col-md-6\">");
                //card temp
                client.println("<div class=\"card mx-auto mt-3\">");
                  client.println("<div class=\"card-header\">Read Temperature</div>");
                  client.println("<div class=\"card-body\">");
                    client.println("<h5 class=\"card-title\"> Temperatura </h5>");
                    client.println("<h2 class=\"text-center mt-4\">");
                    client.print("<h2>");client.print(t);client.println(" °C </h2>");
                  client.println("</div>");
                client.println("</div>");
              client.println("</div>");

                //card temp
                client.println("<div class=\"card mx-auto mt-3\">");
                  client.println("<div class=\"card-header\">Read Temperature</div>");
                  client.println("<div class=\"card-body\">");
                    client.println("<h5 class=\"card-title\"> Temperatura </h5>");
                    client.println("<h2 class=\"text-center mt-4\">");
                    client.print("<h2>");client.print(t);client.println(" °C </h2>");
                  client.println("</div>");
                client.println("</div>");
              client.println("</div>");

              client.println("<div class=\"col-md-6\">");
                //card hum
                client.println("<div class=\"card mx-auto mt-3\">");
                  client.println("<div class=\"card-header\">Read Humidity</div>");
                  client.println("<div class=\"card-body\">");
                    client.println("<h5 class=\"card-title\"> Humedad </h5>");
                    client.println("<h2 class=\"text-center mt-4\">");
                    client.print("<h2>");client.print(h);client.println(" % </h2>");
                  client.println("</div>");
                client.println("</div>");
              client.println("</div>");

              client.println("<div class=\"col-md-6\">");
                //card ven
                client.println("<div class=\"card mx-auto mt-3\">");
                  client.println("<div class=\"card-header\">Fan status</div>");
                  client.println("<div class=\"card-body\">");
                    client.println("<h5 class=\"card-title\"> Status</h5>");
                    client.println("<h2 class=\"text-center mt-4\">");
                    client.print("<h2>");client.print(message);client.println("</h2>");
                  client.println("</div>");
                client.println("</div>");
              client.println("</div>");

            client.println("</div></body></html>");

            client.println();
            break;
          }else{
            currentLine = "";
          }
            }else if(c != '\r'){
            currentLine += c;
        }
      }
      //Clean
      header = "";
      client.stop();
      Serial.println("Client disconnected");
      Serial.println("");
    }
  }

}

void initialize(){
  // definition output
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(rele, OUTPUT);

  // write 
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(rele, HIGH);
}

void connect_wifi(){

  WiFi.begin(ssid,pass);
  Serial.print("Connecting to");
  Serial.print(ssid);
    while(WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }
  Serial.println("\n");
  Serial.println("Connection established!");
  Serial.print("Ip:");
  Serial.println(WiFi.localIP());
}

void control_temp(double t, int rele, String message){
  if(t >= 29.2){
    String message = "Status: ON";
    sendEmail(message);
    Serial.println("Ventilador encendido");
    digitalWrite(rele, LOW);
  }else{
    String message = "Status: OFF";
    sendEmail(message);
    Serial.println("Ventilador apagado");
    digitalWrite(rele, HIGH);
  }
}

void sendEmail(String message){
  datosSMTP.setLogin(host,port,loginEmail,loginPassword);
  datosSMTP.setSender("CASA /ESP32 ", loginEmail);
  datosSMTP.setPriority("Normal");
  datosSMTP.setSubject("Estado AIRE ACONDICIONADO");
  datosSMTP.setMessage("Hola soy ESP32/ CASA, envio estado del aire acondicionado", false);
  datosSMTP.addRecipient(accountEmail);
  if(!MailClient.sendMail(datosSMTP)){
    Serial.println("Error enviando el correo, " + MailClient.smtpErrorReason());
    datosSMTP.empty();
    delay(10000);
  }
}

