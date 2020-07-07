var is2player;
var modal = document.getElementById("modal");

document.getElementById("playbot").onclick = function () {
    hideModal();
    is2player = false;
}
document.getElementById("2player").onclick = function () {
    hideModal();
    is2player = true;
}

function hideModal() {
    modal.style.display = "none";
    document.getElementById("2player").style.display = "none";
    document.getElementById("playbot").style.display = "none";
}

const canvas = document.getElementById("board");

window.addEventListener('resize', resizeCanvas);

function resizeCanvas() {
    canvas.width = Math.min(window.innerHeight, window.innerWidth);
    canvas.height = canvas.width;
    refresh();
}

GetBoard = Module.cwrap('get_board', 'string', [null]);

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
    var board = GetBoard().slice(0, -1);
    DrawBoard(canvas, board);
}

var move = new Int8Array(2);
var depth = 5;
var ply = 1;
function movePiece(canvas, event) {
    const rect = canvas.getBoundingClientRect()

    var tileSize = canvas.width / 8;
    var xSquare = Math.floor((event.clientX - rect.left) / tileSize);
    var ySquare = Math.floor((event.clientY - rect.top) / tileSize);

    var piece = GetBoard().slice(0, -1)[ySquare * 8 + xSquare];

    if ((ply % 2) == (piece == piece.toUpperCase()) && piece != '.') {
        refresh();

        move[0] = ySquare * 8 + xSquare;
        var offset = Module._malloc(5 * 4)
        var squares = Module.HEAP32.subarray(offset / 4, offset / 4 + 5);
        GetSquares(move[0], offset);

        DrawMove(canvas, piece, squares, xSquare, ySquare);
        Module._free(offset);
    } else {
        move[1] = ySquare * 8 + xSquare;
        var valid = UserMove(move[0], move[1]);
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
    var start = Date.now();
    ply++;
    AiMove(depth);
    if (Date.now() - start < 300) { depth++; }
    canvas.style.cursor = 'default';
    refresh();
    endGame();
}

function endGame() {
    if (Module.ccall('is_mainboard_terminal', 'number', [null], [])) {
        refresh();
        setTimeout(function () {
            var score = Module.ccall('score_mainboard', 'number', [null], []);
            var elem = document.createElement("p");
            var btn = document.createElement("button");
            btn.innerHTML = "play again";
            
            document.getElementById("modal-content").appendChild(elem);
            document.getElementById("modal-content").appendChild(btn);
            modal.style.display = "block";
            
            if (score == 1) {
                elem.innerHTML = "White Wins!";
                var color = "#ff45a2";
            } else if (score == -1) {
                elem.innerHTML = "Black Wins!";
                var color = "#6832a8";
            } else {
                elem.innerHTML = "Draw!";
                var color = "#9198e5";
            }
            document.getElementById("modal-content").style = "background-color: "+color+"; margin: 10% auto;"
            elem.style = "font-size: 60px;"
            btn.style = "font-size: 18px;"
            btn.onclick = function () {
                location.reload();
            }
        }, 0);
        return true;
    }
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
    var ctx = can.getContext("2d");
    var w = can.width;
    var h = can.height;
    ctx.fillStyle = "#cfcfcf";
    ctx.fillRect(0, 0, w, h);

    nRow = nCol = 8;

    w /= nCol;
    h /= nRow;

    for (var i = 0; i < nRow; ++i) {
        for (var j = 0; j < nCol; ++j) {
            if (j % 2 == 0) {
                ctx.fillStyle = "#614796";
                ctx.fillRect(j * w + (i % 2 ? 0 : w), i * h, w, h);
            }

            var char = board[i * 8 + j];
            if (char != '.') { DrawImage(char, ctx, j * w, i * h, w, h); }
        }
    }
    ctx.strokeStyle = "#3f3f3f";
    ctx.lineWidth = "5";
    ctx.beginPath();
    ctx.rect(0, 0, can.width, can.height);
    ctx.stroke();
    ctx.strokeStyle = "#000"
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
    var ctx = canvas.getContext("2d");
    var tileSize = canvas.width / 8;
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
