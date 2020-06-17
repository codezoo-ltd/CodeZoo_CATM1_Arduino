#include <Time.h>       /* https://github.com/PaulStoffregen/Time */
#include <TimeAlarms.h> /* https://github.com/PaulStoffregen/TimeAlarms */
#include <Arduino.h>
#include "BG96.h"      /* https://github.com/codezoo-ltd/CAT.M1_Arduino */
#include "DHT.h"	     /* https://github.com/markruys/arduino-DHT */

#define DebugSerial		Serial
#define M1Serial		Serial1
#define PWR_PIN			2
#define STAT_PIN		3
#define DHT_PIN			A0

/*
 * Be careful !!!
 * Keep the communication cycle with ThingSpeak.com for at least 3 minutes.
 */
//#define ALARM_CYCLE 3600  /* Seconds, 1hour */
#define ALARM_CYCLE   180    /*Seconds, 3min */

String WApiKey = "****************";	//Thing Speak Write API Key 16Character
float temp = 0.0;						//Stores temperature value
float humi = 0.0;						//Stores humidity value

String fieldTemp = "field1";    //Air temperature
String fieldHumi = "field2";    //Air humidity

DHT dht;    // AM2302(DHT22) Temperature & Humidity Sensor

int _sendTag = 0;

BG96 BG96(M1Serial, DebugSerial, PWR_PIN, STAT_PIN);

void setup() {

	dht.setup(DHT_PIN);

	M1Serial.begin(115200);
	DebugSerial.begin(115200);

	/* Power On Sequence */
	if ( BG96.isPwrON() )
	{
		DebugSerial.println("BG96 Power ON Status");
		if ( BG96.pwrOFF() ) {
			DebugSerial.println("BG96 Power Off Error");
		} else {
			DebugSerial.println("BG96 Power Off Success");
			DebugSerial.println("Module Power ON Sequence Start");
			if ( BG96.pwrON() ) {
				DebugSerial.println("BG96 Power ON Error");
			} else
				DebugSerial.println("BG96 Power ON Success");
		}
	}
	else
	{
		DebugSerial.println("BG96 Power OFF Status");
		if ( BG96.pwrON() ) {
			DebugSerial.println("BG96 Power ON Error");
		} else
			DebugSerial.println("BG96 Power ON Success");
	}

	/* BG96 Module Initialization */
	if (BG96.init()) {
		DebugSerial.println("BG96 Module Error!!!");
	}

	/* BG96 Module Power Saving Mode Disable */
	if (BG96.disablePSM()) {
		DebugSerial.println("BG96 PSM Disable Error!!!");
	}

	/* Network Regsistraiton Check */
	while (BG96.canConnect() != 0) {
		DebugSerial.println("Network not Ready !!!");
		delay(2000);
	}

	char szCCLK[32];
	int _year, _month, _day, _hour, _min, _sec;

	/* Get Time information from the Telco base station */
	if (BG96.getCCLK(szCCLK, sizeof(szCCLK)) == 0)
	{
		DebugSerial.println(szCCLK);
		sscanf(szCCLK, "%d/%d/%d,%d:%d:%d+*%d", &_year, &_month, &_day, &_hour, &_min, &_sec);

		/* Time Initialization */
		setTime(_hour, _min, _sec, _month, _day, _year);
	}

	/* Alarm Enable */
	Alarm.timerRepeat(ALARM_CYCLE, Repeats);

	DebugSerial.println("BG96 Module Ready!!!");

}

void Repeats()
{
	DebugSerial.println("alarmed timer!");
	_sendTag = 1;
}

void loop() {

	if (_sendTag) {

		while (1) {
			delay(dht.getMinimumSamplingPeriod());

			/* Get DHT22 Sensor */
			if (dht.getStatusString() == "OK") {
				temp = dht.getTemperature();
				humi = dht.getHumidity();
				if ( String(temp) != "nan" && String(humi) != "nan")
					break;
				else {
					DebugSerial.println("case nan ...");
					delay(1000);
				}
			}
		}

		char _IP[] = "api.thingspeak.com"; //ThingSpeak Report Server api.thingspeak.com
		unsigned long  _PORT = 80;

		char recvBuffer[1024];
		int	 recvSize;
		String  data = "GET /update";
		data += "?api_key=" + WApiKey + "&" + fieldTemp + "=" + String(temp) + "&" + fieldHumi + "=" + String(humi);
		data += " HTTP/1.1\r\n";

		data += "Host: api.thingspeak.com\r\n";
		data += "Connection: close\r\n\r\n";

		if (BG96.actPDP() == 0 ) {
			DebugSerial.println("BG96 PDP Activation !!!");
		}

		if (BG96.socketCreate(1, _IP, _PORT) == 0)
			DebugSerial.println("TCP Socket Create!!!");

		/* Socket Send */
		if (BG96.socketSend(data.c_str()) == 0) {
			DebugSerial.print("[TCP Send] >>  ");
			DebugSerial.println(data);
		}
		else
			DebugSerial.println("Send Fail!!!");

		if (BG96.socketRecv( recvBuffer, sizeof(recvBuffer), &recvSize, 10000 ) == 0)
		{
			DebugSerial.print(recvBuffer);
			DebugSerial.println(" <<------ Read Data");
			DebugSerial.println(recvSize, DEC);
			DebugSerial.println("Socket Read!!!");
		}

		if (BG96.socketClose() == 0) {
			DebugSerial.println("Socket Close!!!");
		}

		delay(5000);

		if (BG96.deActPDP() == 0) {
			DebugSerial.println("BG96 PDP DeActivation!!!");
		}

		_sendTag = 0;

	}

	/* Time Test */
	DebugSerial.print(hour());
	DebugSerial.print(":");
	DebugSerial.print(minute());
	DebugSerial.print(":");
	DebugSerial.print(second());
	DebugSerial.println("");

	Alarm.delay(1000);
}
