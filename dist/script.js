var is2player;
const isBlackBot = Math.floor(Math.random()*2);
var modal = document.getElementById("modal");
var boardHistory = [];

//document.body.style.background = '#'+`000000${Math.random().toString(16)}`.substr(-6); /* generate random color as bg */

var lightTileCol = "#cfcfcf";
var darkTileCol = "#614796";
var boardBorderCol = "#3f3f3f";

var lightTheme = 0 == Math.floor(Math.random()*20);

if (lightTheme) { 
    document.body.style.background = "#d8a2cf";
    document.getElementById("modal-content").style.backgroundColor = "rgba(60, 60, 60, 0.85)";
    lightTileCol = "#fff";
    darkTileCol = "#fcb8ea";
    boardBorderCol = "#fff"
}


document.getElementById("playbot").onclick = function () {
    hideModal();
    is2player = false;
    
    if (isBlackBot) { setTimeout(MakeAiMove, 100); }
}
document.getElementById("2player").onclick = function () {
    hideModal();
    is2player = true;
}

function hideModal() {
    if (!lightTheme) { document.body.style.background = "#221c25"; }
    modal.style.visibility = "hidden";
    document.getElementById("modal-content").style.visibility = "visible";
    document.getElementById("modal-content").classList.add("hide");
 
    setTimeout(function () {
        document.getElementById("modal-content").classList.remove("hide");
        document.getElementById("2player").style.display = "none";
        document.getElementById("playbot").style.display = "none";
        modal.style.visibility = "visible";
        modal.style.display = "none";
    }, 300)
}

const canvas = document.getElementById("board");

window.addEventListener('resize', resizeCanvas);

function resizeCanvas() {
    canvas.width = Math.min(window.innerHeight, window.innerWidth);
    canvas.height = canvas.width;
    refresh();
}

GetBoardWasm = Module.cwrap('get_board', 'string', [null]);

function GetBoard() {
    let board = GetBoardWasm().slice(0, 64);
    if (isBlackBot) { board = [...board].reverse().join(""); }
    return board;
}

AiMove = Module.cwrap('ai_move', null, ['number']);
UserMove = Module.cwrap('user_move', 'number', ['number', 'number']);
GetSquares = Module.cwrap('get_squares', null, ['number', 'number']);

Module['onRuntimeInitialized'] = onRuntimeInitialized;
function onRuntimeInitialized() {
    Module.ccall('init_board', null, [null], []);
    resizeCanvas();
    checkLoaded();
}

function refresh() {
    let board = GetBoard();
    DrawBoard(canvas, board);
}

var move = new Int8Array(2);
var depth = 5;
var ply = 1;
function movePiece(canvas, event) {
    const rect = canvas.getBoundingClientRect()

    let tileSize = canvas.width / 8;
    let xSquare = Math.floor((event.clientX - rect.left) / tileSize);
    let ySquare = Math.floor((event.clientY - rect.top) / tileSize);

    let piece = GetBoard()[ySquare * 8 + xSquare];

    if ((ply % 2) == (piece == piece.toUpperCase()) && piece != '.') {
        refresh();

        move[0] = ySquare * 8 + xSquare;

        if (isBlackBot) { move[0] = 63 - move[0]; }

        let offset = Module._malloc(5 * 4)
        let squares = Module.HEAP32.subarray(offset / 4, offset / 4 + 5);
        GetSquares(move[0], offset);

        if (isBlackBot) { squares = squares.map(x => 63 - x); }

        DrawMove(canvas, piece, squares, xSquare, ySquare);
        Module._free(offset);
    } else {
        move[1] = ySquare * 8 + xSquare;

        if (isBlackBot) { move[1] = 63 - move[1]; }

        let valid = UserMove(move[0], move[1]);
        requestAnimationFrame(refresh);
        if (valid) {
            ply++;
            if (endGame()) { return; }
            if (!is2player) {
                canvas.style.cursor = 'wait';
                setTimeout(MakeAiMove, 30);
            }
        }
    }
}

function MakeAiMove() {
    let start = Date.now();
    ply++;
    AiMove(depth);
    if (Date.now() - start < 300) { depth++; }
    canvas.style.cursor = 'default';
    refresh();
    endGame();
}

function endGame() {
    let threefoldRep = [GetBoard(), boardHistory[3], boardHistory[7]].reduce((acc, cur, _, src) => acc = cur == src[0]);
    if (Module.ccall('is_mainboard_terminal', 'number', [null], []) || threefoldRep) {
        refresh();
        setTimeout(function () {
            const score = Module.ccall('score_mainboard', 'number', [null], []);
            let elem = document.createElement("p");
            let btn = document.createElement("button");
            btn.innerHTML = "play again";
            
            document.getElementById("modal-content").appendChild(elem);
            document.getElementById("modal-content").appendChild(btn);
            modal.style.display = "block";
            
            let color, textColor;
            if (score == 100) {
                elem.innerHTML = "White Wins!";
                textColor = "#000"
                color = "#ff7dbe";
            } else if (score == -100) {
                elem.innerHTML = "Black Wins!";
                textColor = "#fff"
                color = "#6832a8";
            } else {
                elem.innerHTML = "Draw!";
                textColor = "#dcdcdc";
                color = "#595fa5";
            }
            document.getElementById("modal-content").style = `background-color: ${color}; margin: 10% auto; color: ${textColor};`
            
            elem.style = "font-size: 60px;"
            btn.classList.add("button-small") 
            btn.onclick = function () {
                location.reload();
            }
        }, 0);
        return true;
    }
    boardHistory.push(GetBoard());
    boardHistory = boardHistory.slice(0, 8);
    return false;
}

canvas.addEventListener('mousedown', function (e) {
    movePiece(canvas, e)
})

var wp = new Image();
wp.src = "/img/pawnwhite.png";
var bp = new Image();
bp.src = "/img/pawnblack.png";
var wk = new Image();
wk.src = "/img/kingwhite.png";
var bk = new Image();
bk.src = "/img/kingblack.png";

function checkLoaded() {
    if (wp.complete && wp.complete && bp.complete && wk.complete && bk.complete) { refresh(); }
    else { setTimeout(checkLoaded, 50); }
}

function DrawBoard(can, board) {
    let ctx = can.getContext("2d");
    let w = can.width;
    let h = can.height;
    ctx.fillStyle = lightTileCol;
    ctx.fillRect(0, 0, w, h);

    nRow = nCol = 8;

    w /= nCol;
    h /= nRow;

    for (let i = 0; i < nRow; ++i) {
        for (let j = 0; j < nCol; ++j) {
            if (j % 2 == 0) {
                ctx.fillStyle = darkTileCol;
                ctx.fillRect(j * w + (i % 2 ? 0 : w), i * h, w, h);
            }

            let char = board[i * 8 + j];
            if (char != '.') { DrawImage(char, ctx, j * w, i * h, w, h); }
        }
    }
    ctx.strokeStyle = boardBorderCol;
    ctx.lineWidth = "5";
    ctx.beginPath();
    ctx.rect(0, 0, can.width, can.height);
    ctx.stroke();
    ctx.strokeStyle = "#000";
}

function DrawImage(char, ctx, x1, y1, x2, y2) {
    if (char == 'p') {
        ctx.drawImage(bp, x1, y1, x2, y2);
    } else if (char == 'P') {
        ctx.drawImage(wp, x1, y1, x2, y2);
    } else if (char == 'k') {
        ctx.drawImage(bk, x1, y1, x2, y2);
    } else if (char == 'K') {
        ctx.drawImage(wk, x1, y1, x2, y2);
    }
}

function DrawMove(canvas, piece, squares, xSquare, ySquare) {
    let ctx = canvas.getContext("2d");
    let tileSize = canvas.width / 8;
    ctx.lineWidth = "3";

    ctx.fillStyle = "#241430";
    ctx.fillRect(xSquare * tileSize, ySquare * tileSize, tileSize, tileSize);
    DrawImage(piece, ctx, xSquare * tileSize, ySquare * tileSize, tileSize, tileSize);

    ctx.beginPath();
    ctx.rect(xSquare * tileSize, ySquare * tileSize, tileSize, tileSize);
    ctx.stroke();
    for (i = 0; i < 5; i++) {
        if (squares[i] != -1) {
            xSquare = squares[i] % 8;
            ySquare = Math.floor(squares[i] / 8);
            ctx.beginPath();
            ctx.rect(xSquare * tileSize, ySquare * tileSize, tileSize, tileSize);
            ctx.stroke();
            ctx.globalAlpha = 0.2;
            ctx.fillStyle = "#fc03ec";
            ctx.fillRect(xSquare * tileSize, ySquare * tileSize, tileSize, tileSize);
            ctx.globalAlpha = 1;
        }
    }
}
