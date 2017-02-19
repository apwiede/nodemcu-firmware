/*
* Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
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
 * File:   dwarfDbgTypeInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 18, 2017
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#include "dwarfDbgInt.h"

// =================================== addTypeStr =========================== 

static uint8_t addTypeStr(dwarfDbgPtr_t self, const char *str, int *typeStrIdx) {
  uint8_t result;
  char **typeStr;
  int strIdx;

  result = DWARF_DBG_ERR_OK;
  for(strIdx = 0; strIdx < self->dwarfDbgTypeInfo->numTypeStr; strIdx++) {
    if (strcmp(self->dwarfDbgTypeInfo->typeStrs[strIdx], str) == 0) {
      *typeStrIdx = strIdx;
      return result;
    }
  }
  if (self->dwarfDbgTypeInfo->maxTypeStr <= self->dwarfDbgTypeInfo->numTypeStr) {
    self->dwarfDbgTypeInfo->maxTypeStr += 10;
    if (self->dwarfDbgTypeInfo->typeStrs == NULL) {
      self->dwarfDbgTypeInfo->typeStrs = (char **)ckalloc(sizeof(char *) * self->dwarfDbgTypeInfo->maxTypeStr);
      if (self->dwarfDbgTypeInfo->typeStrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->typeStrs = (char **)ckrealloc((char *)self->dwarfDbgTypeInfo->typeStrs, sizeof(char *) * self->dwarfDbgTypeInfo->maxTypeStr);
      if (self->dwarfDbgTypeInfo->typeStrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
printf("== numTypeStrs: %d %s\n", self->dwarfDbgTypeInfo->numTypeStr, str);
  typeStr = &self->dwarfDbgTypeInfo->typeStrs[self->dwarfDbgTypeInfo->numTypeStr];
  *typeStr = ckalloc(strlen(str) + 1);
  memset(*typeStr, 0, strlen(str) + 1);
  memcpy(*typeStr, str, strlen(str));
  *typeStrIdx = self->dwarfDbgTypeInfo->numTypeStr;
  self->dwarfDbgTypeInfo->numTypeStr++;
  return result;
}

// =================================== addBaseType =========================== 

static uint8_t addBaseType(dwarfDbgPtr_t self, const char *name, int byteSize, int encoding, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  const char *typeStr;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwBaseType; idx++) {
    dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[idx];
    if (dwBaseType->typeStrIdx == typeStrIdx) {
      if (dwBaseType->byteSize == byteSize) {
        if (dwBaseType->encoding == encoding) {
printf("addBaseType: found: %s byteSize: %d encoding: %d baseTypeIdx: %d\n", name, byteSize, encoding, idx);
          *baseTypeIdx = idx;
          return result;
        }
      }
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addTypeDef =========================== 

static uint8_t addTypeDef(dwarfDbgPtr_t self, const char *name, int pathNameIdx, int lineNo, int dwTypeIdx, int *typeDefIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwTypeDef_t *dwTypeDef = NULL;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwTypeDef; idx++) {
    dwTypeDef = &self->dwarfDbgTypeInfo->dwTypeDefs[idx];
    if (dwTypeDef->typeStrIdx == typeStrIdx) {
      if (dwTypeDef->pathNameIdx == pathNameIdx) {
        if (dwTypeDef->lineNo == lineNo) {
          if (dwTypeDef->dwTypeIdx == dwTypeIdx) {
printf("addTypeDef: found: %s pathNameIdx: %d lineNo: %d dwTypeIdx: %d typeDefIdx: %d\n", name, pathNameIdx, lineNo, dwTypeIdx, idx);
            *typeDefIdx = idx;
            return result;
          }
        }
      }
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwTypeDef <= self->dwarfDbgTypeInfo->numDwTypeDef) {
    self->dwarfDbgTypeInfo->maxDwTypeDef += 5;
    if (self->dwarfDbgTypeInfo->dwTypeDefs == NULL) {
      self->dwarfDbgTypeInfo->dwTypeDefs = (dwTypeDef_t *)ckalloc(sizeof(dwTypeDef_t) * self->dwarfDbgTypeInfo->maxDwTypeDef);
      if (self->dwarfDbgTypeInfo->dwTypeDefs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwTypeDefs = (dwTypeDef_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwTypeDefs, sizeof(dwTypeDef_t) * self->dwarfDbgTypeInfo->maxDwTypeDef);
      if (self->dwarfDbgTypeInfo->dwTypeDefs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwTypeDef = &self->dwarfDbgTypeInfo->dwTypeDefs[self->dwarfDbgTypeInfo->numDwTypeDef];
  dwTypeDef->pathNameIdx = pathNameIdx;
  dwTypeDef->lineNo = lineNo;
  dwTypeDef->dwTypeIdx = dwTypeIdx;
  *typeDefIdx = self->dwarfDbgTypeInfo->numDwTypeDef;
  self->dwarfDbgTypeInfo->numDwTypeDef++;
  return result;
}

// =================================== addConstType =========================== 

static uint8_t addConstType(dwarfDbgPtr_t self, int dwTypeIdx, int *constTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwConstType_t *dwConstType = NULL;

  result = DWARF_DBG_ERR_OK;
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwConstType; idx++) {
    dwConstType = &self->dwarfDbgTypeInfo->dwConstTypes[idx];
    if (dwConstType->dwTypeIdx == dwTypeIdx) {
printf("addConstType: found: dwTypeIdx: %d constTypeIdx: %d\n", dwTypeIdx, idx);
      *constTypeIdx = idx;
      return result;
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwConstType <= self->dwarfDbgTypeInfo->numDwConstType) {
    self->dwarfDbgTypeInfo->maxDwConstType += 5;
    if (self->dwarfDbgTypeInfo->dwConstTypes == NULL) {
      self->dwarfDbgTypeInfo->dwConstTypes = (dwConstType_t *)ckalloc(sizeof(dwConstType_t) * self->dwarfDbgTypeInfo->maxDwConstType);
      if (self->dwarfDbgTypeInfo->dwConstTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwConstTypes = (dwConstType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwConstTypes, sizeof(dwConstType_t) * self->dwarfDbgTypeInfo->maxDwConstType);
      if (self->dwarfDbgTypeInfo->dwConstTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwConstType = &self->dwarfDbgTypeInfo->dwConstTypes[self->dwarfDbgTypeInfo->numDwConstType];
  dwConstType->dwTypeIdx = dwTypeIdx;
  *constTypeIdx = self->dwarfDbgTypeInfo->numDwConstType;
  self->dwarfDbgTypeInfo->numDwConstType++;
  return result;
}

// =================================== addMember =========================== 

static uint8_t addMember(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addPointerType =========================== 

static uint8_t addPointerType(dwarfDbgPtr_t self, int byteSize, int dwTypeIdx, int *pointerTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwPointerType_t *dwPointerType = NULL;

  result = DWARF_DBG_ERR_OK;
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwPointerType; idx++) {
    dwPointerType = &self->dwarfDbgTypeInfo->dwPointerTypes[idx];
    if (dwPointerType->byteSize == byteSize) {
      if (dwPointerType->dwTypeIdx == dwTypeIdx) {
printf("addPointerType: found: byteSize: %d dwTypeIdx: %d pointerTypeIdx: %d\n", byteSize, dwTypeIdx, idx);
        *pointerTypeIdx = idx;
        return result;
      }
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwPointerType <= self->dwarfDbgTypeInfo->numDwPointerType) {
    self->dwarfDbgTypeInfo->maxDwPointerType += 5;
    if (self->dwarfDbgTypeInfo->dwPointerTypes == NULL) {
      self->dwarfDbgTypeInfo->dwPointerTypes = (dwPointerType_t *)ckalloc(sizeof(dwPointerType_t) * self->dwarfDbgTypeInfo->maxDwPointerType);
      if (self->dwarfDbgTypeInfo->dwPointerTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwPointerTypes = (dwPointerType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwPointerTypes, sizeof(dwPointerType_t) * self->dwarfDbgTypeInfo->maxDwPointerType);
      if (self->dwarfDbgTypeInfo->dwPointerTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
printf("addPointerType: new: byteSize: %d dwTypeIdx: %d pointerTypeIdx: %d\n", byteSize, dwTypeIdx, self->dwarfDbgTypeInfo->numDwPointerType);
  dwPointerType = &self->dwarfDbgTypeInfo->dwPointerTypes[self->dwarfDbgTypeInfo->numDwPointerType];
  dwPointerType->byteSize = byteSize;
  dwPointerType->dwTypeIdx = dwTypeIdx;
  *pointerTypeIdx = self->dwarfDbgTypeInfo->numDwPointerType;
  self->dwarfDbgTypeInfo->numDwPointerType++;
  return result;
}

// =================================== addStructureType =========================== 

static uint8_t addStructureType(dwarfDbgPtr_t self, const char *name, int byteSize, int pathNameIdx, int lineNo, int declaration, int *structureTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwStructureType_t *dwStructureType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (name == NULL) {
    typeStrIdx = -1;
  } else {
    result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
    checkErrOK(result);
  }
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwStructureType; idx++) {
    dwStructureType = &self->dwarfDbgTypeInfo->dwStructureTypes[idx];
    if (dwStructureType->typeStrIdx == typeStrIdx) {
      if (dwStructureType->byteSize == byteSize) {
        if (dwStructureType->pathNameIdx == pathNameIdx) {
          if (dwStructureType->lineNo == lineNo) {
            if (dwStructureType->declaration == declaration) {
printf("addStructureType: found: %s byteSize: %d pathNameIdx: %d lineNo: %d declarationx: %d typeDefIdx: %d\n", name, byteSize, pathNameIdx, lineNo, declaration, idx);
              *structureTypeIdx = idx;
              return result;
            }
          }
        }
      }
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwStructureType <= self->dwarfDbgTypeInfo->numDwStructureType) {
    self->dwarfDbgTypeInfo->maxDwStructureType += 5;
    if (self->dwarfDbgTypeInfo->dwStructureTypes == NULL) {
      self->dwarfDbgTypeInfo->dwStructureTypes = (dwStructureType_t *)ckalloc(sizeof(dwStructureType_t) * self->dwarfDbgTypeInfo->maxDwStructureType);
      if (self->dwarfDbgTypeInfo->dwStructureTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwStructureTypes = (dwStructureType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwStructureTypes, sizeof(dwStructureType_t) * self->dwarfDbgTypeInfo->maxDwStructureType);
      if (self->dwarfDbgTypeInfo->dwStructureTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
printf("addStructureType: new %d %s byteSize: %d pathNameIdx: %d lineNo: %d declaration: %d structureTypeIdx: %d\n", self->dwarfDbgTypeInfo->numDwStructureType, name, byteSize, pathNameIdx, lineNo, declaration, self->dwarfDbgTypeInfo->numDwStructureType);
  dwStructureType = &self->dwarfDbgTypeInfo->dwStructureTypes[self->dwarfDbgTypeInfo->numDwStructureType];
  dwStructureType->byteSize = byteSize;
  dwStructureType->pathNameIdx = pathNameIdx;
  dwStructureType->lineNo = lineNo;
  dwStructureType->declaration = declaration;
  dwStructureType->typeStrIdx = typeStrIdx;
  *structureTypeIdx = self->dwarfDbgTypeInfo->numDwStructureType;
  self->dwarfDbgTypeInfo->numDwStructureType++;
  return result;
}

// =================================== addArrayType =========================== 

static uint8_t addArrayType(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addEnumerationType =========================== 

static uint8_t addEnumerationType(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addEnumerator =========================== 

static uint8_t addEnumerator(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addUnionType =========================== 

static uint8_t addUnionType(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addVolatileType =========================== 

static uint8_t addVolatileType(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addSubroutineType =========================== 

static uint8_t addSubroutineType(dwarfDbgPtr_t self, int prototyped, int dwTypeIdx, int siblingIdx, int *subroutineTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwSubroutineType_t *dwSubroutineType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwSubroutineType <= self->dwarfDbgTypeInfo->numDwSubroutineType) {
    self->dwarfDbgTypeInfo->maxDwSubroutineType += 5;
    if (self->dwarfDbgTypeInfo->dwSubroutineTypes == NULL) {
      self->dwarfDbgTypeInfo->dwSubroutineTypes = (dwSubroutineType_t *)ckalloc(sizeof(dwSubroutineType_t) * self->dwarfDbgTypeInfo->maxDwSubroutineType);
      if (self->dwarfDbgTypeInfo->dwSubroutineTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwSubroutineTypes = (dwSubroutineType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwSubroutineTypes, sizeof(dwSubroutineType_t) * self->dwarfDbgTypeInfo->maxDwSubroutineType);
      if (self->dwarfDbgTypeInfo->dwSubroutineTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwSubroutineType = &self->dwarfDbgTypeInfo->dwSubroutineTypes[self->dwarfDbgTypeInfo->numDwSubroutineType];
  dwSubroutineType->prototyped = prototyped;
  dwSubroutineType->dwTypeIdx = dwTypeIdx;
  dwSubroutineType->siblingIdx = siblingIdx;
  *subroutineTypeIdx = self->dwarfDbgTypeInfo->numDwSubroutineType;
  self->dwarfDbgTypeInfo->numDwSubroutineType++;
  return result;
}

// =================================== handlePointerType =========================== 

static uint8_t handlePointerType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int pointerTypeIdx = 0;
  int maxEntries = 0;
  int maxEntries2 = 0;
  int attrIdx = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int byteSize;   // DW_AT_byte_size
  int dwTypeIdx;  // DW_AT_type
  int found = 0;
  int isSibling2 = 0;
  char *attrName = NULL;
  const char *tagName = NULL;
  const char *tagName2 = NULL;

  result = DWARF_DBG_ERR_OK;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  byteSize = -1;
  dwTypeIdx = -1;
  attrName = NULL;
  found = 0;
printf("tag offset: 0x%08x\n", dieInfo->offset);
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
    switch (dieAttr->attr) {
    case DW_AT_byte_size:
      byteSize = dieAttr->byteSize;
printf("byteSize: %d refOffset: 0x%08x\n", byteSize, dieAttr->refOffset);
      found++;
      break;
    case DW_AT_type:
printf("PTR DW_AT_type: refOffset: 0x%08x maxEntries: %d\n", dieAttr->refOffset, maxEntries);
      for (isSibling2 = 0; isSibling2 < 2; isSibling2++) {
      for(typeIdx = 0; typeIdx < maxEntries; typeIdx++) {
        if (isSibling2) {
          dieInfo2 = &dieAndChildrenInfo->dieSiblings[typeIdx];
        } else {
          dieInfo2 = &dieAndChildrenInfo->dieChildren[typeIdx];
        }
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo2->tag, &tagName2);
//printf("tag: %s 0x%04x typeIdx: %d offset: 0x%08x base_type: 0x%04x\n", tagName2, dieInfo2->tag, typeIdx, dieInfo2->offset, DW_TAG_base_type);
        switch (dieInfo2->tag) {
        case DW_TAG_base_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s baseTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            dwTypeIdx = dieInfo2->tagRefIdx;
            found++;
          }
          break;
        case DW_TAG_structure_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s structureTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            dwTypeIdx = dieInfo2->tagRefIdx;
            found++;
          }
          break;
        case DW_TAG_typedef:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s typedefIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            dwTypeIdx = dieInfo2->tagRefIdx;
            found++;
          }
          break;
        case DW_TAG_subroutine_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s subroutineTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            dwTypeIdx = dieInfo2->tagRefIdx;
            found++;
          }
          break;
        case DW_TAG_const_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s constTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            dwTypeIdx = dieInfo2->tagRefIdx;
            found++;
          }
          break;
        case DW_TAG_pointer_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s pointerTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            dwTypeIdx = dieInfo2->tagRefIdx;
            found++;
          }
          break;
        default:
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo2->tag, &tagName);
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found default: %s dieInfo2->offset: 0x%08x dieAttr->refOffset: 0x%08x\n", tagName, dieInfo2->offset, dieAttr->refOffset);
          }
          break;
        }
      }
      }
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF;
    }
    if (found == dieInfo->numAttr) {
printf("found 2 break\n");
      break;
    }
  }
  if (found >= dieInfo->numAttr) {
printf("call addPointerType\n");
    result = self->dwarfDbgTypeInfo->addPointerType(self, byteSize, dwTypeIdx, &pointerTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = pointerTypeIdx;
    dieInfo->flags  = TAG_REF_POINTER_TYPE;
  } else {
printf("ERROR pointerType not found offset: 0x%08x found: %d numAttr: %d\n", dieAttr->refOffset, found, dieInfo->numAttr);
    return DWARF_DBG_ERR_POINTER_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== handleConstType =========================== 

static uint8_t handleConstType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int pointerTypeIdx = 0;
  int maxEntries = 0;
  int maxEntries2 = 0;
  int attrIdx = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int dwTypeIdx;  // DW_AT_type
  int found = 0;
  int isSibling2 = 0;
  char *attrName = NULL;
  const char *tagName = NULL;
  const char *tagName2 = NULL;

  result = DWARF_DBG_ERR_OK;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  dwTypeIdx = -1;
  attrName = NULL;
  found = 0;
printf("handleConstType tag offset: 0x%08x\n", dieInfo->offset);
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
    switch (dieAttr->attr) {
    case DW_AT_type:
printf("CONST DW_AT_type: refOffset: 0x%08x maxEntries: %d\n", dieAttr->refOffset, maxEntries);
      for (isSibling2 = 0; isSibling2 < 2; isSibling2++) {
      for(typeIdx = 0; typeIdx < maxEntries; typeIdx++) {
        if (isSibling2) {
          dieInfo2 = &dieAndChildrenInfo->dieSiblings[typeIdx];
        } else {
          dieInfo2 = &dieAndChildrenInfo->dieChildren[typeIdx];
        }
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo2->tag, &tagName2);
printf("tag: %s typeIdx: %d offset: 0x%08x\n", tagName2, typeIdx, dieInfo2->offset);
        switch (dieInfo2->tag) {
        case DW_TAG_base_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s baseTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            dwTypeIdx = dieInfo2->tagRefIdx;
            found++;
          }
          break;
        case DW_TAG_structure_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s structureTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            dwTypeIdx = dieInfo2->tagRefIdx;
            found++;
          }
          break;
        case DW_TAG_array_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s arrayTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            dwTypeIdx = dieInfo2->tagRefIdx;
            found++;
          }
          break;
        case DW_TAG_typedef:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s typedefIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            dwTypeIdx = dieInfo2->tagRefIdx;
            found++;
          }
          break;
        case DW_TAG_pointer_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s pointerTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            dwTypeIdx = dieInfo2->tagRefIdx;
            found++;
          }
          break;
        default:
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo2->tag, &tagName);
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found default: %s dieInfo2->offset: 0x%08x dieAttr->refOffset: 0x%08x\n", tagName, dieInfo2->offset, dieAttr->refOffset);
          }
          break;
        }
      }
      }
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF;
    }
    if (found == dieInfo->numAttr) {
printf("found 2 break\n");
      break;
    }
  }
  if (found == dieInfo->numAttr) {
printf("call addConstType\n");
    result = self->dwarfDbgTypeInfo->addConstType(self, dwTypeIdx, &pointerTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = pointerTypeIdx;
    dieInfo->flags  = TAG_REF_POINTER_TYPE;
  } else {
printf("ERROR constType not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_POINTER_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== handleStructureType =========================== 

static uint8_t handleStructureType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int structureTypeIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  int typeStrIdx;  // DW_AT_name
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int declaration; // DW_AT_declaration
  int byteSize;    // DW_AT_byte_size
  int siblingIdx;  // DW_AT_sibling
  dieAttr_t *dieAttr = NULL;
  int found = 0;
  const char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  pathNameIdx = -1;
  lineNo = -1;
  byteSize = -1;
  declaration = 0;
  attrName = NULL;
  found = 0;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
    switch (dieAttr->attr) {
    case DW_AT_name:
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      attrName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      found++;
      break;
    case DW_AT_decl_file:
      pathNameIdx = dieAttr->sourceFileIdx;
      found++;
      break;
    case DW_AT_decl_line:
      lineNo = dieAttr->sourceLineNo;
      found++;
      break;
    case DW_AT_byte_size:
      byteSize = dieAttr->uval;
      found++;
      break;
    case DW_AT_declaration:
      declaration = dieAttr->uval;
      found++;
      break;
    case DW_AT_sibling:
      siblingIdx = dieAttr->uval;
      found++;
      break;
    default:
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF: 0x%04x %s offset: 0x%08x\n", dieAttr->attr, attrName, dieInfo->offset);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF;
    }
  }
  if (found) {
    result = self->dwarfDbgTypeInfo->addStructureType(self, attrName, byteSize, pathNameIdx, lineNo, declaration, &structureTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = structureTypeIdx;
    dieInfo->flags  = TAG_REF_STRUCTURE_TYPE;
  } else {
printf("ERROR structureType not found\n");
    return DWARF_DBG_ERR_STRUCTURE_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== handleSubroutineType =========================== 

static uint8_t handleSubroutineType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int pointerTypeIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int byteSize;   // DW_AT_byte_size
  int dwTypeIdx;  // DW_AT_type
  int found = 0;
  char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  byteSize = -1;
  dwTypeIdx = -1;
  attrName = NULL;
  found = 0;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
    switch (dieAttr->attr) {
    case DW_AT_byte_size:
      byteSize = dieAttr->byteSize;
      break;
    case DW_AT_type:
      for(typeIdx = 0; typeIdx < maxEntries; typeIdx++) {
        if (isSibling) {
          dieInfo2 = &dieAndChildrenInfo->dieSiblings[typeIdx];
        } else {
          dieInfo2 = &dieAndChildrenInfo->dieChildren[typeIdx];
        }
        switch (dieInfo2->tag) {
        case DW_TAG_base_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: baseTypeIdx: %d dieInfo2->offset: 0x%08x baseTypeTagRef: %d\n", typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            result = self->dwarfDbgTypeInfo->addPointerType(self, byteSize, dieInfo2->tagRefIdx, &pointerTypeIdx);
printf("new subroutineType: pointerTypIdx: %d\n", pointerTypeIdx);
            checkErrOK(result);
            dieInfo2->tagRefIdx = pointerTypeIdx;
            found = 1;
          }
          break;
        default:
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo2->tag, &tagName);
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found default: %s dieInfo2->offset: 0x%08x dieAttr->refOffset: 0x%08x\n", tagName, dieInfo2->offset, dieAttr->refOffset);
          }
          break;
        }
        if (found) {
          break;
        }
      }
printf("not found pointerType type: baseTypeIdx: %d dieInfo2->offset: 0x%08x\n", typeIdx, dieInfo2->offset);
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF;
    }
  }
  return result;
}

// =================================== handleTypedef =========================== 

static uint8_t handleTypedef(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int typedefIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int typeStrIdx;  // DW_AT_name
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int dwTypeIdx;   // DW_AT_type
  int found = 0;
  char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  typeStrIdx = -1;
  pathNameIdx = -1;
  lineNo = -1;
  dwTypeIdx = -1;
  attrName = NULL;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
    switch (dieAttr->attr) {
    case DW_AT_name:
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      attrName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      break;
    case DW_AT_decl_file:
      pathNameIdx = dieAttr->sourceFileIdx;
      break;
    case DW_AT_decl_line:
      lineNo = dieAttr->sourceLineNo;
      break;
    case DW_AT_type:
      found = 0;
      for(typeIdx = 0; typeIdx < maxEntries; typeIdx++) {
        if (isSibling) {
          dieInfo2 = &dieAndChildrenInfo->dieSiblings[typeIdx];
        } else {
          dieInfo2 = &dieAndChildrenInfo->dieChildren[typeIdx];
        }
        switch (dieInfo2->tag) {
        case DW_TAG_base_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s baseTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", attrName, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            result = self->dwarfDbgTypeInfo->addTypeDef(self, attrName, pathNameIdx, lineNo, dieInfo2->tagRefIdx, &typedefIdx);
printf("new typedef: %s typedefIdx: %d\n", attrName, typedefIdx);
            checkErrOK(result);
            dieInfo2->tagRefIdx = typedefIdx;
            found = 1;
          }
          break;
        case DW_TAG_typedef:
//printf("DW_TAG_typedef: dieInfo2->offset: 0x%08x dieAttr->refOffset: 0x%08x\n", dieInfo2->offset, dieAttr->refOffset);
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s typedefIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", attrName, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            result = self->dwarfDbgTypeInfo->addTypeDef(self, attrName, pathNameIdx, lineNo, dieInfo2->tagRefIdx, &typedefIdx);
printf("new typedef: %s typedefIdx: %d\n", attrName, typedefIdx);
            checkErrOK(result);
            dieInfo2->tagRefIdx = typedefIdx;
            found = 1;
          }
          break;
        case DW_TAG_structure_type:
//printf("DW_TAG_typedef: dieInfo2->offset: 0x%08x dieAttr->refOffset: 0x%08x\n", dieInfo2->offset, dieAttr->refOffset);
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s typedefIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", attrName, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            result = self->dwarfDbgTypeInfo->addTypeDef(self, attrName, pathNameIdx, lineNo, dieInfo2->tagRefIdx, &typedefIdx);
printf("new typedef: %s typedefIdx: %d\n", attrName, typedefIdx);
            checkErrOK(result);
            dieInfo2->tagRefIdx = typedefIdx;
            found = 1;
          }
          break;
        case DW_TAG_subroutine_type:
//printf("DW_TAG_subroutine_type: dieInfo2->offset: 0x%08x dieAttr->refOffset: 0x%08x\n", dieInfo2->offset, dieAttr->refOffset);
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s subroutineTypeIdx: %d dieInfo2->offset: 0x%08x subroutineTypeTagRef: %d\n", attrName, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
//                result = self->dwarfDbgTypeInfo->addSubroutineType(self, attrName, pathNameIdx, lineNo, dieInfo2->tagRefIdx, &typedefIdx);
//printf("new typedef: %s typedefIdx: %d\n", attrName, typedefIdx);
//                checkErrOK(result);
//                dieInfo2->tagRefIdx = typedefIdx;

          }
          break;
        default:
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo2->tag, &tagName);
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found typedef default: %s dieInfo2->offset: 0x%08x dieAttr->refOffset: 0x%08x\n", tagName, dieInfo2->offset, dieAttr->refOffset);
          }
          break;
        }
        if (found) {
          break;
        }
      }
      if (!found) {
printf("not found type: %s baseTypeIdx: %d dieInfo2->offset: 0x%08x\n", attrName, typeIdx, dieInfo2->offset);
      }
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF;
    }
  }
  return result;
}

// =================================== addTypes =========================== 

static uint8_t addTypes(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags, Dwarf_Bool isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int typeDefIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  dieAndChildrenInfo_t *dieAndChildrenInfo = NULL;
  compileUnit_t *compileUnit = NULL;
  dieInfo_t *dieInfo = NULL;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int found = 0;
  int tagToHandle = 0;
  char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  tagToHandle = 0;
  if (flags & TAG_REF_BASE_TYPE) {
    tagToHandle = DW_TAG_base_type;
  }
  if (flags & TAG_REF_TYPEDEF) {
    tagToHandle = DW_TAG_typedef;
  }
  if (flags & TAG_REF_SUBROUTINE_TYPE) {
    tagToHandle = DW_TAG_subroutine_type;
  }
  if (flags & TAG_REF_POINTER_TYPE) {
    tagToHandle = DW_TAG_pointer_type;
  }
  if (flags & TAG_REF_CONST_TYPE) {
    tagToHandle = DW_TAG_const_type;
  }
  if (flags & TAG_REF_STRUCTURE_TYPE) {
    tagToHandle = DW_TAG_structure_type;
  }
  if (tagToHandle == 0) {
    return DWARF_DBG_ERR_BAD_TAG_REF_TYPE;
  }
  for(entryIdx = 0; entryIdx < maxEntries; entryIdx++) {
    if (isSibling) {
      dieInfo = &dieAndChildrenInfo->dieSiblings[entryIdx];
    } else {
      dieInfo = &dieAndChildrenInfo->dieChildren[entryIdx];
    }
    if (dieInfo->tag == tagToHandle) {
      switch (tagToHandle) {
      case DW_TAG_typedef:
        result = self->dwarfDbgTypeInfo->handleTypedef(self, dieAndChildrenInfo, dieInfo, isSibling);
        checkErrOK(result);
        break;
      case DW_TAG_pointer_type:
        result = self->dwarfDbgTypeInfo->handlePointerType(self, dieAndChildrenInfo, dieInfo, isSibling);
        checkErrOK(result);
        break;
      case DW_TAG_const_type:
        result = self->dwarfDbgTypeInfo->handleConstType(self, dieAndChildrenInfo, dieInfo, isSibling);
        checkErrOK(result);
        break;
      case DW_TAG_structure_type:
        result = self->dwarfDbgTypeInfo->handleStructureType(self, dieAndChildrenInfo, dieInfo, isSibling);
        checkErrOK(result);
        break;
      case DW_TAG_subroutine_type:
        result = self->dwarfDbgTypeInfo->handleSubroutineType(self, dieAndChildrenInfo, dieInfo, isSibling);
        checkErrOK(result);
        break;
      default:
printf("ERRROr unexpected tag: 0x%04x\n", tagToHandle);
        break;
      }
    }
  }
  return result;
}

// =================================== addChildrenTypes =========================== 

static uint8_t addChildrenTypes(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags) {
  uint8_t result = 0;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgTypeInfo->addTypes(self, dieAndChildrenIdx, flags, /* isSibling */ 0);
  checkErrOK(result);
  return result;
}

// =================================== addSiblingsTypes =========================== 

static uint8_t addSiblingsTypes(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags) {
  uint8_t result = 0;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgTypeInfo->addTypes(self, dieAndChildrenIdx, flags, /* isSibling */ 1);
  checkErrOK(result);
  return result;
}

// =================================== dwarfDbgTypeInfoInit =========================== 

int dwarfDbgTypeInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgTypeInfo->addTypeStr = &addTypeStr;
  self->dwarfDbgTypeInfo->addBaseType = &addBaseType;
  self->dwarfDbgTypeInfo->addTypeDef = &addTypeDef;
  self->dwarfDbgTypeInfo->addConstType = &addConstType;
  self->dwarfDbgTypeInfo->addMember = &addMember;
  self->dwarfDbgTypeInfo->addPointerType = &addPointerType;
  self->dwarfDbgTypeInfo->addStructureType = &addStructureType;
  self->dwarfDbgTypeInfo->addArrayType = &addArrayType;
  self->dwarfDbgTypeInfo->addEnumerationType = &addEnumerationType;
  self->dwarfDbgTypeInfo->addEnumerator = &addEnumerator;
  self->dwarfDbgTypeInfo->addUnionType = &addUnionType;
  self->dwarfDbgTypeInfo->addVolatileType = &addVolatileType;
  self->dwarfDbgTypeInfo->handlePointerType = &handlePointerType;
  self->dwarfDbgTypeInfo->handleConstType = &handleConstType;
  self->dwarfDbgTypeInfo->handleStructureType = &handleStructureType;
  self->dwarfDbgTypeInfo->handleSubroutineType = &handleSubroutineType;
  self->dwarfDbgTypeInfo->handleTypedef = &handleTypedef;
  self->dwarfDbgTypeInfo->addTypes = &addTypes;
  self->dwarfDbgTypeInfo->addChildrenTypes = &addChildrenTypes;
  self->dwarfDbgTypeInfo->addSiblingsTypes = &addSiblingsTypes;
  return result;
}
