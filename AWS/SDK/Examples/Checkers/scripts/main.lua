-- ============================================================
-- Classic Checkers for InfinityTable
-- Forced captures, king promotion, 2-player
-- ============================================================
local pieces={} local board={} local currentTurn="red" local gameOver=false

local function cellToWorld(c,r) return {(c-4.5)*10,0,(r-4.5)*10} end
local function initBoard()
    for c=1,8 do board[c]={} for r=1,8 do board[c][r]=nil end end
end
local function spawnPiece(color,col,row)
    local pos=cellToWorld(col,row)
    local obj=Objects.spawn("checkers_classic.piece_"..color,{position={pos[1],pos[2],pos[3]+1}})
    local p={obj=obj,color=color,col=col,row=row,isKing=false}
    pieces[#pieces+1]=p; board[col][row]=#pieces; return p
end
local function setupBoard()
    initBoard(); pieces={}
    for row=6,8 do for col=1,8 do if(col+row)%2==0 then spawnPiece("black",col,row) end end end
    for row=1,3 do for col=1,8 do if(col+row)%2==0 then spawnPiece("red",col,row) end end end
    Objects.spawn("checkers_classic.checkers_board",{position={0,-0.5,0}})
    UI.showMessage("Checkers! Red moves first.")
end
local function pieceAt(c,r)
    if c<1 or c>8 or r<1 or r>8 then return nil end
    local idx=board[c][r]; return idx and pieces[idx] or nil
end
local function getMoves(piece)
    local moves,dirs={},{}
    if piece.color=="red"   or piece.isKing then dirs[#dirs+1]={1,1};dirs[#dirs+1]={-1,1} end
    if piece.color=="black" or piece.isKing then dirs[#dirs+1]={1,-1};dirs[#dirs+1]={-1,-1} end
    for _,d in ipairs(dirs) do
        local nc,nr=piece.col+d[1],piece.row+d[2]
        if nc>=1 and nc<=8 and nr>=1 and nr<=8 then
            if not pieceAt(nc,nr) then moves[#moves+1]={col=nc,row=nr,capture=nil}
            elseif pieceAt(nc,nr).color~=piece.color then
                local jc,jr=nc+d[1],nr+d[2]
                if jc>=1 and jc<=8 and jr>=1 and jr<=8 and not pieceAt(jc,jr) then
                    moves[#moves+1]={col=jc,row=jr,capture=pieceAt(nc,nr)} end end end end
    return moves
end
local function hasForcedCapture(color)
    for _,p in ipairs(pieces) do if p.color==color then for _,m in ipairs(getMoves(p)) do if m.capture then return true end end end end
    return false
end
local function checkWin()
    local r,b=0,0
    for _,p in ipairs(pieces) do if p.color=="red" then r=r+1 elseif p.color=="black" then b=b+1 end end
    if r==0 then Table.broadcast("Black wins!"); Game.endGame({winner="black"}); gameOver=true; return true end
    if b==0 then Table.broadcast("Red wins!");   Game.endGame({winner="red"});   gameOver=true; return true end
    return false
end
local function promote(piece)
    if piece.isKing then return end
    if (piece.color=="red" and piece.row==8) or (piece.color=="black" and piece.row==1) then
        piece.isKing=true
        local pos=cellToWorld(piece.col,piece.row); piece.obj.destroy()
        piece.obj=Objects.spawn("checkers_classic.king_"..piece.color,{position={pos[1],pos[2],pos[3]+1}})
        UI.showMessage(piece.color:upper().." piece kinged!")
    end
end
local function findPieceByObj(obj) for _,p in ipairs(pieces) do if p.obj==obj then return p end end end

local function onObjectMoved(player,obj,newPos)
    if gameOver then return end
    local piece=findPieceByObj(obj); if not piece then return end
    if piece.color~=currentTurn then
        local pos=cellToWorld(piece.col,piece.row); piece.obj.setPosition(pos[1],pos[2],pos[3]+1)
        UI.showMessage("It's "..currentTurn:upper().."'s turn!"); return end
    local col=math.max(1,math.min(8,math.floor(newPos[1]/10+4.5+0.5)))
    local row=math.max(1,math.min(8,math.floor(newPos[3]/10+4.5+0.5)))
    local forced=hasForcedCapture(currentTurn)
    for _,m in ipairs(getMoves(piece)) do
        if m.col==col and m.row==row then
            if forced and not m.capture then
                local pos=cellToWorld(piece.col,piece.row); piece.obj.setPosition(pos[1],pos[2],pos[3]+1)
                UI.showMessage("Forced capture available!"); return end
            board[piece.col][piece.row]=nil
            if m.capture then
                board[m.capture.col][m.capture.row]=nil; m.capture.obj.destroy()
                for i=#pieces,1,-1 do if pieces[i]==m.capture then table.remove(pieces,i); break end end
            end
            piece.col=col; piece.row=row
            local npos=cellToWorld(col,row); piece.obj.setPosition(npos[1],npos[2],npos[3]+1)
            for i,p in ipairs(pieces) do if p==piece then board[col][row]=i; break end end
            promote(piece); if checkWin() then return end
            currentTurn=(currentTurn=="red") and "black" or "red"
            UI.showMessage(currentTurn:upper().."'s turn"); return end
    end
    local pos=cellToWorld(piece.col,piece.row); piece.obj.setPosition(pos[1],pos[2],pos[3]+1)
    UI.showMessage("Illegal move!")
end

Events.on("GameStart",   function() setupBoard() end)
Events.on("ObjectMoved", onObjectMoved)
