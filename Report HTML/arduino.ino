/* Create a WiFi access point and provide a web server on it. */
/* Just a little test message.  Go to http://192.168.4.1 in a web browser
   connected to this access point to see it.
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

//#ifndef APSSID
//#define APSSID "Pikasso"
//#define APPSK  "kadjo1999"
//#endif

/* Set these to your desired credentials. */
const char *ssid = "Pikasso";
const char *password = "kadjo1999"; //Password bigger than 8 characters

ESP8266WebServer server(80);

void handleRootMotors() {
  //GENERIC ARGUMENTS
  String message = "Number of args received:";
  String messagePlotter;
  message += server.args();            //Get number of parameters
  message += "\n";                            //Add a new line
  
  for (int i = 0; i < server.args(); i++) {
  
  message += "Arg n" + (String)i + " > ";  //Include the current iteration value
  message += server.argName(i) + ": ";     //Get the name of the parameter
  message += server.arg(i) + "\n";         //Get the value of the parameter
  }

  messagePlotter = "motor," + server.arg(0) + "," + server.arg(1) + "," + server.arg(2) + "," + server.arg(3) + "," + server.arg(4) + "," + server.arg(5) + ",\n\r";

  //SPECIFIC ARGUMENTS
  //  String message = “”;
  //
  //  if (server.arg(“Temperature”)== “”){     //Parameter not found
  //  
  //  message = “Temperature Argument not found”;
  //  
  //  }else{     //Parameter found
  //  
  //  message = “Temperature Argument = “;
  //  message += server.arg(“Temperature”);     //Gets the value of the query parameter
    
  //server.send(200, "text/html", "<h1>You are connected</h1>");
  server.send(200, "text/plain", messagePlotter);       //Response to the HTTP request
  Serial.print(messagePlotter);
}

void handleRootServo() {
  String messagePlotter;
  messagePlotter = server.arg(0) + "\n\r";
  server.send(200, "text/plain", messagePlotter);       //Response to the HTTP request
  Serial.print(messagePlotter);
}

void setup() {
  delay(1000);
  Serial.begin(19200);
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  server.on("/motors", handleRootMotors);
  server.on("/servo", handleRootServo); 
  server.begin();
}

void loop() {
  server.handleClient();
}
