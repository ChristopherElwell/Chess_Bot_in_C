import subprocess
import chess
import tkinter as tk
from PIL import Image, ImageTk
import os
import random
import time
from stockfish import Stockfish
colours = ['#DCE6C9','#BCC6A9','#FCF6E9']

LEVEL = 2
stockfish = Stockfish(path = "C:\\Users\\chris\\OneDrive\\Desktop\\C_Chess\\stockfish\\stockfish-windows-x86-64-avx2.exe",
                      parameters={"Threads":2,"Hash":512})
SRC_DIR = "src"
C_FILE = os.path.join(SRC_DIR, "main.c")
EXE_FILE = os.path.join(SRC_DIR, "chess_release.exe")

class window(tk.Tk):
    def __init__(self):
        super().__init__()
        self.bot = start_bot()
        size = 500
        self.sqr_size = size / 8
        self.selected_piece = (-1,-1)
        self.geometry(f"{size+100}x{size}")
        self.title('Chess')
        self.canvas = tk.Canvas(border=None)
        self.canvas.place(x=0,y=0,width=size,height=size,anchor='nw')  
        
        self.FENbutton = tk.Button(self,text="Print FEN",command = lambda: print(self.board.fen()))
        self.FENbutton.place(x=size,y=0,width = 100,height = 60,anchor='nw')
        
        self.input_fen_btn = tk.Button(self,text="Input Fen",command = self.input_fen)
        self.input_fen_btn.place(x=size,y=size/4,width = 100,height = 60,anchor='nw')
        
        self.start_game_btn = tk.Button(self,text="Start Game",command = lambda: self.start_game(LEVEL))
        self.start_game_btn.place(x=size,y=size / 2,width = 100,height = 60,anchor='nw')
        
        self.get_bot_move = tk.Button(self,text="Get Bot Move",command = lambda: self.run_bot(self.bot,self.board.fen()))
        self.get_bot_move.place(x=size,y=size * 3/4,width = 100,height = 60,anchor='nw')
        
        self.piece_img_map = {
                'p':ImageTk.PhotoImage(Image.open('Pieces\\black-pawn.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS)),
                'n':ImageTk.PhotoImage(Image.open('Pieces\\black-knight.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS)),
                'b':ImageTk.PhotoImage(Image.open('Pieces\\black-bishop.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS)),
                'r':ImageTk.PhotoImage(Image.open('Pieces\\black-rook.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS)),
                'k':ImageTk.PhotoImage(Image.open('Pieces\\black-king.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS)),
                'q':ImageTk.PhotoImage(Image.open('Pieces\\black-queen.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS)),
                'P':ImageTk.PhotoImage(Image.open('Pieces\\white-pawn.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS)),
                'N':ImageTk.PhotoImage(Image.open('Pieces\\white-knight.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS)),
                'B':ImageTk.PhotoImage(Image.open('Pieces\\white-bishop.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS)),
                'R':ImageTk.PhotoImage(Image.open('Pieces\\white-rook.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS)),
                'K':ImageTk.PhotoImage(Image.open('Pieces\\white-king.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS)),
                'Q':ImageTk.PhotoImage(Image.open('Pieces\\white-queen.png').convert("RGBA").resize((int(self.sqr_size),int(self.sqr_size)),Image.LANCZOS))
                 }
        
        self.board = chess.Board()
        self.draw_board(self.get_piece_map(self.board))

        self.bind("<Button-1>",func = lambda x: self.on_click(x,self.board))

    def input_fen(self):
        self.board = chess.Board(fen=self.clipboard_get())
        self.draw_board(self.get_piece_map(self.board))

    def on_click(self, event, board):
        sq = (int(event.x // self.sqr_size), int(7 - event.y // self.sqr_size))
        if board.piece_at(sq[0] + sq[1] * 8) is not None and board.piece_at(sq[0] + sq[1] * 8).color == board.turn:
            self.selected_piece = sq
        elif self.selected_piece is not None:
            move = chess.Move(chess.square(*self.selected_piece), chess.square(*sq))
            
            piece = board.piece_at(chess.square(*self.selected_piece))
            if piece and piece.piece_type == chess.PAWN:
                if (board.turn == chess.WHITE and sq[1] == 7) or (board.turn == chess.BLACK and sq[1] == 0):
                    move = chess.Move(chess.square(*self.selected_piece), chess.square(*sq), promotion=chess.QUEEN)
            
            if board.is_legal(move):
                board.push(move)
                self.selected_piece = None
        
        if board.is_checkmate():
            print("CHECKMATE")
        elif board.is_stalemate():
            print("STALEMATE")
        
        self.draw_board(self.get_piece_map(board))
 
        
    def make_grid(self):
        for i in range(8):
            for j in range(8):
                self.canvas.create_rectangle(self.sqr_size*i,
                                            self.sqr_size*j,
                                            self.sqr_size*(i+1),
                                            self.sqr_size*(j+1),
                                            fill=colours[(i+j)%2],
                                            outline=colours[(i+j)%2])
        board = chess.Board()
        return board
    
    def get_piece_map(self,board):
        return {key:piece.symbol() for key, piece in board.piece_map().items()}    
    
    def draw_board(self,pieces):
        self.canvas.delete("all")
        self.make_grid()
        if self.selected_piece is not None:
            self.canvas.create_rectangle(self.sqr_size*( self.selected_piece[0]),
                                        self.sqr_size*(8 - self.selected_piece[1]),
                                        self.sqr_size*((self.selected_piece[0]+1)),
                                        self.sqr_size*(8 - (self.selected_piece[1]+1)),
                                        fill=colours[2],
                                        outline=colours[2])
        for sqr, pc in pieces.items():
            self.canvas.create_image((sqr%8 + 0.5) * self.sqr_size,(7 - sqr//8 + 0.5) * self.sqr_size,image = self.piece_img_map[pc],anchor = 'center')

    def run_bot(self,bot,fen):
        bot_return = call_bot(bot,f"PLAY {fen}")
        print("BOT RETURNED: ",bot_return)
        try:
            move = chess.Move.from_uci(bot_return)
        except Exception:
            print("Failed to convert to chess move",bot_return)
            return
        if move in self.board.legal_moves:
            self.board.push(move)
            self.draw_board(self.get_piece_map(self.board))
        else:
            print("move is not legal", bot_return)

    def start_game(self,level):
        print("GAME STARTED AT LEVEL:",level)
        self.board.set_board_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR")
        stockfish.set_skill_level(level)
        while(True):
            stockfish.set_fen_position(self.board.fen())
            fish_move = stockfish.get_best_move()
            self.board.push(chess.Move.from_uci(fish_move))
            self.draw_board(self.get_piece_map(self.board))
            print("STOCKFISH PLAYS",fish_move)
            self.update_idletasks()
            bot_move = call_bot(self.bot,f"PLAY {self.board.fen()}")
            print("BOT PLAYS",bot_move)
            self.board.push(chess.Move.from_uci(bot_move))
            self.draw_board(self.get_piece_map(self.board))
            self.update_idletasks()
        

def call_bot(process, message):
    command = f"GET {message}\n"
    process.stdin.write(command + "\n")
    process.stdin.flush()
    response = process.stderr.readline().strip()
    if response:
        return response
    else:
        return None

def start_bot():
    return subprocess.Popen(
        EXE_FILE, 
        stdin=subprocess.PIPE,
        stdout=None, 
        stderr=subprocess.PIPE, 
        text=True,
        cwd=SRC_DIR
    )
if __name__ == "__main__":
    w = window()
    w.mainloop()