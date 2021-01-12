#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Wavey Davey"; // Home wifi!
const char* password = "HDAKD7YC"; // password

//const char* ssid = "Brilliant Noise Guest";
//const char* password = "Adv0c4cy";

//const char* ssid = "virginmedia8520225";
//const char* password = "xxqwnths";

const char* getURL = "http://thefinds.co.uk/dev/room/index.php";

int ledPin1 = D1; int ledPin2 = D2;
int l1 = 1; int l2 = 1;

int buttonPin     = D7;
int buttonPressed = false;
int buttonLightIncrement = 2; // amount the breathing light increments by
int buttonTimer   = millis(); // for the longpress functionality
int buttonLightPin = D3;
float buttonLightCounter = 0;
int buttonLightPhase = 0;

String roomState = "";
int loopTimer     = millis();
int loopTimerLen  = 30000;

WiFiServer server(80);

//____________________________________________________________________________________________________________________

void setup() {
  Serial.begin(115200); delay(10);
  pinMode(ledPin1, OUTPUT); digitalWrite(ledPin1, LOW); // light off at start
  pinMode(ledPin2, OUTPUT); digitalWrite(ledPin2, LOW);
  pinMode(buttonPin, INPUT);
  
  // Connect to WiFi network
  Serial.println(); Serial.println(); Serial.print("Connecting to "); Serial.println(ssid);  WiFi.begin(ssid, password);
  // give update during connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(""); Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  
  // print to serial
  Serial.println("Server started"); Serial.print("Use this URL to connect: "); Serial.print("http://"); Serial.print(WiFi.localIP()); Serial.println("/");
}
//____________________________________________________________________________________________________________________

void check_room(){
  Serial.println("checking room");
  if (WiFi.status() == WL_CONNECTED) {        // Check WiFi connection status
      Serial.print("sending get");
      HTTPClient http;                        // Declare an object of class HTTPClient
      http.begin(getURL);                     // Specify request destination
      int httpCode = http.GET();              // Send the request
      if (httpCode > 0) {                     // Check the returning code
        String payload = http.getString();    // Get the request response payload
        roomState = payload;
        if (roomState == "busy" || roomState == "becoming free" ){
          Serial.println("roomState:"+roomState);
          l1 = 1; l2 = 0;
          digitalWrite(ledPin1, HIGH); digitalWrite(ledPin2, LOW);
        }else if (roomState == "free" || roomState == "becoming busy" ){
          Serial.println("roomState:"+roomState);
          l1 = 0; l2 = 1;       
          digitalWrite(ledPin1, LOW); digitalWrite(ledPin2, HIGH);
        }else if (roomState == "off" ){
          Serial.println("roomState:"+roomState);
          l1 = 0; l2 = 0;       
          digitalWrite(ledPin1, LOW); digitalWrite(ledPin2, LOW);
        }else{ // error - nothing came back from the server
          Serial.print("PROBLEM:");Serial.print(payload);Serial.println("."); // Print the response payload
          l1 = 0; l2 = 0;       
          digitalWrite(ledPin1, LOW); digitalWrite(ledPin2, LOW);
        }
      }
      http.end(); // Close connection
  }
}
//____________________________________________________________________________________________________________________

void flash(){
  analogWrite(buttonLightPin, 0);
  delay(70);
  analogWrite(buttonLightPin, 255);
  delay(70);
  analogWrite(buttonLightPin, 0);
  delay(70);
  analogWrite(buttonLightPin, 255);
  delay(70);
  analogWrite(buttonLightPin, 0);
  }
  
void button_press(){
  flash();
  Serial.println("pressed function");
  if (WiFi.status() == WL_CONNECTED) {        //Check WiFi connection status
      Serial.print("sending get request.");
      HTTPClient http;                        //Declare an object of class HTTPClient
      //Specify request destination
      http.begin("http://18.188.146.164:3000/book");
      
      int httpCode = http.GET();              //Send the request
      if (httpCode > 0) {                     //Check the returning code
        String payload = http.getString();    //Get the request response payload
        Serial.println(payload);              //Print the response payload //**** do something here with the confirmation?
      }
      http.end();  //Close connection
      flash();
      check_room(); // check room asap so the lights can update
      flash();
  }
}



//____________________________________________________________________________________________________________________

void animateButton(){
 if (roomState == "free" || roomState == "becoming free" ){ 
      if (buttonLightCounter>255){buttonLightPhase = 0;}
      if (buttonLightCounter<1){buttonLightPhase = 1;}
      if (buttonLightPhase){buttonLightCounter+=buttonLightIncrement;}
      if (!buttonLightPhase){buttonLightCounter-=buttonLightIncrement;}
    }else{
      buttonLightCounter = 0;
    }
    analogWrite(buttonLightPin, buttonLightCounter);
}

//____________________________________________________________________________________________________________________
  
void loop() {
  delay(10);
  
  // Button pressed, calls function once //.......................
  if (digitalRead(buttonPin)){
    if (buttonPressed != true){
      button_press(); // call the buttom press funtion
      buttonPressed = true; // set the flag
      buttonTimer = millis(); // start timer for longpress
    } else {
      // Check for a long press //.................................
       Serial.print("-"); 
       if (millis()-buttonTimer > 5000){
          Serial.println("reset");
          //resetFunc();  //call a reset
          ESP.restart();
       }
    } 
  }else{
    buttonPressed = false;
  }//...............................................................
  
  if (WiFi.status() == WL_CONNECTED) {    // poll the server fo info on the status of the meeting room if there is wifi connection

    if (millis()-loopTimer > loopTimerLen){
      Serial.println("looping");
      loopTimer = millis();
      check_room();
    }

    animateButton();      // breathe the button when wifi is connected ... and room is free or becoming free .................................................

  }else{
    // restart WIFI ? 
  }
}