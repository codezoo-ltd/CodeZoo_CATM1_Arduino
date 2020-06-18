/* ESP32 Node32s Board Example */
#include "BG96.h"

#define DebugSerial Serial
#define M1Serial  Serial2     //16번핀 -- CAT.M1 TXD
                              //17번핀 -- CAT.M1 RXD
#define PWR_PIN   A19         //25번핀 -- CAT.M1 STATUS
#define STAT_PIN  A18         //26번핀 -- CAT.M1 POWER_KEY

BG96 BG96(M1Serial, DebugSerial, PWR_PIN, STAT_PIN);

void setup() {
	// put your setup code here, to run once:
	M1Serial.begin(115200);
	DebugSerial.begin(115200);

	/* BG96 Power On Sequence */
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

	DebugSerial.println("BG96 Module Ready!!!");
	char _IP[] = "echo.mbedcloudtesting.com";
	int  _PORT = 7;
	char sendBuffer[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

	char sendBuffer2[] = "Hello CodeZoo!!!";
	char recvBuffer[32];
	int  recvSize;    //Data size from BG96 module


	if (BG96.actPDP() == 0) {
		DebugSerial.println("BG96 PDP Activation!!!");
	}

	/* 1: TCP Socket */
	if (BG96.socketCreate(1, _IP, _PORT) == 0)
		DebugSerial.println("TCP Socket Create!!!");
	/* 0: UDP Socket */
	//  if (BG96.socketCreate(0, _IP, _PORT) == 0)
	//    DebugSerial.println("UDP Socket Create!!!");

	if (BG96.socketSend( sendBuffer, sizeof(sendBuffer) ) == 0)
		DebugSerial.println("Socket Send!!!");

	if (BG96.socketRecv( recvBuffer, sizeof(recvBuffer), &recvSize, 5000 ) == 0)
	{
		/* Data size from BG96 module */
		DebugSerial.print("Socket recvSize : [ ");
		DebugSerial.print(recvSize, DEC);
		DebugSerial.println(" ] ");

		for (int i = 0; i < recvSize; i++)
			DebugSerial.print(recvBuffer[i], DEC);
		DebugSerial.println(" <<------- Read Data");
	}

	if (BG96.socketSend( sendBuffer2 ) == 0)
		DebugSerial.println("Socket Send!!!");

	if (BG96.socketRecv( recvBuffer, sizeof(recvBuffer), &recvSize, 5000 ) == 0)
	{
		/* Data size from BG96 module */
		DebugSerial.print("Socket recvSize : [ ");
		DebugSerial.print(recvSize, DEC);
		DebugSerial.println(" ] ");
		DebugSerial.print(recvBuffer);
		DebugSerial.println(" <<------- Read Data");
	}

	if (BG96.socketClose() == 0) {
		DebugSerial.println("Socket Close!!!");
	}

	delay(5000);

	if (BG96.deActPDP() == 0) {
		DebugSerial.println("BG96 PDP DeActivation!!!");
	}

}

void loop() {

	delay(100);

}
