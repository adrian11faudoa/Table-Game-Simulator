-- ============================================================
-- Classic Chess for InfinityTable
-- Full 8x8 chess with turn enforcement and basic move rules
-- ============================================================

local BOARD_SIZE = 8
local pieces     = {}        -- { obj, type, color, col, row, moved }
local board      = {}        -- board[col][row] = piece index or nil
local currentTurn = "white"
local selectedPiece = nil
local gameOver    = false
local capturedWhite = {}
local capturedBlack = {}
local moveCount   = 0

-- ── Helpers ───────────────────────────────────────────────

local function cellToWorld(col, row)
    return { (col - 4.5) * 12, 0, (row - 4.5) * 12 }
end

local function initBoard()
    for c = 1, BOARD_SIZE do
        board[c] = {}
        for r = 1, BOARD_SIZE do
            board[c][r] = nil
        end
    end
end

local function placePiece(typeID, color, col, row)
    local pos = cellToWorld(col, row)
    local obj = Objects.spawn("chess_classic." .. typeID .. "_" .. color, {
        position = { pos[1], pos[2], pos[3] + 2 },
        scale    = { 0.8, 0.8, 0.8 }
    })
    local p = { obj=obj, type=typeID, color=color, col=col, row=row, moved=false }
    pieces[#pieces+1] = p
    board[col][row] = #pieces
    return p
end

-- ── Setup ─────────────────────────────────────────────────

local function setupBoard()
    initBoard()
    pieces = {}

    local backRow = { "rook","knight","bishop","queen","king","bishop","knight","rook" }

    for col = 1, BOARD_SIZE do
        -- Black back row (row 8)
        placePiece(backRow[col], "black", col, 8)
        -- Black pawns (row 7)
        placePiece("pawn", "black", col, 7)
        -- White pawns (row 2)
        placePiece("pawn", "white", col, 2)
        -- White back row (row 1)
        placePiece(backRow[col], "white", col, 1)
    end

    -- Spawn board
    Objects.spawn("chess_classic.chess_board", {
        position = { 0, -1, 0 },
        scale    = { 1, 1, 1 }
    })

    UI.showMessage("Chess! White moves first.")
end

-- ── Move validation ───────────────────────────────────────

local function isInBounds(col, row)
    return col >= 1 and col <= 8 and row >= 1 and row <= 8
end

local function pieceAt(col, row)
    if not isInBounds(col, row) then return nil end
    local idx = board[col][row]
    return idx and pieces[idx] or nil
end

local function isEnemy(piece, col, row)
    local target = pieceAt(col, row)
    return target and target.color ~= piece.color
end

local function isEmpty(col, row)
    return isInBounds(col, row) and board[col][row] == nil
end

local function getLegalMoves(piece)
    local moves = {}
    local c, r = piece.col, piece.row
    local dir = (piece.color == "white") and 1 or -1

    local function add(tc, tr)
        if isInBounds(tc, tr) and not (pieceAt(tc,tr) and pieceAt(tc,tr).color == piece.color) then
            moves[#moves+1] = { tc, tr }
        end
    end

    local function addSlide(dc, dr)
        local tc, tr = c + dc, r + dr
        while isInBounds(tc, tr) do
            if pieceAt(tc, tr) then
                if isEnemy(piece, tc, tr) then add(tc, tr) end
                break
            end
            add(tc, tr)
            tc, tr = tc + dc, tr + dr
        end
    end

    if piece.type == "pawn" then
        if isEmpty(c, r + dir) then
            add(c, r + dir)
            if not piece.moved and isEmpty(c, r + 2*dir) then
                add(c, r + 2*dir)
            end
        end
        for dc = -1, 1, 2 do
            if isEnemy(piece, c + dc, r + dir) then add(c + dc, r + dir) end
        end

    elseif piece.type == "rook" then
        for _, d in ipairs({{1,0},{-1,0},{0,1},{0,-1}}) do addSlide(d[1],d[2]) end

    elseif piece.type == "bishop" then
        for _, d in ipairs({{1,1},{1,-1},{-1,1},{-1,-1}}) do addSlide(d[1],d[2]) end

    elseif piece.type == "queen" then
        for _, d in ipairs({{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}}) do
            addSlide(d[1],d[2])
        end

    elseif piece.type == "knight" then
        for _, d in ipairs({{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}}) do
            add(c + d[1], r + d[2])
        end

    elseif piece.type == "king" then
        for dc = -1, 1 do for dr = -1, 1 do
            if dc ~= 0 or dr ~= 0 then add(c+dc, r+dr) end
        end end
    end

    return moves
end

-- ── Move execution ────────────────────────────────────────

local function movePiece(piece, toCol, toRow)
    -- Capture
    local target = pieceAt(toCol, toRow)
    if target then
        if target.color == "white" then
            capturedWhite[#capturedWhite+1] = target
        else
            capturedBlack[#capturedBlack+1] = target
        end
        board[target.col][target.row] = nil
        target.obj.destroy()

        -- Check win
        if target.type == "king" then
            gameOver = true
            Table.broadcast("♛ " .. piece.color:upper() .. " wins by capturing the King!")
            Game.endGame({ winner = piece.color })
            return
        end
    end

    -- Update board
    board[piece.col][piece.row] = nil
    piece.col   = toCol
    piece.row   = toRow
    piece.moved = true
    board[toCol][toRow] = nil
    for i, p in ipairs(pieces) do
        if p == piece then board[toCol][toRow] = i; break end
    end

    -- Move mesh
    local pos = cellToWorld(toCol, toRow)
    piece.obj.setPosition(pos[1], pos[2], pos[3] + 2)

    -- Pawn promotion
    if piece.type == "pawn" then
        if (piece.color == "white" and toRow == 8) or
           (piece.color == "black" and toRow == 1) then
            piece.type = "queen"
            UI.showMessage(piece.color:upper() .. " pawn promoted to Queen!")
        end
    end

    moveCount = moveCount + 1
    currentTurn = (currentTurn == "white") and "black" or "white"
    selectedPiece = nil
    UI.showMessage("Move " .. moveCount .. " — " .. currentTurn:upper() .. "'s turn")
    Events.emit("TurnStart", currentTurn)
end

-- ── Object interaction ────────────────────────────────────

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
        UI.showMessage("It's " .. currentTurn:upper() .. "'s turn!")
        -- Snap back
        local pos = cellToWorld(piece.col, piece.row)
        piece.obj.setPosition(pos[1], pos[2], pos[3] + 2)
        return
    end

    -- Snap to nearest grid cell
    local col = math.floor(newPos[1] / 12 + 4.5 + 0.5)
    local row = math.floor(newPos[3] / 12 + 4.5 + 0.5)
    col = math.max(1, math.min(8, col))
    row = math.max(1, math.min(8, row))

    -- Validate move
    local legal = getLegalMoves(piece)
    for _, m in ipairs(legal) do
        if m[1] == col and m[2] == row then
            movePiece(piece, col, row)
            return
        end
    end

    -- Illegal — snap back
    local pos = cellToWorld(piece.col, piece.row)
    piece.obj.setPosition(pos[1], pos[2], pos[3] + 2)
    UI.showMessage("Illegal move!")
end

-- ── Entry ─────────────────────────────────────────────────

local function onGameStart()
    if Table.getPlayerCount() < 2 then
        UI.showMessage("Chess requires 2 players!")
        return
    end
    setupBoard()
end

Events.on("GameStart",   onGameStart)
Events.on("ObjectMoved", onObjectMoved)
