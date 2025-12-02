
from __future__ import annotations
import random
from typing import Tuple
from ..controller import Controller
from ..move import Move
from ..board import get_capturing_grids
from ..custom_logger import log

import time
# random.seed(round(time.time()))

def count_sides(board, r, c):
    cnt = 0
    if board.horizontal_lines[r][c]: cnt += 1
    if board.horizontal_lines[r + 1][c]: cnt += 1
    if board.vertical_lines[r][c]: cnt += 1
    if board.vertical_lines[r][c + 1]: cnt += 1
    return cnt


def is_dangerous_move(board, mv):
    rows, cols = board.rows, board.cols
    if mv.is_horizontal:
        if mv.row > 0 and count_sides(board, mv.row - 1, mv.col) == 2:
            return True
        if mv.row < rows - 1 and count_sides(board, mv.row, mv.col) == 2:
            return True
    else:
        if mv.col > 0 and count_sides(board, mv.row, mv.col - 1) == 2:
            return True
        if mv.col < cols - 1 and count_sides(board, mv.row, mv.col) == 2:
            return True
    return False


def make_move(controller: Controller) -> Tuple[bool, Move]:
    board = controller.get_current_board()
    moves = board.get_valid_moves()
    total = len(moves)
    if total == 0:
        return False, Move(0, 0, True)

    # 1️⃣ Take capturing move first
    for mv in moves:
        if board.is_capturing_move(mv):
            cont = controller.make_move(mv)
            return cont, mv

    # 2️⃣ Take safe (non-dangerous) move
    i=120
    while i>0:
        i-=1
        # mv = random.choice(moves)
        mv = moves[round(time.time()*10000) % len(moves)]
        if not is_dangerous_move(board, mv):
            cont = controller.make_move(mv)
            return cont, mv

    # 3️⃣ No safe moves: choose the move that minimizes opponent captures
    best_move = moves[0]
    min_opp_captures = 1000

    my_side = controller.get_my_side()
    opp_side = controller.get_opponent_side()

    for mv in moves:
        tmp = board.clone()
        # simulate our move
        tmp.make_move(mv, my_side)

        # Now simulate opponent's turn(s): sequential captures
        opp_captures = 0

        while True:
            opp_valid = tmp.get_valid_moves()
            # find any capturing move for opponent
            capturing_found = False
            for om in opp_valid:
                if tmp.is_capturing_move(om):
                    # count how many boxes this opponent move would capture
                    caps = get_capturing_grids(tmp, om)
                    opp_captures += len(caps)
                    # play the opponent move on tmp to allow chaining
                    tmp.make_move(om, opp_side)
                    capturing_found = True
                    break  # opponent will continue from this new board
            if not capturing_found:
                break

        if opp_captures < min_opp_captures:
            min_opp_captures = opp_captures
            best_move = mv

    cont = controller.make_move(best_move)
    return cont, best_move

    __all__ = ["make_move"]
