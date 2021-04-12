/*
 * A library for controlling BG96 LTE CAT.M1
 *
 * @Author Rooney.Jang  [CodeZoo]
 * @Date 01/09/2020
 *
 */

#include "BG96.h"
#include "Arduino.h"

extern "C" {
#include "stdlib.h"
#include "string.h"
#ifndef __AVR__
#include <cstdarg>
#endif
}

#define SWIR_TRACE(...) BG96_trace(__VA_ARGS__);
#define TRACE_BUFFER_SIZE 256
#define BG_LINE 30 // Limit 30 Line

//#define __BG_96_DEBUG		//Debug mode

BG96::BG96(Stream &serial, Stream &debug, uint8_t pwr_pin, uint8_t status_pin)
    : _serial(serial), _debug(debug), _pwr_pin(pwr_pin),
      _status_pin(status_pin) {
  _timeOut = 2000; /* default Timeout */
  _serial.setTimeout(
      (_timeOut + 500)); /* +500ms, Serial TX/RX Timeout default 2000 */
  pinMode(_pwr_pin, OUTPUT);
  pinMode(_status_pin, INPUT); /* PULL None */
}

int BG96::pwrON() {
  int ret;

  delay(50); // Setup Time : Greater than or equal to 30ms
  digitalWrite(_pwr_pin, HIGH);
  delay(600); // Hold Time : Greater than or equal to 500ms
  digitalWrite(_pwr_pin, LOW);
  delay(5000); // Release Time : Greater than or equal to 4800ms

  ret = !(digitalRead(_status_pin));

  return ret;
}

int BG96::pwrOFF() {
  Countdown oCountDown(60000); /* Max Timeout 1 minute */
  int ret = 1;

  digitalWrite(_pwr_pin, HIGH);
  delay(800); // Hold Time : Greater than or equal to 650ms
  digitalWrite(_pwr_pin, LOW);
  delay(3000); // Release Time : Greater than or equal to 2000ms

  do {
    if (digitalRead(_status_pin)) {
      delay(1000);
    } else {
      ret = 0;
      break;
    }
  } while (!oCountDown.expired());

  return ret;
}

int BG96::isPwrON() {
  int ret;

  delay(50); // Setup Time : Greater than or equal to 30ms

  ret = digitalRead(_status_pin);

  return ret;
}

int BG96::init() {
  char szCmd[16];
  char resBuffer[16];
  int cnt = 0;
  int ret;

  BG96_serial_clearbuf();

  memset(resBuffer, 0, sizeof(resBuffer));
  strcpy(szCmd, "AT");

  do {

    ret = sendATcmd(szCmd, resBuffer, sizeof(resBuffer), "OK", 3000);

    if (ret == 0)
      break;

  } while (cnt < 10);

  strcpy(szCmd, "ATE0"); // Echo Off
  ret = sendATcmd(szCmd, resBuffer, sizeof(resBuffer), "OK", 3000);

  strcpy(szCmd, "AT+CEREG=2");
  ret = sendATcmd(szCmd, resBuffer, sizeof(resBuffer), "OK", 3000);

  return ret;
}

int BG96::getCCLK(char *szCCLK, int nBufferSize) {
  char szCmd[16];
  int ret;

  BG96_serial_clearbuf();

  memset(szCCLK, 0, nBufferSize);

  strcpy(szCmd, "AT+CCLK?");
  ret = sendATcmd(szCmd, szCCLK, nBufferSize, "+CCLK:");

  return ret;
}

int BG96::getCGMR(char *szCGMR, int nBufferSize) {
  char szCmd[16];
  char *aLine[BG_LINE]; // read up to 20 lines

  BG96_serial_clearbuf();

  strcpy(szCmd, "AT+CGMR");
  int nNbLine = sendATcmd(szCmd, aLine, BG_LINE);

  char *sLine;
  memset(szCGMR, 0, nBufferSize);

  for (int i = 0; i < nNbLine; i++) {
    sLine = aLine[i];

    SWIR_TRACE(F("getCGMR, line[%d]: %s\n"), i, sLine);

    char *pTemp = sLine;
    while (pTemp < (sLine + strlen(sLine))) // trim ending
    {
      if (*pTemp == '\r') // remove cariage return
      {
        *pTemp = 0; // zero terminate string
        break;
      }
      if (*pTemp == '\n') // remove cariage return
      {
        *pTemp = 0; // zero terminate string
        break;
      }
      pTemp++;
    }

    int nLen = strlen(sLine);
    if (nLen < 15) {
      free(aLine[i]);
      continue;
    }
    for (int k = 0; k < nLen; k++) {
      if (sLine[k] < '-' || sLine[k] > 'z') {
        continue;
      }
    }

    strcpy(szCGMR, sLine);
    free(aLine[i]);
  }

  return (strlen(szCGMR) > 0 ? 0 : 1);
}

int BG96::getCFUN(int *value) {
  char szCmd[16];
  char resBuffer[16];
  int ret, _value;

  BG96_serial_clearbuf();

  strcpy(szCmd, "AT+CFUN?");

  ret = sendATcmd(szCmd, resBuffer, sizeof(resBuffer), "+CFUN:");

  _value = atoi(resBuffer);
  *value = _value;

  return ret;
}

int BG96::setCFUN(int value) {
  char szCmd[16];
  char resBuffer[16];
  int ret;

  BG96_serial_clearbuf();

  sprintf(szCmd, "AT+CFUN=%d", value);

  ret = sendATcmd(szCmd, resBuffer, sizeof(resBuffer), "OK", 3000);

  return ret;
}

int BG96::getCIMI(char *szCIMI, int nBufferSize) {
  char szCmd[16];
  char *aLine[BG_LINE]; // read up to 20 lines

  BG96_serial_clearbuf();

  strcpy(szCmd, "AT+CIMI");
  int nNbLine = sendATcmd(szCmd, aLine, BG_LINE);

  char *sLine;
  memset(szCIMI, 0, nBufferSize);

  for (int i = 0; i < nNbLine; i++) {
    sLine = aLine[i];

    SWIR_TRACE(F("getCIMI, line[%d]: %s\n"), i, sLine);

    char *pTemp = sLine;
    while (pTemp < (sLine + strlen(sLine))) // trim ending
    {
      if (*pTemp == '\r') // remove cariage return
      {
        *pTemp = 0; // zero terminate string
        break;
      }
      if (*pTemp == '\n') // remove cariage return
      {
        *pTemp = 0; // zero terminate string
        break;
      }
      pTemp++;
    }

    int nLen = strlen(sLine);
    if (nLen != 15) {
      free(aLine[i]);
      continue;
    }
    for (int k = 0; k < nLen; k++) {
      if (sLine[k] < '0' || sLine[k] > '9') {
        continue;
      }
    }

    strcpy(szCIMI, sLine);
    free(aLine[i]);
  }

  return (strlen(szCIMI) > 0 ? 0 : 1);
}

int BG96::getIMEI(char *szIMEI, int nBufferSize) {
  char szCmd[16];
  char *aLine[BG_LINE]; // read up to 20 lines

  BG96_serial_clearbuf();

  strcpy(szCmd, "AT+CGSN");
  int nNbLine = sendATcmd(szCmd, aLine, BG_LINE);

  char *sLine;
  memset(szIMEI, 0, nBufferSize);

  for (int i = 0; i < nNbLine; i++) {
    sLine = aLine[i];

    SWIR_TRACE(F("getIMEI, line[%d]: %s\n"), i, sLine);

    char *pTemp = sLine;
    while (pTemp < (sLine + strlen(sLine))) // trim ending
    {
      if (*pTemp == '\r') // remove cariage return
      {
        *pTemp = 0; // zero terminate string
        break;
      }
      if (*pTemp == '\n') // remove cariage return
      {
        *pTemp = 0; // zero terminate string
        break;
      }
      pTemp++;
    }

    int nLen = strlen(sLine);
    if (nLen != 15) {
      free(aLine[i]);
      continue;
    }
    for (int k = 0; k < nLen; k++) {
      if (sLine[k] < '0' || sLine[k] > '9') {
        continue;
      }
    }

    strcpy(szIMEI, sLine);
    free(aLine[i]);
  }

  return (strlen(szIMEI) > 0 ? 0 : 1);
}

int BG96::isActPDP(char *szPDP, int nBufferSize) {
  char szCmd[16];
  int ret;

  BG96_serial_clearbuf();

  memset(szPDP, 0, nBufferSize);
  strcpy(szCmd, "AT+QIACT?");

  ret = sendATcmd(szCmd, szPDP, nBufferSize, "+QIACT:");

  return ret;
}

int BG96::actPDP() {
  char szCmd[16];
  char resBuffer[16];
  int ret;

  BG96_serial_clearbuf();

  strcpy(szCmd, "AT+QIACT=1,1");

  ret = sendATcmd(szCmd, resBuffer, sizeof(resBuffer), "OK");

  delay(5000); // Maximum Response Time 150 seconds, determined by network

  return ret;
}

int BG96::deActPDP() {
  char szCmd[16];
  char resBuffer[16];
  int ret;

  BG96_serial_clearbuf();

  strcpy(szCmd, "AT+QIDEACT=1");

  ret = sendATcmd(szCmd, resBuffer, sizeof(resBuffer), "OK");

  return ret;
}

int BG96::canConnect() {
  char szATcmd[16];
  char resBuffer[32];

  BG96_serial_clearbuf();

  sprintf(szATcmd, "AT+CEREG?");
  if (0 == sendATcmd(szATcmd, resBuffer, sizeof(resBuffer), "+CEREG")) {
    char *pszState = NULL;

    pszState = strstr(resBuffer, ",");
    if (pszState != NULL) {
      pszState++;
      if (*pszState == '1' || *pszState == '5') {
        SWIR_TRACE(F("ready to make data call..."));
        return 0; // ready to connect. CEREG equals to 1 (home) nor 5 (roaming)
      }
    }
  }

  SWIR_TRACE(F("Not ready to make data call..."));
  return 1; // not attached to GPRS yet
}

int BG96::getRSSI(int *rssi) {
  char szATcmd[16];
  char resBuffer[32];
  int Rssi;

  BG96_serial_clearbuf();

  sprintf(szATcmd, "AT+QCSQ");
  if (0 == sendATcmd(szATcmd, resBuffer, sizeof(resBuffer),
                     "+QCSQ: \"CAT-M1\",", 4000)) {
    Rssi = atoi(resBuffer);

    SWIR_TRACE(F("RSSI %d \r\n"), Rssi);

    *rssi = Rssi;
    return 0;
  }

  SWIR_TRACE(F("get RSSI Fail..."));
  return 1;
}

int BG96::getTxPower(int *txPower) {
  char szATcmd[16];
  char resBuffer[32];
  int TxPower;

  BG96_serial_clearbuf();

  sprintf(szATcmd, "AT+QCPWR");
  if (0 == sendATcmd(szATcmd, resBuffer, sizeof(resBuffer),
                     "+QCPWR: \"CAT-M\",", 4000)) {
    TxPower = atoi(resBuffer);

    SWIR_TRACE(F("TX Power %d \r\n"), TxPower);

    *txPower = TxPower;
    return 0;
  }

  SWIR_TRACE(F("get Signal Power Fail..."));
  return 1;
}

int BG96::getServingCell(char *servingCell) {
  char szATcmd[32];
  char resBuffer[128];

  BG96_serial_clearbuf();

  sprintf(szATcmd, "AT+QENG=\"servingcell\"");
  if (0 == sendATcmd(szATcmd, resBuffer, sizeof(resBuffer), "+QENG")) {
    char *pszState = NULL;
    char *pszState2 = NULL;

    pszState2 = strstr(resBuffer, ",");

    for (int i = 0; i < 5; i++) {
      pszState = strstr(pszState2, ",");
      if (pszState != NULL) {
        pszState++;
        pszState2 = pszState;
      } else {
        SWIR_TRACE(F("get Serving Cell Fail..."));
        return 1;
      }
    }

    if (pszState != NULL) {
      // Serving Cell
      pszState2 = strstr(pszState, ",");
      SWIR_TRACE(F("getServingCell pszState2: %s\r\n"), pszState2 + 1);
      pszState = strstr(pszState2 + 1, ",");
      SWIR_TRACE(F("getServingCell pszState: %s\r\n"), pszState);
      SWIR_TRACE(F("getServingCell size : %d\r\n"), pszState - pszState2 - 1);
      if (pszState != NULL) {
        int j;

        for (j = 0; j < (pszState - pszState2 - 1); j++) {
          *(servingCell + j) = *(pszState2 + j + 1);
        }

        *(servingCell + j) = 0;

        return 0;
      }
    }
  }

  SWIR_TRACE(F("get Serving Cell Fail..."));
  return 1;
}

int BG96::getRSRP(int *rsrp) {
  char szATcmd[32];
  char resBuffer[128];
  char intBuffer[8];

  BG96_serial_clearbuf();

  sprintf(szATcmd, "AT+QENG=\"servingcell\"");
  if (0 == sendATcmd(szATcmd, resBuffer, sizeof(resBuffer), "+QENG")) {
    char *pszState = NULL;
    char *pszState2 = NULL;

    pszState2 = strstr(resBuffer, ",");

    for (int i = 0; i < 12; i++) {
      pszState = strstr(pszState2, ",");
      if (pszState != NULL) {
        pszState++;
        pszState2 = pszState;
      } else {
        SWIR_TRACE(F("get RSRP Fail..."));
        return 1;
      }
    }

    if (pszState != NULL) {
      // Serving Cell
      pszState2 = strstr(pszState, ",");
      SWIR_TRACE(F("getRSRP pszState2: %s\r\n"), pszState2 + 1);
      pszState = strstr(pszState2 + 1, ",");
      SWIR_TRACE(F("getRSRP pszState: %s\r\n"), pszState);
      SWIR_TRACE(F("getRSRP size : %d\r\n"), pszState - pszState2 - 1);
      if (pszState != NULL) {
        int j;

        for (j = 0; j < (pszState - pszState2 - 1); j++) {
          intBuffer[j] = *(pszState2 + j + 1);
        }

        intBuffer[j] = 0;

        *rsrp = atoi(intBuffer);

        return 0;
      }
    }
  }

  SWIR_TRACE(F("get RSRP Fail..."));
  return 1;
}

int BG96::getSINR(int *sinr) {
  char szATcmd[32];
  char resBuffer[128];
  char intBuffer[8];

  BG96_serial_clearbuf();

  sprintf(szATcmd, "AT+QENG=\"servingcell\"");
  if (0 == sendATcmd(szATcmd, resBuffer, sizeof(resBuffer), "+QENG")) {
    char *pszState = NULL;
    char *pszState2 = NULL;

    pszState2 = strstr(resBuffer, ",");

    for (int i = 0; i < 15; i++) {
      pszState = strstr(pszState2, ",");
      if (pszState != NULL) {
        pszState++;
        pszState2 = pszState;
      } else {
        SWIR_TRACE(F("get SINR Fail..."));
        return 1;
      }
    }

    if (pszState != NULL) {
      // Serving Cell
      pszState2 = strstr(pszState, ",");
      SWIR_TRACE(F("getSINR pszState2: %s\r\n"), pszState2 + 1);
      pszState = strstr(pszState2 + 1, ",");
      SWIR_TRACE(F("getSINR pszState: %s\r\n"), pszState);
      SWIR_TRACE(F("getSINR size : %d\r\n"), pszState - pszState2 - 1);
      if (pszState != NULL) {
        int j;

        for (j = 0; j < (pszState - pszState2 - 1); j++) {
          intBuffer[j] = *(pszState2 + j + 1);
        }

        intBuffer[j] = 0;

        *sinr = atoi(intBuffer);

        return 0;
      }
    }
  }

  SWIR_TRACE(F("get SINR Fail..."));
  return 1;
}

int BG96::socketCreate(int service_type, char *remote_address,
                       long remote_port) {
  char szCmd[128];
  char recvBuf[32];
  char *service[] = {"UDP", "TCP"};

  BG96_serial_clearbuf();

  sprintf(szCmd, "AT+QIOPEN=1,%d,\"%s\",\"%s\",%ld,0,0", _nSocket,
          service[service_type], remote_address, remote_port);

  //	if (0 == sendSckATcmd(szCmd, recvBuf, sizeof(recvBuf), " 0,", 10000))
  if (0 == sendSckATcmd(szCmd, recvBuf, sizeof(recvBuf), "+QIOPEN: 0,", 20000))
  //	if (0 == sendSckATcmd(szCmd, recvBuf, sizeof(recvBuf), "0,", 20000))

  {

    _nSocket = atoi(recvBuf);
    if (_nSocket > 11)
      _nSocket = 0;

    SWIR_TRACE(F("Socket : %d \r\n"), _nSocket);

    return 0;
  }

  SWIR_TRACE(F("socket Create Fail..."));
  return 1;
}

int BG96::socketClose() {
  char szCmd[16];
  char resBuffer[16];
  int ret;

  BG96_serial_clearbuf();

  /* force to close the socket 3 second */
  //	sprintf(szCmd, "AT+QICLOSE=%d",_nSocket);
  sprintf(szCmd, "AT+QICLOSE=%d,%d", _nSocket, 3);
  //	sprintf(szCmd, "AT+QICLOSE=%d,%d",_nSocket,10);

  /* timeout Max 10Sec */
  ret = sendATcmd(szCmd, resBuffer, sizeof(resBuffer), "OK", 15000);

  return ret;
}

int BG96::socketSend(char *buffer, int size) {
  char sendBuffer[16];
  char resBuffer[16];
  char *aLine[3]; // read up to 20 lines

  int ret, i;

  BG96_serial_clearbuf();

  _serial.setTimeout(5500);
  _timeOut = 5000;

  SWIR_TRACE(F("size - %d..."), size);
  if (size < 1 || size > 1460) // Send Data Size up to 1460 bytes
    return 1;

  sprintf(sendBuffer, "AT+QISEND=%d,%d", _nSocket, size);
  _serial.println(sendBuffer);
  SWIR_TRACE(F("sendATcmd (%s) - %d..."), sendBuffer, _timeOut);

  _serial.flush();

  /* Wait ">" */
  i = readATresponseLine(aLine, 3, _timeOut);
  SWIR_TRACE(F("readLine %d..."), i);

  if (i > 0) {
    for (int j = 0; j < i; j++)
      free(aLine[j]);
  }

  /* Send Data by Socket */
  for (i = 0; i < size; i++) {
    _serial.write(buffer[i]);
  }
  _serial.flush();

  /* Wait "SEND OK" */
  i = readATresponseLine(aLine, 3, 5000);
  if (i > 0) {
    for (int j = 0; j < i; j++)
      free(aLine[j]);
    return 0;
  }

  return 1;
}

int BG96::socketSend(const char *str) {
  return socketSend((char *)str, strlen(str));
}

int BG96::socketRecv(char *buffer, int bufferSize, int *recvSize,
                     unsigned long timeout) {
  char szCmd[32];
  char *aLine[BG_LINE]; // read up to 20 lines

  int i;

  _serial.setTimeout(5500);
  _timeOut = 5000;

  i = readATresponseLine(aLine, 3, _timeOut);

  if (i > 0) {
    for (int j = 0; j < i; j++)
      free(aLine[j]);
  } else {
    return 1; /* No Read Data */
  }

  if (bufferSize > 1460)
    bufferSize = 1460;

  sprintf(szCmd, "AT+QIRD=%d,%d", _nSocket, bufferSize);

  if (0 == sendServicecmd(szCmd, buffer, bufferSize, recvSize, timeout))
    return 0;

  return 1;
}

int BG96::sendServicecmd(char *szCmd, char *szResponse, int nResponseBufSize,
                         int *recvSize, unsigned long ulWaitDelay) {
  int nRet = 0;

  memset(szResponse, 0, nResponseBufSize);

  SWIR_TRACE(F("sendATcmd (%s) - %d..."), szCmd, ulWaitDelay);

  _serial.setTimeout(ulWaitDelay + 500);
  _serial.println(szCmd);

  _serial.flush();

  nRet = readServiceResponseLine(szResponse, nResponseBufSize, recvSize,
                                 ulWaitDelay);

  if (nRet == 0) {
    SWIR_TRACE(F("...sendATcmd OK"));
  } else {
    SWIR_TRACE(F("...sendATcmd Fails"));
  }

  return nRet;
}

int BG96::sendSckATcmd(char *szCmd, char *szResponse, int nResponseBufSize,
                       const char *szResponseFilter,
                       unsigned long ulWaitDelay) {
  int nRet = 0;

  memset(szResponse, 0, nResponseBufSize);
  SWIR_TRACE(F("sendATcmd (%s) - %d..."), szCmd, ulWaitDelay);

  _serial.setTimeout(ulWaitDelay + 500);
  _serial.println(szCmd);

  _serial.flush();

  nRet = readSckresponseLine(szResponse, nResponseBufSize, szResponseFilter,
                             ulWaitDelay);

  if (nRet == 0) {
    SWIR_TRACE(F("...sendATcmd OK"));
  } else {
    SWIR_TRACE(F("...sendATcmd Fails"));
  }

  return nRet;
}

int BG96::sendATcmd(char *szCmd, char *szResponse, int nResponseBufSize,
                    const char *szResponseFilter, unsigned long ulWaitDelay) {
  int nRet = 0;

  memset(szResponse, 0, nResponseBufSize);

  SWIR_TRACE(F("sendATcmd (%s) - %d..."), szCmd, ulWaitDelay);

  _serial.setTimeout(ulWaitDelay + 500);
  _serial.println(szCmd);

  _serial.flush();

  nRet = readATresponseLine(szResponse, nResponseBufSize, szResponseFilter,
                            ulWaitDelay);

  if (nRet == 0) {
    SWIR_TRACE(F("...sendATcmd OK"));
  } else {
    SWIR_TRACE(F("...sendATcmd Fails"));
  }

  return nRet;
}

int BG96::sendATcmd(char *szCmd, char *aLine[], int nMaxLine,
                    unsigned long ulWaitDelay) {
  int nRet = 0;

  SWIR_TRACE(F("sendATcmd2 (%s) - %d..."), szCmd, ulWaitDelay);

  _serial.setTimeout(ulWaitDelay + 500);
  _serial.println(szCmd);

  _serial.flush();

  nRet = readATresponseLine(aLine, nMaxLine, ulWaitDelay);

  if (nRet > 0) {
    SWIR_TRACE(F("...sendATcmd OK"));
  } else {
    SWIR_TRACE(F("...sendATcmd Fails"));
  }

  return nRet;
}

int BG96::readServiceResponseLine(char *szLine, int nLineBufSize, int *recvSize,
                                  unsigned long ulDelay) {
  int nbLine = 0;
  char *aLine[BG_LINE];
  Countdown oCountdown(ulDelay);
  char *pszSubstr = NULL;
  int realRead;
  int nRet = 1;
  char *sLine;

  bool isRead = false;
  int get_idx;

  int _allocSize = 0;
  char _allocIdx[BG_LINE];

  int sumLen = 0;
  bool countEnable = false;

  memset(szLine, 0, nLineBufSize);

  int startIdx = 0;
  int stopIdx = 0;

  do {
    if (_serial.available()) {
      String sStr;
      sStr = _serial.readStringUntil('\n');
      int nLen = sStr.length();

      SWIR_TRACE(F("sStr.length() : %d"), nLen);

      if (countEnable)
        sumLen += nLen;

      if (nLen > 1) {
        _allocIdx[_allocSize] = nbLine;
        _allocSize++;

        aLine[nbLine] = (char *)malloc(nLen + 1);
        sStr.toCharArray(aLine[nbLine], nLen);
        aLine[nbLine][nLen] = 0;

        pszSubstr = strstr(aLine[nbLine], "+QIRD:");
        if (pszSubstr != NULL) {
          realRead = atoi(pszSubstr + 7);
          *recvSize = realRead;
          stopIdx = realRead;
          SWIR_TRACE(F("Found +QIRD"));
          SWIR_TRACE(F("realRead : %d"), realRead);
          nRet = 0;
          break;
        }
        nbLine++;
      }
    }
    if (nbLine >= BG_LINE)
      break;
  } while (!oCountdown.expired());

  do {
    if (_serial.available()) {
      String sStr;

      sStr = _serial.readStringUntil('\n');
      int nLen = sStr.length();

      if ((stopIdx - startIdx) > nLen) {
        memcpy(szLine + startIdx, sStr.c_str(), nLen);
        startIdx += nLen;
        szLine[startIdx++] = '\n';
      } else {
        int cpySize = stopIdx - startIdx;
        memcpy(szLine + startIdx, sStr.c_str(), cpySize);
        startIdx += cpySize;
        SWIR_TRACE(F("cpySize : %d"), cpySize);
        break;
      }
    }
  } while (!oCountdown.expired());

  SWIR_TRACE(F("startIdx : %d \n"), startIdx);

  /* Serial Buffer Clear */
  BG96_serial_clearbuf();

  for (int i = 0; i < _allocSize; i++) {
    //		SWIR_TRACE(F("_alloc Index : %d\n"),_allocIdx[i]);
    free(aLine[_allocIdx[i]]);
  }

  return nRet;
}

int BG96::readSckresponseLine(char *szLine, int nLineBufSize,
                              const char *szResponseFilter,
                              unsigned long ulDelay) {
  int nbLine = 0;
  char *aLine[BG_LINE];
  Countdown oCountdown(ulDelay);
  char *pszSubstr = NULL;

  memset(szLine, 0, nLineBufSize);

  do {
    if (_serial.available()) {
      String sStr;
      sStr = _serial.readStringUntil('\n');
      int nLen = sStr.length();

      if (nLen > 1) {
        aLine[nbLine] = (char *)malloc(nLen + 1);
        sStr.toCharArray(aLine[nbLine], nLen);
        aLine[nbLine][nLen] = 0;

        pszSubstr = strstr(aLine[nbLine], "+QIOPEN:");
        if (pszSubstr != NULL) {
          nbLine++;
          SWIR_TRACE(F("Found OK"));
          break;
        }

        nbLine++;
      }
    }
    if (nbLine >= BG_LINE) {
      break;
    }
  } while (!oCountdown.expired());

  SWIR_TRACE(F("readATresponseLine: %d line(s)\n"), nbLine);

  int i;

  int nRet = 3;

  for (i = 0; i < nbLine; i++) {
    SWIR_TRACE(F("line[%d]: %s\n"), i, aLine[i]);

    if (szResponseFilter == NULL) {
      // Not filtering response
      strcpy(szLine, aLine[i]);
      nRet = 0;
    } else if (strlen(szResponseFilter) > 0) {
      // Filtering Response
      char *pszSubstr = NULL;

      pszSubstr = strstr(aLine[i], szResponseFilter);
      if (pszSubstr != NULL) {
        pszSubstr += strlen(szResponseFilter);
        while (isSpace(*pszSubstr)) // trim heading
        {
          pszSubstr++;
        }
        char *pTemp = pszSubstr;
        while (pTemp < (aLine[i] + strlen(aLine[i]))) // trim ending
        {
          if (*pTemp == '\n') // remove cariage return
          {
            *pTemp = 0; // zero terminate string
            break;
          }
          pTemp++;
        }

        SWIR_TRACE(F("Filtered response: %s\n"), pszSubstr);
        strcpy(szLine, pszSubstr);
        nRet = 0;
      }
    } else {
      // Not filtering response
      strcpy(szLine, aLine[i]);
      nRet = 0;
    }

    free(aLine[i]);
  }

  return nRet;
}

int BG96::readATresponseLine(char *szLine, int nLineBufSize,
                             const char *szResponseFilter,
                             unsigned long ulDelay) {
  int nbLine = 0;
  char *aLine[BG_LINE];
  Countdown oCountdown(ulDelay);
  char *pszSubstr = NULL;

  memset(szLine, 0, nLineBufSize);

  do {
    if (_serial.available()) {
      String sStr;
      sStr = _serial.readStringUntil('\n');
      int nLen = sStr.length();

      if (nLen > 1) {
        aLine[nbLine] = (char *)malloc(nLen + 1);
        sStr.toCharArray(aLine[nbLine], nLen);
        aLine[nbLine][nLen] = 0;

        pszSubstr = strstr(aLine[nbLine], "OK");
        if (pszSubstr != NULL) {
          nbLine++;
          SWIR_TRACE(F("Found OK"));
          break;
        }

        nbLine++;
      }
    }
    if (nbLine >= BG_LINE) {
      break;
    }
  } while (!oCountdown.expired());

  SWIR_TRACE(F("readATresponseLine: %d line(s)\n"), nbLine);

  int i;

  int nRet = 3;

  for (i = 0; i < nbLine; i++) {
    SWIR_TRACE(F("line[%d]: %s\n"), i, aLine[i]);

    if (szResponseFilter == NULL) {
      // Not filtering response
      strcpy(szLine, aLine[i]);
      nRet = 0;
    } else if (strlen(szResponseFilter) > 0) {
      // Filtering Response
      char *pszSubstr = NULL;

      pszSubstr = strstr(aLine[i], szResponseFilter);
      if (pszSubstr != NULL) {
        pszSubstr += strlen(szResponseFilter);
        while (isSpace(*pszSubstr)) // trim heading
        {
          pszSubstr++;
        }
        char *pTemp = pszSubstr;
        while (pTemp < (aLine[i] + strlen(aLine[i]))) // trim ending
        {
          if (*pTemp == '\n') // remove cariage return
          {
            *pTemp = 0; // zero terminate string
            break;
          }
          pTemp++;
        }

        SWIR_TRACE(F("Filtered response: %s\n"), pszSubstr);
        strcpy(szLine, pszSubstr);
        nRet = 0;
      }
    } else {
      // Not filtering response
      strcpy(szLine, aLine[i]);
      nRet = 0;
    }

    free(aLine[i]);
  }

  return nRet;
}

int BG96::readATresponseLine(char *aLine[], int nMaxLine,
                             unsigned long ulDelay) {
  int nbLine = 0;
  Countdown oCountDown(ulDelay);
  char *pszSubstr = NULL;
  int i;

  do {
    if (_serial.available()) {
      String sStr;
      sStr = _serial.readStringUntil('\n');

      int nLen = sStr.length();
      if (nLen > 1) {
        aLine[nbLine] = (char *)malloc(nLen + 1);
        sStr.toCharArray(aLine[nbLine], nLen);

        aLine[nbLine][nLen] = 0;

        pszSubstr = strstr(aLine[nbLine], ">");
        if (pszSubstr != NULL) {
          nbLine++;
          SWIR_TRACE(F("Found >"));
          break;
        }

        pszSubstr = strstr(aLine[nbLine], "+QIURC: \"recv\"");
        if (pszSubstr != NULL) {
          nbLine++;
          SWIR_TRACE(F("Found recv"));
          break;
        }

        pszSubstr = strstr(aLine[nbLine], "OK");
        if (pszSubstr != NULL) {
          nbLine++;
          SWIR_TRACE(F("Found OK"));
          break;
        }

        nbLine++;
      }
    }
    if (nbLine >= nMaxLine) {
      break;
    }
  } while (!oCountDown.expired());

  SWIR_TRACE(F("readATresponseLine2: %d line(s)\n"), nbLine);

  return nbLine;
}

void BG96::BG96_serial_clearbuf() {
  _serial.flush();

  while (_serial.available()) {
    _serial.read();
  }
}

void BG96::BG96_trace(const __FlashStringHelper *szTrace, ...) {
#ifdef __BG_96_DEBUG
  char szBuf[TRACE_BUFFER_SIZE];
  va_list args;

  va_start(args, szTrace);
#ifdef __AVR__
  vsnprintf_P(szBuf, sizeof(szBuf), (const char *)szTrace,
              args); // program for AVR
#else
  vsnprintf(szBuf, sizeof(szBuf), (const char *)szTrace,
            args); // for the rest of the world
#endif
  va_end(args);

  _debug.println(szBuf);
#endif
}

int BG96::actGPS(void) {
  char resBuffer[8];
  return sendATcmd("AT+QGPS=1", resBuffer, sizeof(resBuffer), "OK", 3000);
}

int BG96::deActGPS(void) {
  char resBuffer[8];
  return sendATcmd("AT+QGPS=0", resBuffer, sizeof(resBuffer), "OK", 3000);
}

int BG96::getGPSLoc(char *buffer, int bufferSize) {
  return sendATcmd("AT+QGPSLOC=2", buffer, bufferSize, "+QGPSLOC", 3000);
}

int BG96::disablePSM() {
  char szCmd[16];
  char resBuffer[16];
  int ret;

  int value = 0;

  BG96_serial_clearbuf();

  sprintf(szCmd, "AT+CPSMS=%d", value);

  ret = sendATcmd(szCmd, resBuffer, sizeof(resBuffer), "OK", 3000);

  return ret;
}
