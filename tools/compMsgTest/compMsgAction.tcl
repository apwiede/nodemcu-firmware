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

# actionName2Action dict
#   actionName
#   action
#   u16CmdKey
#   u8CmdKey
#   mode

# compMsgActionEntries dict
#   actionEntries
#   numActionEntries
#   maxActionEntries

# compMsgActions dict
#   actions
#   numActions
#   maxActions

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgAction

  namespace eval compMsgAction {
    namespace ensemble create
      
    namespace export getActionCallbackName 

      variable compMsgActionEntries
      variable compMsgActions

    # ================================= runClientMode ====================================
    
    proc runClientMode {compMsgDispatcherVar mode} {
      upvar $compMsgDispatcherVar cmdisp
    
      cmdisp websocketRunClientMode{cmdisp, mode};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= runAPMode ====================================
    
   proc runAPMode {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      cmdisp websocketRunAPMode{cmdisp};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= runLightSleepWakeupMode ====================================
    
    proc runLightSleepWakeupMode {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= runLightSleepNoWakeupMode ====================================
    
    proc runLightSleepNoWakeupMode {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= runWpsMode ====================================
    
    proc runWpsMode {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= runModulTestMode ====================================
    
    proc runModulTestMode {compMsgDispatcherBVar} {
      upvar $compMsgDispatcherVar cmdisp
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= runDeletePasswdCMode ====================================
    
    proc runDeletePasswdCMode {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
    ets_printf{"§runDeletePasswdC§"};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getMACAddr ====================================
    
    proc getMACAddr {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_MACAddr, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getIPAddr ====================================
    
    proc getIPAddr {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_IPAddr, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getFirmwareVersion ====================================
    
    proc getFirmwareVersion {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_FirmwareVersion, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getSerieNumber ====================================
    
    proc getSerieNumber {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_SerieNumber, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getRSSI ====================================
    
    proc getRSSI {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_RSSI, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getModuleConnection ====================================
    
    proc getModuleConnection {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_ModuleConnection, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getDeviceMode ====================================
    
    proc getDeviceMode {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_DeviceMode, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getDeviceSecurity ====================================
    
    proc getDeviceSecurity{compMsgDispatcher_t *cmdisp} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_DeviceSecurity, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getErrorMain ====================================
    
    proc getErrorMain {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_ErrorMain, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getErrorSub ====================================
    
    proc getErrorSub {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_ErrorSub, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getDateAndTime ====================================
    
    proc getDateAndTime {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_DateAndTime, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getSSIDs ====================================
    
    proc getSSIDs {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_SSIDs, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getReserve1 ====================================
    
    proc getReserve1 {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_Reserve1, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getReserve2 ====================================
    
    proc getReserve2 {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_Reserve2, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getReserve3 ====================================
    
    proc getReserve3 {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_Reserve3, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getGUID ====================================
    
    proc getGUID {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_GUID, DATA_VIEW_FIELD_UINT8_VECTOR};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getSrcId ====================================
    
    proc getSrcId {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleValue{cmdisp, MODULE_INFO_srcId, DATA_VIEW_FIELD_UINT16_T};
      return COMP_DISP_ERR_OK;
    }
    
    # ================================= getAPList ====================================
    
    proc getAPList {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getBssScanInfo{cmdisp};
      return result;
    }
    
    # ================================= getTableValue ====================================
    
    proc getTableValue {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getModuleTableFieldValue{cmdisp, MODULE_INFO_AP_LIST_CALL_BACK};
      return result;
    }
    
    # ================================= getWifiKeyValueInfos ====================================
    
    proc getWifiKeyValueInfos {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getWifiKeyValueInfo{cmdisp};
      return result;
    }
    
    # ================================= getWifiKeyValues ====================================
    
    proc getWifiKeyValues {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      set result [::compMsg cmdisp getWifiKeyValue{cmdisp};
      return result;
    }
    
    set actionName2Actions [dict create]
if {0} {
    dict set actionName2Actions runClientMode             runClientMode
             0, 0, 0 
    dict set actionName2Actions runAPMode                 runAPMode
                 0, 0, 0 
    dict set actionName2Actions runLightSleepWakeupMode   runLightSleepWakeupMode
   0, 0, 0 
    dict set actionName2Actions runLightSleepNoWakeupMode runLightSleepNoWakeupMode
 0, 0, 0 
    dict set actionName2Actions runWpsMode                runWpsMode
                0, 0, 0 
    dict set actionName2Actions runTestMode               runModulTestMode
          0, 0, 0 
    dict set actionName2Actions runDeletePasswdCMode      runDeletePasswdCMode      0, 0, 0 
    dict set actionName2Actions getMACAddr                getMACAddr                0, 0, 0 
    dict set actionName2Actions getIPAddr                 getIPAddr                 0, 0, 0 
    dict set actionName2Actions getFirmwareVersion        getFirmwareVersion        0, 0, 0 
    dict set actionName2Actions getSerieNumber            getSerieNumber            0, 0, 0 
    dict set actionName2Actions getRSSI                   getRSSI                   0, 0, 0 
    dict set actionName2Actions getModuleConnection       getModuleConnection       0, 0, 0 
    dict set actionName2Actions getDeviceMode             getDeviceMode             0, 0, 0 
    dict set actionName2Actions getDeviceSecurity         getDeviceSecurity         0, 0, 0 
    dict set actionName2Actions getErrorMain              getErrorMain              0, 0, 0 
    dict set actionName2Actions getErrorSub               getErrorSub               0, 0, 0 
    dict set actionName2Actions getDateAndTime            getDateAndTime            0, 0, 0 
    dict set actionName2Actions getSSIDs                  getSSIDs                  0, 0, 0 
    dict set actionName2Actions getReserve1               getReserve1               0, 0, 0 
    dict set actionName2Actions getReserve2               getReserve2               0, 0, 0 
    dict set actionName2Actions getReserve3               getReserve3               0, 0, 0 
    dict set actionName2Actions getGUID                   getGUID                   0, 0, 0 
    dict set actionName2Actions getSrcId                  getSrcId                  0, 0, 0 
    dict set actionName2Actions getAPList                 getAPList                 0, 0, MODULE_INFO_AP_LIST_CALL_BACK 
    dict set actionName2Actions getTableValue             getTableValue             0x4141, 0, MODULE_INFO_AP_LIST_CALL_BACK 
    dict set actionName2Actions getWifiKeyValueInfos      getWifiKeyValueInfos      0x4141, 0, 8 
    dict set actionName2Actions getWifiKeyValues          getWifiKeyValues          0x4141, 0, 8 
}
    
    # ================================= getActionMode ====================================
    
    proc getActionMode {compMsgDispatcherVar actionNam,actionMode} {
      upvar $compMsgDispatcherVar cmdisp
      actionName2Action_t *actionEntry;
      int idx;
    
      idx = 0;
      actionEntry = &actionName2Actions[idx];
      while {actionEntry->actionName != NULL} { 
        if {c_strcmp{actionEntry->actionName, actionName} == 0} {
          *actionMode = actionEntry->mode;
    ets_printf{"actionMode: %d\n", *actionMode};
          return COMP_DISP_ERR_OK;
        }
        idx++;
        actionEntry = &actionName2Actions[idx];
      }
      return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
    }
    
    # ================================= getActionCallback ====================================
    
    proc getActionCallback {compMsgDispatcherVar actionName callback} {
      upvar $compMsgDispatcherVar cmdisp
    
      idx = 0;
      actionEntry = &actionName2Actions[idx];
      while {actionEntry->actionName != NULL} { 
        if {c_strcmp{actionEntry->actionName, actionName} == 0} {
          *callback = actionEntry->action;
          return COMP_DISP_ERR_OK;
        }
        idx++;
        actionEntry = &actionName2Actions[idx];
      }
      return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
    }
    
    # ================================= getActionCallbackName ====================================
    
    proc getActionCallbackName {compMsgDispatcherVar callback actionNameVar} {
      upvar $compMsgDispatcherVar cmdisp
      upvar $actionNameVar actionName
    
      if {[string range $callback 0 0] eq "@"} {
        set actionName [string range $callback 1 end]
      } else {
        set actionName $callback
      }
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= setActionEntry ====================================
    
    proc setActionEntry {compMsgDispatcherVar actionName mode u8CmdKey u16CmdKey} {
      upvar $compMsgDispatcherVar cmdisp
    
      if {compMsgActionEntries.numActionEntries >= compMsgActionEntries.maxActionEntries} {
        compMsgActionEntries.maxActionEntries += 5;
        compMsgActionEntries.actionEntries = {actionName2Action_t **}os_realloc{compMsgActionEntries.actionEntries, {compMsgActionEntries.maxActionEntries * sizeof{actionName2Action_t *}}};
        checkAllocOK{compMsgActionEntries.actionEntries};
      }
      idx = 0;
      actionEntry = &actionName2Actions[idx];
      while {actionEntry->actionName != NULL} { 
        if {c_strcmp{actionEntry->actionName, actionName} == 0} {
          compMsgActionEntries.actionEntries[compMsgActionEntries.numActionEntries] = actionEntry;
          if {actionEntry->mode != 0} {
            return COMP_DISP_ERR_DUPLICATE_ENTRY;
          }
          actionEntry->mode = mode;
          actionEntry->u8CmdKey = u8CmdKey;
          actionEntry->u16CmdKey = u16CmdKey;
          compMsgActionEntries.numActionEntries++;
          return COMP_DISP_ERR_OK;
        }
        idx++;
        actionEntry = &actionName2Actions[idx];
      }
      return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
    }
    
    # ================================= runAction ====================================
    
    proc runAction {compMsgDispatcherVar answerType} {
      upvar $compMsgDispatcherVar cmdisp
    
      received = &cmdisp received;
      dataView = cmdisp compMsgDataView->dataView;
      if {received->u16CmdKey == 0x4244} { # "BD"
        # FIXME need to get the real offset here instead of 7!!
        set result [::compMsg dataView->getUint8{dataView, 7, &actionMode};
        checkErrOK{result};
        idx = 0;
        actionEntry = &actionName2Actions[idx];
        while {actionEntry->actionName != NULL} { 
    #ets_printf{"§runActionBu8!%s!%c!%c!%c!§", actionEntry->actionName, {received->u16CmdKey>>8}&0xFF, received->u16CmdKey&0xFF, actionMode};
          if {{actionEntry->u16CmdKey == received->u16CmdKey} && {actionMode == actionEntry->mode}} {
    ets_printf{"§runAction!%s!%d!§", actionEntry->actionName, actionEntry->mode};
            set result [::compMsg actionEntry->action{cmdisp};
            checkErrOK{result};
            return COMP_DISP_ERR_OK;
          }
          idx++;
          actionEntry = &actionName2Actions[idx];
        }
        return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
      } else {
    #ets_printf{"§runAction u16!%c%c!%c!§\n", {received->u16CmdKey>>8}&0xFF, received->u16CmdKey&0xFF, *answerType};
        dataView = cmdisp compMsgDataView->dataView;
        switch {cmdisp actionMode} {
        case 8:
        case MODULE_INFO_AP_LIST_CALL_BACK:
          idx = 0;
          actionEntry = &actionName2Actions[idx];
          while {actionEntry->actionName != NULL} { 
    #ets_printf{"an2: %s am: %d %d\n", actionEntry->actionName, actionMode, actionEntry->mode};
            if {cmdisp actionMode == actionEntry->mode} {
    #ets_printf{"§runAction2 G!%d!%c!§\n", cmdisp actionMode, *answerType};
    ets_printf{"§runAction!%s!%d!§", actionEntry->actionName, actionEntry->mode};
              set result [::compMsg actionEntry->action{cmdisp};
              checkErrOK{result};
              return COMP_DISP_ERR_OK;
            }
            idx++;
            actionEntry = &actionName2Actions[idx];
          }
          return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
          break;
        }
      }
      return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
    }
    
    # ================================= fillMsgValue ====================================
    
    proc fillMsgValue {compMsgDispatcherVar callbackName answerType fieldTypeId} {
      upvar $compMsgDispatcherVar cmdisp
    
      # skip the '@' char!
      callbackName++;
      idx = 0;
      actionEntry = &actionName2Actions[idx];
      while {actionEntry->actionName != NULL} { 
        if {c_strcmp{actionEntry->actionName, callbackName} == 0} {
          set result [::compMsg actionEntry->action{cmdisp};
          checkErrOK{result};
          return COMP_DISP_ERR_OK;
        }
        idx++;
        actionEntry = &actionName2Actions[idx];
      }
      return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
    }
    
    # ================================= compMsgActionInit ====================================
    
    proc compMsgActionInit {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar cmdisp
    
      compMsgActionEntries.numActionEntries = 0;
      compMsgActionEntries.maxActionEntries = 10;
      compMsgActionEntries.actionEntries = {actionName2Action_t **}os_zalloc{compMsgActionEntries.maxActionEntries * sizeof{actionName2Action_t *}};
      checkAllocOK{compMsgActionEntries.actionEntries};
    
      compMsgActions.numActions = 0;
      compMsgActions.maxActions = 10;
      compMsgActions.actions = {actionName2Action_t **}os_zalloc{compMsgActions.maxActions * sizeof{actionName2Action_t  **}};
      checkAllocOK{compMsgActions.actions};
    
      set result [::compMsg cmdisp compMsgMsgDesc->readActions{cmdisp, COMP_MSG_ACTIONS_FILE_NAME};
      return result;
    }

  } ; # namespace compMsgAction
} ; # namespace compMsg
