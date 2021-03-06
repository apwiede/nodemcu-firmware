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
 * File:   structmsgIdentify.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 6st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "structmsgData.h"

#define RECEIVED_CHECK_HEADER_SIZE 7

#define DISP_FLAG_SHORT_CMD_KEY    (1 << 0)
#define DISP_FLAG_HAVE_CMD_LGTH    (1 << 1)
#define DISP_FLAG_IS_ENCRYPTED     (1 << 2)
#define DISP_FLAG_IS_TO_WIFI_MSG   (1 << 3)
#define DISP_FLAG_IS_FROM_MCU_MSG  (1 << 4)

#define RECEIVED_CHECK_TO_SIZE            2
#define RECEIVED_CHECK_FROM_SIZE          4
#define RECEIVED_CHECK_TOTAL_LGTH_SIZE    6
#define RECEIVED_CHECK_SHORT_CMD_KEY_SIZE 7
#define RECEIVED_CHECK_CMD_KEY_SIZE       8
#define RECEIVED_CHECK_CMD_LGTH_SIZE      10

typedef struct flag2Str {
  uint32_t flagVal;
  uint8_t *flagStr;
} flag2Str_t;

flag2Str_t flag2Strs [] = {
  { STRUCT_DISP_U16_DST,           "STRUCT_DISP_U16_DST" },
  { STRUCT_DISP_U16_SRC,           "STRUCT_DISP_U16_SRC" },
  { STRUCT_DISP_U8_TARGET,         "STRUCT_DISP_U8_TARGET" },
  { STRUCT_DISP_U16_TOTAL_LGTH,    "STRUCT_DISP_U16_TOTAL_LGTH" },
  { STRUCT_DISP_U8_EXTRA_KEY_LGTH, "STRUCT_DISP_U8_EXTRA_KEY_LGTH" },
  { STRUCT_DISP_IS_ENCRYPTED,      "STRUCT_DISP_IS_ENCRYPTED" },
  { STRUCT_DISP_IS_NOT_ENCRYPTED,  "STRUCT_DISP_IS_NOT_ENCRYPTED" },
  { STRUCT_DISP_SEND_TO_APP,       "STRUCT_DISP_SEND_TO_APP" },
  { STRUCT_DISP_RECEIVE_FROM_APP,  "STRUCT_DISP_RECEIVE_FROM_APP" },
  { STRUCT_DISP_SEND_TO_UART,      "STRUCT_DISP_SEND_TO_UART" },
  { STRUCT_DISP_RECEIVE_FROM_UART, "STRUCT_DISP_RECEIVE_FROM_UART" },
  { STRUCT_DISP_TRANSFER_TO_UART,  "STRUCT_DISP_TRANSFER_TO_UART" },
  { STRUCT_DISP_TRANSFER_TO_CONN,  "STRUCT_DISP_TRANSFER_TO_CONN" },
  { STRUCT_DISP_U8_CMD_KEY,        "STRUCT_DISP_U8_CMD_KEY" },
  { STRUCT_DISP_U16_CMD_KEY,       "STRUCT_DISP_U16_CMD_KEY" },
  { STRUCT_DISP_U0_CMD_LGTH,       "STRUCT_DISP_U0_CMD_LGTH" },
  { STRUCT_DISP_U8_CMD_LGTH,       "STRUCT_DISP_U8_CMD_LGTH" },
  { STRUCT_DISP_U16_CMD_LGTH,      "STRUCT_DISP_U16_CMD_LGTH" },
  { 0,                             NULL },
};

// ================================= getIntFromLine ====================================

static uint8_t getIntFromLine(uint8_t *myStr, long *ulgth, uint8_t **ep) {
  uint8_t *cp;
  char *endPtr;

  cp = myStr;
  while (*cp != ',') {
    cp++;
  }
  *cp++ = '\0';
  *ulgth = c_strtoul(myStr, &endPtr, 10);
  if (cp-1 != (uint8_t *)endPtr) {
     return STRUCT_MSG_ERR_BAD_VALUE;
  }
  *ep = cp;
  return STRUCT_DISP_ERR_OK;
}

// ================================= getStartFieldsFromLine ====================================

static uint8_t getStartFieldsFromLine(structmsgDataView_t *dataView, msgHeaderInfos_t *hdrInfos, uint8_t *myStr, uint8_t **ep, int *seqIdx) {
  int result;
  uint8_t *cp;
  uint8_t fieldNameId;

//ets_printf("numHeaderParts: %d seqidx: %d\n", hdrInfos->numHeaderParts, *seqIdx);
  cp = myStr;
  while (*cp != ',') {
    cp++;
  }
  *cp++ = '\0';
  if (myStr[0] != '@') {
    return STRUCT_DISP_ERR_FIELD_NOT_FOUND;
  }
  hdrInfos->headerStartLgth = 0;
  result = dataView->getFieldNameIdFromStr(dataView, myStr, &fieldNameId, STRUCT_MSG_NO_INCR);
  checkErrOK(result);
  switch (fieldNameId) {
  case STRUCT_MSG_SPEC_FIELD_SRC:
    hdrInfos->headerSequence[(*seqIdx)++] = STRUCT_DISP_U16_SRC;
    hdrInfos->headerFlags |= STRUCT_DISP_U16_SRC;
    hdrInfos->headerStartLgth += sizeof(uint16_t);
    break;
  case STRUCT_MSG_SPEC_FIELD_DST:
    hdrInfos->headerSequence[(*seqIdx)++] = STRUCT_DISP_U16_DST;
    hdrInfos->headerFlags |= STRUCT_DISP_U16_DST;
    hdrInfos->headerStartLgth += sizeof(uint16_t);
    break;
  case STRUCT_MSG_SPEC_FIELD_TARGET_CMD:
    hdrInfos->headerSequence[(*seqIdx)++] = STRUCT_DISP_U8_TARGET;
    hdrInfos->headerFlags |= STRUCT_DISP_U8_TARGET;
    hdrInfos->headerStartLgth += sizeof(uint8_t);
    break;
  default:
    checkErrOK(STRUCT_DISP_ERR_BAD_FIELD_NAME);
    break;
  }
  myStr = cp;
  while (*cp != ',') {
    cp++;
  }
  *cp++ = '\0';
  if (myStr[0] != '@') {
    return STRUCT_DISP_ERR_FIELD_NOT_FOUND;
  }
  result = dataView->getFieldNameIdFromStr(dataView, myStr, &fieldNameId, STRUCT_MSG_NO_INCR);
  checkErrOK(result);
  switch (fieldNameId) {
  case STRUCT_MSG_SPEC_FIELD_SRC:
    if (hdrInfos->headerFlags & STRUCT_DISP_U16_SRC) {
      return STRUCT_DISP_ERR_DUPLICATE_FIELD;
    }
    hdrInfos->headerSequence[(*seqIdx)++] = STRUCT_DISP_U16_SRC;
    hdrInfos->headerStartLgth += sizeof(uint16_t);
    break;
  case STRUCT_MSG_SPEC_FIELD_DST:
    if (hdrInfos->headerFlags & STRUCT_DISP_U16_DST) {
      return STRUCT_DISP_ERR_DUPLICATE_FIELD;
    }
    hdrInfos->headerStartLgth += sizeof(uint16_t);
    hdrInfos->headerSequence[(*seqIdx)++] = STRUCT_DISP_U16_DST;
    break;
  case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
    hdrInfos->headerStartLgth += sizeof(uint16_t);
    hdrInfos->headerSequence[(*seqIdx)++] = STRUCT_DISP_U16_TOTAL_LGTH;
    break;
  default:
    checkErrOK(STRUCT_DISP_ERR_BAD_FIELD_NAME);
    break;
  }
  myStr = cp;
  while ((*cp != '\n') && (*cp != '\0')) {
    cp++;
  }
  *cp++ = '\0';
  if (myStr[0] != '\0') {
    if (myStr[0] != '@') {
      return STRUCT_DISP_ERR_FIELD_NOT_FOUND;
    }
    result = dataView->getFieldNameIdFromStr(dataView, myStr, &fieldNameId, STRUCT_MSG_NO_INCR);
    checkErrOK(result);
    switch (fieldNameId) {
    case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
      if (hdrInfos->headerFlags & STRUCT_DISP_U16_TOTAL_LGTH) {
        return STRUCT_DISP_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerStartLgth += sizeof(uint16_t);
      hdrInfos->headerSequence[(*seqIdx)++] = STRUCT_DISP_U16_TOTAL_LGTH;
    default:
      checkErrOK(STRUCT_DISP_ERR_BAD_FIELD_NAME);
      break;
    }
  }
ets_printf("§headerStartLgth!%d§", hdrInfos->headerStartLgth);
  *ep = cp;
  return STRUCT_DISP_ERR_OK;
}
  
// ================================= getFlagStr ====================================

static uint8_t *getFlagStr(uint32_t flags) {
  flag2Str_t *entry;
  int idx;
  
  idx = 0;
  while (1) {
    entry = &flag2Strs[idx];
    if (entry->flagStr == NULL) {
      return "??";
    }
    if (flags & entry->flagVal) {
      return entry->flagStr;
    }
    idx++;
  }
}

// ================================= initHeadersAndFlags ====================================

static uint8_t initHeadersAndFlags(structmsgDispatcher_t *self) {
  uint8_t fileName[30];
  int result;
  uint8_t numEntries;
  uint8_t fieldNameId;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  long ulgth;
  uint8_t *myStr;
  int idx;
  int seqIdx = 0;
  int seqIdx2 = 0;
  uint8_t*cp;
  headerParts_t *hdr;
  structmsgDataView_t *dataView;

  self->dispFlags = 0;

  self->McuPart = 0x4D00;
  self->WifiPart = 0x5700;
  self->AppPart = 0x4100;
  self->CloudPart = 0x4300;
  return STRUCT_DISP_ERR_OK;
}

#undef checkErrOK
#define checkErrOK(result) if(result != DATA_VIEW_ERR_OK) { self->closeFile(self); return result; }
// ================================= readHeadersAndSetFlags ====================================

static uint8_t readHeadersAndSetFlags(structmsgDispatcher_t *self) {
  uint8_t fileName[30];
  int result;
  int result2;
  uint8_t numEntries;
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  char *endPtr;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  long ulgth;
  uint8_t *myStr;
  int idx;
  int headerEndIdx;
  int seqIdx;
  int seqIdx2;
  uint8_t*cp;
  uint8_t found;
  uint8_t isEnd;
  uint8_t fieldOffset;
  headerParts_t *hdr;
  msgHeaderInfos_t *hdrInfos;
  structmsgDataView_t *dataView;
  dataView_t *myDataView;

  dataView = self->structmsgDataView;
  hdrInfos = &self->msgHeaderInfos;
  hdrInfos->currPartIdx = 0;
  os_sprintf(fileName, "MsgHeads.txt");
  result = self->openFile(self, fileName, "r");
  checkErrOK(result);
  result = self->readLine(self, &buffer, &lgth);
  checkErrOK(result);
  if ((lgth < 4) || (buffer[0] != '#')) {
     return STRUCT_DISP_ERR_BAD_FILE_CONTENTS;
  }
  ulgth = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)ulgth;
  hdrInfos->headerParts = (headerParts_t *)os_zalloc(numEntries * (sizeof(headerParts_t)));
  checkAllocOK(self->msgHeaderInfos.headerParts);
  hdrInfos->numHeaderParts = 0;
  hdrInfos->maxHeaderParts = numEntries;
  hdrInfos->headerFlags = 0;
  // parse header start description
  result = self->readLine(self, &buffer, &lgth);
  checkErrOK(result);
  seqIdx = 0;
  buffer[lgth] = 0;
  myStr = buffer;
  result = getStartFieldsFromLine(dataView, hdrInfos, myStr, &cp, &seqIdx);
  myDataView = newDataViewHandle();
  checkAllocOK(myDataView);
  fieldOffset = 0;
  idx = 0;
  while(idx < numEntries) {
    result = self->readLine(self, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return STRUCT_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    hdr = &hdrInfos->headerParts[idx];
    if (idx == 0) {
      seqIdx2 = 0;
      while (seqIdx2 < seqIdx) {
        hdr->fieldSequence[seqIdx2] = hdrInfos->headerSequence[seqIdx2];
        seqIdx2++;
      }
    }
    hdr->hdrFlags = 0;
    myDataView->data = buffer;
    myDataView->lgth = lgth;
    buffer[lgth] = 0;
    myStr = buffer;
    seqIdx2 = 0;
    result = getIntFromLine(myStr, &ulgth, &cp);
    checkErrOK(result);
    found = 0;
    if (hdrInfos->headerSequence[seqIdx2] & STRUCT_DISP_U16_SRC) {
      hdr->hdrFromPart = (uint16_t)ulgth;
      found = 1;
    }
    if (hdrInfos->headerSequence[seqIdx2] & STRUCT_DISP_U16_DST) {
      hdr->hdrToPart = (uint16_t)ulgth;
      found = 1;
    }
    if (hdrInfos->headerSequence[seqIdx2] & STRUCT_DISP_U8_TARGET) {
      hdr->hdrTargetPart = (uint8_t)ulgth;
      found = 1;
    }
    if (!found) {
      checkErrOK(STRUCT_MSG_ERR_FIELD_NOT_FOUND);
    }
    seqIdx2++;
    myStr = cp;
    result = getIntFromLine(myStr, &ulgth, &cp);
    checkErrOK(result);
    found = 0;
    if (hdrInfos->headerSequence[seqIdx2] & STRUCT_DISP_U16_SRC) {
      hdr->hdrFromPart = (uint16_t)ulgth;
      found = 1;
    }
    if (hdrInfos->headerSequence[seqIdx2] & STRUCT_DISP_U16_DST) {
      hdr->hdrToPart = (uint16_t)ulgth;
      found = 1;
    }
    if (hdrInfos->headerSequence[seqIdx2] & STRUCT_DISP_U16_TOTAL_LGTH) {
      hdr->hdrTotalLgth = (uint16_t)ulgth;
      found = 1;
    }
    if (!found) {
      result = STRUCT_MSG_ERR_FIELD_NOT_FOUND;
      checkErrOK(result);
    }
    seqIdx2++;
    if (seqIdx > seqIdx2) {
      found = 0;
      myStr = cp;
      result = getIntFromLine(myStr, &ulgth, &cp);
      checkErrOK(result);
      if (hdrInfos->headerSequence[seqIdx2] & STRUCT_DISP_U16_TOTAL_LGTH) {
        hdr->hdrTotalLgth = (uint16_t)ulgth;
        found = 1;
      }
      if (!found) {
        result = STRUCT_MSG_ERR_FIELD_NOT_FOUND;
        checkErrOK(result);
      }
      seqIdx2++;
    }
    // extra field lgth 0/<number>
    myStr = cp;
    result = getIntFromLine(myStr, &ulgth, &cp);
    checkErrOK(result);
    hdr->hdrExtraLgth = (uint8_t)ulgth;
    hdr->fieldSequence[seqIdx2] = STRUCT_DISP_U8_EXTRA_KEY_LGTH;
    hdrInfos->headerSequence[seqIdx2++] = STRUCT_DISP_U8_EXTRA_KEY_LGTH;
    // encryption E/N
    myStr = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    hdr->fieldSequence[seqIdx2] = STRUCT_DISP_U8_ENCRYPTION;
    hdrInfos->headerSequence[seqIdx2++] = STRUCT_DISP_U8_ENCRYPTION;
    hdr->hdrEncryption = myStr[0];
    if (myStr[0] == 'E') {
//ets_printf("§idx!%d!0x%04x!0x%04x!0x%04x!enc!§", idx, hdr->hdrToPart, hdr->hdrFromPart, hdr->hdrTotalLgth);
      hdr->hdrFlags |= STRUCT_DISP_IS_ENCRYPTED;
    } else {
//ets_printf("§idx!%d!0x%04x!0x%04x!0x%04x!noenc!§", idx, hdr->hdrToPart, hdr->hdrFromPart, hdr->hdrTotalLgth);
      hdr->hdrFlags |= STRUCT_DISP_IS_NOT_ENCRYPTED;
    }
    // handleType A/G/S/R/U/W/N
    myStr = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    hdr->fieldSequence[seqIdx2] = STRUCT_DISP_U8_HANDLE_TYPE;
    hdrInfos->headerSequence[seqIdx2] = STRUCT_DISP_U8_HANDLE_TYPE;
    hdr->hdrHandleType = myStr[0];
    switch (myStr[0]) {
    case 'A':
      hdr->hdrFlags |= STRUCT_DISP_SEND_TO_APP;
      break;
    case 'G':
      hdr->hdrFlags |= STRUCT_DISP_RECEIVE_FROM_APP;
      break;
    case 'S':
      hdr->hdrFlags |= STRUCT_DISP_SEND_TO_UART;
      break;
    case 'R':
      hdr->hdrFlags |= STRUCT_DISP_RECEIVE_FROM_UART;
      break;
    case 'U':
      hdr->hdrFlags |= STRUCT_DISP_TRANSFER_TO_UART;
      break;
    case 'W':
      hdr->hdrFlags |= STRUCT_DISP_TRANSFER_TO_CONN;
      break;
    case 'N':
      hdr->hdrFlags |= STRUCT_DISP_NOT_RELEVANT;
      break;
    default:
      checkErrOK(STRUCT_DISP_ERR_BAD_VALUE);
      break;
    }
//ets_printf("§handleType!%s!§", getFlagStr(hdrInfos->headerSequence[seqIdx2]));
    seqIdx2++;
    // type of cmdKey
    myStr = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    result = dataView->dataView->getFieldTypeIdFromStr(dataView->dataView, myStr, &fieldTypeId);
//ets_printf("cmdKey type: %s %d result: %d, seqIdx2: %d\n", myStr, fieldTypeId, result, seqIdx2);
    checkErrOK(result);
    // cmdKey
    myStr = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_UINT8_T:
      hdr->fieldSequence[seqIdx2] = STRUCT_DISP_U8_CMD_KEY;
      hdrInfos->headerSequence[seqIdx2++] = STRUCT_DISP_U8_CMD_KEY;
      hdr->hdrFlags |= STRUCT_DISP_U8_CMD_KEY;
      hdr->hdrU8CmdKey = myStr[0];
//ets_printf("§u8CmdKey!0x%02x!§", hdr->hdrU8CmdKey);
      self->dispFlags |= STRUCT_MSG_U8_CMD_KEY;
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      hdr->fieldSequence[seqIdx2] = STRUCT_DISP_U16_CMD_KEY;
      hdrInfos->headerSequence[seqIdx2++] = STRUCT_DISP_U16_CMD_KEY;
      hdr->hdrFlags |= STRUCT_DISP_U16_CMD_KEY;
      hdr->hdrU16CmdKey = (myStr[0]<<8)|myStr[1];
//ets_printf("§u16CmdKey!0x%04x!§\n", hdr->hdrU16CmdKey);
      self->dispFlags &= ~STRUCT_MSG_U8_CMD_KEY;
      break;
    default:
      checkErrOK(STRUCT_DISP_ERR_BAD_FIELD_TYPE);
    }
    // type of cmdLgth
    myStr = cp;
    while ((*cp != ',') && (*cp != '\n') && (*cp != '\r') && (*cp != '\0')) {
      cp++;
    }
    isEnd = 0;
    if ((*cp == '\n') || (*cp == '\r')) {
      isEnd = 1;
    }
    *cp++ = '\0';
    result = dataView->dataView->getFieldTypeIdFromStr(dataView->dataView, myStr, &fieldTypeId);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_NONE:
      hdr->fieldSequence[seqIdx2] = STRUCT_DISP_U0_CMD_LGTH;
      hdrInfos->headerSequence[seqIdx2++] = STRUCT_DISP_U0_CMD_LGTH;
      hdr->hdrFlags |= STRUCT_DISP_U0_CMD_LGTH;
      break;
    case DATA_VIEW_FIELD_UINT8_T:
      hdr->fieldSequence[seqIdx2] = STRUCT_DISP_U8_CMD_LGTH;
      hdrInfos->headerSequence[seqIdx2++] = STRUCT_DISP_U8_CMD_LGTH;
      hdr->hdrFlags |= STRUCT_DISP_U8_CMD_LGTH;
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      hdr->fieldSequence[seqIdx2] = STRUCT_DISP_U16_CMD_LGTH;
      hdrInfos->headerSequence[seqIdx2++] = STRUCT_DISP_U16_CMD_LGTH;
      hdr->hdrFlags |= STRUCT_DISP_U16_CMD_LGTH;
      break;
    default:
      checkErrOK(STRUCT_DISP_ERR_BAD_FIELD_TYPE);
    }
    // type of crc
    if (!isEnd) {
      myStr = cp;
      while ((*cp != ',') && (*cp != '\n') && (*cp != '\n') && (*cp != '\0')) {
        cp++;
      }
      *cp++ = '\0';
    }
    result = dataView->dataView->getFieldTypeIdFromStr(dataView->dataView, myStr, &fieldTypeId);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_NONE:
      hdr->fieldSequence[seqIdx2] = STRUCT_DISP_U0_CRC;
      hdrInfos->headerSequence[seqIdx2++] = STRUCT_DISP_U0_CRC;
      hdr->hdrFlags |= STRUCT_DISP_U0_CRC;
      break;
    case DATA_VIEW_FIELD_UINT8_T:
      hdr->fieldSequence[seqIdx2] = STRUCT_DISP_U8_CRC;
      hdrInfos->headerSequence[seqIdx2++] = STRUCT_DISP_U8_CRC;
      hdr->hdrFlags |= STRUCT_DISP_U8_CRC;
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      hdr->fieldSequence[seqIdx2] = STRUCT_DISP_U16_CRC;
      hdrInfos->headerSequence[seqIdx2++] = STRUCT_DISP_U16_CRC;
      hdr->hdrFlags |= STRUCT_DISP_U16_CRC;
      break;
    default:
      checkErrOK(STRUCT_DISP_ERR_BAD_FIELD_TYPE);
//ets_printf("§flag idx!%d!%s!§", flagIdx, getFlagStr(hdrInfos->headerSequence[flagIdx]));
    }
    hdrInfos->numHeaderParts++;
    idx++;
  }
  os_free(myDataView);
  result2 = self->closeFile(self);
  checkErrOK(result2);
  return result;
}
#undef checkErrOK
#define checkErrOK(result) if(result != DATA_VIEW_ERR_OK) return result

// ================================= nextFittingEntry ====================================

static uint8_t nextFittingEntry(structmsgDispatcher_t *self, uint8_t u8CmdKey, uint16_t u16CmdKey) {
  msgParts_t *received;
  msgHeaderInfos_t *hdrInfos;
  dataView_t *dataView;
  headerParts_t *hdr;
  int hdrIdx;
  int found;

  dataView = self->structmsgDataView->dataView;
  received = &self->received;
  hdrInfos = &self->msgHeaderInfos;
  hdrIdx = hdrInfos->currPartIdx;
  hdr = &hdrInfos->headerParts[hdrIdx];
//ets_printf("§HEAD:!0x%04x!0x%04x!0x%04x!nH!%d!seqIdx!%d!§", received->toPart, received->fromPart, received->totalLgth, hdrInfos->numHeaderParts, hdrInfos->seqIdx);
  // and now search in the headers to find the appropriate message
  hdrInfos->seqIdx = hdrInfos->seqIdxAfterStart;
  found = 0;
  while (hdrIdx < hdrInfos->numHeaderParts) {
    hdr = &hdrInfos->headerParts[hdrIdx];
    if (hdr->hdrToPart == received->toPart) {
      if (hdr->hdrFromPart == received->fromPart) {
        if ((hdr->hdrTotalLgth == received->totalLgth) || (hdr->hdrTotalLgth == 0)) {
          if (u8CmdKey != 0) {
            if (u8CmdKey == received->u8CmdKey) {
              found = 1;
              break;
            }
          } else {
            if (u16CmdKey != 0) {
              if (u16CmdKey == received->u16CmdKey) {
                found = 1;
                break;
              }
            } else {
              found = 1;
              break;
            }
          }
        }
      }
    }
    hdrIdx++;
  }
  if (!found) {
    return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
  }
  hdrInfos->currPartIdx = hdrIdx;
  // next sequence field is extraLgth (skip, we have it in hdr fields)
  hdrInfos->seqIdx++;
  // next sequence field is encryption (skip, we have it in hdr fields)
  hdrInfos->seqIdx++;
  // next sequence field is handle type (skip, we have it in hdr fields)
  hdrInfos->seqIdx++;
  if (hdr->hdrEncryption == 'N') {
    received->partsFlags |= STRUCT_DISP_IS_NOT_ENCRYPTED;
    // skip extraLgth, encrypted and handle Type
  } else {
    received->partsFlags |= STRUCT_DISP_IS_ENCRYPTED;
  }
//ets_printf("§found!%d!hdrIdx!%d§\n", found, hdrIdx);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getHeaderIndexFromHeaderFields ====================================

static uint8_t getHeaderIndexFromHeaderFields(structmsgDispatcher_t *self, msgParts_t *received, msgHeaderInfos_t *hdrInfos) {
  int result;
  dataView_t *dataView;
  headerParts_t *hdr;
  int hdrIdx;
  int found;
  uint8_t myHeaderLgth;

  dataView = self->structmsgDataView->dataView;
  received->fieldOffset = 0;
  myHeaderLgth = 0;
  hdrInfos->seqIdx = 0;
  switch(hdrInfos->headerSequence[hdrInfos->seqIdx]) {
  case STRUCT_DISP_U16_DST:
    result = dataView->getUint16(dataView, received->fieldOffset, &received->toPart);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint16_t);
    break;
  case STRUCT_DISP_U16_SRC:
    result = dataView->getUint16(dataView, received->fieldOffset, &received->fromPart);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint16_t);
    break;
  case STRUCT_DISP_U8_TARGET:
    result = dataView->getUint8(dataView, received->fieldOffset, &received->targetPart);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint8_t);
    break;
  }
  hdrInfos->seqIdx++;
  switch(hdrInfos->headerSequence[hdrInfos->seqIdx]) {
  case STRUCT_DISP_U16_DST:
    result = dataView->getUint16(dataView, received->fieldOffset, &received->toPart);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint16_t);
    break;
  case STRUCT_DISP_U16_SRC:
    result = dataView->getUint16(dataView, received->fieldOffset, &received->fromPart);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint16_t);
    break;
  case STRUCT_DISP_U16_TOTAL_LGTH:
    result = dataView->getUint16(dataView, received->fieldOffset, &received->totalLgth);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint16_t);
    break;
  }
  if (received->fieldOffset < hdrInfos->headerStartLgth) {
    hdrInfos->seqIdx++;
    switch(hdrInfos->headerSequence[hdrInfos->seqIdx]) {
    case STRUCT_DISP_U16_TOTAL_LGTH:
      result = dataView->getUint16(dataView, received->fieldOffset, &received->totalLgth);
      checkErrOK(result);
      received->fieldOffset += sizeof(uint16_t);
      break;
    }
  }
  hdrInfos->seqIdx++;
  hdrInfos->seqIdxAfterStart = hdrInfos->seqIdx;
  hdrInfos->currPartIdx = 0;
  result = nextFittingEntry(self, 0, 0);
//ets_printf("§IndexFromHeaderFields!%d!%d!§\n", result, hdrInfos->currPartIdx);
  return result;
}

// ================================= handleReceivedPart ====================================

static uint8_t handleReceivedPart(structmsgDispatcher_t *self, const uint8_t * buffer, uint8_t lgth) {
  int idx;
  msgParts_t *received;
  dataView_t *dataView;
  msgHeaderInfos_t *hdrInfos;
  headerParts_t *hdr;
  int startIdx;
  int hdrIdx;
  uint8_t u8;
  structmsgField_t fieldInfo;
  int result;

  hdrInfos = &self->msgHeaderInfos;
  received = &self->received;
  dataView = self->structmsgDataView->dataView;
  dataView->data = received->buf;
//ets_printf("§receivedLgth: %d fieldOffset: %d!\n§", received->lgth, received->fieldOffset);
  idx = 0;
  while (idx < lgth) {
    received->buf[received->lgth++] = buffer[idx];
    received->realLgth++;
    dataView->lgth++;
    if (received->lgth == hdrInfos->headerStartLgth) {
      result = getHeaderIndexFromHeaderFields(self, received, hdrInfos);
//ets_printf("getHeaderIndexFromHeaderFields result: %d currPartIdx: %d\n", result, hdrInfos->currPartIdx);
    }
    if (received->lgth > hdrInfos->headerStartLgth) {
      hdrIdx = hdrInfos->currPartIdx;
      hdr = &hdrInfos->headerParts[hdrIdx];
      if (received->partsFlags & STRUCT_DISP_IS_NOT_ENCRYPTED) {
ets_printf("STRUCT_DISP_IS_NOT_ENCRYPTED\n");
        if (hdr->hdrEncryption == 'N') {
          result = self->handleNotEncryptedPart(self, received, hdrInfos);
          checkErrOK(result);
        } else {
ets_printf("partsFlags is not encrypted and hdrEncryption is E\n");
        }
      } else {
        if (hdr->hdrEncryption == 'E') {
          if (received->lgth == received->totalLgth) {
            uint8_t *cryptedPtr;
            uint8_t *cryptKey;
            uint8_t *decrypted;;
            uint8_t mlen;
            uint8_t klen;
            uint8_t ivlen;
            int decryptedLgth;
              // decrypt encrypted message part (after header)
cryptKey = "a1b2c3d4e5f6g7h8";
              mlen = received->totalLgth - hdrInfos->headerStartLgth;
              ivlen = 16;
              klen = 16;
              cryptedPtr = received->buf + hdrInfos->headerStartLgth;
              result = self->decryptMsg(cryptedPtr, mlen, cryptKey, klen, cryptKey, ivlen, &decrypted, &decryptedLgth);
              checkErrOK(result);
              c_memcpy(cryptedPtr, decrypted, decryptedLgth);

              // set received->lgth to end of header for correct working of handleEncryptedPart!!
              received->lgth = hdrInfos->headerStartLgth;
              result = self->handleEncryptedPart(self, received, hdrInfos);
ets_printf("handleEncryptedPart end idx: %d result: %d\n", idx, result);
              checkErrOK(result);
          }
        } else {
ets_printf("partsFlags is encrypted and hdrEncryption is N\n");
        }
      }
    }
    idx++;
  }
  return STRUCT_DISP_ERR_OK;
}

// ================================= structmsgIdentifyInit ====================================

uint8_t structmsgIdentifyInit(structmsgDispatcher_t *self) {
  uint8_t result;

  self->readHeadersAndSetFlags = &readHeadersAndSetFlags;
  self->handleReceivedPart = &handleReceivedPart;
  self->nextFittingEntry = &nextFittingEntry;
  initHeadersAndFlags(self);
  result=self->readHeadersAndSetFlags(self);
  checkErrOK(result);
  return STRUCT_DISP_ERR_OK;
}
