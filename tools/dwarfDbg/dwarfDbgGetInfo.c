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
 * File:   dwarfDbgGetInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 24, 2017
 */

#include <tcl.h>
#include "dwarfDbgInt.h"

// =================================== addCompileUnit =========================== 

static uint8_t addCompileUnit(dwarfDbgPtr_t self, size_t *compileUnitIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgGetInfo->maxCompileUnit <= self->dwarfDbgGetInfo->numCompileUnit) {
    self->dwarfDbgGetInfo->maxCompileUnit += 50;
    if (self->dwarfDbgGetInfo->compileUnits == NULL) {
      self->dwarfDbgGetInfo->compileUnits = (compileUnit_t *)ckalloc(sizeof(compileUnit_t) * self->dwarfDbgGetInfo->maxCompileUnit);
      if (self->dwarfDbgGetInfo->compileUnits == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgGetInfo->compileUnits = (compileUnit_t *)ckrealloc((char *)self->dwarfDbgGetInfo->compileUnits, sizeof(compileUnit_t) * self->dwarfDbgGetInfo->maxCompileUnit);
      if (self->dwarfDbgGetInfo->compileUnits == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
printf("compileUnitIdx: %d\n", self->dwarfDbgGetInfo->numCompileUnit);
  compileUnit = &self->dwarfDbgGetInfo->compileUnits[self->dwarfDbgGetInfo->numCompileUnit];
  memset(compileUnit, 0, sizeof(compileUnit_t));
  *compileUnitIdx = self->dwarfDbgGetInfo->numCompileUnit;
  self->dwarfDbgGetInfo->numCompileUnit++;
  return result;
}

static Dwarf_Off  DIEOffset = 0;      /* DIE offset in compile unit */
static Dwarf_Off  DIEOverallOffset = 0;  /* DIE offset in .debug_info */
static dwarfDbgEsb_t esbShortCuName;
static dwarfDbgEsb_t esbLongCuName;

// =================================== getAttrValue =========================== 

static int getAttrValue(Dwarf_Debug dbg, Dwarf_Half tag, Dwarf_Die die, Dwarf_Off compileUnitOffset,
    Dwarf_Half attr, Dwarf_Attribute attr_in, char **srcfiles, int cnt, char **attrStr)
{
  Dwarf_Attribute attrib = 0;
  Dwarf_Unsigned uval = 0;
  const char * atName = 0;
  const char * tagName = 0;
  dwarfDbgEsb_t valName;
  dwarfDbgEsb_t esb_extra;
  int tres = 0;
  int append_extra_string = 0;
  int found_search_attr = FALSE;
  int bTextFound = FALSE;
  Dwarf_Bool is_info = FALSE;
  Dwarf_Addr elf_max_address = 0;
  Dwarf_Error paerr = 0;

  char * temps = NULL;
  int res = 0;
  int fres = 0;
  int sres = 0;
  int bres = 0;
  Dwarf_Half theform = 0;
  Dwarf_Error err = 0;
  Dwarf_Addr addr = 0;
  
  res = dwarf_get_AT_name(attr, &atName);
printf("getAttrValue attr: 0x%08x atname: %s\n", attr_in, atName);

  tres = dwarf_tag(die, &tag, &err);
  fres = dwarf_whatform(attr_in, &theform, &err);
  res = dwarf_get_TAG_name(tag, &tagName);
printf("getAttrValue: tag: %s attr: 0x%08x theform: 0x%08x\n", tagName, attr_in, theform);

  switch (theform) {
  case DW_FORM_addr:
    bres = dwarf_formaddr(attr_in, &addr, &err);
printf("DW_FORM_addr: attrib: 0x%08x addr: 0x%08x\n", attr_in, addr);
    break;
  case DW_FORM_data1:
    fres = dwarf_whatattr(attr_in, &attr, &err);
printf("DW_FORM_data1 attr: 0x%08x\n", attr);
    switch(attr) {
    case DW_AT_language:
printf("DW_AT_language\n");
      break;
    case DW_AT_byte_size:
printf("DW_AT_byte_size\n");
      break;
    case DW_AT_encoding:
printf("DW_AT_encoding\n");
      break;
    case DW_AT_decl_file:
printf("DW_AT_decl_file\n");
      break;
    case DW_AT_decl_line:
printf("DW_AT_decl_line\n");
      break;
    case DW_AT_upper_bound:
printf("DW_AT_upper_bound\n");
      break;
    case DW_AT_data_member_location:
printf("DW_AT_data_member_location\n");
      break;
    case DW_AT_inline:
printf("DW_AT_inline\n");
      break;
    case DW_AT_const_value:
printf("DW_AT_const_value\n");
      break;
    case DW_AT_call_file:
printf("DW_AT_call_file\n");
      break;
    case DW_AT_call_line:
printf("DW_AT_call_line\n");
      break;
    case DW_AT_bit_offset:
printf("DW_AT_bit_offset\n");
      break;
    case DW_AT_bit_size:
printf("DW_AT_bit_size\n");
      break;
    default:
printf("ERROR attribute: 0x%08x not yet implemented\n", attr);
      break;
    }
    break;
  case DW_FORM_strp:
    sres = dwarf_formstring(attr_in, &temps, &err);
printf("DW_FORM_strp: attrib: 0x%08x temps: %s\n", attr_in, temps);
*attrStr = temps;
    break;
  case DW_FORM_sec_offset:
printf("DW_FORM_sec_offset\n");
    break;
  case DW_FORM_ref4:
printf("DW_FORM_ref4\n");
    break;
printf("ERROR theform: 0x&08x not yet implemented\n", theform);
  default:
    break;
  }
  return 1;
}


// =================================== getProducerName =========================== 

/*  Returns the producer of the CU
  Caller must ensure producernameout is
  a valid, constructed, empty dwarfDbgEsb_t instance before calling.
  Never returns DW_DLV_ERROR.  */
static int getProducerName(dwarfDbgPtr_t self, Dwarf_Debug dbg, Dwarf_Die compileUnitDie, Dwarf_Off dieprintCuOffset, char **producerName) {
  Dwarf_Attribute producerAttr = 0;
  Dwarf_Error pnerr = 0;

  int ares = dwarf_attr(compileUnitDie, DW_AT_producer, &producerAttr, &pnerr);
  if (ares == DW_DLV_ERROR) {
    printf("hassattr on DW_AT_producer ares: %d pnerr: %d", ares, pnerr);
  }
  if (ares == DW_DLV_NO_ENTRY) {
    /*  We add extra quotes so it looks more like
      the names for real producers that getAttrValue
      produces. */
    *producerName = "\"<CU-missing-DW_AT_producer>\"";
  } else {
    /*  DW_DLV_OK */
    /*  The string return is valid until the next call to this
      function; so if the caller needs to keep the returned
      string, the string must be copied (makename()). */
    getAttrValue(dbg, DW_AT_producer, compileUnitDie, dieprintCuOffset, 1, producerAttr, NULL, 0, producerName);
  }
  return ares;
}

// =================================== getCompileUnitName =========================== 

/* Returns the name of the compile unit. In case of error, give up, do not return. */
static int getCompileUnitName(dwarfDbgPtr_t self, Dwarf_Debug dbg, Dwarf_Die compileUnitDie, Dwarf_Off offset, char **shortName, char **longName) {
  Dwarf_Attribute nameAttr = 0;
  Dwarf_Error lerr = 0;
  int ares;

  ares = dwarf_attr(compileUnitDie, DW_AT_name, &nameAttr, &lerr);
  if (ares == DW_DLV_ERROR) {
    printf("hassattr on DW_AT_name ares: %d lerr: %d", ares, lerr);
  } else {
    if (ares == DW_DLV_NO_ENTRY) {
      *shortName = "<unknown name>";
      *longName = "<unknown name>";
    } else {
      /* DW_DLV_OK */
      /*  The string return is valid until the next call to this
        function; so if the caller needs to keep the returned
        string, the string must be copied (makename()). */
      char *fileName = 0;

//      self->dwarfDbgEsb->esbEmptyString(self, &esbLongCuName);
      getAttrValue(dbg, DW_TAG_compile_unit, compileUnitDie, offset, 1, nameAttr, NULL, 0, longName);
      /* Generate the short name (fileName) */
      fileName = strrchr(*longName,'/');
      if (!fileName) {
        fileName = strrchr(*longName,'\\');
      }
      if (fileName) {
        ++fileName;
      } else {
        fileName = *longName;
      }
      *shortName = fileName;
    }
  }
  dwarf_dealloc(dbg, nameAttr, DW_DLA_ATTR);
  return ares;
}

// =================================== getAttribute =========================== 

static uint8_t getAttribute(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Off dieprint_cu_goffset, Dwarf_Half attr,
    Dwarf_Attribute attr_in,  char **srcfiles, Dwarf_Signed cnt)
{
  const char *atName = NULL;
  char *templateNameStr = NULL;
  int res;
  int tres;
  int vres;
  Dwarf_Half tag = 0;
  Dwarf_Unsigned uval = 0;
  Dwarf_Error err;
  Dwarf_Half theform = 0;
  Dwarf_Half directform = 0;
  const char *langName = NULL;

  res = dwarf_get_AT_name(attr, &atName);
printf("getAttribute: atName: %s\n", atName);
  tres = dwarf_tag(die, &tag, &err);
printf(">>dwarf_tag: tag: %d 0x%08x\n", tag, tag);
  if (tres == DW_DLV_ERROR) {
    tag = 0;
  } else if (tres == DW_DLV_NO_ENTRY) {
    tag = 0;
  } else {
    /* ok */
  }
  switch (attr) {
  case DW_AT_language:
    vres = dwarf_formudata(attr_in, &uval, &err);
    dwarf_get_LANG_name((Dwarf_Half) uval, &langName);
printf("STR: %s\n", langName);
    res = dwarf_whatform(attr_in, &theform, &err);
    res = dwarf_whatform_direct(attr_in, &directform, &err);
printf("language: 0x%08x theform: 0x%08x directform: 0x%08x\n", uval, theform, directform);
    break;
  case DW_AT_name:
  case DW_AT_comp_dir:
    getAttrValue(dbg, tag, die, dieprint_cu_goffset, attr, attr_in, NULL, 0, &templateNameStr);
printf("NAME: %s\n", templateNameStr);
    break;
  default:
    break;
  }

}

// =================================== handleOneDieSection =========================== 

static uint8_t handleOneDieSection(dwarfDbgPtr_t self) {
  uint8_t result;
  const char * sectionName = 0;
  int res = 0;
  Dwarf_Bool isInfo = TRUE;
  Dwarf_Error err;
  unsigned loopCount = 0;
  int nres = DW_DLV_OK;
  int   compileUnitCount = 0;
  char * compileUnitShortName = NULL;
  char * compileUnitLongName = NULL;
  size_t compileUnitIdx = 0;


printf("handleOneDieSection dbg: %p\n", self->elfInfo.dbg);
  isInfo = TRUE;
  result = DWARF_DBG_ERR_OK;
  res = dwarf_get_die_section_name(self->elfInfo.dbg, isInfo, &sectionName, &err);
  if (res != DW_DLV_OK || !sectionName || !strlen(sectionName)) {
    sectionName = ".debug_info";
  }
printf("sectionName: %s\n", sectionName);
  /* Loop over compile units until it fails.  */
  for (;;++loopCount) {
    int sres = DW_DLV_OK;
    compileUnit_t *compileUnit;
//        struct Dwarf_Debug_Fission_Per_CU_s fission_data;
//        int fission_data_result = 0;

    result = self->dwarfDbgGetInfo->addCompileUnit(self, &compileUnitIdx);
printf("addCompileUnit: result: %d compileUnitIdx: %d\n", result, compileUnitIdx);
    checkErrOK(result);
//        memset(&fission_data,0,sizeof(fission_data));
    compileUnit = &self->dwarfDbgGetInfo->compileUnits[compileUnitIdx];
printf("call dwarf_next_cu_header_d\n");
    nres = dwarf_next_cu_header_d(self->elfInfo.dbg, 1,
      &compileUnit->compileUnitHeaderLength,
      &compileUnit->versionStamp,
      &compileUnit->abbrevOffset,
      &compileUnit->addressSize,
      &compileUnit->lengthSize,
      &compileUnit->extensionSize,
      &compileUnit->signature,
      &compileUnit->typeOffset,
      &compileUnit->nextCompileUnitOffset,
      &compileUnit->compileUnitType,
      &err);
printf("after dwarf_next_cu_header_d nres: %d loop_count: %d\n", nres, loopCount);
    if (nres == DW_DLV_NO_ENTRY) {
      // we have processed all entries
      break;
    }
    if (nres != DW_DLV_OK) {
      return DWARF_DBG_ERR_CANNOT_GET_NEXT_COMPILE_UNIT;
    }
    /*  get basic information about the current compile unit: producer, name */
    sres = dwarf_siblingof_b(self->elfInfo.dbg, NULL,/* is_info */1, &compileUnit->compileUnitDie, &err);
    if (sres != DW_DLV_OK) {
printf("siblingof cu header sres: %d err: %p", sres, err);
      return DWARF_DBG_ERR_CANNOT_GET_SIBLING_OF_COMPILE_UNIT;
    }
printf("compileUnitDie1: %p\n", compileUnit->compileUnitDie);
    dwarf_die_offsets(compileUnit->compileUnitDie, &compileUnit->overallOffset, &DIEOffset, &err);
#ifdef NOTDEF
    {
    /* Get producer name for this compile unit */
      char *producerName;
      
      getProducerName(self, self->elfInfo.dbg, compileUnit->compileUnitDie, compileUnit->overallOffset, &producerName);
printf("producerName: %s\n", producerName);
    }
#endif
    DIEOverallOffset = compileUnit->overallOffset;
printf("OFFSET0: %d %d\n", DIEOffset, compileUnit->overallOffset);
    getCompileUnitName(self, self->elfInfo.dbg, compileUnit->compileUnitDie, compileUnit->overallOffset, &compileUnitShortName, &compileUnitLongName);
printf("compileUnitShortName: %s\n", compileUnitShortName);
    compileUnit->compileUnitShortName = ckalloc(strlen(compileUnitShortName) + 1);
    compileUnit->compileUnitShortName[strlen(compileUnitShortName)] = '\0';
    memcpy(compileUnit->compileUnitShortName, compileUnitShortName, strlen(compileUnitShortName));
#ifdef NOTDEF
// FIXME is that at all necessary??
    /*  Release the 'compileUnitDie' created by the call
        to 'dwarf_siblingof' at the top of the main loop. */
    dwarf_dealloc(self->elfInfo.dbg, compileUnit->compileUnitDie, DW_DLA_DIE);
    compileUnit->compileUnitDie = NULL; /* For debugging, stale die should be NULL. */

    sres = dwarf_siblingof_b(self->elfInfo.dbg, NULL, /* is_info */ 1, &compileUnit->compileUnitDie, &err);
printf("compileUnitDie2: %p shortName: %s sres: %d\n", compileUnit->compileUnitDie, compileUnit->compileUnitShortName, sres);
    dwarf_die_offsets(compileUnit->compileUnitDie, &compileUnit->overAllOffset, &DIEOffset, &err);
    DIEOverallOffset = compileUnit->overallOffset;
printf("OFFSET1: %d %d\n", DIEOffset, compileUnit->overallOffset);
#endif

    {
      int tres = 0;
      int ores = 0;
      int atres = 0;
      int res = 0;
      const char * tagName = 0;
      Dwarf_Half tag = 0;
      Dwarf_Off offset = 0;
      Dwarf_Signed atCnt = 0;
      Dwarf_Attribute *atList = 0;

      tres = dwarf_tag(compileUnit->compileUnitDie, &tag, &err);
      if (tres != DW_DLV_OK) {
        printf("accessing tag of die! tres: %d, err: %p", tres, err);
      }
      res = dwarf_get_TAG_name(tag, &tagName);
printf("tagname: %s\n", tagName);

#ifdef NOTDEF
      ores = dwarf_die_CU_offset(compileUnit->compileUnitDie, &offset, &err);
printf("OFFSET2: %d %d\n", offset, compileUnit->overallOffset);
      if (ores != DW_DLV_OK) {
        printf("dwarf_die_CU_offset ores: %d err: %p", ores, err);
      }
printf("dwarf_die_cu_offset: offset: %d\n", offset);
#endif

      atres = dwarf_attrlist(compileUnit->compileUnitDie, &atList, &atCnt, &err);
printf("atcnt: %d\n", atCnt);
      {
        int i;
        
        for (i = 0; i < atCnt; i++) {
          Dwarf_Half attr;
          int ares;

          ares = dwarf_whatattr(atList[i], &attr, &err);
          if (ares == DW_DLV_OK) {
            // NULL -> srcfiles 0 ->cnt
            result = getAttribute(self->elfInfo.dbg, compileUnit->compileUnitDie, compileUnit->overallOffset,
                 attr, atList[i],  NULL, 0);
            
          }
        }
      }
    }
  }
  return result;
}

// =================================== dwarfDbgGetInfos =========================== 

int dwarfDbgGetInfos(dwarfDbgPtr_t self) {
  uint8_t result;

printf("dwrafDbgGetInfos\n");
  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgGetInfo->handleOneDieSection(self);
  checkErrOK(result);

  return result;
}

// =================================== dwarfDbgGetInfoInit =========================== 

int dwarfDbgGetInfoInit (dwarfDbgPtr_t self) {
printf("dwarfDbgGetInfoInit self: %p\n", self);

  self->dwarfDbgGetInfo->maxCompileUnit = 0;
  self->dwarfDbgGetInfo->numCompileUnit = 0;
  self->dwarfDbgGetInfo->compileUnits = NULL;

  self->dwarfDbgGetInfo->addCompileUnit = &addCompileUnit;
  self->dwarfDbgGetInfo->handleOneDieSection = &handleOneDieSection;
  return DWARF_DBG_ERR_OK;
}
