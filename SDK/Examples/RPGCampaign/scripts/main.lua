-- ============================================================
-- Dungeon Campaign RPG Module for InfinityTable
-- GM tools, fog of war, initiative, character sheets
-- ============================================================

local GM_INDEX     = 1       -- Player index 1 is always GM
local CELL_SIZE    = 100     -- cm = 5ft
local VISION_RANGE = 600     -- cm = 30ft

-- Character data (indexed by player index)
local characters   = {}
local enemies      = {}
local tokens       = {}
local inCombat     = false
local combatRound  = 0
local mapLoaded    = false

-- ── Character Sheet ───────────────────────────────────────

local function defaultChar(name, playerIdx)
    return {
        name        = name,
        playerIdx   = playerIdx,
        class       = "Fighter",
        level       = 1,
        hp          = 10,
        maxHP       = 10,
        ac          = 14,
        speed       = 30,   -- ft
        initiative  = 0,
        str=15, dex=12, con=14, int=10, wis=13, cha=8,
        conditions  = {},
        token       = nil
    }
end

local function abilityMod(score)
    return math.floor((score - 10) / 2)
end

local function rollD20()
    return math.random(1, 20)
end

local function rollStat(char, stat)
    local base = char[stat] or 10
    return rollD20() + abilityMod(base)
end

-- ── Fog of War ────────────────────────────────────────────

local function revealAroundToken(token)
    if not token then return end
    local pos = token.getPosition()
    RPG.fogOfWar.revealCircle({ pos[1], pos[2], pos[3] }, VISION_RANGE)
end

local function onTokenMoved(player, obj, newPos)
    -- Find which character token was moved
    for _, char in ipairs(characters) do
        if char.token == obj then
            -- Reveal fog
            revealAroundToken(obj)

            -- Check for enemy proximity (within 150cm = 30ft)
            for _, enemy in ipairs(enemies) do
                if enemy.token then
                    local ep = enemy.token.getPosition()
                    local dx = ep[1] - newPos[1]
                    local dz = ep[3] - newPos[3]
                    local dist = math.sqrt(dx*dx + dz*dz)
                    if dist < 150 and not inCombat then
                        startCombat()
                    end
                end
            end

            -- Notify GM
            local cellX = math.floor(newPos[1] / CELL_SIZE + 0.5)
            local cellZ = math.floor(newPos[3] / CELL_SIZE + 0.5)
            Table.notifyPlayer(GM_INDEX,
                char.name .. " moved to cell [" .. cellX .. "," .. cellZ .. "]")
            return
        end
    end
end

-- ── Initiative & Combat ───────────────────────────────────

local function rollInitiative()
    local combatants = {}

    for _, char in ipairs(characters) do
        if char.playerIdx ~= GM_INDEX then
            char.initiative = rollD20() + abilityMod(char.dex)
            combatants[#combatants+1] = {
                name       = char.name,
                initiative = char.initiative,
                hp         = char.hp,
                maxHP      = char.maxHP,
                isPlayer   = true,
                token      = char.token
            }
        end
    end

    for _, enemy in ipairs(enemies) do
        enemy.initiative = rollD20() + abilityMod(enemy.dex or 10)
        combatants[#combatants+1] = {
            name       = enemy.name,
            initiative = enemy.initiative,
            hp         = enemy.hp,
            maxHP      = enemy.maxHP,
            isPlayer   = false,
            token      = enemy.token
        }
    end

    -- Sort descending
    table.sort(combatants, function(a,b) return a.initiative > b.initiative end)
    return combatants
end

local function startCombat()
    if inCombat then return end
    inCombat    = true
    combatRound = 1

    Table.broadcast("⚔️  COMBAT BEGINS!")
    local order = rollInitiative()
    RPG.initiative.setOrder(order)
    RPG.initiative.show(true)
    RPG.initiative.startCombat()

    local msg = "=== Initiative Order (Round 1) ===\n"
    for i, c in ipairs(order) do
        msg = msg .. i .. ". " .. c.name .. " [" .. c.initiative .. "]\n"
    end
    Table.broadcast(msg)
end

local function endCombat()
    inCombat = false
    RPG.initiative.endCombat()
    RPG.initiative.show(false)
    Table.broadcast("⚔️  Combat ended!")
end

-- ── Damage & Healing ──────────────────────────────────────

local function applyDamage(targetName, amount)
    -- Search characters
    for _, char in ipairs(characters) do
        if char.name == targetName then
            char.hp = math.max(0, char.hp - amount)
            local msg = targetName .. " takes " .. amount ..
                " damage! HP: " .. char.hp .. "/" .. char.maxHP
            Table.broadcast(msg)
            if char.hp == 0 then
                Table.broadcast(targetName .. " is DOWN!")
                char.conditions[#char.conditions+1] = "unconscious"
            end
            return
        end
    end
    -- Search enemies
    for i, enemy in ipairs(enemies) do
        if enemy.name == targetName then
            enemy.hp = math.max(0, enemy.hp - amount)
            local msg = targetName .. " takes " .. amount ..
                " damage! HP: " .. enemy.hp .. "/" .. enemy.maxHP
            Table.broadcast(msg)
            if enemy.hp <= 0 then
                Table.broadcast("💀 " .. targetName .. " defeated!")
                if enemy.token then enemy.token.destroy() end
                table.remove(enemies, i)
                -- Check if all enemies defeated
                if #enemies == 0 then endCombat() end
            end
            return
        end
    end
end

local function applyHeal(targetName, amount)
    for _, char in ipairs(characters) do
        if char.name == targetName then
            char.hp = math.min(char.maxHP, char.hp + amount)
            Table.broadcast("💚 " .. targetName .. " healed " .. amount ..
                " HP! (" .. char.hp .. "/" .. char.maxHP .. ")")
            return
        end
    end
end

-- ── Map Setup ─────────────────────────────────────────────

local function buildDungeonRoom(cx, cy, w, h)
    -- Floor tiles
    for x = cx, cx+w-1 do
        for y = cy, cy+h-1 do
            Objects.spawn("rpg_dungeon_campaign.dungeon_floor", {
                position = { x * CELL_SIZE, -5, y * CELL_SIZE },
                scale    = { 1, 0.1, 1 }
            })
        end
    end
    -- Walls
    for x = cx-1, cx+w do
        Objects.spawn("rpg_dungeon_campaign.dungeon_wall", {
            position = { x * CELL_SIZE, 0, (cy-1) * CELL_SIZE }
        })
        Objects.spawn("rpg_dungeon_campaign.dungeon_wall", {
            position = { x * CELL_SIZE, 0, (cy+h) * CELL_SIZE }
        })
    end
    for y = cy, cy+h-1 do
        Objects.spawn("rpg_dungeon_campaign.dungeon_wall", {
            position = { (cx-1) * CELL_SIZE, 0, y * CELL_SIZE }
        })
        Objects.spawn("rpg_dungeon_campaign.dungeon_wall", {
            position = { (cx+w) * CELL_SIZE, 0, y * CELL_SIZE }
        })
    end
end

local function placeEnemy(name, col, row, hp, dex)
    local pos = { col * CELL_SIZE, 2, row * CELL_SIZE }
    local obj = Objects.spawn("rpg_dungeon_campaign.enemy_token", {
        position = pos,
        color    = { 0.8, 0.1, 0.1 },
        label    = name
    })
    enemies[#enemies+1] = {
        name  = name,
        col   = col,
        row   = row,
        hp    = hp,
        maxHP = hp,
        dex   = dex or 10,
        token = obj
    }
end

local function loadMap()
    if mapLoaded then return end
    mapLoaded = true

    -- Room 1: Entrance hall (8x6)
    buildDungeonRoom(-4, -3, 8, 6)

    -- Room 2: Guard room (6x6)
    buildDungeonRoom(6, -3, 6, 6)

    -- Corridor connecting rooms
    for x = 4, 5 do
        for y = -1, 1 do
            Objects.spawn("rpg_dungeon_campaign.dungeon_floor", {
                position = { x * CELL_SIZE, -5, y * CELL_SIZE },
                scale    = { 1, 0.1, 1 }
            })
        end
    end

    -- Place enemies in guard room
    placeEnemy("Goblin Guard A", 7,  0, 7,  14)
    placeEnemy("Goblin Guard B", 10, 1, 7,  12)
    placeEnemy("Orc Sergeant",   9, -1, 15, 10)

    -- Place chest
    Objects.spawn("rpg_dungeon_campaign.chest_token", {
        position = { 11 * CELL_SIZE, 2, 2 * CELL_SIZE }
    })

    Table.broadcast("Map loaded! The party enters the dungeon entrance.")
end

local function placeHeroTokens()
    local players = Table.getPlayers()
    local startPositions = { {-3,0}, {-2,1}, {-2,-1}, {-3,-1} }

    for i, player in ipairs(players) do
        if player.index ~= GM_INDEX then
            local sp  = startPositions[i] or {0,0}
            local pos = { sp[1] * CELL_SIZE, 2, sp[2] * CELL_SIZE }
            local obj = Objects.spawn("rpg_dungeon_campaign.hero_token", {
                position = pos,
                color    = { 0.2, 0.5, 1.0 },
                label    = player.name
            })

            local char = defaultChar(player.name, player.index)
            char.token = obj
            characters[#characters+1] = char
        end
    end
end

-- ── GM commands (via chat /commands) ─────────────────────

local function handleChatCommand(player, message)
    if player.index ~= GM_INDEX then return false end

    local cmd, args = message:match("^/(%S+)%s*(.*)")
    if not cmd then return false end

    if cmd == "damage" then
        local target, amount = args:match("(%S+)%s+(%d+)")
        if target and amount then
            applyDamage(target, tonumber(amount))
            return true
        end
    elseif cmd == "heal" then
        local target, amount = args:match("(%S+)%s+(%d+)")
        if target and amount then
            applyHeal(target, tonumber(amount))
            return true
        end
    elseif cmd == "combat" then
        if args == "start" then startCombat()
        elseif args == "end" then endCombat() end
        return true
    elseif cmd == "reveal" then
        RPG.fogOfWar.revealAll()
        return true
    elseif cmd == "hide" then
        RPG.fogOfWar.hideAll()
        return true
    elseif cmd == "spawn" then
        local etype, col, row, hp = args:match("(%S+)%s+(%d+)%s+(%d+)%s*(%d*)")
        if etype and col and row then
            placeEnemy(etype, tonumber(col), tonumber(row), tonumber(hp) or 5)
            Table.broadcast("GM spawned " .. etype)
        end
        return true
    elseif cmd == "next" then
        RPG.initiative.next()
        return true
    elseif cmd == "help" then
        Table.notifyPlayer(GM_INDEX,
            "GM Commands:\n" ..
            "/damage <name> <amount>\n" ..
            "/heal <name> <amount>\n" ..
            "/combat start|end\n" ..
            "/reveal — reveal all fog\n" ..
            "/hide — hide all fog\n" ..
            "/spawn <type> <col> <row> [hp]\n" ..
            "/next — next initiative turn")
        return true
    end
    return false
end

-- ── Entry ─────────────────────────────────────────────────

local function onGameStart()
    -- Setup RPG systems
    RPG.grid.setType("square")
    RPG.grid.setCellSize(CELL_SIZE)
    RPG.grid.show(true)
    RPG.fogOfWar.enable(true)
    RPG.fogOfWar.hideAll()

    loadMap()
    placeHeroTokens()

    -- Reveal starting area
    RPG.fogOfWar.revealCircle({ 0, 0, 0 }, VISION_RANGE * 2)

    Table.broadcast(
        "🗡️  Welcome to Dungeon Campaign!\n" ..
        "Players: move your tokens to explore.\n" ..
        "GM: use /help for commands.")
end

Events.on("GameStart",    onGameStart)
Events.on("ObjectMoved",  onTokenMoved)
Events.on("ChatMessage",  function(player, msg)
    if msg:sub(1,1) == "/" then handleChatCommand(player, msg) end
end)
Events.on("TurnStart",    function(entry)
    Table.broadcast("⚔️  " .. entry.name .. "'s turn (Round " .. combatRound .. ")")
end)
