/*
 Name:		BFSketch1.ino
 Created:	12/26/2017 10:47:23 AM
 Author:	brian
*/





#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ESP8266WiFiMulti.h>
#include <Hash.h>


const char* ssid = "LinksysF";
const char* password = "1234567890";
char host[] = "bf-light-control.herokuapp.com";
//const char* host = "bfascione.herokuapp.com/";
//const char* host = "bf-skill-set.herokuapp.com";
//boolean connectioWasAlive = true;

const int relayPin = BUILTIN_LED;
bool BUILTIN_LED_Value = true;
int pingCount = 0;

int port = 80;
char path[] = "/ws";

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

DynamicJsonBuffer jsonBuffer;
String currState, oldState, message;
int count_D = 0;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{
	

	switch (type) {
	case WStype_DISCONNECTED:
		//USE_SERIAL.printf("[WSc] Disconnected!\n");
		Serial.println("Disconnected! ");
		count_D++;
		Serial.println(count_D);
		Serial.println(type);
		// digitalWrite(relayPin, HIGH);
		break;

	case WStype_CONNECTED:
	{
		Serial.println("Connected! ");
		// send message to server when Connected
		webSocket.sendTXT("Connected");
		//digitalWrite(relayPin, LOW);
		count_D = 0;
	}
	break;

	case WStype_TEXT:
		Serial.println("Got data");

		Serial.println(currState + "uuuuuuu");
		if (currState=="off") {
			Serial.println(currState +"xxxxxxx");
			currState = "on";
			Serial.println(relayPin + "!");
			digitalWrite(relayPin,LOW); //Switch on

			}

		else {
			Serial.println(currState +"YYYYYYY");
			currState = "off";
			Serial.println(relayPin +"!");
			digitalWrite(relayPin,HIGH); //Switch Off
			

			
		}


		
		processWebScoketRequest((char*)payload);

		break;

	case WStype_BIN:

		hexdump(payload, length);
		Serial.print("Got bin");
		// send data to server
		webSocket.sendBIN(payload, length);
		break;
	}

}

/*void monitorWiFi()
{
	Serial.println("Called Monitor Correctly");
	delay(500);

	if (WiFiMulti.run() != WL_CONNECTED)
	{
		if (connectioWasAlive == true)
		{
			connectioWasAlive = false;
			Serial.print("Looking for WiFi ");
		}
		Serial.print(".");
		delay(500);
	}
	else if (connectioWasAlive == false)
	{
		connectioWasAlive = true;
		Serial.printf(" connected to %s\n", WiFi.SSID().c_str());
	}
}*/



// the setup function runs once when you press reset or power the board
void setup() {

	Serial.begin(115200);
	// Setup Relay
	pinMode(relayPin, OUTPUT);
	digitalWrite(relayPin, HIGH); // set lights to off sate when power is allied

	Serial.setDebugOutput(true);
	///////////////////////////////////////////////////////////////////////////////
//	WiFiMulti.addAP("LinksysF", "1234567890");

////////////////////////////////////////////////////////////////////////////////////
	for (uint8_t t = 4; t > 0; t--) {
		delay(1000);
	}
	Serial.println();
	Serial.println();
	Serial.println("Connecting to ");

	String s = WiFi.macAddress();
	Serial.println(s); //print out mac address 

	//Serial.println(ssid);
	WiFiMulti.addAP(ssid, password);

	//WiFi.disconnect();
	while (WiFiMulti.run() != WL_CONNECTED) {
		Serial.print(".");
		delay(1000);
	}
	webSocket.begin(host, port, path);
	Serial.println(WiFi.localIP());
	

	Serial.println("Connected to wi-fi");
	Serial.println(host);
	Serial.println(port);
	Serial.println(path);
	
	webSocket.onEvent(webSocketEvent);

}


// the loop function runs over and over again until power down or reset
void loop()
{

		webSocket.loop();
		delay(100);
	
	
///	{
//		Serial.println("Entering Loop");
//		monitorWiFi();
//		delay(500);
//		String s = WiFi.macAddress();
//		Serial.println(WiFi.localIP());
///		Serial.println(s);
///	}

	
	
	
	// make sure after every 40 seconds send a ping to Heroku
	//so it does not terminate the websocket connection
	//This is to keep the conncetion alive between ESP and Heroku
	if (pingCount > 20) {
		pingCount = 0;
		webSocket.sendTXT("\"heartbeat\":\"keepalive\"");
		//Serial.println("Keep Alive Heartbeat");
		//delay(500);
	}
	else {
		pingCount += 1;
	}
	
  
}

void processWebScoketRequest(String data) {
	String jsonResponse = "{\"version\": \"1.0\",\"sessionAttributes\": {},\"response\": {\"outputSpeech\": {\"type\": \"PlainText\",\"text\": \"<text>\"},\"shouldEndSession\": true}}";
	JsonObject& req = jsonBuffer.parseObject(data);

	String instance = req["instance"];
	String state = req["state"];
	String query = req["query"];
	String message = "{\"event\": \"OK\"}";
	String message2 ="{\"text\"}";

	//Serial.println("Data2-->" + data);
	//Serial.println("State-->" + state);
	
	Serial.println("instance value =  " + instance ); // see what is being sent from Alexia
	Serial.println("state value =  " + state );
	Serial.println("query value =  " + query );
	//Serial.println("xxxxxxx  message value =  " + message + "   xxxxxxx");
	//Serial.println("xxxxxxx  message3 value =  " + message2 + "   xxxxxxx");

	if (query == "?") 
	{ //if command then execute
		Serial.println("Recieved query");
		if (currState == "off") 
		{
			message = "off";  //This is off
			Serial.println("Light is Off");
		}		
		else
		{
			message = "on"; //This is on
			Serial.println("Light is On");
		}

		jsonResponse.replace("<text>", "Lights" + instance + " are " + message);
		webSocket.sendTXT(jsonResponse);

	}
	else if (query == "cmd") { //if query check state
		Serial.println("Recieved command");
		if (state != currState) {
			if (currState == "off tttttt") {
				message = "Switching on"; // Switching On
				Serial.println("Switching on");
				digitalWrite(relayPin,LOW); //Switch On
			}
			else {
				message = "Switching Off!"; //Switching Off
				Serial.println("Switching Off!");
				digitalWrite(relayPin,HIGH); //Switch Off
				
			}
			//digitalWrite(relayPin, HIGH); //Switch on
			//delay(1000);
			//digitalWrite(relayPin1, LOW);
		}
		else {
			if (currState == "on") {
				message = "already on"; //This could say already Off
			}
			else {
				message = "already off"; //This Could say already On
			}
		}
		jsonResponse.replace("<text>", "Lights in " + instance + " are " + message);
		webSocket.sendTXT(jsonResponse);


	}
	else {//can not recognized the command
		Serial.println("Command is not recognized!");
		jsonResponse.replace("<text>", "Command is not recognized by Light Control Alexa skill");
		webSocket.sendTXT(jsonResponse);
	}
	Serial.print("Sending response back");
	Serial.println(jsonResponse);
	// send message to server
	webSocket.sendTXT(jsonResponse);
}
