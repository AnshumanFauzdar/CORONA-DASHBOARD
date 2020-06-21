//  _______ ______          __  __            __  __ ______ _____ _    _       _______ _____   ____  _   _ _____ _____  _____ 
// |__   __|  ____|   /\   |  \/  |          |  \/  |  ____/ ____| |  | |   /\|__   __|  __ \ / __ \| \ | |_   _/ ____|/ ____|
//    | |  | |__     /  \  | \  / |  ______  | \  / | |__ | |    | |__| |  /  \  | |  | |__) | |  | |  \| | | || |    | (___  
//    | |  |  __|   / /\ \ | |\/| | |______| | |\/| |  __|| |    |  __  | / /\ \ | |  |  _  /| |  | | . ` | | || |     \___ \ 
//    | |  | |____ / ____ \| |  | |          | |  | | |___| |____| |  | |/ ____ \| |  | | \ \| |__| | |\  |_| || |____ ____) |
//    |_|  |______/_/    \_\_|  |_|          |_|  |_|______\_____|_|  |_/_/    \_\_|  |_|  \_\\____/|_| \_|_____\_____|_____/ 
//

// Import required libraries
#include "ESP8266WiFi.h"
#include "ESPAsyncWebServer.h"
#include "ESP8266HTTPClient.h"
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

// Ultrasonic

const int trigPin = D6;
const int echoPin = D7;

long duration;
int distance;

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

// Replace with your network credentials

const char* ssid = "Your SSID";
const char* password = "PASSWORD";

// Covid 19 API
const String endpoint = "http://api.thingspeak.com/apps/thinghttp/send_request?api_key=";
const String key = "YOUR API KEY";

// Web Server HTTP Authentication credentials
const char* http_username = "admin";
const char* http_password = "admin";

Adafruit_BMP280 bmp;         // BME280 connect to ESP32 I2C (GPIO 21 = SDA, GPIO 22 = SCL)
const int buttonPin = 18;    // Pushbutton
const int ledPin = D4;       // Status LED
const int output = D3;       // Output socket
const int motionSensor = 27; // PIR Motion Sensor

int ledState = HIGH;           // current state of the output pin
int buttonState;              // current reading from the input pin
int lastButtonState = LOW;    // previous reading from the input pin
bool motionDetected = false;  // flag variable to send motion alert message
bool clearMotionAlert = true; // clear last motion alert message from web page

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncEventSource events("/events");

const char* PARAM_INPUT_1 = "state";

// Checks if motion was detected
void IRAM_ATTR detectsMovement() {
  //Serial.println("MOTION DETECTED!!!");
  motionDetected = true;
  clearMotionAlert = false;
}

// Main HTML web page in root url /
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
    <head>
      <title>Corona Dashboard</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link rel="icon" href="data:,">
      <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
      <style>
        html {font-family: Arial; display: inline-block; text-align: center;}
        h3 {font-size: 1.8rem; color: white;}
        h4 { font-size: 1.2rem;}
        p { font-size: 1.4rem;}
        body {  margin: 0; background-color: #000000;}
        .switch {position: relative; display: inline-block; width: 120px; height: 68px; margin-bottom: 20px;}
        .switch input {display: none;}
        .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #000000; border-radius: 68px;   opacity: 0.8;   cursor: pointer;}
        .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
        input:checked+.slider {background-color: #204051}
        input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
        .topnav { overflow: hidden; background-color: #3b6978;}
        .content { padding: 20px;}
        .card { background-color: #32e0c4; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);}
        .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));}
        .slider2 { -webkit-appearance: none; margin: 14px;  height: 20px; background: #ccc; outline: none; opacity: 0.8; -webkit-transition: .2s; transition: opacity .2s; margin-bottom: 40px; }
        .slider:hover, .slider2:hover { opacity: 1; }
        .slider2::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 40px; height: 40px; background: #008B74; cursor: pointer; }
        .slider2::-moz-range-thumb { width: 40px; height: 40px; background: #008B74; cursor: pointer;}
        .reading { font-size: 2.6rem;}
        .card-switch {color: #000000; }
        .card-light{ color: #000000;}
        .card-bme{ color: #000000;}
        .card-motion{ color: #000000; cursor: pointer;}
        .icon-pointer{ cursor: pointer;}
      </style>
    </head>
    <body>
      <div class="topnav">
        <h3>CORONA DASHBOARD <span style="text-align:right;">&nbsp;&nbsp; <i class="fas fa-user-slash icon-pointer" onclick="logoutButton()"></i></span></h3>
      </div>
      <div class="content">
        <div class="cards">
          <div class="card card-switch"><h4><i class="fas fa-lightbulb"></i> OUTPUT LED</h4><label class="switch"><input type="checkbox" onchange="controlOutput(this)" id="output" ><span class="slider"></span></label></div><div class="card card-switch"><h4><i class="fas fa-lightbulb"></i> STATUS LED!</h4><label class="switch"><input type="checkbox" onchange="toggleLed(this)" id="led" ><span class="slider"></span></label></div>
          <div class="card card-bme">
            <h4><i class="fas fa-chart-bar"></i> TEMPERATURE</h4><div><p class="reading"><span id="temp"></span>&deg;C</p></div>
          </div>
          <div class="card card-bme">
            <h4><i class="fas fa-chart-bar"></i> Pressure</h4><div><p class="reading"><span id="humi"></span> Pa</p></div>
          </div>
          <div class="card card-light">
            <h4><i class="fas fa-chart-area"></i> Total Cases (India)</h4><div><p class="reading"><span id="light"></span></p></div>
          </div>
          <div class="card card-motion" onClick="clearMotionAlert()">
            <h4><i class="fas fa-running"></i> Person Near you?</h4><div><p class="reading"><span id="motion">No Body yet!</span><span id="humi"></span> Cm</p></div>
          </div>
      </div>
    <script>
    function logoutButton() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/logout", true);
      xhr.send();
      setTimeout(function(){ window.open("/logged-out","_self"); }, 1000);
    }
    function controlOutput(element) {
      var xhr = new XMLHttpRequest();
      if(element.checked){ xhr.open("GET", "/output?state=1", true); }
      else { xhr.open("GET", "/output?state=0", true); }
      xhr.send();
    }
    function toggleLed(element) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/toggle", true);
      xhr.send();
    }
    function clearMotionAlert() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/clear-motion", true);
      xhr.send();
      setTimeout(function(){
        document.getElementById("motion").innerHTML = "No motion";
        document.getElementById("motion").style.color = "#3b3b3b";
      }, 1000);
    }
    if (!!window.EventSource) {
     var source = new EventSource('/events');
     source.addEventListener('open', function(e) {
      console.log("Events Connected");
     }, false);
     source.addEventListener('error', function(e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
      }
     }, false);
     source.addEventListener('message', function(e) {
      console.log("message", e.data);
     }, false);
     source.addEventListener('led_state', function(e) {
      console.log("led_state", e.data);
      var inputChecked;
      if( e.data == 1){ inputChecked = true; }
      else { inputChecked = false; }
      document.getElementById("led").checked = inputChecked;
     }, false);
     source.addEventListener('motion', function(e) {
      console.log("motion", e.data);
      document.getElementById("motion").innerHTML = e.data;
      document.getElementById("motion").style.color = "#b30000";
     }, false); 
     source.addEventListener('temperature', function(e) {
      console.log("temperature", e.data);
      document.getElementById("temp").innerHTML = e.data;
     }, false);
     source.addEventListener('humidity', function(e) {
      console.log("humidity", e.data);
      document.getElementById("humi").innerHTML = e.data;
     }, false);
     source.addEventListener('light', function(e) {
      console.log("light", e.data);
      document.getElementById("light").innerHTML = e.data;
     }, false);
    }</script>
    </body>
    </html>)rawliteral";

String outputState(int gpio){
  if(digitalRead(gpio)){
    return "checked";
  }
  else {
    return "";
  }
}

String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons;
    String outputStateValue = outputState(D3);
    buttons+="<div class=\"card card-switch\"><h4><i class=\"fas fa-lightbulb\"></i> OUTPUT</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"controlOutput(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label></div>";
    outputStateValue = outputState(D4);
    buttons+="<div class=\"card card-switch\"><h4><i class=\"fas fa-lightbulb\"></i> STATUS LED</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleLed(this)\" id=\"led\" " + outputStateValue + "><span class=\"slider\"></span></label></div>";
    return buttons;
  }
  else if(var == "MOTIONMESSAGE"){
    if(!clearMotionAlert) {
      return String("<span style=\"color:#b30000;\">MOTION DETECTED!</span>");
    }
    else {
      return String("No motion");
    }
  }
  return String();
}

// Logged out web page
const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <p>Logged out or <a href="/">return to homepage</a>.</p>
  <p><strong>Note:</strong> close all web browser tabs to complete the logout process.</p>
</body>
</html>
)rawliteral";

void setup(){

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  // Serial port for debugging purposes
  Serial.begin(115200);
    
  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
                  
  // initialize the pushbutton pin as an input
  pinMode(buttonPin, INPUT);
  // initialize the LED pin as an output
  pinMode(ledPin, OUTPUT);
  // initialize the LED pin as an output
  pinMode(output, OUTPUT);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
   if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", logout_html, processor);
  });
  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(401);
  });
  // Send a GET request to control output socket <ESP_IP>/output?state=<inputMessage>
  server.on("/output", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    String inputMessage;
    // GET gpio and state value
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      digitalWrite(output, inputMessage.toInt());
      request->send(200, "text/plain", "OK");
    }
    request->send(200, "text/plain", "Failed");
  });
  // Send a GET request to control on board status LED <ESP_IP>/toggle
  server.on("/toggle", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    request->send(200, "text/plain", "OK");
  });
  // Send a GET request to clear the "Motion Detected" message <ESP_IP>/clear-motion
  server.on("/clear-motion", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    clearMotionAlert = true;
    request->send(200, "text/plain", "OK");
  });
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis and set reconnect delay to 1 second
    client->send("hello!",NULL,millis(),500);
  });
  server.addHandler(&events);
  
  // Start server
  server.begin();
}
 
void loop(){

  
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 500;
  // read the state of the switch into a local variable
  int reading = digitalRead(buttonPin);

  // If the switch changed
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
        digitalWrite(ledPin, ledState);
        events.send(String(digitalRead(ledPin)).c_str(),"led_state",millis());
      }
    }
  }

  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
duration = pulseIn(echoPin, HIGH);

distance= duration*0.034/2;



    HTTPClient http;
 
    http.begin(endpoint + key); //Specify the URL
    
    int httpCode = http.GET(); //Make the request
  //Check for the returning code


        String payload = http.getString();
        String firstThree = payload.substring(0,7);
        
    
    events.send("ping",NULL,millis());
    events.send(String(bmp.readTemperature()).c_str(),"temperature",millis());
    events.send(String(bmp.readPressure()).c_str(),"humidity",millis());
    events.send(String(distance).c_str(),"motion",millis());
    events.send(String(firstThree).c_str(),"light",millis());
    lastEventTime = millis();
    http.end(); //Free the resources
  }
  
  
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}
