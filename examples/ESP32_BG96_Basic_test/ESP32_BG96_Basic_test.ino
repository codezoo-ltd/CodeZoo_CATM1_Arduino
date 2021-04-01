/* ESP32 Node32s Board Example */
#include "BG96.h"

#define DebugSerial Serial
#define M1Serial                                                               \
  Serial2 // 16번핀 -- CAT.M1 TXD
          // 17번핀 -- CAT.M1 RXD
#define PWR_PIN A19  // 25번핀 -- CAT.M1 STATUS
#define STAT_PIN A18 // 26번핀 -- CAT.M1 POWER_KEY

BG96 BG96(M1Serial, DebugSerial, PWR_PIN, STAT_PIN);

void setup() {
  // put your setup code here, to run once:
  M1Serial.begin(115200);
  DebugSerial.begin(115200);

  /* Power On Sequence */
  if (BG96.isPwrON()) {
    DebugSerial.println("BG96 Power ON Status");
    if (BG96.pwrOFF()) {
      DebugSerial.println("BG96 Power Off Error");
    } else {
      DebugSerial.println("BG96 Power Off Success");
      DebugSerial.println("Module Power ON Sequence Start");
      if (BG96.pwrON()) {
        DebugSerial.println("BG96 Power ON Error");
      } else
        DebugSerial.println("BG96 Power ON Success");
    }
  } else {
    DebugSerial.println("BG96 Power OFF Status");
    if (BG96.pwrON()) {
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

  /*** BG96 Basic Test Code ***/
  /* Get Phone Number */
  char szCIMI[16];
  if (BG96.getCIMI(szCIMI, sizeof(szCIMI)) == 0) {
    DebugSerial.print("CIMI : ");
    DebugSerial.println(szCIMI);
  }

  /* Get IMEI */
  char szIMEI[16];
  if (BG96.getIMEI(szIMEI, sizeof(szIMEI)) == 0) {
    DebugSerial.print("IMEI : ");
    DebugSerial.println(szIMEI);
  }

  /* Get Fimrware version */
  char szCGMR[20];
  if (BG96.getCGMR(szCGMR, sizeof(szCGMR)) == 0) {
    DebugSerial.print("CGMR : ");
    DebugSerial.println(szCGMR);
  }

  /* Get ServingCell */
  char servingCell[10];
  if (BG96.getServingCell(servingCell) == 0) {
    DebugSerial.print("ServingCell : ");
    DebugSerial.println(servingCell);
  }

  /* Get RSRP */
  int rsrp;
  if (BG96.getRSRP(&rsrp) == 0) {
    DebugSerial.print("RSRP : ");
    DebugSerial.println(rsrp);
  }

  /* Get SINR */
  int sinr;
  if (BG96.getSINR(&sinr) == 0) {
    DebugSerial.print("SINR : ");
    DebugSerial.println(sinr);
  }

  /* Get SINR */
  int txPower;
  if (BG96.getTxPower(&txPower) == 0) {
    DebugSerial.print("TX Power : ");
    DebugSerial.println(txPower);
  }

  /* Get RSSI */
  int rssi;
  if (BG96.getRSSI(&rssi) == 0) {
    DebugSerial.print("RSSI : ");
    DebugSerial.println(rssi);
  }

  /* Get Time (GMT, (+36/4) ==> Korea +9hour) */
  char szTime[32];
  if (BG96.getCCLK(szTime, sizeof(szTime)) == 0) {
    DebugSerial.print("Time : ");
    DebugSerial.println(szTime);
  }

  /* Activate/Deactivate PDP Context */
  char szPDP[32];
  if (BG96.isActPDP(szPDP, sizeof(szPDP))) {
    if (BG96.actPDP() == 0) {
      if (BG96.isActPDP(szPDP, sizeof(szPDP)) == 0) {
        DebugSerial.print("Activation PDP info : ");
        /* 1(ContextID), 1(Activated), 1(IPV4), IP Addr(xxx.xxx.xxx.xxx) */
        DebugSerial.println(szPDP);
      }
    }
  }

  if (BG96.deActPDP() == 0) {
    DebugSerial.println("Deactivation PDP Context");
  }
}

void loop() { delay(100); }
