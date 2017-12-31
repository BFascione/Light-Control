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

const int relayPin = BUILTIN_LED; // declare pin 
bool BUILTIN_LED_Value ; // pin state value True of False
int pingCount = 0; // keep alive ping count

int port = 80;
char path[] = "/ws";

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

DynamicJsonBuffer jsonBuffer;
String currState,  message;


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{	
	switch (type)
	{
	case WStype_DISCONNECTED:
		//USE_SERIAL.printf("[WSc] Disconnected!\n");
		Serial.println("Disconnected! ");
		Serial.println(type);
		break;

	case WStype_CONNECTED:
		Serial.println("Connected! ");
		// send message to server when Connected
		webSocket.sendTXT("Connected");
		break;

	case WStype_TEXT:
		Serial.println("Recived Data");
		// Recieved message from server when Connected
		processWebScoketRequest((char*)payload);
		break;

	case WStype_BIN:
		hexdump(payload, length);
		Serial.print("Send Data");
		// send data to server
		webSocket.sendBIN(payload, length);
		break;
	}

}

// Switch state Method
String switchState() 
{	
	if (currState == "off")
	{
		digitalWrite(relayPin, LOW); //Switch on
		currState = "on";
	}
	else
	{
		digitalWrite(relayPin, HIGH); //Switch Off
		currState = "off";
	}
	return currState;
} 





// the setup function runs once when you press reset or power the board
void setup() 
{

	Serial.begin(115200);
	Serial.setDebugOutput(true);

	// Setup Relay
	pinMode(relayPin, OUTPUT);
	digitalWrite(relayPin, HIGH); // set lights to off sate when power is allied
	currState = "off"; // Current state of pin at initalization 
	
	
	for (uint8_t t = 4; t > 0; t--) {
		delay(1000);
	}
	Serial.println();
	Serial.print("Connecting to ");
	Serial.print(ssid); 
	Serial.print(" from Mac Address ");
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
	Serial.print("Connecting to ");
	Serial.print(host);
	Serial.print("on port ");
	Serial.println(port);
	Serial.println(path);
	
	webSocket.onEvent(webSocketEvent);

}


// the loop function runs over and over again until power down or reset
void loop()
{
		webSocket.loop();
		delay(100);
	
		
	// make sure after every 40 seconds send a ping to Heroku
	//so it does not terminate the websocket connection
	//This is to keep the conncetion alive between ESP and Heroku
	if (pingCount > 20) {
		pingCount = 0;
		webSocket.sendTXT("\"heartbeat\":\"keepalive\"");
		
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

		jsonResponse.replace("<text>", "Lights in " + instance + " are " + message);
		webSocket.sendTXT(jsonResponse);

	}



	else if (query == "cmd") //if query check state
		{ 
			Serial.println("Recieved command");

			if (state == "off")
			{
				if (state == currState)
				{
					message = "Already Off"; //Switching Off
					Serial.print("Already Off");
				}

				else
				{
					message = "Switching off"; //Switching Off
					Serial.println("Switching off");
					currState = switchState();
					//digitalWrite(relayPin, HIGH); //Switch Off
				}
			}
		
			if (state == "on")
				{
					if (state == currState)
					{
						message = "Already on"; //Switching Off
						Serial.print("Already on");
					}

					else
					{
						message = "Switching on"; //Switching Off
						Serial.println("Switching on");
						currState = switchState();
						//digitalWrite(relayPin,LOW); //Switch Off
					}

				}

		
		
			jsonResponse.replace("<text>", "Lights in " + instance + " are " + message);
			webSocket.sendTXT(jsonResponse);


		}
	//can not recognized the command
	else
		
		{
			Serial.println("Command is not recognized!");
			jsonResponse.replace("<text>", "Command is not recognized by Light Control Alexa skill");
			webSocket.sendTXT(jsonResponse);
		}


	Serial.print("Sending response back");
	Serial.println(jsonResponse);
	// send message to server
	webSocket.sendTXT(jsonResponse);
}
