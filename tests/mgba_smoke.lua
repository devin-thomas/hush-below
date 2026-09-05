local state=assert(tonumber(assert(os.getenv("HUSH_BELOW_STATE")),16))
local offsets=assert(tonumber(assert(os.getenv("HUSH_BELOW_OFFSETS")),16))
local output=assert(os.getenv("HUSH_BELOW_OUTPUT"));local result_path=assert(os.getenv("HUSH_BELOW_RESULT"))
local function off(i)return emu:read32(offsets+i*4)end
local SCREEN,DIVE,MASK,TOTAL,FAILED,RADIUS,OXYGEN,HITS,PINGS=off(0),off(1),off(2),off(3),off(4),off(5),off(6),off(9),off(10)
local TITLE,BRIEF,PLAY,PAUSE,LOG,REPORT,ENDING=0,1,2,3,4,5,6
local function frames(n)for _=1,n do emu:runFrame()end end
local function tap(k)emu:addKey(k);frames(4);emu:clearKey(k);frames(3)end
local function expect(v,n)local a=emu:read8(state+SCREEN);if a~=v then error(n..": expected "..v.." got "..a)end end
local function shot(n)emu:screenshot(output.."/"..n..".png")end
local ok,err=xpcall(function()
 emu:setKeys(0);emu:reset();frames(120);expect(TITLE,"reset");if emu:getGameTitle()~="HUSH BELOW!" then error("wrong title")end;if emu:getGameCode()~="AGB-HSHB" then error("wrong code")end;shot("01-title")
 tap(0);expect(BRIEF,"brief");shot("02-dive-brief");tap(0);expect(PLAY,"play");frames(20);shot("03-silent-water")
 tap(0);expect(PLAY,"ping");frames(10);if emu:read8(state+RADIUS)==0 then error("ping did not expand")end;shot("04-sonar-ping")
 tap(3);expect(PAUSE,"pause");shot("05-pause");tap(3);expect(PLAY,"resume");tap(2);expect(LOG,"log");shot("06-dive-log")
 emu:write8(state+DIVE,2);emu:write8(state+MASK,7);emu:write8(state+TOTAL,9);emu:write8(state+FAILED,0);emu:write16(state+OXYGEN,2800);emu:write8(state+HITS,2);emu:write8(state+PINGS,12);emu:write8(state+SCREEN,REPORT);frames(8);shot("07-dive-report")
 emu:write8(state+SCREEN,ENDING);frames(8);shot("08-ending")
end,debug.traceback)
emu:setKeys(0);local temp=result_path..".tmp";os.remove(temp);local f=assert(io.open(temp,"w"));if ok then f:write("PASS\n")else f:write("FAIL\n",err,"\n")end;assert(f:close());assert(os.rename(temp,result_path))
