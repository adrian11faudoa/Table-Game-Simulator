-- ============================================================
-- Classic Chess for InfinityTable
-- Full 8x8 board, move validation, pawn promotion
-- ============================================================

local BOARD_SIZE  = 8
local pieces      = {}
local board       = {}
local currentTurn = "white"
local gameOver    = false
local moveCount   = 0

local function cellToWorld(col, row)
    return { (col - 4.5) * 12, 0, (row - 4.5) * 12 }
end

local function initBoard()
    for c = 1, BOARD_SIZE do
        board[c] = {}
        for r = 1, BOARD_SIZE do board[c][r] = nil end
    end
end

local function placePiece(typeID, color, col, row)
    local pos = cellToWorld(col, row)
    local obj = Objects.spawn("chess_classic." .. typeID .. "_" .. color, {
        position = { pos[1], pos[2], pos[3] + 2 }
    })
    local p = { obj=obj, type=typeID, color=color, col=col, row=row, moved=false }
    pieces[#pieces+1] = p
    board[col][row] = #pieces
    return p
end

local function setupBoard()
    initBoard(); pieces = {}
    local backRow = {"rook","knight","bishop","queen","king","bishop","knight","rook"}
    for col = 1, BOARD_SIZE do
        placePiece(backRow[col], "black", col, 8)
        placePiece("pawn", "black", col, 7)
        placePiece("pawn", "white", col, 2)
        placePiece(backRow[col], "white", col, 1)
    end
    Objects.spawn("chess_classic.chess_board", { position={0,-1,0} })
    UI.showMessage("Chess! White moves first.")
end

local function isInBounds(c,r) return c>=1 and c<=8 and r>=1 and r<=8 end
local function pieceAt(c,r)
    if not isInBounds(c,r) then return nil end
    local idx = board[c][r]; return idx and pieces[idx] or nil
end
local function isEnemy(piece,c,r) local t=pieceAt(c,r); return t and t.color~=piece.color end
local function isEmpty(c,r) return isInBounds(c,r) and board[c][r]==nil end

local function getLegalMoves(piece)
    local moves,c,r = {},piece.col,piece.row
    local dir = (piece.color=="white") and 1 or -1
    local function add(tc,tr)
        if isInBounds(tc,tr) and not(pieceAt(tc,tr) and pieceAt(tc,tr).color==piece.color) then
            moves[#moves+1]={tc,tr} end
    end
    local function slide(dc,dr) local tc,tr=c+dc,r+dr
        while isInBounds(tc,tr) do
            if pieceAt(tc,tr) then if isEnemy(piece,tc,tr) then add(tc,tr) end break end
            add(tc,tr); tc,tr=tc+dc,tr+dr end
    end
    if piece.type=="pawn" then
        if isEmpty(c,r+dir) then add(c,r+dir)
            if not piece.moved and isEmpty(c,r+2*dir) then add(c,r+2*dir) end end
        for _,dc in ipairs({-1,1}) do if isEnemy(piece,c+dc,r+dir) then add(c+dc,r+dir) end end
    elseif piece.type=="rook" then
        for _,d in ipairs({{1,0},{-1,0},{0,1},{0,-1}}) do slide(d[1],d[2]) end
    elseif piece.type=="bishop" then
        for _,d in ipairs({{1,1},{1,-1},{-1,1},{-1,-1}}) do slide(d[1],d[2]) end
    elseif piece.type=="queen" then
        for _,d in ipairs({{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}}) do slide(d[1],d[2]) end
    elseif piece.type=="knight" then
        for _,d in ipairs({{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}}) do add(c+d[1],r+d[2]) end
    elseif piece.type=="king" then
        for dc=-1,1 do for dr=-1,1 do if dc~=0 or dr~=0 then add(c+dc,r+dr) end end end
    end
    return moves
end

local function movePiece(piece, toCol, toRow)
    local target = pieceAt(toCol,toRow)
    if target then
        board[target.col][target.row]=nil; target.obj.destroy()
        if target.type=="king" then
            gameOver=true
            Table.broadcast(piece.color:upper().." wins by capturing the King!")
            Game.endGame({winner=piece.color}); return
        end
    end
    board[piece.col][piece.row]=nil
    piece.col=toCol; piece.row=toRow; piece.moved=true
    for i,p in ipairs(pieces) do if p==piece then board[toCol][toRow]=i; break end end
    local pos=cellToWorld(toCol,toRow); piece.obj.setPosition(pos[1],pos[2],pos[3]+2)
    if piece.type=="pawn" and ((piece.color=="white" and toRow==8) or (piece.color=="black" and toRow==1)) then
        piece.type="queen"; UI.showMessage(piece.color:upper().." pawn promoted to Queen!") end
    moveCount=moveCount+1; currentTurn=(currentTurn=="white") and "black" or "white"
    UI.showMessage("Move "..moveCount.." - "..currentTurn:upper().."'s turn")
end

local function findPieceByObj(obj)
    for _,p in ipairs(pieces) do if p.obj==obj then return p end end
end

local function onObjectMoved(player, obj, newPos)
    if gameOver then return end
    local piece=findPieceByObj(obj); if not piece then return end
    if piece.color~=currentTurn then
        local pos=cellToWorld(piece.col,piece.row); piece.obj.setPosition(pos[1],pos[2],pos[3]+2)
        UI.showMessage("It's "..currentTurn:upper().."'s turn!"); return
    end
    local col=math.max(1,math.min(8,math.floor(newPos[1]/12+4.5+0.5)))
    local row=math.max(1,math.min(8,math.floor(newPos[3]/12+4.5+0.5)))
    for _,m in ipairs(getLegalMoves(piece)) do
        if m[1]==col and m[2]==row then movePiece(piece,col,row); return end
    end
    local pos=cellToWorld(piece.col,piece.row); piece.obj.setPosition(pos[1],pos[2],pos[3]+2)
    UI.showMessage("Illegal move!")
end

Events.on("GameStart",   function() setupBoard() end)
Events.on("ObjectMoved", onObjectMoved)
