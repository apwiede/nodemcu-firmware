#!/usr/bin/env tclsh8.6

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

source pdict.tcl
source structmsgCmd.tcl
source structmsgEncodeDecode.tcl
source structmsgDefinitions.tcl
source structmsgApi.tcl

structmsg def initFieldTypeDefines
structmsg def initSpecialFieldNames

proc checkErrOK {result} {
  if {$result ne "OK"} {
    puts stderr "result: $result!"
  }
}

source structmsgTest2.tcl

if {1} {
set def "pwd1"
set result [structmsg create_definition $def 15]
set result [structmsg add_fieldDefinition $def "@src" "uint16_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@dst" "uint16_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@totalLgth" "uint16_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@cmdKey" "uint16_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@cmdLgth" "uint16_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@randomNum" "uint32_t" 4]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@sequenceNum" "uint32_t" 4]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "pwd1" "uint8_t*" 16]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@tablerows" "uint8_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@tablerowfields" "uint8_t" 3]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "ssid1" "uint8_t*" 16]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "rssid1" "uint8_t*" 6]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "channel1" "uint8_t*" 6]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@filler" "uint8_t*" 1]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@crc" "uint16_t" 2]
checkErrOK $result
}
if {1} {
set def "pwd2"
puts stderr "++ $def START"
set result [structmsg create_definition "pwd2" 15]
set result [structmsg add_fieldDefinition $def "@src" "uint16_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@dst" "uint16_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@totalLgth" "uint16_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@cmdKey" "uint16_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@cmdLgth" "uint16_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@randomNum" "uint32_t" 4]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@sequenceNum" "uint32_t" 4]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "pwd2" "uint8_t*" 16]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@tablerows" "uint8_t" 2]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@tablerowfields" "uint8_t" 3]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "ssid1" "uint8_t*" 16]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "rssid1" "uint8_t*" 6]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "channel1" "uint8_t*" 6]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@filler" "uint8_t*" 1]
checkErrOK $result
set result [structmsg add_fieldDefinition $def "@crc" "uint16_t" 2]
checkErrOK $result
set result [structmsg dump_fieldDefinition $def]
checkErrOK $result
puts stderr "++ $def ENCODE FIELD DEFINITION"
set encoded [structmsg encode_fieldDefinition $def]
puts stderr "++ $def ENCODED"
set def pwd1
set result [structmsg delete_fieldDefinition $def]
checkErrOK $result
set def pwd2
set result [structmsg delete_fieldDefinition $def]
checkErrOK $result
set decoded [structmsg decode_fieldDefinition $def $encoded]
set result [structmsg dump_fieldDefinition $def]
checkErrOK $result
}

puts stderr "==createMsgFromDefinition"
set result [structmsg create_msgFromDefinition $def handle2]
checkErrOK $result
puts stderr "msgfromdef done"
puts stderr "HANDLE: $handle2!"
structmsg dump $handle2
puts stderr ">>encoded: $pwd1_encoded!"
set decoded [structmsg decode $handle2 $pwd1_encoded]
puts stderr "FILLED:"
structmsg dump $handle2
puts stderr "DEFINITIONS TEST DONE SUCCESSFULLY!!!"
set result [::structmsg get_definitionNormalFieldNames $def normalFieldNames]
checkErrOK $result
puts stderr "normalFieldNames: [join $normalFieldNames \n]!"
set result [::structmsg get_definitionTableFieldNames $def tableFieldNames]
checkErrOK $result
puts stderr "tableFieldNames: [join $tableFieldNames \n]!"
set result [::structmsg get_definitionNumTableRows $def numTableRows]
checkErrOK $result
puts stderr "numTableRows: $numTableRows!"
set result [::structmsg get_definitionNumTableRowFields $def numTableRowFields]
checkErrOK $result
puts stderr "numTableRowFields: $numTableRowFields!"
set result [::structmsg get_definitionFieldInfo $def pwd2 fieldInfo]
checkErrOK $result
puts stderr "fieldInfo: pwd2: $fieldInfo!"
set result [::structmsg get_definitionFieldInfo $def @totalLgth fieldInfo]
checkErrOK $result
puts stderr "fieldInfo: @totalLgth: $fieldInfo!"
set result [::structmsg get_fieldValue $handle2 @totalLgth value]
checkErrOK $result
puts stderr "@totalLgth: value $value!"
set result [::structmsg get_tableFieldValue $handle2 rssid1 1 value]
checkErrOK $result
puts stderr "rssid1 row 1: value $value!"


package require Tk
package require tablelist

namespace eval apwWin {
    variable ns [namespace current]

    namespace ensemble create

    namespace export CreateScrolledWidget Init CreateScrolledTablelist


    proc Init {} {
        variable infoWinNo
        variable xPosShowEntryBox
        variable yPosShowEntryBox

        set infoWinNo 1
        set xPosShowEntryBox -1
        set yPosShowEntryBox -1
    }

    proc CreateScrolledWidget { wType w useAutoScroll titleStr args } {
        variable ns
        variable sApw

        if { [winfo exists $w.par] } {
            destroy $w.par
        }
        ttk::frame $w.par
        pack $w.par -side top -fill both -expand 1
        if { $titleStr ne "" } {
            label $w.par.label -text "$titleStr" -anchor center
            set sApw(bgDefaultColor) [$w.par.label cget -background]
            set sApw(fgDefaultColor) [$w.par.label cget -foreground]
        }
        $wType $w.par.widget \
               -xscrollcommand "$w.par.xscroll set" \
               -yscrollcommand "$w.par.yscroll set" {*}$args
        ttk::scrollbar $w.par.xscroll -command "$w.par.widget xview" -orient horizontal
        ttk::scrollbar $w.par.yscroll -command "$w.par.widget yview" -orient vertical
        set rowNo 0
        if { $titleStr ne "" } {
            set rowNo 1
            grid $w.par.label -sticky ew -columnspan 2
        }
        grid $w.par.widget $w.par.yscroll -sticky news
        grid $w.par.xscroll               -sticky ew

        grid rowconfigure    $w.par $rowNo -weight 1
        grid columnconfigure $w.par 0      -weight 1

        bind $w.par.widget <MouseWheel>       "${ns}::MouseWheelCB $w.par.widget %D y"
        bind $w.par.widget <Shift-MouseWheel> "${ns}::MouseWheelCB $w.par.widget %D x"

        if { $useAutoScroll } {
            autoscroll::autoscroll $w.par.xscroll
            autoscroll::autoscroll $w.par.yscroll
        }

        return $w.par.widget
    }

    proc CreateScrolledTablelist { w useAutoScroll titleStr args } {
        return [CreateScrolledWidget tablelist::tablelist $w $useAutoScroll $titleStr {*}$args]
    }

}

namespace eval showDefinition {
    variable ns [namespace current]

    namespace ensemble create

    namespace export Init CreateScrolledTablelist

    # The following variables must be set, before reading parameters and
    # before calling LoadSettings.
    proc Init {} {
        variable sApw

        set sApw(tw)      ".apwDef" ; # Name of toplevel window
        set sApw(appName) "apwDef"  ; # Name of tool
        set sApw(cfgDir)  ""        ; # Directory containing config files

        set sApw(startDir) [pwd]
        set sApw(dir1)     $sApw(startDir)
        set sApw(dir2)     $sApw(startDir)

        set sApw(infoWinList) {}         ; # Start with empty file info window list
        set sApw(leftFile)    ""         ; # Left  file for selective diff
        set sApw(rightFile)   ""         ; # Right file for selective diff
        set sApw(curSession)  "Default"  ; # Default session name
        set sApw(sessionList) [list]
        set sApw(curListbox)  ""

        # Default values for command line options.
        set sApw(optSync)             false
        set sApw(optSyncDelete)       false
        set sApw(optCopyDate)         false
        set sApw(optCopyDays)         ""
        set sApw(optSearch)           false
        set sApw(optConvert)          false
        set sApw(optConvertFmt)       ""
        set sApw(optDiffOnStartup)    false
        set sApw(optSessionOnStartup) ""
        # Add command line options which should not be expanded by file matching.
#        apwApps AddFileMatchIgnoreOption "filematch"
    }

    proc CreateScrolledTablelist { fr title } {
        set id [apwWin CreateScrolledTablelist $fr true $title \
                    -columns {50 "col1"   "left"
                               0 "col2"   "right"
                               0 "col3" "right" } \
                    -exportselection false \
                    -stretch 0 \
                    -stripebackground #e0e8f0 \
                    -selectmode extended \
                    -labelcommand tablelist::sortByColumn \
                    -showseparators true]
        $id columnconfigure 0 -sortmode dictionary
        $id columnconfigure 1 -sortmode integer
        $id columnconfigure 2 -sortmode dictionary
        return $id
    }
}

source autoscroll.tcl

apwWin Init
showDefinition Init
frame .tbl -width 600 -height 100
set tableFr .tbl
set tableId [showDefinition CreateScrolledTablelist $tableFr showDefinition]
set lst [list]
set valueLst [list]
set rowLst [list]
set row 0
puts stderr "numTableRows: $numTableRows!"
while {$row < $numTableRows} {
  foreach name $tableFieldNames {
    structmsg get_definitionTableFieldInfo $def $name 0 fieldInfo
puts stderr "$name: $fieldInfo!"
    if {$row == 0} {
#      lappend lst [dict get $fieldInfo fieldLgth] $name
      lappend lst 0 $name
    }
    set result [structmsg get_tableFieldValue $handle2 $name $row value]
    lappend rowLst $value
  }
  lappend valueLst $rowLst
  set rowLst [list]
  incr row
}
$tableId configure -width 40
$tableId configure -columns $lst
puts stderr "valueLst: $valueLst!"
foreach rowLst $valueLst {
  $tableId insert end $rowLst
}
pack $tableFr
