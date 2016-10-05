function checkOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end

function writeMADescs()
  local handleMA=structmsg2.createDispatcher();
  result=structmsg2.openFile(handleMA,"DescMA.txt","w")
  checkOK(result,"open")  
  result=structmsg2.writeLine(handleMA, "#,18\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA, "@dst,uint16_t,2\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"@src,uint16_t,2\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"@totalLgth,uint16_t,2\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"@cmdKey,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"MacAddr,uint8_t*,6\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"IPAddr,uint8_t*,4\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"FirmVers,uint8_t*,6\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"SerieNum,uint8_t*,4\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"rssi,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"ModConn,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"DevMode,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"DevSec,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"ErrMain,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"ErrSub,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"date,uint8_t*,6\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"numSsid,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"Reserve,uint8_t*,3\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"@crc,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.closeFile(handleMA)
  checkOK(result,"close") 
end

McuPart=tonumber("4D00",16)
WifiPart=tonumber("5700",16)
MacAddr=tonumber("ABCD",16)..tonumber("EF12",16)..tonumber("3456")
IPAddr=tonumber("D4C3",16)..tonumber("1234",16)
FirmVers=tonumber("1234",16)..tonumber("56AB",16)..tonumber("CDEF",16)
SerieNum=tonumber("0213",16)..tonumber("2A10",16)
rssi=tonumber("8A",16)
ModConn=tonumber("41",16)
DevMode=tonumber("34",16)
DevSec=tonumber("00",16)
ErrMain=0
ErrSub=0
date=tonumber("0000",16)..tonumber("0000",16)..tonumber("0000",16)
numSsid=2



function writeMAVals()
  local handleMA=structmsg2.createDispatcher();
  result=structmsg2.openFile(handleMA,"ValMA.txt","w")
  checkOK(result,"open")  
  result=structmsg2.writeLine(handleMA, "#,15\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA, "@dst,"..McuPart.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"@src,"..WifiPart.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"MacAddr,"..MacAddr.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"IPAddr,"..IPAddr.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"FirmVers,"..FirmVers.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"SerieNum,"..SerieNum.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"rssi,"..rssi.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"ModConn,"..ModConn.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"DevMode,"..DevMode.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"DevSec,"..DevSec.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"ErrMain,"..ErrMain.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"ErrSub,"..ErrSub.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"date,"..date.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"numSsid,"..numSsid.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleMA,"Reserve,ABC\n")
  checkOK(result,"write")    
  result=structmsg2.closeFile(handleMA)
  checkOK(result,"close") 
end

writeMADescs()
writeMAVals()
print("done")
