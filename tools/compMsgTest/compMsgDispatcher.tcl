# ===========================================================================
# * Copyright (c) 2016, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
# * All rights reserved.
# *
# * License: BSD/MIT
# *
# * Redistribution and use in source and binary forms, with or without
# * modification, are permitted provided that the following conditions
# * are met:
# *
# * 1. Redistributions of source code must retain the above copyright
# * notice, this list of conditions and the following disclaimer.
# * 2. Redistributions in binary form must reproduce the above copyright
# * notice, this list of conditions and the following disclaimer in the
# * documentation and/or other materials provided with the distribution.
# * 3. Neither the name of the copyright holder nor the names of its
# * contributors may be used to endorse or promote products derived
# * from this software without specific prior written permission.
# *
# * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# * POSSIBILITY OF SUCH DAMAGE.
# *
# ==========================================================================

# compMsgDispatcher dict
#   prepareValuesCbName

#   currHdr
#   msgDescParts
#   msgValParts
#   compMsgMsgDesc
#   compMsgData
#   msgDescPart
#     fieldNameId
#     fieldTypeId
#     fieldKey
#     fieldSize
#     fieldSizeCallback
#     fieldNameStr
#     fieldTypeStr
#     fieldLgth
#   msgValPart
#     fieldNameId
#     fieldFlags
#     fieldKeyValueStr
#     fieldValue
#     fieldValueCallback
#     fieldNameStr
#     fieldValueStr



set ::DISP_HANDLE_PREFIX "stmsgdisp_"

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgDispatcher

  namespace eval compMsgDispatcher {
    namespace ensemble create
      
    namespace export compMsgDispatcherInit newCompMsgDispatcher
    namespace export setMsgValuesFromLines createDispatcher setMsgParts
    namespace export encryptMsg decryptMsg resetMsgInfo initDispatcher
    namespace export dumpMsgParts dumpHeaderParts dumpMsgHeaderInfos
    namespace export setSocketForAnswer

    variable compMsgDispatcher [dict create]
    variable compMsgDispatcherHandles
    variable compMsgData [list]
    variable numMsgHeaders 0
    variable maxMsgHeaders 4
    variable msgHeader2MsgPtrs [list]

    dict set compMsgDispatcher id 0
    dict set compMsgDispatcher handle [list]
    dict set compMsgDispatcher received [dict create]
    dict set compMsgDispatcher toSend [dict create]

    dict set compMsgDispatcherHandles handles [list]
    dict set compMsgDispatcherHandles nnumHandles 0

    # ============================= setMsgParts ========================
    
    proc setMsgParts {which part} {
      variable compMsgDispatcher

      switch $which {
        received -
        toSend {
          dict set compMsgDispatcher $which $parts
          return [checkErrOK OK]
        }
        default {
          checkErrOK BAD_VALUE
        }
      }
    }

    # ============================= addHandle ========================
    
    proc addHandle {handle} {
      variable compMsgDispatcher
      variable compMsgDispatcherHandles

      if {[dict get $compMsgDispatcherHandles handles] eq [list]} {
        set entry [dict create]
        dict set entry handle $handle
        dict set entry compMsgDispatcher $compMsgDispatcher
        dict lappend compMsgDispatcherHandles handles $entry
        dict incr compMsgDispatcherHandles numHandles 1
        return [checkErrOK OK]
      } else {
        # check for unused slot first
        set idx 0
        while {$idx < [dict get $compMsgDispatcherHandles numHandles]} {
          set handles [dict get $compMsgDispatcherHandles handles]
          set entry [lindex $handles $idx]
          if {[dict get $entry handle] eq [list]} {
            dict set entry handle $handle
            dict set entry compMsgDispatcher $compMsgDispatcher
            set handles [lreplace $handles $iidx $idx $entry
            dict set compMsgDispatcherHandles $handles
            return [checkErrOK OK]
          }
          incr idx
        }
        set handles [dict get $compMsgDispatcherHandles handles]
        set entry [dict create]
        dict set entry handle $handle
        dict set entry compMsgDispatcher $compMsgDispatcher
        dict lappend compMsgDispatcherHandles handles $entry
        dict incr compMsgDispatcherHandles numHandles 1
      }
      return [checkErrOK OK]
    }
    
    # ============================= deleteHandle ========================
    
    proc deleteHandle {handle} {
      variable compMsgDispatcher

      if {compMsgDispatcherHandles.handles == NULL} {
        checkErrOK HANDLE_NOT_FOUND
      }
      found = 0;
      idx = 0;
      numUsed = 0;
      while {idx < compMsgDispatcherHandles.numHandles} {
        if {(compMsgDispatcherHandles.handles[idx].handle != NULL} && (c_strcmp(compMsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
          compMsgDispatcherHandles.handles[idx].handle = NULL;
          found++;
        } else {
          if {compMsgDispatcherHandles.handles[idx].handle != NULL} {
            numUsed++;
          }
        }
        idx++;
      }
      if {numUsed == 0} {
        os_free {compMsgDispatcherHandles.handles};
        compMsgDispatcherHandles.handles = NULL;
      }
      if {found} {
          return [checkErrOK OK]
      }
      checkErrOK HANDLE_NOT_FOUND
    }
    
    # ============================= checkHandle ========================
    
    proc checkHandle {handle} {
      variable compMsgDispatcher
    
      if {compMsgDispatcherHandles.handles == NULL} {
        checkErrOK HANDLE_NOT_FOUND
      }
      idx = 0;
      while {idx < compMsgDispatcherHandles.numHandles} {
        if {(compMsgDispatcherHandles.handles[idx].handle != NULL} && (c_strcmp(compMsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
          *compMsgDispatcher = compMsgDispatcherHandles.handles[idx].compMsgDispatcher;
          return [checkErrOK OK]
        }
        idx++;
      }
      checkErrOK HANDLE_NOT_FOUND
    }
    
    # ================================= dumpMsgParts ====================================
    
    proc dumpMsgParts {msgParts} {
      variable compMsgDispatcher
    
      puts stderr "dumpMsgParts:\n"
      puts stderr [format "MsgParts1 form: 0x%04x to: 0x%04x totalLgth: %d u16_cmdLgth: %d u16CmdKey: 0x%04x targetPart: 0x%02x\n" [dict get $msgParts fromPart] [dict get $msgParts toPart] [dict get $msgParts totalLgth] [dict get $ msgParts u16CmdLgth] [dict get $msgParts u16CmdKey] [dict get $msgParts targetPart]]
    
      puts stderr [formaat "MsgParts2 u8CmdLgth: %d u8CmdKey: 0x%02x lgth: %d fieldOffset: %d\n" [dict get $msgParts u8CmdLgth] [dict get $msgParts u8CmdKey] [dict get $msgParts lgth] [dict get $msgParts fieldOffset]]
      puts stderr "buf"
      set idx 0
      while {$idx < [expr {[dict get $msgParts realLgth] - 1}]} {
        puts stderr [format " %d %s" $idx [lindex [dict get $msgParts buf] $idx]]
        incr idx
      }
      puts stderr "partFlags: [dict get $msgParts partsFlags]"
      return [checkErrOK OK]
    }

    # ================================= getMsgPtrFromMsgParts ====================================
    
    proc getMsgPtrFromMsgParts {msgParts incrRefCnt} {
      variable compMsgDispatcher
      variable compMsgData
      variable numMsgHeaders
      variable maxMsgHeaders
      variable msgHeader2MsgPtrs

      # build header from msgParts
      set offset 0
      set result [::compMsg dataView getDataViewData saveData saveLgth]
      set lgth [dict get $msgParts totalLgth]
puts stderr "compMsgDispatcher1 setDataViewData"
      ::compMsg dataView setDataViewData [string repeat "\x00" $lgth] $lgth
      set result [::compMsg dataView setUint16 $offset [dict get $msgParts fromPart]]
      checkErrOK $result
      incr offset 2
      set result [::compMsg dataView setUint16 $offset [dict get $msgParts toPart]]
      checkErrOK $result
      incr offset 2
      set result [::compMsg dataView setUint16 $offset [dict get $msgParts totalLgth]]
      checkErrOK $result
      incr offset 2
      # FIXME dispFlags !!
      if {[lsearch $::compMsg::compMsgIdentify::dispFlags COMP_MSG_U8_CMD_KEY] >= 0} {
        set result [::compMsg dataView setUint8 $offset [dict get $msgParts u8CmdKey]]
        checkErrOK $result
        incr offset 1
      } else {
        set result [::compMsg dataView setUint16 $offset [dict get $msgParts u16CmdKey]]
        checkErrOK $result
        incr offset 2
      }
      set result [::compMsg dataView getDataViewData header headerLgth]
      checkErrOK $result
puts stderr "compMsgDispatcher2 setDataViewData"
      set result [::compMsg dataView setDataViewData $saveData $saveLgth]
      checkErrOK $result
      # end build header from msgParts
      
      set firstFreeEntry [list]
      set firstFreeEntryId 0
      if {$numMsgHeaders > 0} {
        # find header 
        set headerIdx 0
        while {$headerIdx < $numMsgHeaders} {
          set headerEntry [lindex msgHeader2MsgPtrs $headerIdx]
          if {([dict get $headerEntry compMsgData] ne [list]) && ([string range [dict get $headerEntry header] 0 [expr {$offset - 1}]] eq $header)} {
            if {$incrRefCnt < 0} {
              dict set headerEntry headerLgth 0
              dict set headerEntry compMsgData [list]
              set msgHeader2MsgPtrs [lreplace $msgHeader2MsgPtrs $headerIdx $headerIdx $headerEntry]
              return [checkErrOK OK]
            }
            set compMsgData [dict get $headerEntry compMsgData]
# FIXME!!            dict set compMsgData flags [dict delete [dict get $compMsgData flags] COMP_MSG_IS_PREPARED]
            return [checkErrOK OK]
          }
          if {($incrRefCnt == $::COMP_MSG_INCR} && ([dict get $headerEntry compMsgData] eq [list]) && ($firstFreeEntry eq [list])) {
            set firstFreeEntry $headerEntry
            dict set firstFreeEntry compMsgData [dict create]
            dict set firstFreeEntry headerLgth $offset
            dict set firstFreeEntry header [string range $header 0 [expr {$offset - 1}]]
            set compMsgData [dict get $firstFreeEntry compMsgData]
            set msgHeader2MsgPtrs [lappend msgHeader2MsgPtrs $firstFreeEntry]
          }
          incr headerIdx
        }
      }
      if {$incrRefCnt < 0} {
        return [checkErrOK OK] ; # just ignore silently
      } else {
        if {$incrRefCnt == 0} {
          checkErrOK HEADER_NOT_FOUND;
        } else {
          if {$firstFreeEntry ne [list]} {
            set compMsgData [dict get $firstFreeEntry compMsgData]
            dict set firstFreeEntry headerLgth $offset
            dict set firstFreeEntry header [string range $header 0 [expr {$offset - 1}]]
          } else {
            set newHeaderEntry [dict create]
            dict set newHeaderEntry headerLgth $offset
            dict set newHeaderEntry header [string range $header 0 [expr {$offset - 1}]]
            set compMsg [dict create]
            dict set compMsg flags [list]
            dict set newHeaderEntry compMsgData $compMsg
            set compMsgData [dict get $newHeaderEntry compMsgData]
            incr numMsgHeaders
          }
        }
      }
puts stderr "compMsgDispatcher3 setDataViewData"
      set result [::compMsg dataView setDataViewData $saveData $saveLgth]
      return [checkErrOK OK]
    }
    
    # ================================= setMsgValuesFromLines ====================================
    
    proc setMsgValuesFromLines {fd numEntries handle type} {
      variable compMsgDispatcher
    
      set idx 0
      while {$idx < $numEntries} {
        gets $fd line
        set flds [split $line ","]
        foreach {fieldNameStr fieldValueStr} $flds break
        # fieldName
        set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldNameStr fieldNameId $::COMP_MSG_NO_INCR]
        checkErrOK $result
        set result [::compMsg compMsgData getFieldTypeFromFieldNameId $fieldNameId fieldTypeId]
        checkErrOK $result
    
        # fieldValue
        if {[string range $fieldValueStr 0 0] eq "@"} {
          # call the callback function vor the field!!
          set result [fillMsgValue $fieldValueStr value $type $fieldTypeId]
          checkErrOK $result
        } else {
          set value $fieldValueStr
        }
        switch $fieldNameId {
          COMP_MSG_SPEC_FIELD_DST {
            set value [dict get $compMsgDispatcher received fromPart]
            set result [::compMsg compMsgData setFieldValue $fieldNameStr $value]
          }
          COMP_MSG_SPEC_FIELD_SRC {
            set value [dict get $compMsgDispatcher received toPart]
            set result [::compMsg compMsgData setFieldValue $fieldNameStr $value]
          }
          COMP_MSG_SPEC_FIELD_CMD_KEY {
            # check for u8CmdKey/u16CmdKey here
            if {[lsearch [dict get $compMsgDispatcher dispFlags] COMP_MSG_U8_CMD_KEY] >= 0} {
              set value[dict get $compMsgDispatcher received u8CmdKey]
            } else {
              set value = [dict get $compMsgDispatcher received u16CmdKey]
            }
            set result [::compMsg compMsgData setFieldValue $fieldNameStr $value]
          }
          default {
            set result [::compMsg compMsgData setFieldValue $fieldNameStr $value]
          }
        }
        checkErrOK $result
        incr idx
      }
#      set result [::compMsg compMsgData setFieldValue "@cmdKey" $type]
      checkErrOK $result
      set result [::compMsg compMsgData prepareMsg]
#  ::compMsg compMsgData dumpMsg
      checkErrOK $result
      return [checkErrOK OK]
    }

    # ================================= resetMsgInfo ====================================
    
    proc resetMsgInfo {partsVar} {
      variable compMsgDispatcher

      set parts [dict create]
      dict set parts lgth 0
      dict set parts fieldOffset 0
      dict set parts fromPart 0
      dict set parts toPart 0
      dict set parts totalLgth 0
      dict set parts u16CmdLgth 0
      dict set parts u16CmdKey 0
      dict set parts u8CmdKey 0
      dict set parts u8CmdLgth 0
      dict set parts buf "" 
      dict set compMsgDispatcher $partsVar $parts
      return [checkErrOK OK]
    }
    
    # ============================= compMsgDispatcherGetPtrFromHandle ========================
    
    proc compMsgDispatcherGetPtrFromHandle {handle} {
      variablle compMsgDispatcher
    
      if {checkHandle(handle, compMsgDispatcher} != COMP_MSG_ERR_OK) {
        return COMP_MSG_ERR_HANDLE_NOT_FOUND;
      }
      return [checkErrOK OK]
    }
    
    # ================================= setSocketForAnswer ====================================
    
    proc setSocketForAnswer {comMsgDispatcherVar sock} {
      upvar $comMsgDispatcherVar compMsgDispatcher

      dict set compMsgDispatcher socketForAnswer $sock
      return [checkErrOK OK]
    }

    # ================================= initDispatcher ===============================
    
    proc initDispatcher {comMsgDispatcherVar} {
      upvar $comMsgDispatcherVar compMsgDispatcher

puts stderr "initDispatcher!$comMsgDispatcherVar!"
      set result [::compMsg compMsgTypesAndNames compMsgTypesAndNamesInit compMsgDispatcher]
      checkErrOK $result
      set result [::compMsg compMsgDataValue compMsgDataValueInit compMsgDispatcher]
      checkErrOK $result
      set result [::compMsg compMsgMsgDesc compMsgMsgDescInit compMsgDispatcher MsgFiles.txt]
      checkErrOK $result
      set result [::compMsg compMsgIdentify compMsgIdentifyInit compMsgDispatcher]
      checkErrOK $result
      set result [::compMsg compMsgSendReceive compMsgSendReceiveInit compMsgDispatcher]
      checkErrOK $result
      set result [::compMsg compMsgAction compMsgActionInit compMsgDispatcher]
      checkErrOK $result
      set result [::compMsg compMsgModuleData compMsgModuleDataInit compMsgDispatcher]
      checkErrOK $result
#      set result [::compMsg compMsgWebsocket compMsgWebsocketInit compMsgDispatcher]
#      checkErrOK $result
      return [checkErrOK OK]
    }
    
    # ================================= createDispatcher ====================================
    
    proc createDispatcher {handleVar} {
      variable dispatcher
      upvar $handleVar handle
    
      dict incr compMsgDispatcher id 1
      dict set compMsgDispatcher handle "${::DISP_HANDLE_PREFIX}efff00[format %02d [dict get $compMsgDispatcher id]]"
      set result [addHandle [dict get $compMsgDispatcher handle]]
      if {$result != $::COMP_MSG_ERR_OK} {
#        deleteHandle(self->handle)
        return $result;
      }
      resetMsgInfo received
      resetMsgInfo toSend
      set handle [dict get $compMsgDispatcher handle]
      return [checkErrOK OK]
    }

    # ================================= newCompMsgDispatcher ====================================
    
     proc newCompMsgDispatcher {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      dict set compMsgDispatcher numMsgHeaders 0
      dict set compMsgDispatcher maxMsgHeaders 0
      dict set compMsgDispatcher msgHeader2MsgPtrs [dict create]
    
      dict set compMsgDispatcher msgHeaderInfos headerFlags 0
      dict set compMsgDispatcher msgHeaderInfos headerParts [list]
      dict set compMsgDispatcher msgHeaderInfos numHeaderParts 0
      dict set compMsgDispatcher msgHeaderInfos maxHeaderParts 0
    
#      compMsgDispatcher->compMsgDataDescription = newCompMsgDataDescription{};
    
      return [checkErrOK OK]
    }
    
    # ================================= freeCompMsgDispatcher ====================================
    
    proc freeCompMsgDispatcher {} {
      variable compMsgDispatcher
    }

  } ; # namespace compMsgDispatcher
} ; # namespace compMsg
