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
 * File:   compMsgMsgDesc.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 1st, 2016
 */

/* composite message data descriptions handling */

/* ==========================================================================
 * Layout of message description files:
 *   #< ><text>                           comment line (possible every where)
 *   #,<number lines>[,<action callback>] number of lines in this file
 *                                        eventually a callback for an action to call
 *                                        before building this message
 *   #use,<file name>                     use this file name (relative path)
 *   #include,<file name>                 include this file name (relative path)
 *   @<special field name>,<type>,<lgth>  special fields
 *   @#<key/value field name>,<type>,<lgth>,<numValues>[,@<field size callback>]
 *                                        (see key/value fields description)
 *   <field name>,<type>,<lgth>           normal field
 *
 * Example:
 *
 * Example MsgDescUse.txt
 *   # this is an example message use description file
 *   #,3
 *   #use,MsgDescHeader.txt
 *   #use,MsgDescMidPart.txt
 *   #use,MsgDescTrailer.txt
 *
 * Example MsgDescA1.txt
 *   # this is an example message description file
 *   #,7,@getAPList
 *   #include,MsgDescHeader.txt
 *   #include,MsgDescMidPart.txt
 *   @numKeyValues,unit8_t,1
 *   @#sequenceNum,uint32_t,4,0,@getSequenceNumSize
 *   @#ssid,uint8_t*,0,0,@getWifiAPSssidSize
 *   @#channel,uint8_t*,0,0,@getChannelSize
 *   #include,MsgDescTrailer.txt
 *
 * Example MsgDescHeader.txt
 *   # this is an example message header file
 *   @src,uint8_t,1
 *   @grp,uint8_t,1
 *   @dst,uint8_t,1
 *   @ipAddr,uint32_t,4
 *   @totalLgth,uint16_t,2
 *   @hdrFiller,uint8_t*,7
 *
 * Example MsgDescMidPart.txt
 *   # this is an example message mid part file
 *   @cmdKey,uint16_t,2
 *   @cmdLgth,uint16_t,2
 *
 * Example MsgDescTrailer.txt
 *   # this is an example message trailer file
 *   @filler,uint8_t*,0
 *   @crc,uint16_t,2
 *   @totalCrc,uint16_t,2
 *
 * ==========================================================================
 */

/* ==========================================================================
 * Layout of message value files:
 *   #< ><text>                           comment line (possible every where)
 *   #,<number lines>[,<action callback>] number of lines in this file
 *                                        eventually a callback for an action to call
 *                                        before building this message
 *   #use,<file name>                     use this file name (relative path)
 *   #include,<file name>                 include this file name (relative path)
 *   @<special field name>,<valueType>,<value/callback>
 *                                        special fields
 *   @#<key/value field name>,<valueType>,<value/callback>
 *                                        (see key/value fields description)
 *   <field name>,<valueType>,<value/callback>
 *                                        normal field
 *
 * Example MsgValA1.txt
 *   # this is an example message value file
 *   #,5
 *   #include,MsgValHeader.txt
 *   @numKeyValues,I,3
 *   @#sequenceNum,I,@getClientSequenceNum
 *   @#ssid,S,@getWifiAPSsids
 *   @#channel,I,@getWifiAPChannels
 *
 * Example MsgValHeader.txt
 *   # this is an example message value header file
 *   #,5
 *   @src,I,12
 *   @grp,I,2
 *   @dst,I,15
 *   @srcId,I,@getSrcId
 *   @hdrReserve,S,@getHdrReserve
 *
 * ==========================================================================
 */

#ifndef COMP_MSG_MSG_DESC_H
#define	COMP_MSG_MSG_DESC_H

#define DISP_BUF_LGTH 1024

// handle types
// A/G/R/S/W/U/N
#define COMP_DISP_SEND_TO_APP       'A'
#define COMP_DISP_RECEIVE_FROM_APP  'G'
#define COMP_DISP_SEND_TO_UART      'R'
#define COMP_DISP_RECEIVE_FROM_UART 'S'
#define COMP_DISP_TRANSFER_TO_UART  'W'
#define COMP_DISP_TRANSFER_TO_CONN  'U'
#define COMP_DISP_NOT_RELEVANT      'N'

// encryption
enum compMsgEncyptedCode
{
  COMP_DISP_IS_NOT_ENCRYPTED  = 0,
  COMP_DISP_IS_ENCRYPTED      = 1,
  COMP_DISP_U8_ENCRYPTION     = 2,
  COMP_DISP_U8_HANDLE_TYPE    = 4,
};

#define COMP_MSG_HDR_DST              0x01
#define COMP_MSG_HDR_SRC              0x02
#define COMP_MSG_HDR_GROUP            0x04
#define COMP_MSG_HDR_TOTAL_LGTH       0x08
#define COMP_MSG_HDR_GUID             0x10
#define COMP_MSG_HDR_IP_ADDR          0x20
#define COMP_MSG_HDR_FILLER           0x40
#define COMP_MSG_PAYLOAD_CMD_KEY      0x80
#define COMP_MSG_PAYLOAD_CMD_LGTH     0x100
#define COMP_MSG_PAYLOAD_CRC          0x200
#define COMP_MSG_TOTAL_CRC            0x400

// the next value must be equal to the number of defines above!!
#define COMP_MSG_MAX_SEQUENCE         11

#define COMP_MSG_U8_DST               1
#define COMP_MSG_U16_DST              2
#define COMP_MSG_U8_SRC               3
#define COMP_MSG_U16_SRC              4
#define COMP_MSG_U8_GROUP             5
#define COMP_MSG_U16_GROUP            6
#define COMP_MSG_U8_TOTAL_LGTH        7
#define COMP_MSG_U16_TOTAL_LGTH       8
#define COMP_MSG_IP_ADDR              9
#define COMP_MSG_VECTOR_GUID          10
#define COMP_MSG_VECTOR_HDR_FILLER    12
//#define COMP_MSG_U8_CMD_KEY           13
#define COMP_MSG_U16_CMD_KEY          14
#define COMP_MSG_U0_CMD_LGTH          15
#define COMP_MSG_U8_CMD_LGTH          16
#define COMP_MSG_U16_CMD_LGTH         17
#define COMP_MSG_U0_CRC               18
#define COMP_MSG_U8_CRC               19
#define COMP_MSG_U16_CRC              20
#define COMP_MSG_U0_TOTAL_CRC         21
#define COMP_MSG_U8_TOTAL_CRC         22
#define COMP_MSG_U16_TOTAL_CRC        23

#define COMP_MSG_DESC_VALUE_IS_NUMBER (1 << 0)

#define COMP_MSG_VAL_IS_JOKER          0x01

#define MSG_GUID_LGTH 16
#define MSG_MAX_HDR_FILLER_LGTH 16
#define MSG_MAX_LINE_FIELDS 10

#define MSG_FILES_FILE_NAME           "MsgFiles.txt"

typedef struct msgFieldDesc msgFieldDesc_t;
typedef struct msgFieldVal msgFieldVal_t;

typedef uint8_t (* msgFieldSizeCallback_t)(compMsgDispatcher_t *self, msgFieldDesc_t *msgFieldDesc);
typedef uint8_t (* msgFieldValueCallback_t)(compMsgDispatcher_t *self, msgFieldVal_t *msgFieldVal);

typedef struct msgHeaderValues {
  uint8_t u8Src;
  uint16_t u16Src;
  uint8_t u8Dst;
  uint16_t u16Dst;
  uint8_t u8Group;
  uint16_t u16Group;
  uint32_t ipAddr;
  uint8_t u8TotalLgth;
  uint16_t u16TotalLgth;
  uint8_t hdrFiller[MSG_MAX_HDR_FILLER_LGTH + 1];
} msgHeaderValues_t;

typedef struct msgMidPartValues {
  uint8_t u8CmdKey;
  uint16_t u16CmdKey;
  uint8_t u8CmdLgth;
  uint16_t u16CmdLgth;
} msgMidPartValues_t;

typedef struct msgTrailerValues {
  uint8_t u8Crc;
  uint16_t u16Crc;
  uint8_t u8TotalCrc;
  uint16_t u16TotalCrc;
} msgTrailerValues_t;

// infos about the fields sequence within a mesgDescription

typedef struct msgFieldSequence {
  uint16_t field;
  uint16_t fieldType;
  uint8_t isJoker;
} msgFieldSequence_t;

// infos about a message with the message description infos

typedef struct msgDescription {
  msgHeaderValues_t msgHeaderValues;
  uint16_t headerLgth;
  uint8_t encrypted;
  uint8_t handleType;
  msgMidPartValues_t msgMidPartValues;
  msgFieldSequence_t fieldSequence[COMP_MSG_MAX_SEQUENCE];
} msgDescription_t;

// infos about all possible messages used here

typedef struct msgDescriptionInfos {
  uint8_t numMsgDescriptions;
  uint8_t maxMsgDescriptions;
  msgDescription_t *msgDescriptions;
  uint8_t currMsgDescriptionIdx;
  uint8_t currSequenceIdx;
  uint8_t sequenceIdxAfterHeader;
} msgDescriptionInfos_t;

// infos about a key/value field

typedef struct msgKeyValueDesc {
  uint16_t keyId;
  uint8_t keyAccess;
  uint8_t keyType;
  uint16_t keyLgth;
  uint16_t keyNumValues;
} msgKeyValueDesc_t;

// all infos about a message field value

typedef struct msgFieldVal {
  union {
    uint8_t u8;
    int8_t i8;
    uint16_t u16;
    int16_t i16;
    uint32_t u32;
    int32_t i32;
    uint8_t *u8vec;
    int8_t *i8vec;
    uint16_t *u16vec;
    int16_t *i16vec;
    uint32_t *u32vec;
    int32_t *i32vec;
    int intVal;
  } value;
  uint8_t fieldValueType;
  msgFieldValueCallback_t fieldValueCallback;
} msgFieldVal_t;

// all infos about a message field

typedef struct msgFieldDesc {
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  uint16_t fieldLgth;
  uint16_t fieldFlags;
  msgKeyValueDesc_t *msgKeyValueDesc;
  msgFieldSizeCallback_t fieldSizeCallback;
  msgFieldVal_t msgFieldVal;
} msgFieldDesc_t;

// all infos about a message use/include file

typedef struct msgDescIncludeInfo {
  uint16_t includeType;
  char *fileName;
  uint8_t maxMsgFieldDesc;
  uint8_t numMsgFieldDesc;
  msgFieldDesc_t *msgFieldDescs;
  uint8_t maxMsgFieldVal;
  uint8_t numMsgFieldVal;
  msgFieldVal_t *msgFieldVals;
} msgDescIncludeInfo_t;







typedef struct msgInfo {
  msgHeaderValues_t msgHeaderValues;
  msgMidPartValues_t msgMidPartValues;
  msgTrailerValues_t msgTrailerValues;
  size_t lgth;
  uint8_t realLgth;
  size_t fieldOffset;
  uint8_t buf[DISP_BUF_LGTH];
  compMsgDataView_t *compMsgDataView;
} msgInfo_t;

typedef struct keyValueDesc {
  uint16_t keyId;
  uint8_t keyAccess;
  uint8_t keyType;
  uint16_t keyLgth;
  uint16_t keyNumValues;
} keyValueDesc_t;

typedef struct fieldDesc {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldTypeStr;
  uint8_t fieldTypeId;
  uint16_t fieldLgth;
  uint16_t fieldSize;
  keyValueDesc_t *keyValueDesc;
  fieldSizeCallback_t fieldSizeCallback;
} fieldDesc_t;

#define COMP_DISP_HDR_DST              0x01
#define COMP_DISP_HDR_SRC              0x02
#define COMP_DISP_HDR_TOTAL_LGTH       0x04
#define COMP_DISP_HDR_GUID             0x08
#define COMP_DISP_HDR_SRC_ID           0x10
#define COMP_DISP_HDR_RESERVE          0x20
#define COMP_DISP_HDR_FILLER           0x40
#define COMP_DISP_PAYLOAD_CMD_KEY      0x80
#define COMP_DISP_PAYLOAD_CMD_LGTH     0x100
#define COMP_DISP_PAYLOAD_CRC          0x200
#define COMP_DISP_TOTAL_CRC            0x400

#define COMP_DISP_U16_DST              0x01
#define COMP_DISP_U16_SRC              0x02
#define COMP_DISP_U16_TOTAL_LGTH       0x04
#define COMP_DISP_U8_VECTOR_GUID       0x08
#define COMP_DISP_U16_SRC_ID           0x10
#define COMP_DISP_HDR_RESERVE          0x20
#define COMP_DISP_U8_VECTOR_HDR_FILLER 0x40
#define COMP_DISP_U16_CMD_KEY          0x80
#define COMP_DISP_U0_CMD_LGTH          0x100
#define COMP_DISP_U8_CMD_LGTH          0x200
#define COMP_DISP_U16_CMD_LGTH         0x400
#define COMP_DISP_U0_CRC               0x800
#define COMP_DISP_U8_CRC               0x1000
#define COMP_DISP_U16_CRC              0x2000
#define COMP_DISP_U0_TOTAL_CRC         0x4000
#define COMP_DISP_U8_TOTAL_CRC         0x8000
#define COMP_DISP_U16_TOTAL_CRC        0x10000

// the next value must equal the number of defines above!!
#define COMP_DISP_MAX_SEQUENCE     17

#define COMP_DISP_DESC_VALUE_IS_NUMBER (1 << 0)

#define DISP_GUID_LGTH 16
#define DISP_MAX_HDR_FILLER_LGTH 40
typedef struct headerPart {
  uint16_t hdrFromPart;
  uint16_t hdrToPart;
  uint16_t hdrTotalLgth;
  uint8_t hdrGUID[DISP_GUID_LGTH+1];
  uint16_t hdrSrcId;
  uint8_t hdrReserve[3];
  uint8_t hdrFiller[DISP_MAX_HDR_FILLER_LGTH+1];
  uint16_t hdrU16CmdKey;
  uint16_t hdrU16CmdLgth;
  uint16_t hdrU16Crc;
  uint16_t hdrU16TotalCrc;
  uint8_t hdrTargetPart;
  uint8_t hdrU8CmdKey;
  uint8_t hdrU8CmdLgth;
  uint8_t hdrU8Crc;
  uint8_t hdrU8TotalCrc;
  uint8_t hdrOffset;
  uint8_t hdrEncryption;
  uint8_t hdrHandleType;
  uint8_t hdrLgth;
  uint32_t hdrFlags;
  uint32_t fieldSequence[COMP_DISP_MAX_SEQUENCE];
} headerPart_t;

typedef struct msgHeaderInfos {
  uint32_t headerFlags;        // these are the flags for the 2nd line in the heads file!!
  uint16_t headerSequence[COMP_DISP_MAX_SEQUENCE];  // this is the sequence of the 2nd line in the heads file!!
  uint8_t headerLgth;
  size_t lgth;
  headerPart_t *headerParts;
  uint8_t numHeaderParts;
  uint8_t maxHeaderParts;
  uint8_t currPartIdx;
  uint8_t seqIdx;
  uint8_t seqIdxAfterHeader;
} msgHeaderInfos_t;

typedef struct msgParts {
  uint16_t fromPart;
  uint16_t toPart;
  uint16_t totalLgth;
  uint8_t GUID[DISP_GUID_LGTH];
  uint32_t srcId;
  uint16_t hdrReserve;
  uint8_t hdrFiller[DISP_MAX_HDR_FILLER_LGTH];
  uint16_t partsFlags;
  uint16_t u16CmdKey;
  uint8_t u8CmdLgth;
  uint16_t u16CmdLgth;
  size_t lgth;
  uint8_t encryption;
  uint8_t realLgth;
  size_t fieldOffset;
  uint8_t buf[DISP_BUF_LGTH];
  compMsgDataView_t *compMsgDataView;
} msgParts_t;


typedef struct msgDescPart {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldTypeStr;
  uint8_t fieldTypeId;
  uint8_t fieldLgth;
  uint8_t fieldType;
  uint8_t fieldRW;
  uint16_t fieldKey;
  uint16_t fieldSize;
  uint16_t fieldNumValues;
  fieldValueCallback_t fieldSizeCallback;
  uint8_t fieldValueCallbackType;
  uint8_t *fieldValueActionCb; // the name of a callback to run an action
} msgDescPart_t;

typedef struct msgValPart {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldValueStr;    // the value or the callback for getting the value
  uint8_t *fieldKeyValueStr; // the value for a string
  uint32_t fieldValue;       // the value for an integer
  uint8_t fieldFlags;
  uint8_t fieldValueCallbackType;
  fieldValueCallback_t fieldValueCallback;
} msgValPart_t;

typedef struct msgKeyValueDescPart {
  uint8_t *keyNameStr;
  uint16_t keyId;
  uint8_t keyType;
  uint8_t keyLgth;
} msgKeyValueDescPart_t;

typedef struct fieldsToSave {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldValueStr;    // the value
  uint32_t fieldValue;       // the value for an integer
} fieldsToSave_t;

typedef struct compMsgMsgDesc compMsgMsgDesc_t;
typedef struct compMsgData compMsgData_t;
typedef struct compMsgWifiData compMsgWifiData_t;

typedef uint8_t (* openFile_t)(compMsgMsgDesc_t *self, const uint8_t *fileName, const uint8_t *fileMode);
typedef uint8_t (* closeFile_t)(compMsgMsgDesc_t *self);
typedef uint8_t (* flushFile_t)(compMsgMsgDesc_t *self);
typedef uint8_t (* readLine_t)(compMsgMsgDesc_t *self, uint8_t **buffer, uint8_t *lgth);
typedef uint8_t (* writeLine_t)(compMsgMsgDesc_t *self, const uint8_t *buffer, uint8_t lgth);
typedef uint8_t (* getLineFields_t)(compMsgDispatcher_t *self, uint8_t *myStr, uint8_t lgth);

typedef uint8_t (* getIntFieldValue_t)(compMsgDispatcher_t *self, uint8_t *cp, char **ep, int base, int *uval);
typedef uint8_t (* getStringFieldValue_t)(compMsgDispatcher_t *self, uint8_t *cp, uint8_t **strVal);
typedef uint8_t (* addUseFileName_t)(compMsgDispatcher_t *self, char *fileName);
typedef uint8_t (* handleMsgFileNameLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgHeadsLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgUseLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgCommonLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgActionsLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgValuesLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgValHeaderLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgFieldsToSaveLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgFile_t)(compMsgDispatcher_t *self, uint8_t *fileName, handleMsgLine_t handleMsgLine);

#ifdef OLD
typedef uint8_t (* getIntFromLine_t)(compMsgDispatcher_t *self, uint8_t *myStr, long *ulgth, uint8_t **ep, bool *isEnd);
typedef uint8_t (* getStrFromLine_t)(compMsgDispatcher_t *self, uint8_t *myStr, uint8_t **ep, bool *isEnd);
typedef uint8_t (* getHeaderFieldsFromLine_t)(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos, uint8_t *myStr, uint8_t **ep, int *seqIdx);
typedef uint8_t (*readActions_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (*readModuleDataValues_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (*readWifiValues_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (* readHeadersAndSetFlags_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (* getHeaderFromUniqueFields_t)(compMsgDispatcher_t *self, uint16_t dst, uint16_t src, uint16_t cmdKey, headerPart_t **hdr);
typedef uint8_t (* getMsgPartsFromHeaderPart_t)(compMsgDispatcher_t *self, headerPart_t *hdr, uint8_t **handle);
typedef uint8_t (* getMsgKeyValueDescParts_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (* getFieldsToSave_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (* getWifiKeyValueKeys_t)(compMsgDispatcher_t *self, uint8_t *fileName);
#endif

typedef uint8_t (* compMsgMsgDescInit_t)(compMsgDispatcher_t *self);
typedef void (* freeCompMsgMsgDesc_t)(compMsgMsgDesc_t *compMsgMsgDesc);

typedef struct compMsgMsgDesc {
  const uint8_t *fileName;
  uint8_t fileId;
  size_t fileSize;
  uint8_t *lineFields[MSG_MAX_LINE_FIELDS];
  int expectedLines;
  int numLineFields;
  msgDescriptionInfos_t msgDescriptionInfos;
  int numMsgDescIncludeInfo;
  int maxMsgDescIncludeInfo;
  int currMsgDescIncludeInfo;
  msgDescIncludeInfo_t *msgDescIncludeInfos;
  uint8_t *msgUseFileName;
  uint8_t *msgHeadsFileName;
  uint8_t *msgFieldsToSaveFileName;
  uint8_t *msgActionsFileName;
  uint8_t *msgDescHeaderFileName;
  uint8_t *msgDescMidPartFileName;
  uint8_t *msgDescTrailerFileName;

  openFile_t openFile;
  closeFile_t closeFile;
  flushFile_t flushFile;
  readLine_t readLine;
  writeLine_t writeLine;
  getLineFields_t getLineFields;

  addUseFileName_t addUseFileName;
  getIntFieldValue_t getIntFieldValue;
  getStringFieldValue_t getStringFieldValue;
  handleMsgFile_t handleMsgFile;
  handleMsgUseLine_t handleMsgUseLine;
  handleMsgCommonLine_t handleMsgCommonLine;
  handleMsgFieldsToSaveLine_t handleMsgFieldsToSaveLine;
  handleMsgActionsLine_t handleMsgActionsLine;
  handleMsgValuesLine_t handleMsgValuesLine;
  handleMsgValHeaderLine_t handleMsgValHeaderLine;
  handleMsgHeadsLine_t handleMsgHeadsLine;
  handleMsgFileNameLine_t handleMsgFileNameLine;

  freeCompMsgMsgDesc_t freeCompMsgMsgDesc;
  compMsgMsgDescInit_t compMsgMsgDescInit;

#ifdef OLD
  getHeaderFieldsFromLine_t getHeaderFieldsFromLine;
  readModuleDataValues_t readModuleDataValues;
  readWifiValues_t readWifiValues;
  getMsgPartsFromHeaderPart_t getMsgPartsFromHeaderPart;
  getMsgKeyValueDescParts_t getMsgKeyValueDescParts;
  getWifiKeyValueKeys_t getWifiKeyValueKeys;
#endif
} compMsgMsgDesc_t;

compMsgMsgDesc_t *newCompMsgMsgDesc();

#endif	/* COMP_MSG_MSG_DESC_H */
