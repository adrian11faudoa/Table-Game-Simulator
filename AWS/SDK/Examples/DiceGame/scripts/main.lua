-- ============================================================
-- Dice Battle for InfinityTable
-- Roll 2d6 per turn. Doubles = roll again. First to 50 wins.
-- ============================================================
local TARGET=50; local players={}; local scores={}
local rolls={}; local currentIdx=1; local round=1; local gameOver=false

local COLORS={{1,0.2,0.2},{0.2,0.5,1},{0.2,0.8,0.2},{1,0.8,0},{0.8,0.2,1},{1,0.5,0}}

local function nextTurn()
    rolls={}; currentIdx=currentIdx%#players+1
    if currentIdx==1 then round=round+1 end
    UI.showMessage(players[currentIdx].name.."'s turn! Roll your 2 dice.")
end

local function showScores()
    local msg="=== Scores (Round "..round..") ===\n"
    for i,p in ipairs(players) do
        msg=msg..p.name..": "..(scores[i] or 0)..(i==currentIdx and " <" or "")..("\n")
    end
    UI.showMessage(msg)
end

local function onDiceRolled(player,diceObj,result)
    if gameOver then return end
    local pIdx=nil
    for i,p in ipairs(players) do if p.name==player.name then pIdx=i; break end end
    if not pIdx or pIdx~=currentIdx then return end
    rolls[#rolls+1]=result
    if #rolls==1 then UI.showMessage(player.name.." rolled "..result.." - roll the second die!"); return end
    if #rolls==2 then
        local r1,r2=rolls[1],rolls[2]; local total=r1+r2; local doubles=(r1==r2)
        scores[pIdx]=(scores[pIdx] or 0)+total
        local msg=player.name.." rolled "..r1.."+"..r2.."="..total.."! Total: "..scores[pIdx]
        if doubles then msg=msg.." DOUBLES! Roll again!" end
        Table.broadcast(msg)
        if scores[pIdx]>=TARGET then
            Table.broadcast(player.name.." WINS with "..scores[pIdx].." points!")
            Game.endGame({winner=player,score=scores[pIdx]}); gameOver=true; return end
        if doubles then rolls={} else showScores(); nextTurn() end
    end
end

local function onGameStart()
    players=Table.getPlayers() or {}
    if #players<2 then UI.showMessage("Need at least 2 players!"); return end
    for i=1,#players do scores[i]=0 end
    for i,p in ipairs(players) do
        local c=COLORS[i] or {1,1,1}; local xOff=(i-1)*25-60
        local d1=Objects.spawn("d6",{position={xOff,0,80}})
        local d2=Objects.spawn("d6",{position={xOff+12,0,80}})
        if d1 then d1.setColor(c[1],c[2],c[3]) end
        if d2 then d2.setColor(c[1],c[2],c[3]) end
    end
    UI.showMessage("Dice Battle! First to "..TARGET.." wins. "..players[1].name.." goes first!")
end

Events.on("GameStart",  onGameStart)
Events.on("DiceRolled", onDiceRolled)
