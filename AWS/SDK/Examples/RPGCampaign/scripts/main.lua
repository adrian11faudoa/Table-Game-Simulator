-- ============================================================
-- Dungeon Campaign RPG Module for InfinityTable
-- Fog of war, initiative tracker, GM commands, character stats
-- ============================================================
local GM_INDEX=1; local CELL=100; local VISION=600
local characters={}; local enemies={}; local inCombat=false; local combatRound=0

local function abilityMod(s) return math.floor((s-10)/2) end
local function rollD20() return math.random(1,20) end

local function defaultChar(name,idx)
    return {name=name,playerIdx=idx,class="Fighter",level=1,
            hp=10,maxHP=10,ac=14,speed=30,dex=12,str=15,con=14,int=10,wis=13,cha=8,
            conditions={},token=nil}
end

-- Fog reveal around a token
local function revealAround(token)
    if not token then return end
    local pos=token.getPosition()
    RPG.fogOfWar.revealCircle({pos[1],pos[2],pos[3]},VISION)
end

-- Start combat: roll initiative, show tracker
local function startCombat()
    if inCombat then return end
    inCombat=true; combatRound=1
    Table.broadcast("COMBAT BEGINS!")
    local combatants={}
    for _,c in ipairs(characters) do
        if c.playerIdx~=GM_INDEX then
            c.initiative=rollD20()+abilityMod(c.dex)
            combatants[#combatants+1]={name=c.name,initiative=c.initiative,hp=c.hp,maxHP=c.maxHP,isPlayer=true}
        end
    end
    for _,e in ipairs(enemies) do
        e.initiative=rollD20()+abilityMod(e.dex or 10)
        combatants[#combatants+1]={name=e.name,initiative=e.initiative,hp=e.hp,maxHP=e.maxHP,isPlayer=false}
    end
    table.sort(combatants,function(a,b) return a.initiative>b.initiative end)
    RPG.initiative.setOrder(combatants)
    RPG.initiative.show(true)
    RPG.initiative.startCombat()
    local msg="=== Initiative (Round 1) ===\n"
    for i,c in ipairs(combatants) do msg=msg..i..". "..c.name.." ["..c.initiative.."]\n" end
    Table.broadcast(msg)
end

local function endCombat()
    inCombat=false; RPG.initiative.endCombat(); RPG.initiative.show(false)
    Table.broadcast("Combat ended!")
end

local function applyDamage(name,amount)
    for _,c in ipairs(characters) do
        if c.name==name then
            c.hp=math.max(0,c.hp-amount)
            Table.broadcast(name.." takes "..amount.." dmg! HP: "..c.hp.."/"..c.maxHP)
            if c.hp==0 then Table.broadcast(name.." is DOWN!") end; return end end
    for i,e in ipairs(enemies) do
        if e.name==name then
            e.hp=math.max(0,e.hp-amount)
            Table.broadcast(name.." takes "..amount.." dmg! HP: "..e.hp.."/"..e.maxHP)
            if e.hp<=0 then
                Table.broadcast(name.." defeated!")
                if e.token then e.token.destroy() end
                table.remove(enemies,i)
                if #enemies==0 then endCombat() end
            end; return end end
end

local function applyHeal(name,amount)
    for _,c in ipairs(characters) do
        if c.name==name then
            c.hp=math.min(c.maxHP,c.hp+amount)
            Table.broadcast(name.." healed "..amount.." HP! ("..c.hp.."/"..c.maxHP..")"); return end end
end

local function placeEnemy(name,col,row,hp,dex)
    local pos={col*CELL,2,row*CELL}
    local obj=Objects.spawn("rpg_dungeon.enemy_token",{position=pos,label=name})
    enemies[#enemies+1]={name=name,col=col,row=row,hp=hp,maxHP=hp,dex=dex or 10,token=obj}
end

local function loadMap()
    -- Spawn dungeon floor tiles (simple 8x6 room)
    for x=-4,3 do for y=-3,2 do
        Objects.spawn("rpg_dungeon.dungeon_floor",{
            position={x*CELL,-5,y*CELL}, scale={1,0.1,1}})
    end end
    -- Guard room enemies
    placeEnemy("Goblin A",7,0,7,14); placeEnemy("Goblin B",10,1,7,12)
    placeEnemy("Orc Sgt",9,-1,15,10)
    Objects.spawn("rpg_dungeon.chest_token",{position={11*CELL,2,2*CELL}})
    Table.broadcast("Map loaded! The party enters the dungeon.")
end

local function placeHeroTokens()
    local allPlayers=Table.getPlayers() or {}
    local startPos={{-3,0},{-2,1},{-2,-1},{-3,-1}}
    for i,player in ipairs(allPlayers) do
        if player.index~=GM_INDEX then
            local sp=startPos[i] or {0,0}
            local obj=Objects.spawn("rpg_dungeon.hero_token",{
                position={sp[1]*CELL,2,sp[2]*CELL},label=player.name})
            local char=defaultChar(player.name,player.index); char.token=obj
            characters[#characters+1]=char
        end
    end
end

local function handleCommand(player,msg)
    if player.index~=GM_INDEX then return end
    local cmd,args=msg:match("^/(%S+)%s*(.*)")
    if not cmd then return end
    if cmd=="damage" then
        local t,a=args:match("(%S+)%s+(%d+)"); if t and a then applyDamage(t,tonumber(a)) end
    elseif cmd=="heal" then
        local t,a=args:match("(%S+)%s+(%d+)"); if t and a then applyHeal(t,tonumber(a)) end
    elseif cmd=="combat" then
        if args=="start" then startCombat() elseif args=="end" then endCombat() end
    elseif cmd=="reveal" then RPG.fogOfWar.revealAll()
    elseif cmd=="hide"   then RPG.fogOfWar.hideAll()
    elseif cmd=="next"   then RPG.initiative.next()
    elseif cmd=="help"   then
        Table.broadcast("/damage <name> <n>\n/heal <name> <n>\n/combat start|end\n/reveal\n/hide\n/next")
    end
end

local function onTokenMoved(player,obj,newPos)
    for _,char in ipairs(characters) do
        if char.token==obj then
            revealAround(obj)
            -- Check proximity to enemies
            for _,e in ipairs(enemies) do
                if e.token then
                    local ep=e.token.getPosition()
                    local dx=ep[1]-newPos[1]; local dz=ep[3]-newPos[3]
                    if math.sqrt(dx*dx+dz*dz)<150 and not inCombat then startCombat() end
                end
            end; return
        end
    end
end

local function onGameStart()
    RPG.grid.setType("square"); RPG.grid.setCellSize(CELL); RPG.grid.show(true)
    RPG.fogOfWar.enable(true); RPG.fogOfWar.hideAll()
    loadMap(); placeHeroTokens()
    RPG.fogOfWar.revealCircle({0,0,0},VISION*2)
    Table.broadcast("Dungeon Campaign! Players explore, GM uses /help for commands.")
end

Events.on("GameStart",   onGameStart)
Events.on("ObjectMoved", onTokenMoved)
Events.on("ChatMessage", function(player,msg) if msg and msg:sub(1,1)=="/" then handleCommand(player,msg) end end)
