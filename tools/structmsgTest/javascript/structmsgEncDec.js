/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg EncDec for Esp (structmsgEncDec.js)
 *
 * Part of this code is taken from:
 * http://yuilibrary.com/ YUI 3.3 version
 *
 * Released under BSD/MIT license.
 * (BSD license found at <http://www.tcl.tk/software/tcltk/license.html>)
 *
 * Arnulf P. Wiedemann arnulf@wiedemann-pri.de (C)2016
 *
 */

EM.addModule("Esp-structmsgEncDec", function(T, name) {

  /* ==================== structmsgEncDec constructor ======================= */

  function structmsgEncDec() {
    T.log('constructor called', 'info', 'structmsgEncDec', true);

    var encDec = this;
    var constructor = encDec.constructor;
    structmsgEncDec.superclass.constructor.apply(encDec, arguments);

    T.log('constructor end', 'info', 'structmsgEncDec', true);
  }

  T.extend(structmsgEncDec, T.Defines, {
    my_name: "structmsgEncDec",
    type_name: "encDec",
    flags: 0,
    
    /* ==================== toString ===================================== */
    
    toString: function () {
      var encdec = this;
      return encdec.mySelf()+"!";
    },
    
    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var encdec = this;
      var str = encdec.mySelf()+"\n";
      return str;
    },
    
    // ============================= normalFieldNamesDecode ========================
    
    normalFieldNamesDecode: function(data, offset) {
      return offset;
    },
    
    // ============================= structmsg_fillHdrInfo ========================
    
    structmsg_fillHdrInfo: function(handle, structmsg) {
      var hdrInfoPtr;
      var offset;
    
      // fill the hdrInfo
      structmsg = structmsg_get_structmsg_ptr(handle);
      hdrInfoPtr = structmsg.handleHdrInfoPtr;
      hdrInfoPtr = (structmsg.hdr.hdrInfo);
      offset = 0;
      offset = uint16Encode(hdrInfoPtr.hdrId, offset, structmsg.hdr.hdrInfo.hdrKeys.src);
      checkEncodeOffset(offset);
      offset = uint16Encode(hdrInfoPtr.hdrId, offset, structmsg.hdr.hdrInfo.hdrKeys.dst);
      checkEncodeOffset(offset);
      offset = uint16Encode(hdrInfoPtr.hdrId, offset, structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      checkEncodeOffset(offset);
      offset = uint16Encode(hdrInfoPtr.hdrId, offset, structmsg.hdr.hdrInfo.hdrKeys.cmdKey);
      checkEncodeOffset(offset);
      offset = uint16Encode(hdrInfoPtr.hdrId, offset, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
      checkEncodeOffset(offset);
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= stmsg_getEncoded ========================
    
    stmsg_getEncoded: function(handle, encoded, lgth) {
      var structmsg;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
      if (structmsg.encoded == NULL) {
        return STRUCT_MSG_ERR_NOT_ENCODED;
      }
      encoded = structmsg.encoded;
      lgth = structmsg.hdr.hdrInfo.hdrKeys.totalLgth;
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= getSpecFieldSizes ========================
    
    getSpecFieldSizes: function(numFields, namesSize) {
      numFields = 0;
      namesSize = 0;
      var entry = structmsgSpecialFieldNames[0];
      while (entry.str != NULL) {
        (numFields)++;
        namesSize += c_strlen(entry.str) + 1;  // +1 for "," as separator
        //ets_printf("%s: %d %d\n", entry.str, *numFields, *namesSize);
        entry++;
      }
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= structmsg_encodeDefinition ========================
    
    structmsg_encodeDefinition: function(name, data, lgth, structmsgDefinitions, fieldNameDefinitions) {
      var numSpecFields;
      var namesSpecSize;
      var numNormFields;
      var normNamesSize;
      var totalLgth;
      var cmdLgth;
      var cmdKey = STRUCT_MSG_DEFINITION_CMD_KEY;
      var fillerSize;
      var result;
      var idx;
      var found = 0;
      var definitionsIdx;
      var fieldName;
      var definitionPayloadSize;
      var payloadSize;
      var myLgth;
      var randomNum;
      var definition;
      var fieldInfo;
      var encoded;
      var offset;
        var crc;
      var normNamesOffsets;
      // FIXME!!
      var src = 123;
      var dst = 987;
    
      result =  structmsg_getDefinitionPtr(name, definition, definitionsIdx);
      checkErrOK(result);
      numNormFields = 0;
      normNamesSize = 0;
      idx = 0;
      while (idx < definition.numFields) {
        fieldInfo = definition.fieldInfos[idx];
        if (fieldInfo.fieldId < STRUCT_MSG_SPEC_FIELD_LOW) {
          result = getFieldIdName(fieldInfo.fieldId, fieldNameDefinitions, fieldName);
          checkErrOK(result);
    //ets_printf("fieldName: %s\n", fieldName);
          numNormFields++;
          normNamesSize += c_strlen(fieldName) + 1;
        }
        idx++;
      }
      //      normNamesOffsets = os_zalloc(numNormFields * sizeof(id2offset_t) + 1);
      checkAllocOK(normNamesOffsets);
      payloadSize = STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
      // randomNum
      payloadSize += sizeof(uint32_t);
      // len ids + ids (numNormFields * (uint16_t)) + len Names + names size
      payloadSize += sizeof(uint8_t) + (numNormFields * sizeof(uint16_t)) + sizeof(uint16_t) + normNamesSize;
      // definitionPayloadSize
    
      // definitionLgth + nameLgth + name of Definition
      definitionPayloadSize = sizeof(uint16_t) + sizeof(uint8_t) + (c_strlen(name) + 1);
      // numFields (uint8_t) + numFields * (fieldId uint16_t, fieldType uint8_t, fieldLgth uint16_t)
      definitionPayloadSize += sizeof(uint8_t) + definition.numFields * (sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t));
    
      payloadSize += definitionPayloadSize;
      fillerSize = 0;
      myLgth = payloadSize + sizeof(uint16_t); // sizeof(uint16_t) for CRC
      while ((myLgth % 16) != 0) {
        myLgth++;
        fillerSize++;
      }
      cmdLgth = payloadSize + fillerSize + sizeof(uint16_t);
      totalLgth = STRUCT_MSG_HEADER_LENGTH + cmdLgth;
      definition.totalLgth = totalLgth;
      //      definition.encoded = os_zalloc(totalLgth);
      checkAllocOK(definition.encoded);
      encoded = definition.encoded;
      offset = 0;
      offset = uint16Encode(encoded, offset, src); 
      checkEncodeOffset(offset);
      offset = uint16Encode(encoded, offset, dst); 
      checkEncodeOffset(offset);
      offset = uint16Encode(encoded, offset, totalLgth); 
      checkEncodeOffset(offset);
      offset = uint16Encode(encoded, offset, cmdKey); 
      checkEncodeOffset(offset);
      offset = uint16Encode(encoded, offset, cmdLgth); 
      checkEncodeOffset(offset);
      offset = randomNumEncode(encoded, offset, randomNum); 
      checkEncodeOffset(offset);
      offset = normalFieldNamesEncode(encoded, offset, definition, fieldNameDefinitions, normNamesOffsets, numNormFields, normNamesSize);
      offset = uint16Encode(encoded, offset, definitionPayloadSize); 
      checkEncodeOffset(offset);
      offset = uint8Encode(encoded, offset, c_strlen(name)+1); 
      checkEncodeOffset(offset);
      offset = uint8VectorEncode(encoded, offset, name, c_strlen(name)); 
      checkEncodeOffset(offset);
      offset = uint8Encode(encoded, offset, '\0'); 
      checkEncodeOffset(offset);
      offset = definitionEncode(encoded, offset, definition, fieldNameDefinitions, normNamesOffsets);
      checkEncodeOffset(offset);
      os_free(normNamesOffsets); // no longer needed
      var dummy;
      offset = fillerEncode(encoded, offset, fillerSize, dummy[0]);
      checkEncodeOffset(offset);
      offset = crcEncode(encoded, offset, totalLgth, crc, STRUCT_MSG_HEADER_LENGTH);
      checkEncodeOffset(offset);
      ets_printf("after crc offset: %d totalLgth :%d crc: 0x%04x\n", offset, totalLgth, crc);
      data = encoded;
      lgth = totalLgth;
      return STRUCT_MSG_ERR_OK;
    
    },
    
    // ============================= structmsg_decodeDefinition ========================
    
    structmsg_decodeDefinition: function(name, data, structmsgDefinitions, fieldNameDefinitions) {
      var numSpecFields;
      var namesSpecSize;
      var numNormFields;
      var namesNormSize;
      var fillerSize;
      var result;
      var idx;
      var found = 0;
      var fieldName;
      var definitionPayloadSize;
      var payloadSize;
      var myLgth;
      var definition;
      var fieldInfo;
      var offset;
      var crcOffset;
      var src;
      var dst;
      var totalLgth;
      var cmdKey;
      var cmdLgth;
      var crc;
      var randomNum;
      var fillerStr;
      var filler = fillerStr;;
      var numEntries;
    
      offset = 0;
      offset = uint16Decode(data, offset, src); 
      checkDecodeOffset(offset);
      offset = uint16Decode(data, offset, dst); 
      checkDecodeOffset(offset);
      offset = uint16Decode(data, offset, totalLgth); 
      checkDecodeOffset(offset);
      offset = uint16Decode(data, offset, cmdKey); 
      checkDecodeOffset(offset);
      if (cmdKey != STRUCT_MSG_DEFINITION_CMD_KEY) {
        return STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY;
      }
      offset = uint16Decode(data, offset, cmdLgth); 
      checkDecodeOffset(offset);
      offset = uint32Decode(data, offset, randomNum); 
      checkDecodeOffset(offset);
      // now check the crc
      crcOffset = totalLgth - sizeof(uint16_t);
      crcOffset = crcDecode(data, crcOffset, cmdLgth, crc, STRUCT_MSG_HEADER_LENGTH);
      offset = definitionDecode(data, offset, definition, fieldNameDefinitions);
      checkDecodeOffset(offset);
      myLgth = offset + sizeof(uint16_t);
      fillerSize = 0;
      while ((myLgth % 16) != 0) {
        myLgth++;
        fillerSize++;
      }
      offset = fillerDecode(data, offset, fillerSize, filler);
      checkDecodeOffset(offset);
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= stmsg_getDefinitionName ========================
    
    stmsg_getDefinitionName: function(decrypted, name) {
      var nameOffset;
      var numNormFields;
      var normNamesSize;
      var nameLgth;
      var numEntries;
    
      name = NULL;
      nameOffset = STRUCT_MSG_HEADER_LENGTH; // src + dst + totalLgth
      nameOffset += STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
      // randomNum
      nameOffset += sizeof(uint32_t);
      // len ids 
      nameOffset = uint8Decode(decrypted, nameOffset, numNormFields);
      // ids vector
      nameOffset += numNormFields * sizeof(uint16_t);
      // size of name strings (normnamesSize)
      nameOffset = uint16Decode(decrypted, nameOffset, normNamesSize);
      // names vector
      nameOffset += normNamesSize; 
      // definitionSize + nameLgth
      nameOffset += sizeof(uint16_t) + sizeof(uint8_t);
      // here the name starts
      name = decrypted + nameOffset;
      return STRUCT_MSG_ERR_OK;
    },
  
  });

  T.structmsgEncDec = structmsgEncDec;

  T.log("module: "+name+" initialised!", "info", "structmsgEncDec.js");
}, "0.0.1", {});
