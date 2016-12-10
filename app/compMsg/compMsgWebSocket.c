/*
* Copyright (c) 2016, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
* All rights reserved.
*
* License: BSD/MIT
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/* 
 * File:   compMsgWebsocket.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 10th, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "lwip/err.h"
#include "lwip/app/espconn.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "compMsgDispatcher.h"

#define TCP ESPCONN_TCP

typedef struct socketInfo {
  struct espconn *pesp_conn;
} socketInfo_t;

#define MAX_SOCKET 5
static int socket_num = 0;
static socketInfo_t *socket[MAX_SOCKET] = { NULL, NULL, NULL, NULL, NULL };

static const char *header_key = "Sec-WebSocket-Key: ";
static const char *ws_uuid ="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

static const char *HEADER_WEBSOCKETLINE = "Upgrade: websocket";

static char *HEADER_OK = "HTTP/1.x 200 OK \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: max-age=3600, public \r\n\
Content-Type: text/html \r\n\
Content-Encoding: gzip \r\n\r\n";

static char *HEADER_WEBSOCKET_START = "\
HTTP/1.1 101 WebSocket Protocol Handshake\r\n\
Connection: Upgrade\r\n\
Upgrade: WebSocket\r\n\
Access-Control-Allow-Origin: http://";

static char *HEADER_WEBSOCKET_URL = "192.168.178.67";

static char *HEADER_WEBSOCKET_END = "\r\n\
Access-Control-Allow-Credentials: true\r\n\
Access-Control-Allow-Headers: content-type \r\n\
Sec-WebSocket-Accept: ";

static uint8_t err_opcode[5] = {0};

// #define checkAllocOK(addr) if(addr == NULL) checkErrOK(WEBSOCKET_ERR_OUT_OF_MEMORY, "")

enum compMsg_error_code
{
  WEBSOCKET_ERR_OK = 0,
  WEBSOCKET_ERR_OUT_OF_MEMORY = -1,
  WEBSOCKET_ERR_TOO_MUCH_DATA = -2,
  WEBSOCKET_ERR_INVALID_FRAME_OPCODE = -3,
  WEBSOCKET_ERR_USERDATA_IS_NIL = -4,
  WEBSOCKET_ERR_WRONG_METATABLE = -5,
  WEBSOCKET_ERR_PESP_CONN_IS_NIL = -6,
  WEBSOCKET_ERR_MAX_SOCKET_REACHED = -7,
};

#define BASE64_INVALID '\xff'
#define BASE64_PADDING '='
#define ISBASE64(c) (unbytes64[c] != BASE64_INVALID)

#define SSL_BUFFER_SIZE 5120

static const uint8 b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

typedef void (* websocketBinaryReceived_t)(void *arg, void *wud, char *pdata, unsigned short len);
typedef void (* websocketTextReceived_t)(void *arg, void *wud, char *pdata, unsigned short len);

#define TIMER_MODE_OFF 3
#define TIMER_MODE_SINGLE 0
#define TIMER_MODE_SEMI 2
#define TIMER_MODE_AUTO 1
#define TIMER_IDLE_FLAG (1<<7) 

typedef struct compMsgTimer {
  os_timer_t timer;
  compMsgDispatcher_t *self;
  uint32_t interval;
  uint8_t mode;
} compMsgTimer_t;

static compMsgTimer_t compMsgTimers[NUM_TMR];
static int isMstimer = 1;
static os_timer_t apTimer;
static int cnt = 0;
static uint16_t tcp_server_timeover = 30;

// ============================ websocketSendData =======================

/**
 * \brief send message data to a web scoket
 * \param wud The websocket user data
 * \param payload The message data
 * \param size The number of characters of the message
 * \param opcode The type for sending the data (text/binary/...)
 * \return Error code or ErrorOK
 *
 */
static uint8_t websocketSendData(websocketUserData_t *wud, const char *payload, int size, int opcode)
{
  uint8_t hdrBytes[4]; // we have either 2 or 4 bytes depending on length of message
  int hdrLgth;
  int fsize;
  uint8_t*buff;
  uint8_t result;
  int i;

//ets_printf("websocketSendData: size: %d\n", size);
  hdrLgth = 2;
  hdrBytes[0] = 0x80; //set first bit
  hdrBytes[0] |= opcode; //frame->opcode; //set op code
  if (size < 126) {
    hdrBytes[1] = size;
  } else {
    if (size < SSL_BUFFER_SIZE - sizeof(hdrBytes)) {
      hdrLgth += 2;
      hdrBytes[1] = 126;
      hdrBytes[2] = (size >> 8) & 0xFF;
      hdrBytes[3] = size & 0xFF;
    } else {
      return WEBSOCKET_ERR_TOO_MUCH_DATA;
    }
  }
  fsize = size + hdrLgth;
  buff = os_malloc(fsize);
  if (buff == NULL) {
    return WEBSOCKET_ERR_OUT_OF_MEMORY;
  }
  for (i = 0; i < hdrLgth; i++) {
    buff[i] = hdrBytes[i];
  }

  os_memcpy(&buff[hdrLgth], (uint8_t *)payload, size);
  result = espconn_sent(wud->pesp_conn, (unsigned char *)buff, fsize);
//ets_printf("espconn_sent: result: %d\n", result);
  os_free(buff);
//ets_printf("espconn_sent: done\n");
  return WEBSOCKET_ERR_OK;
}

// ============================ websocket_parse =========================================

static int ICACHE_FLASH_ATTR websocket_parse(char * data, size_t dataLenb, char **resData, int *len, websocketUserData_t *wud) {
  uint8_t byte = data[0];
  uint8_t FIN = byte & 0x80;
  uint8_t opcode = byte & 0x0F;

  if ((opcode > 0x03 && opcode < 0x08) || opcode > 0x0B) {
    ets_sprintf(err_opcode, "%d", opcode);
    checkErrOK(WEBSOCKET_ERR_INVALID_FRAME_OPCODE);
  }

  //   opcodes: {1 text 2 binary 8 close 9 ping 10 pong}
  switch (opcode) {
  case OPCODE_TEXT:
//ets_printf("parse text\n");
    break;
  case OPCODE_BINARY:
//ets_printf("parse binary\n");
    break;
  case OPCODE_CLOSE:
    break;
  case OPCODE_PING:
    websocketSendData(wud, " ", 1, OPCODE_PONG);
    return WEBSOCKET_ERR_OK;
    break;
  case OPCODE_PONG:
    return WEBSOCKET_ERR_OK;
    break;
  }
  byte = data[1];

  char * recv_data;
  int masked = byte & 0x80;
  int size = byte & 0x7F;
  int offset = 2;

  if (size == 126) {
    size = 0;
    size = data[3];                  //LSB
    size |= (uint64_t) data[2] << 8; //MSB
    offset = 4;
  } else if (size == 127) {
    size = 0;
    size |= (uint64_t) data[2] << 56;
    size |= (uint64_t) data[3] << 48;
    size |= (uint64_t) data[4] << 40;
    size |= (uint64_t) data[5] << 32;
    size |= (uint64_t) data[6] << 24;
    size |= (uint64_t) data[7] << 16;
    size |= (uint64_t) data[8] << 8;
    size |= (uint64_t) data[9];
    offset = 10;
  }

  if (masked) {
    //read mask key
    char mask[4];
    uint64_t i;

    mask[0] = data[offset];
    mask[1] = data[offset + 1];
    mask[2] = data[offset + 2];
    mask[3] = data[offset + 3];
    offset += 4;
    for (i = 0; i < size; i++) {
      data[i + offset] ^= mask[i % 4];
    }
  }
  recv_data = &data[offset];
for (int i = 0; i < size; i++) {
//  ets_printf("i: %d 0x%02x\n", i, recv_data[i]&0xFF);
}
  switch (opcode) {
  case OPCODE_TEXT:
//ets_printf("cb text\n");
    wud->websocketTextReceived(wud->compMsgDispatcher, wud, recv_data, size);
    break;
  case OPCODE_BINARY:
//ets_printf("cb binary\n");
//ets_printf("parse binary: size: %d\n", size);
    wud->websocketBinaryReceived(wud->compMsgDispatcher, wud, recv_data, size);
    break;
  }
  *resData = recv_data;
  *len = size;
  recv_data[size] = 0;

  if (size + offset < dataLenb) {
    websocket_parse(&data[size + offset], dataLenb - (size + offset), resData, len, wud);
  }
  return WEBSOCKET_ERR_OK;
}

// ================================= websocket_recv ====================================

static uint8_t websocket_recv(char *string, websocketUserData_t *wud, char **data, int *lgth) {
  char * key = NULL;
  int idx;
  int found;
  uint8_t result;

  idx = 0;
//ets_printf("websocket_recv: %s!remote_port: %d\n", wud->curr_url, wud->remote_port);
//ets_printf("websocket_recv: %s!%s!remote_port: %d\n",string,  wud->curr_url, wud->remote_port);
  if ((wud->curr_url != NULL) && (strstr(string, wud->curr_url) != NULL)) {
    if (strstr(string, header_key) != NULL) {
      char *begin = strstr(string, header_key) + os_strlen(header_key);
      char *end = strstr(begin, "\r");
      key = os_malloc((end - begin) + 1);
      checkAllocOK(key);
      os_memcpy(key, begin, end - begin);
      key[end - begin] = 0;
    }
//ets_printf("websocket_recv2: key: %s\n", key);
    const char *trailer;
    trailer = "\r\n\r\n";
    int trailerLen;
    trailerLen = os_strlen(trailer);
    size_t digestLen;
    digestLen = 20; //sha1 is always 20 byte long
    uint8_t digest[digestLen];
    int payloadLen;
    char *payload;
    uint8_t *base64Digest;
    SHA1_CTX ctx;
    // Use the SHA* functions in the rom
    SHA1Init(&ctx);
    SHA1Update(&ctx, key, os_strlen(key));
    SHA1Update(&ctx, ws_uuid, os_strlen(ws_uuid));
    SHA1Final(digest, &ctx);

    result = wud->compMsgDispatcher->toBase64(digest, &digestLen, &base64Digest);
    checkErrOK(result);
    checkAllocOK(base64Digest);
    payloadLen = os_strlen(HEADER_WEBSOCKET_START) + os_strlen(HEADER_WEBSOCKET_URL) +os_strlen(HEADER_WEBSOCKET_END) + digestLen + trailerLen;
    payload = os_malloc(payloadLen + 1); // +1 for trailing '\0' character
    checkAllocOK(payload);
    os_sprintf(payload, "%s%s%s%s%s\0", HEADER_WEBSOCKET_START, HEADER_WEBSOCKET_URL, HEADER_WEBSOCKET_END, base64Digest, trailer);
//ets_printf("§payloadLen: %d payload: %d§\n", payloadLen, c_strlen(payload));
    os_free(base64Digest);
    struct espconn *pesp_conn = NULL;
    pesp_conn = wud->pesp_conn;

    // FIXME!! reboot called if setting here for socket!!
    if (wud->isWebsocket != 1) {
      wud->isWebsocket = 1;
    }

//ets_printf("payload: %d!%s!\n", payloadLen, payload);
    result = espconn_sent(wud->pesp_conn, (unsigned char *)payload, payloadLen);
    os_free(key);
//ets_printf("espconn_sent: result: %d\n", result);
    checkErrOK(result);
  } else if (wud->isWebsocket == 1) {
//ets_printf("websocket_parse: %d!curr_url: %s!\n", os_strlen(string), wud->curr_url);
    websocket_parse(string, os_strlen(string), data, lgth, wud);
  }
  return WEBSOCKET_ERR_OK;

}

// ================================= webSocketRunClientMode ====================================

static uint8_t websocketRunClientMode(compMsgDispatcher_t *self, uint8_t mode) {
  int result;
  return COMP_DISP_ERR_OK;
}

// ================================= serverDisconnected  ====================================

static void serverDisconnected(void *arg) {
  struct espconn *pesp_conn;

  pesp_conn = (struct espconn *)arg;
//ets_printf("serverDisconnected: arg: %p\n", arg);

}

// ================================= serverReconnected  ====================================

static void serverReconnected(void *arg, int8_t err) {
  struct espconn *pesp_conn;

  pesp_conn = (struct espconn *)arg;
//ets_printf("serverReconnected: arg: %p\n", arg);

}

// ================================= socketReceived  ====================================

static void socketReceived(void *arg, char *pdata, unsigned short len) {
  struct espconn *pesp_conn;
  char url[50] = { 0 };
  int idx;
  websocketUserData_t *wud;

  pesp_conn = (struct espconn *)arg;
//ets_printf("§webSocketReceived: arg: %p len: %d§\n", arg, len);
//ets_printf("socketReceived: arg: %p pdata: %s len: %d\n", arg, pdata, len);
  char temp[20] = {0};
  c_sprintf(temp, IPSTR, IP2STR( &(pesp_conn->proto.tcp->remote_ip) ) );
  ets_printf("§remote ");
  ets_printf(temp);
  ets_printf(":");
  ets_printf("%d",pesp_conn->proto.tcp->remote_port);
  ets_printf(" received.§\n");

  wud = (websocketUserData_t *)pesp_conn->reverse;
  wud->remote_ip[0] = pesp_conn->proto.tcp->remote_ip[0];
  wud->remote_ip[1] = pesp_conn->proto.tcp->remote_ip[1];
  wud->remote_ip[2] = pesp_conn->proto.tcp->remote_ip[2];
  wud->remote_ip[3] = pesp_conn->proto.tcp->remote_ip[3];
  wud->remote_port = pesp_conn->proto.tcp->remote_port;
//ets_printf("==received remote_port: %d\n", wud->remote_port);
  if (strstr(pdata, "GET /") != 0) {
    char *begin = strstr(pdata, "GET /") + 4;
    char *end = strstr(begin, " ");
    os_memcpy(url, begin, end - begin);
    url[end - begin] = 0;
//ets_printf("url: %s\n", url);
  }
  if ((url[0] != 0) && (strstr(pdata, HEADER_WEBSOCKETLINE) != 0)) {
    idx = 0;
    wud->curr_url = NULL;
    while (idx < wud->num_urls) {
      if (c_strcmp(url, wud->urls[idx]) == 0) {
        wud->curr_url = wud->urls[idx];
        wud->isWebsocket = 1;
        break;
      }
      idx++;
    }
  }
//ets_printf("iswebsocket: %d %s\n", wud->isWebsocket, wud->curr_url);

  if(wud->isWebsocket == 1) {
    char *data = "";
    int lgth = 0;
    int result;

    result = websocket_recv(pdata, wud, &data, &lgth);
//    checkErrOK(result);
  }
}

// ================================= socketSent  ====================================

static void socketSent(void *arg) {
  struct espconn *pesp_conn;
  websocketUserData_t *wud;
  int result;
  bool boolResult;

  pesp_conn = (struct espconn *)arg;
//ets_printf("websocketSent: arg: %p\n", arg);
  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
    return;
  }
  wud = (websocketUserData_t *)pesp_conn->reverse;
  if(wud == NULL) {
    return;
  }
  if (wud->compMsgDispatcher->stopAP) {
//ets_printf("§stopAP§\n");
    wud->compMsgDispatcher->stopAP = 0;
    boolResult = wifi_set_opmode(OPMODE_STATION);
    if (!boolResult) {
ets_printf("§websocketSent: COMP_DISP_ERR_CANNOT_SET_OPMODE§");
//      return COMP_DISP_ERR_CANNOT_SET_OPMODE;
    }
  }
}

// ================================= serverConnected  ====================================

static void serverConnected(void *arg) {
  struct espconn *pesp_conn;
  int result;
  int i;

//ets_printf("serverConnected: arg: %p\n", arg);
  pesp_conn = arg;
  for(i = 0; i < MAX_SOCKET; i++) {
    if (socket[i] == NULL) { // found empty slot
      break;
    }
  }
  if(i>=MAX_SOCKET) {// can't create more socket
    pesp_conn->reverse = NULL;    // not accept this conn
    if(pesp_conn->proto.tcp->remote_port || pesp_conn->proto.tcp->local_port) {
      espconn_disconnect(pesp_conn);
    }
//    checkErrOK(gL, WEBSOCKET_ERR_MAX_SOCKET_REACHED, "websocket_server_connected");
    return;
  }
//ets_printf("registstart\n");
  result = espconn_regist_recvcb(pesp_conn, socketReceived);
  if (result != COMP_DISP_ERR_OK) {
//ets_printf("regist socketReceived err: %d\n", result);
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != COMP_DISP_ERR_OK) {
//ets_printf("regist socketSent err: %d\n", result);
  }
  result = espconn_regist_disconcb(pesp_conn, serverDisconnected);
  if (result != COMP_DISP_ERR_OK) {
//ets_printf("regist serverDisconnected err: %d\n", result);
  }
  result = espconn_regist_reconcb(pesp_conn, serverReconnected);
  if (result != COMP_DISP_ERR_OK) {
//ets_printf("regist serverReconnected err: %d\n", result);
  }

}

// ================================= alarmTimerAP ====================================

static  void alarmTimerAP(void *arg) {
  compMsgDispatcher_t *self;
  struct ip_info pTempIp;
  uint8_t timerId;
  uint8_t status;
  char temp[64];
  compMsgTimer_t *tmr;
  uint8_t mode;
  int numericValue;
  uint8_t *stringValue;
  struct espconn *pesp_conn;
  unsigned port;
  ip_addr_t ipaddr;
  unsigned type;
  int result;
  websocketUserData_t *wud;

ets_printf("§alarmTimerAP§\n");
  pesp_conn = NULL;
  mode = SOFTAP_IF;
  timerId = (uint8_t)((uint32_t)arg);
//ets_printf("§alarmTimerAP timerId: %d§\n", timerId);
  tmr = &compMsgTimers[timerId];
  self = tmr->self;
//ets_printf("§alarmTimerAP: timerId: %d self: %p§\n", timerId, self);
  status = wifi_station_get_connect_status();
ets_printf("alarmTimerAP:wifi is in mode: %d status: %d ap_id: %d hostname: %s!\n", wifi_get_opmode(), status, wifi_station_get_current_ap_id(), wifi_station_get_hostname());
  switch (status) {
  case STATION_IDLE:
ets_printf("§STATION_IDLE§\n");
    break;
  case STATION_CONNECTING:
ets_printf("§STATION_CONNECTING§\n");
    break;
  case STATION_WRONG_PASSWORD:
ets_printf("§STATION_WRONG_PASSWORD§\n");
    tmr->mode |= TIMER_IDLE_FLAG;
    ets_timer_disarm(&tmr->timer);
    self->websocketSendConnectError(self, status);
    return;
    break;
  case STATION_NO_AP_FOUND:
ets_printf("§STATION_NO_AP_FOUND§\n");
    tmr->mode |= TIMER_IDLE_FLAG;
    ets_timer_disarm(&tmr->timer);
    self->websocketSendConnectError(self, status);
    return;
    break;
  case STATION_CONNECT_FAIL:
ets_printf("§STATION_CONNECT_FAIL§\n");
    tmr->mode |= TIMER_IDLE_FLAG;
    ets_timer_disarm(&tmr->timer);
    self->websocketSendConnectError(self, status);
    return;
    break;
  case STATION_GOT_IP:
ets_printf("§STATION_GOT_IP§\n");
    break;
  }
  wifi_get_ip_info(mode, &pTempIp);
  if(pTempIp.ip.addr==0){
ets_printf("ip: nil\n");
    return;
  }
  tmr->mode |= TIMER_IDLE_FLAG;
  ets_timer_disarm(&tmr->timer);
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&pTempIp.ip));
ets_printf("IP: %s\n", temp);

  result = self->getWifiValue(self, WIFI_INFO_PROVISIONING_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
//ets_printf("port: %d!%p!%d!\n", numericValue, stringValue, result);
//  checkErrOK(result);
  port = numericValue;
//  result = self->getWifiValue(self, WIFI_INFO_PROVISIONING_IP_ADDR, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
//  checkErrOK(result);

  wud = (websocketUserData_t *)os_zalloc(sizeof(websocketUserData_t));
//   checkAllocOK(wud);
//ets_printf("wud0: %p\n", wud);
  wud->isWebsocket = 0;
  wud->num_urls = 0;
  wud->max_urls = 4;
  wud->urls = (char **)c_zalloc(sizeof(char *) * wud->max_urls);
//  checkAllocgLOK(wud->urls);
  wud->urls[0] = NULL;
  wud->curr_url = NULL;
wud->urls[0] = "/getaplist";
wud->urls[1] = "/getapdeflist";
wud->num_urls = 2;
  result = self->getWifiValue(self, WIFI_INFO_BINARY_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
//ets_printf("binaryCallback: %p!%d!\n", numericValue, result);
  wud->websocketBinaryReceived = (websocketBinaryReceived_t)numericValue;
  result = self->getWifiValue(self, WIFI_INFO_TEXT_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
//ets_printf("binaryCallback: %p!%d!\n", numericValue, result);
  wud->websocketTextReceived = (websocketTextReceived_t)numericValue;
  wud->compMsgDispatcher = self;

  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  wud->pesp_conn = pesp_conn;
//  checkAllocOK(pesp_conn);

  type = ESPCONN_TCP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = wud;

  pesp_conn->proto.tcp = NULL;
  pesp_conn->proto.udp = NULL;

  pesp_conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  if(!pesp_conn->proto.tcp){
    os_free(pesp_conn);
    pesp_conn = NULL;
//    checkErrOK(COMP_DISP_ERR_OUT_OF_MEMORY);
  }
  pesp_conn->proto.tcp->local_port = port;
//ets_printf("port: %d\n", port);

//ets_printf("call regist connectcb\n");
    result = espconn_regist_connectcb(pesp_conn, serverConnected);
    if (result != COMP_DISP_ERR_OK) {
//      return COMP_DISP_ERR_REGIST_CONNECT_CB;
    }
//ets_printf("regist connectcb result: %d\n", result);
    result = espconn_accept(pesp_conn);    // if it's a server, no need to dns.
    if (result != COMP_DISP_ERR_OK) {
//      return COMP_DISP_ERR_TCP_ACCEPT;
    }
//ets_printf("regist_accept result: %d\n", result);
    result =espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
    if (result != COMP_DISP_ERR_OK) {
//      return COMP_DISP_ERR_REGIST_TIME;
    }
//ets_printf("regist_time result: %d\n", result);
}

// ================================= initTimers ====================================

static uint8_t initTimers(compMsgDispatcher_t *self) {
  int result;

  for(int i = 0; i < NUM_TMR; i++) {
    compMsgTimers[i].self = self;
    compMsgTimers[i].mode = TIMER_MODE_OFF;
    ets_timer_disarm(&compMsgTimers[i].timer);
  }
  return COMP_DISP_ERR_OK;
}

// ================================= websocketRunAPMode ====================================

static uint8_t websocketRunAPMode(compMsgDispatcher_t *self) {
  int result;
  bool boolResult;
  uint8_t status;
  struct softap_config softap_config;
  int numericValue;
  uint8_t *stringValue;
  
//ets_printf("§websocketRunAPMode§\n");
  boolResult = wifi_station_disconnect();
  if (!boolResult) {
ets_printf("§websocketRunAPMode COMP_DISP_ERR_CANNOT_DISCONNECT§\n");
    return COMP_DISP_ERR_CANNOT_DISCONNECT;
  }
  boolResult = wifi_set_opmode(OPMODE_STATIONAP);
  if (!boolResult) {
ets_printf("§websocketRunAPMode COMP_DISP_ERR_CANNOT_SET_OPMODE§\n");
    return COMP_DISP_ERR_CANNOT_SET_OPMODE;
  }
  c_memset(softap_config.ssid,0,sizeof(softap_config.ssid));
  result = self->getWifiValue(self, WIFI_INFO_PROVISIONING_SSID, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
//ets_printf("§websocketRunAPMode get_PROVISIONING_SSID result: %d§\n", result);
  checkErrOK(result);
  c_memcpy(softap_config.ssid, stringValue, c_strlen(stringValue));
  softap_config.ssid_len = c_strlen(stringValue);
  softap_config.ssid_hidden = 0;
  softap_config.authmode = AUTH_OPEN;
  softap_config.channel = 6;
  softap_config.max_connection = 4;
  softap_config.beacon_interval = 100;
//ets_printf("§websocketRunAPMode wifi_softap_set_config§\n");
  boolResult = wifi_softap_set_config(&softap_config);
  if (!boolResult) {
    return COMP_DISP_ERR_CANNOT_SET_OPMODE;
  }
  status = wifi_station_get_connect_status();
ets_printf("wifi is in mode: %d status: %d ap_id: %d hostname: %s!\n", wifi_get_opmode(), status, wifi_station_get_current_ap_id(), wifi_station_get_hostname());
  switch (status) {
  case STATION_IDLE:
    break;
  case STATION_CONNECTING:
    break;
  case STATION_WRONG_PASSWORD:
    break;
  case STATION_NO_AP_FOUND:
    break;
  case STATION_CONNECT_FAIL:
    break;
  case STATION_GOT_IP:
    break;
  }

  int repeat = 1;
  int interval = 1000;
  int timerId = 0;
  int mode = TIMER_MODE_AUTO;
  compMsgTimer_t *tmr = &compMsgTimers[timerId];
  if (!(tmr->mode & TIMER_IDLE_FLAG) && (tmr->mode != TIMER_MODE_OFF)) {
    ets_timer_disarm(&tmr->timer);
  }
  // this is only preparing
//ets_printf("§websocketRunAPMode timer_setfcn§\n");
  ets_timer_setfn(&tmr->timer, alarmTimerAP, (void*)timerId);
  tmr->mode = mode | TIMER_IDLE_FLAG;
  // here is the start
  tmr->interval = interval;
  tmr->mode &= ~TIMER_IDLE_FLAG;
//ets_printf("§websocketRunAPMode timer_arm_new§\n");
  ets_timer_arm_new(&tmr->timer, interval, repeat, isMstimer);
  return COMP_DISP_ERR_OK;
}

// ================================= compMsgWebsocketInit ====================================

uint8_t compMsgWebsocketInit(compMsgDispatcher_t *self) {
  uint8_t result;

  result = initTimers(self);
  checkErrOK(result);

  self->websocketRunClientMode = &websocketRunClientMode;
  self->websocketRunAPMode = &websocketRunAPMode;
  self->websocketSendData = websocketSendData;
  return COMP_DISP_ERR_OK;
}
