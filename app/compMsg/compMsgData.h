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
 * File:   compMsgData.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on August 1, 2016
 */

#ifndef COMP_MSG_DATA_H
#define	COMP_MSG_DATA_H

#include "c_types.h"
typedef struct compMsgData compMsgData_t;
#include "compMsgDispatcher.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define COMP_MSG_HAS_CRC              (1 << 0)
#define COMP_MSG_UINT8_CRC            (1 << 1)
#define COMP_MSG_HAS_FILLER           (1 << 2)
#define COMP_MSG_U8_CMD_KEY           (1 << 3)
#define COMP_MSG_HAS_TABLE_ROWS       (1 << 4)
#define COMP_MSG_IS_INITTED           (1 << 5)
#define COMP_MSG_IS_PREPARED          (1 << 6)
#define COMP_DEF_IS_INITTED           (1 << 7)
#define COMP_DEF_IS_PREPARED          (1 << 8)
#define COMP_MSG_CRC_USE_HEADER_LGTH  (1 << 9)
#define COMP_LIST_IS_INITTED          (1 << 10)
#define COMP_LIST_IS_PREPARED         (1 << 11)

#define COMP_MSG_FIELD_IS_SET   (1 << 0)

#define COMP_DEF_NUM_DEF_FIELDS 15
#define COMP_DEF_CMD_KEY 0x5959

#define COMP_DEF_TO_DATA        (1 << 0)
#define COMP_DEF_FROM_DATA      (1 << 1)
#define COMP_LIST_TO_DATA       (1 << 2)
#define COMP_LIST_FROM_DATA     (1 << 3)

#define COMP_LIST_NUM_LIST_FIELDS 9
#define COMP_LIST_CMD_KEY 0x5A5A

#define checkHandleOK(addr) if(addr == NULL) return COMP_MSG_ERR_BAD_HANDLE


typedef uint8_t (* createMsg_t)(compMsgData_t *self, int numFields, uint8_t **handle);
typedef uint8_t (* deleteMsg_t)(compMsgData_t *self);
typedef uint8_t (* addField_t)(compMsgData_t *self, const uint8_t *fieldName, const uint8_t *fieldType, uint8_t fieldLgth);
typedef uint8_t (* getFieldValue_t)(compMsgData_t *self, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* setFieldValue_t)(compMsgData_t *self, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue);

typedef uint8_t (* getTableFieldValue_t)(compMsgData_t *self, const uint8_t *fieldName, int row, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* setTableFieldValue_t)(compMsgData_t *self, const uint8_t *fieldName, int row, int numericValue, const uint8_t *stringValue);
typedef uint8_t (* dumpMsg_t)(compMsgData_t *self);
typedef uint8_t (* initMsg_t)(compMsgData_t *self);
typedef uint8_t (* prepareMsg_t)(compMsgData_t *self);
typedef uint8_t (* getMsgData_t)(compMsgData_t *compMsgData, uint8_t **data, int *lgth);
typedef uint8_t (* setMsgData_t)(compMsgData_t *compMsgData, const uint8_t *data);
typedef uint8_t (* setMsgFieldFromList_t)(compMsgData_t *selfconst, const uint8_t **listVector, uint8_t numEntries, uint16_t flags);
typedef uint8_t ( *setDispatcher_t)(compMsgData_t *self, compMsgDispatcher_t *dispatcher);

// definitionMsg
typedef uint8_t (* dumpDefFields_t)(compMsgData_t *self);
typedef uint8_t (* initDefMsg_t)(compMsgData_t *self);
typedef uint8_t (* prepareDefMsg_t)(compMsgData_t *self);
typedef uint8_t (* addDefField_t)(compMsgData_t *self, uint8_t fieldNameId, uint8_t fieldTypeId, uint8_t fieldLgth);
typedef uint8_t (* getDefFieldValue_t)(compMsgData_t *self, const uint8_t fieldNameId, int *numericValue, uint8_t **stringValue, int idx);
typedef uint8_t (* setDefFieldValue_t)(compMsgData_t *self, uint8_t fieldNameId, int numericValue, const uint8_t *stringValue, int idx);
typedef uint8_t (* setDefData_t)(compMsgData_t *self, const uint8_t *data);
typedef uint8_t (* getDefData_t)(compMsgData_t *self, uint8_t **data, int *lgth);
typedef uint8_t (* createMsgFromDef_t)(compMsgData_t *self);

// listMsg
typedef uint8_t (* dumpListFields_t)(compMsgData_t *self);
typedef uint8_t (* getListFieldValue_t)(compMsgData_t *self, uint8_t fieldNameId, int *numericValue, uint8_t **stringValue, int fieldIdx);
typedef uint8_t (* setListFieldValue_t)(compMsgData_t *self, uint8_t fieldNameId, int numericValue, const uint8_t *stringValue, int fieldIdx);
typedef uint8_t (* initListMsg_t)(compMsgData_t *self);
typedef uint8_t (* prepareListMsg_t)(compMsgData_t *self);
typedef uint8_t (* setListData_t)(compMsgData_t *self, const  uint8_t *data);
typedef uint8_t (* getListData_t)(compMsgData_t *self, uint8_t **data, int *lgth);
typedef uint8_t (* addListMsg_t)(compMsgData_t *self, size_t msgLgth, uint8_t *msgData);
typedef uint8_t (* addListField_t)(compMsgData_t *self, uint8_t fieldNameId, uint8_t fieldTypeId, uint8_t fieldLgth);

typedef struct compMsgData {
  compMsgDataView_t *compMsgDataView;
  compMsgDispatcher_t *compMsgDispatcher;
  char handle[16];
  compMsgField_t *fields;
  compMsgField_t *tableFields;
  uint16_t flags;
  size_t numFields;
  size_t maxFields;
  size_t numTableRows;         // number of list rows
  size_t numTableRowFields;    // number of fields within a table row
  size_t numRowFields;         // for checking how many tableRowFields have been processed
  size_t fieldOffset;
  size_t totalLgth;
  size_t cmdLgth;
  size_t headerLgth;
  uint8_t *header;

  // definitionMsg
  compMsgDataView_t *compMsgDefMsgDataView;
  compMsgField_t *defFields;
  size_t numDefFields;         // for checking how many defFields have been processed
  size_t defFieldOffset;
  size_t defTotalLgth;
  size_t defHeaderLgth;
  size_t defNumNormFields;
  size_t defNormNamesSize;
  size_t defDefsSize;

  initDefMsg_t initDefMsg;
  prepareDefMsg_t prepareDefMsg;
  dumpDefFields_t dumpDefFields;
  addDefField_t addDefField;
  getDefFieldValue_t getDefFieldValue;
  setDefFieldValue_t setDefFieldValue;
  setDefData_t setDefData;
  getDefData_t getDefData;
  createMsgFromDef_t createMsgFromDef;

  // listMsg
  compMsgDataView_t *compMsgListDataView;
  compMsgField_t *listFields;
  size_t numListFields;         // for checking how many listMsgs have been processed
  size_t numListMsgs;
  size_t listMsgSizesSize;
  size_t listMsgsSize;
  size_t listFieldOffset;
  size_t listTotalLgth;
  size_t listHeaderLgth;
  uint16_t listSrc;
  uint16_t listDst;
  uint16_t *listMsgSizes;
  uint8_t *listMsgs;

  dumpListFields_t dumpListFields;
  setListFieldValue_t setListFieldValue;
  getListFieldValue_t getListFieldValue;
  setListData_t setListData;
  getListData_t getListData;
  initListMsg_t initListMsg;
  prepareListMsg_t prepareListMsg;
  addListMsg_t addListMsg;
  addListField_t addListField;

  // normalMsg
  createMsg_t createMsg;
  deleteMsg_t deleteMsg;
  addField_t addField;
  getFieldValue_t getFieldValue;
  setFieldValue_t setFieldValue;
  getTableFieldValue_t getTableFieldValue;
  setTableFieldValue_t setTableFieldValue;
  dumpMsg_t dumpMsg;
  initMsg_t initMsg;
  prepareMsg_t prepareMsg;
  getMsgData_t getMsgData;
  setMsgData_t setMsgData;
  setMsgFieldFromList_t setMsgFieldsFromList;

  setDispatcher_t setDispatcher;

} compMsgData_t;


compMsgData_t *newCompMsgData(void);
uint8_t compMsgGetPtrFromHandle(const char *handle, compMsgData_t **compMsgData);
uint8_t newCompMsgDataFromList(const uint8_t **listVector, uint8_t numEntries, uint8_t numRows, uint16_t flags, uint8_t **handle);
uint8_t newCompMsgDefMsg(compMsgData_t *compMsgData);
uint8_t newCompMsgList(compMsgData_t *compMsgData);
 
#ifdef	__cplusplus
}
#endif

#endif	/* COMP_MSG_DATA_H */
