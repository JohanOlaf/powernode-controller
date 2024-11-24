#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
String ip = "";
String optical = "";//ip + "Play?url=Capture%3Ahw%3Aimxspdif%2C0%2F1%2F25%2F2%3Fid%3Dinput1";
String analog  = "";//ip + "Play?url=Capture%3Aplughw%3A2%2C0%2F48000%2F24%2F2%3Fid%3Dinput0";
String request = "";

bool wificonnected = HIGH;
const int btnpin = 5;
const int redpin = 23;
const int greenpin = 22;
const int bluepin = 21;

const char* ssid = "";
const char* password = "";

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
int lastState = HIGH;  // Forrige stabile tilstand for knappen
bool lastBtnState = LOW;
bool analogbool = 0;

class LED{
	public:
		LED(int redpin, int greenpin, int bluepin){
			r = redpin;
			g = greenpin;
			b = bluepin;

			r_intensity = 0;
			g_intensity = 0;
			b_intensity = 0;
		}
		void write_led(bool io){
			analogWrite(this->r, io ? this->r_intensity : 0);
			analogWrite(this->g, io ? this->g_intensity : 0);
			analogWrite(this->b, io ? this->b_intensity : 0);
		}
		void set_color(int r, int g, int b){
			this->set_r(r);
			this->set_g(g);
			this->set_b(b);
		}
	private:
		int r;
		int g;
		int b;

		int r_intensity;
		int g_intensity;
	 	int b_intensity;

    float strength = 0.1;
		
		void set_r(int value){
			this->r_intensity = int(value*this->strength);
		}
		void set_g(int value){
			this->g_intensity = int(value*this->strength);
		}
		void set_b(int value){
		  this->b_intensity = int(value*this->strength);
		}
};

bool checkIP(String ip, int timeout = 100){
  HTTPClient http;
  http.setTimeout(timeout);
  http.setConnectTimeout(timeout);
  http.begin(ip.c_str());
  if(http.GET() and http.getString() != ""){
    http.end();
    return true;
    }
  http.end();
  return false;
  }

String ipScan(String ip){
  //antar at 192.168.0.X, at bare siste er ukjent
  String baseIP = "";
  int dotCounter = 0;
  for (int i = 0; i < ip.length(); i++){
    if (ip[i] == '.'){
      dotCounter ++;
      }
    if(dotCounter < 3){
      baseIP += ip[i];
      }
    }
	for(int i = 0; i < 4; i++){ 
	  for(int j = 0; j < 255; j++){
	    String ipToCheck = baseIP + '.' + String(j);
	    Serial.println("checking ip: " + ipToCheck);
	    if(checkIP("http://" + ipToCheck + ":11000/Status", 100 + i*25)){ //timeout er 50ms + 50*i 
	      Serial.println(ipToCheck + " ok");
	      wificonnected = true;
	      return "http://" + ipToCheck + ":11000/";
	      }
	    }
	}
  return "0";
}

LED led(redpin, greenpin, bluepin);


void setup() {
  pinMode(btnpin, INPUT);
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  led.set_color(0,0,255);
  while(WiFi.status() != WL_CONNECTED){
      led.write_led(HIGH);
      delay(500);
      led.write_led(LOW);
      delay(500);
    }
  //connected
	led.set_color(255,255,0);
  led.write_led(HIGH);
	//getip
  String local = WiFi.localIP().toString();
	ip = ipScan(local);
	if (ip == "0"){
		led.set_color(255,0,0);
	}
	else{
		led.set_color(0,255,0);
	}
  analog = ip + "Play?url=Capture%3Aplughw%3A2%2C0%2F48000%2F24%2F2%3Fid%3Dinput0";
  optical = ip + "Play?url=Capture%3Ahw%3Aimxspdif%2C0%2F1%2F25%2F2%3Fid%3Dinput1"; 
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
      led.write_led(HIGH);
      int currentBtnState = digitalRead(btnpin);
    
      if (currentBtnState != lastBtnState) {
        lastDebounceTime = millis();
      }
        
      if ((millis() - lastDebounceTime) > debounceDelay) {
        if (currentBtnState == HIGH) {
          if((millis()-lastDebounceTime) < 1000){
            analogbool = 1;
            led.set_color(0,255,255);
          }
          else{
              analogbool = 0;
              led.set_color(51,153,255);
          }
        }
      }
      if (currentBtnState == LOW && lastBtnState == HIGH){
        //send request
        HTTPClient http;
        http.begin(analogbool ? analog.c_str() : optical.c_str());
        int httpRes = http.GET();
        Serial.println(analogbool ? "analog" : "optical");
        http.end();
      }
      
    lastBtnState = currentBtnState; 
  }
  else{
    led.write_led(LOW);
  }
}
