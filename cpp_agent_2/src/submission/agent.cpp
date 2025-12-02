#include "agent.h"
#include "customLogger.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <utility>

using namespace std;

void Agent::init() { srand(time(NULL)); }

// helper to count filled sides of a box (r,c)
static int countSides(const Board &board, int r, int c) {
    int cnt = 0;
    if (board.horizontalLines[r][c]) cnt++;
    if (board.horizontalLines[r + 1][c]) cnt++;
    if (board.verticalLines[r][c]) cnt++;
    if (board.verticalLines[r][c + 1]) cnt++;
    return cnt;
}

// helper to check if move makes any adjacent box have 3 sides
static bool isDangerousMove(const Board &board, const Move &mv) {
    int rows = board.rows, cols = board.cols;

    if (mv.isHorizontal) {
        if (mv.row > 0 && countSides(board, mv.row - 1, mv.col) == 2) return true;
        if (mv.row < rows - 1 && countSides(board, mv.row, mv.col) == 2) return true;
    } else {
        if (mv.col > 0 && countSides(board, mv.row, mv.col - 1) == 2) return true;
        if (mv.col < cols - 1 && countSides(board, mv.row, mv.col) == 2) return true;
    }
    return false;
}

pair<bool, Move> makeMove(Controller &controller) {
    Board board = controller.getCurrentBoard();
    const auto validMoves = board.getValidMoves();
    const int totalMoves = validMoves.size();
    if (totalMoves == 0)
        return pair<bool, Move>(false, Move{0, 0, true});

    int i, j; // loop indices reused
    const Move *mvPtr;

    // 1️⃣ Try capturing move first
    for (i = 0; i < totalMoves; ++i) {
        mvPtr = &validMoves[i];
        if (board.isCapturingMove(*mvPtr)) {
            bool cont = controller.makeMove(*mvPtr);
            Log("Scoring move: " + to_string(mvPtr->row) + " " + to_string(mvPtr->col));
            return pair<bool, Move>(cont, *mvPtr);
        }
    }

    // 2️⃣ Try safe move (that doesn't create 3rd side)
    for (i = 0; i < totalMoves; ++i) {
        mvPtr = &validMoves[i];
        if (!isDangerousMove(board, *mvPtr)) {
            bool cont = controller.makeMove(*mvPtr);
            Log("Safe move: " + to_string(mvPtr->row) + " " + to_string(mvPtr->col));
            return pair<bool, Move>(cont, *mvPtr);
        }
    }

    

    const Move &bestMove = validMoves[rand() % validMoves.size()];
    bool cont = controller.makeMove(bestMove);
    Log("Choosing random agent as no safe move left");
    return pair<bool, Move>(cont, bestMove);
}

void Agent::run() {
    while (!controller->getCurrentBoard().isCompleted()) {
        while (true) {
            pair<bool, Move> result = makeMove(*controller);
            bool requiresMoreMoves = result.first;
            if (!requiresMoreMoves || controller->getCurrentBoard().isCompleted())
                break;
        }
        if (controller->getCurrentBoard().isCompleted())
            break;
    }
}
