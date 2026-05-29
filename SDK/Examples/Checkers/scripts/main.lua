-- ============================================================
-- Classic Checkers for InfinityTable
-- 8x8 board, forced captures, king promotion
-- ============================================================

local BOARD_SIZE  = 8
local pieces      = {}
local board       = {}
local currentTurn = "red"
local gameOver    = false
local moveCount   = 0

local function cellToWorld(col, row)
    return { (col - 4.5) * 10, 0, (row - 4.5) * 10 }
end

local function initBoard()
    for c = 1, BOARD_SIZE do
        board[c] = {}
        for r = 1, BOARD_SIZE do board[c][r] = nil end
    end
end

local function spawnPiece(color, col, row)
    local pos  = cellToWorld(col, row)
    local obj  = Objects.spawn("checkers_classic.piece_" .. color, {
        position = { pos[1], pos[2], pos[3] + 1 }
    })
    local p = { obj=obj, color=color, col=col, row=row, isKing=false }
    pieces[#pieces+1] = p
    board[col][row] = #pieces
    return p
end

local function setupBoard()
    initBoard()
    pieces = {}
    -- Black pieces: rows 6-8 on dark squares
    for row = 6, 8 do
        for col = 1, BOARD_SIZE do
            if (col + row) % 2 == 0 then spawnPiece("black", col, row) end
        end
    end
    -- Red pieces: rows 1-3 on dark squares
    for row = 1, 3 do
        for col = 1, BOARD_SIZE do
            if (col + row) % 2 == 0 then spawnPiece("red", col, row) end
        end
    end
    -- Board
    Objects.spawn("checkers_classic.checkers_board", { position = {0, -0.5, 0} })
    UI.showMessage("Checkers! Red moves first.")
end

local function pieceAt(col, row)
    if col < 1 or col > 8 or row < 1 or row > 8 then return nil end
    local idx = board[col][row]
    return idx and pieces[idx] or nil
end

local function getMoves(piece)
    local moves = {}
    local dirs  = {}
    if piece.color == "red" or piece.isKing then
        dirs[#dirs+1] = { 1, 1}
        dirs[#dirs+1] = {-1, 1}
    end
    if piece.color == "black" or piece.isKing then
        dirs[#dirs+1] = { 1,-1}
        dirs[#dirs+1] = {-1,-1}
    end

    for _, d in ipairs(dirs) do
        local nc, nr = piece.col + d[1], piece.row + d[2]
        if nc >= 1 and nc <= 8 and nr >= 1 and nr <= 8 then
            if not pieceAt(nc, nr) then
                moves[#moves+1] = { col=nc, row=nr, capture=nil }
            elseif pieceAt(nc,nr).color ~= piece.color then
                local jc, jr = nc + d[1], nr + d[2]
                if jc>=1 and jc<=8 and jr>=1 and jr<=8 and not pieceAt(jc,jr) then
                    moves[#moves+1] = { col=jc, row=jr, capture=pieceAt(nc,nr) }
                end
            end
        end
    end
    return moves
end

local function hasForcedCapture(color)
    for _, p in ipairs(pieces) do
        if p.color == color then
            for _, m in ipairs(getMoves(p)) do
                if m.capture then return true end
            end
        end
    end
    return false
end

local function checkWin()
    local redCount, blackCount = 0, 0
    for _, p in ipairs(pieces) do
        if p.color == "red"   then redCount   = redCount   + 1 end
        if p.color == "black" then blackCount = blackCount + 1 end
    end
    if redCount == 0 then
        Table.broadcast("⬛ Black wins!")
        Game.endGame({ winner = "black" })
        gameOver = true
        return true
    elseif blackCount == 0 then
        Table.broadcast("🔴 Red wins!")
        Game.endGame({ winner = "red" })
        gameOver = true
        return true
    end
    return false
end

local function promote(piece)
    if piece.isKing then return end
    if (piece.color == "red" and piece.row == 8) or
       (piece.color == "black" and piece.row == 1) then
        piece.isKing = true
        local pos = cellToWorld(piece.col, piece.row)
        piece.obj.destroy()
        piece.obj = Objects.spawn("checkers_classic.king_" .. piece.color, {
            position = { pos[1], pos[2], pos[3]+1 }
        })
        UI.showMessage(piece.color:upper() .. " piece kinged!")
    end
end

local function findPieceByObj(obj)
    for _, p in ipairs(pieces) do
        if p.obj == obj then return p end
    end
    return nil
end

local function onObjectMoved(player, obj, newPos)
    if gameOver then return end

    local piece = findPieceByObj(obj)
    if not piece then return end
    if piece.color ~= currentTurn then
        local pos = cellToWorld(piece.col, piece.row)
        piece.obj.setPosition(pos[1], pos[2], pos[3]+1)
        UI.showMessage("It's " .. currentTurn:upper() .. "'s turn!")
        return
    end

    local col = math.floor(newPos[1] / 10 + 4.5 + 0.5)
    local row = math.floor(newPos[3] / 10 + 4.5 + 0.5)
    col = math.max(1, math.min(8, col))
    row = math.max(1, math.min(8, row))

    local legalMoves = getMoves(piece)
    local forced     = hasForcedCapture(currentTurn)

    for _, m in ipairs(legalMoves) do
        if m.col == col and m.row == row then
            if forced and not m.capture then
                local pos = cellToWorld(piece.col, piece.row)
                piece.obj.setPosition(pos[1], pos[2], pos[3]+1)
                UI.showMessage("Forced capture available!")
                return
            end

            -- Apply move
            board[piece.col][piece.row] = nil
            if m.capture then
                board[m.capture.col][m.capture.row] = nil
                m.capture.obj.destroy()
                -- Remove from pieces
                for i = #pieces, 1, -1 do
                    if pieces[i] == m.capture then table.remove(pieces, i); break end
                end
            end

            piece.col = col
            piece.row = row
            local npos = cellToWorld(col, row)
            piece.obj.setPosition(npos[1], npos[2], npos[3]+1)

            for i, p in ipairs(pieces) do
                if p == piece then board[col][row] = i; break end
            end

            promote(piece)
            if checkWin() then return end

            moveCount = moveCount + 1
            currentTurn = (currentTurn == "red") and "black" or "red"
            UI.showMessage("Move " .. moveCount .. " — " .. currentTurn:upper() .. "'s turn")
            return
        end
    end

    -- Illegal — snap back
    local pos = cellToWorld(piece.col, piece.row)
    piece.obj.setPosition(pos[1], pos[2], pos[3]+1)
    UI.showMessage("Illegal move!")
end

Events.on("GameStart",   function() setupBoard() end)
Events.on("ObjectMoved", onObjectMoved)
