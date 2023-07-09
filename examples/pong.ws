print("ping pong")

let winW = 900
let winH = 500

let paddleW = 20
let paddleH = 150

let lPaddleY = (winW/2)-(paddleH/2)
let rPaddleY = (winW/2)-(paddleH/2)

let leftPad = 10
let rightPad = winW - paddleW - leftPad

let ballX = winW/2
let ballY = winH/2
let ballR = 20.0
let deltaSpeed = 0.01


let speedX = 0.2
let speedY = 0.2


let gameOver = false
let win = false
let gameOverText = ""

set_log_level("log_none")
make_window(winW, winH, "hello_pong")

while(windows_should_close() != true){
    begin_drawing()
    clr_bg("white")

    if (gameOver) {
        clr_bg("white")
        if(is_key_down("space")){
            ballY = winH/2
            ballX = winW/2
            speedX = 0.2
            speedY = 0.3
            win = false
            gameOver = false
        }
        if (win){
            gameOverText = "You won"
        } else {
            gameOverText = "You lost"
        }
        draw_text(gameOverText , winW/2 - 50, winH/2 - 15, 30, "black")
    } else {

        if (ballX + ballR >= rightPad+paddleW) {
            gameOver = true
            win = false
        }

        if(is_key_down("up")){
            if ((rPaddleY - 0.6) >= 0) {
                rPaddleY = rPaddleY - 0.6
            }
        }

        if(is_key_down("down")){
            if ((rPaddleY + paddleH) <= winH) {
                rPaddleY = rPaddleY + 0.6
            }
        }

        if (ballX + ballR >= rightPad) {
            if (ballY > rPaddleY and ballY < (rPaddleY + paddleH)){
                speedX = speedX + deltaSpeed
                speedX = speedX * -1
            } else {
                gameOver = true
                win = false
            }
        }

        if (ballX - ballR <= leftPad + paddleW / 2) {
            if (ballY > lPaddleY and ballY < (lPaddleY + paddleH)){
                speedX = speedX + deltaSpeed
                speedX = speedX * -1
            } else {
                gameOver = true
                win = true
            }
        }

        if (ballX - ballR >= leftPad + paddleW) {
            lPaddleY = ballY - paddleH/2
        }

        if (ballY - ballR <= 0 or ballY+ballR >= winH) {
            speedY = speedY * -1
        }

        ballX = ballX + speedX
        ballY = ballY + speedY

        draw_circle(to_int(ballX), to_int(ballY), ballR, "pink")
        draw_rec(leftPad, to_int(lPaddleY), paddleW, paddleH, "black")
        draw_rec(rightPad, to_int(rPaddleY), paddleW, paddleH, "black")
    }
    wait_time(1000)
    end_drawing()
}

close_window()

print("end")
