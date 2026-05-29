-- ============================================================
-- Dice Battle for InfinityTable
-- Each player rolls 2d6 per turn. Accumulate points.
-- First to TARGET_SCORE wins. Rolling doubles = roll again!
-- ============================================================

local TARGET_SCORE = 50
local players      = {}
local scores       = {}
local rollsThisTurn = {}
local currentPlayerIdx = 1
local round        = 1
local gameOver     = false
local dice         = {}
local waitingForSecondRoll = false

-- Spawn dice for each player color
local PLAYER_COLORS = {
    {1,0.2,0.2},  -- red
    {0.2,0.5,1},  -- blue
    {0.2,0.8,0.2},-- green
    {1,0.8,0},    -- yellow
    {0.8,0.2,1},  -- purple
    {1,0.5,0},    -- orange
}

local function spawnPlayerDice(idx)
    local xOffset = (idx - 1) * 25 - 60
    local d1 = Objects.spawn("d6", { position = { xOffset,    0, 80 } })
    local d2 = Objects.spawn("d6", { position = { xOffset+10, 0, 80 } })
    local c  = PLAYER_COLORS[idx] or {1,1,1}
    d1.setColor(c[1], c[2], c[3])
    d2.setColor(c[1], c[2], c[3])
    return { d1, d2 }
end

local function showScoreboard()
    local msg = "=== SCORES (Round " .. round .. ") ===\n"
    for i, p in ipairs(players) do
        local arrow = (i == currentPlayerIdx) and " ◄" or ""
        msg = msg .. p.name .. ": " .. (scores[i] or 0) .. arrow .. "\n"
    end
    UI.showMessage(msg)
end

local function nextTurn()
    rollsThisTurn = {}
    waitingForSecondRoll = false

    repeat
        currentPlayerIdx = currentPlayerIdx % #players + 1
        if currentPlayerIdx == 1 then round = round + 1 end
    until true  -- Extend: skip eliminated players here

    local p = players[currentPlayerIdx]
    UI.showMessage(p.name .. "'s turn! Roll your dice.")
    Events.emit("TurnStart", p)
end

local function checkWin(playerIdx)
    if scores[playerIdx] >= TARGET_SCORE then
        local winner = players[playerIdx]
        Table.broadcast("🎲 " .. winner.name .. " wins with " ..
            scores[playerIdx] .. " points!")
        Game.endGame({ winner = winner, score = scores[playerIdx], rounds = round })
        gameOver = true
        return true
    end
    return false
end

local function isPlayerDie(obj, playerIdx)
    if not dice[playerIdx] then return false end
    return obj == dice[playerIdx][1] or obj == dice[playerIdx][2]
end

local function onDiceRolled(player, diceObj, result)
    if gameOver then return end

    -- Find which player this is
    local pIdx = nil
    for i, p in ipairs(players) do
        if p.name == player.name then pIdx = i; break end
    end
    if not pIdx then return end

    -- Only current player's dice count
    if pIdx ~= currentPlayerIdx then
        UI.showMessage("Wait for your turn!")
        return
    end

    -- Collect roll
    rollsThisTurn[#rollsThisTurn+1] = result

    if #rollsThisTurn == 1 then
        UI.showMessage(player.name .. " rolled " .. result .. " — roll the second die!")
        return
    end

    if #rollsThisTurn == 2 then
        local r1, r2  = rollsThisTurn[1], rollsThisTurn[2]
        local total   = r1 + r2
        local doubles = (r1 == r2)

        scores[pIdx]  = scores[pIdx] + total

        local msg = player.name .. " rolled " .. r1 .. " + " .. r2 ..
                    " = " .. total .. "! (Total: " .. scores[pIdx] .. ")"
        if doubles then msg = msg .. " DOUBLES! Roll again!" end
        Table.broadcast(msg)

        if checkWin(pIdx) then return end

        if doubles then
            rollsThisTurn = {}
            UI.showMessage("Doubles! " .. player.name .. " rolls again.")
        else
            showScoreboard()
            nextTurn()
        end
    end
end

local function onGameStart()
    players = Table.getPlayers()
    if #players < 2 then
        UI.showMessage("Dice Battle needs at least 2 players!")
        return
    end

    scores = {}
    for i = 1, #players do scores[i] = 0 end

    -- Spawn dice per player
    dice = {}
    for i, p in ipairs(players) do
        dice[i] = spawnPlayerDice(i)
    end

    -- Spawn score tokens
    for i, p in ipairs(players) do
        local xOff = (i-1)*15 - (#players*7)
        Objects.spawn("token", {
            position = { xOff, 0, -80 },
            color    = PLAYER_COLORS[i] or {1,1,1},
            label    = p.name .. ": 0"
        })
    end

    UI.showMessage("Dice Battle! First to " .. TARGET_SCORE ..
        " wins. " .. players[1].name .. " goes first!")
    Events.emit("TurnStart", players[1])
end

Events.on("GameStart",  onGameStart)
Events.on("DiceRolled", onDiceRolled)
