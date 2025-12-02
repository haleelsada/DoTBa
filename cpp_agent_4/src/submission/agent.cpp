#include "agent.h"
#include "customLogger.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <utility>
#include <../../cpp_board_lib/include/lib/utils.h>
using namespace std;

void Agent::init() { srand(static_cast<unsigned>(time(nullptr))); }

// helper to count filled sides of a box (r,c)
static int countSides(const Board &board, int r, int c) {
    int cnt = 0;
    const auto &h = board.horizontalLines;
    const auto &v = board.verticalLines;
    if (h[r][c]) ++cnt;
    if (h[r + 1][c]) ++cnt;
    if (v[r][c]) ++cnt;
    if (v[r][c + 1]) ++cnt;
    return cnt;
}

// helper to check if move makes any adjacent box have 3 sides
static bool isDangerousMove(const Board &board, const Move &mv) {
    const int rows = board.rows;
    const int cols = board.cols;
    if (mv.isHorizontal) {
        if (mv.row > 0 && countSides(board, mv.row - 1, mv.col) == 2) return true;
        if (mv.row < rows - 1 && countSides(board, mv.row, mv.col) == 2) return true;
    } else {
        if (mv.col > 0 && countSides(board, mv.row, mv.col - 1) == 2) return true;
        if (mv.col < cols - 1 && countSides(board, mv.row, mv.col) == 2) return true;
    }
    return false;
}

// helper to check if any safe move exists
static bool hasSafeMove(const Board &board, const vector<Move> &moves) {
    for (const auto &mv : moves)
        if (!isDangerousMove(board, mv)) return true;
    return false;
}

// helper: simulate how many captures opponent could get after a move
static int simulateOpponentCaptures(Board board, const Move &mv, PlayerSide mySide, PlayerSide oppSide) {
    board.makeMove(mv, mySide);
    int oppCaptures = 0;

    while (true) {
        const auto &oppMoves = board.getValidMoves();
        bool captured = false;
        for (const auto &oppMv : oppMoves) {
            if (board.isCapturingMove(oppMv)) {
                const auto &capturedGrids = getCapturingGrids(board, oppMv);
                oppCaptures += static_cast<int>(capturedGrids.size());
                board.makeMove(oppMv, oppSide);
                captured = true;
                break;
            }
        }
        if (!captured) break;
    }
    return oppCaptures;
}

pair<bool, Move> makeMove(Controller &controller) {
    // Log("adv agent");

    const int time = controller.getTimeMs();
    Log(time);

    const Board &currBoard = controller.getCurrentBoard();
    vector<Move> validMoves = currBoard.getValidMoves();
    if (validMoves.empty())
        return {false, Move{0, 0, true}};

    if (time < 5000) {
        for (const Move &mv : validMoves) {
            if (currBoard.isCapturingMove(mv)) {
                const bool cont = controller.makeMove(mv);
                // Log("Scoring move: " + to_string(mv.row) + " " + to_string(mv.col));
                return {cont, mv};
            }
        }

        const Move &mv = validMoves[rand() % validMoves.size()];
        const bool cont = controller.makeMove(mv);
        // Log("Random move (low time): " + to_string(mv.row) + " " + to_string(mv.col));
        return {cont, mv};
    }

    const PlayerSide mySide = controller.getMySide();
    const PlayerSide oppSide = controller.getOpponentSide();

    const int dimcon1 = static_cast<int>(currBoard.rows * currBoard.cols * 0.1);
    const bool safeExists = hasSafeMove(currBoard, validMoves);

    // If safe moves exist → normal aggressive play
    if (safeExists) {
        // capture first
        for (const Move &mv : validMoves) {
            if (currBoard.isCapturingMove(mv)) {
                const bool cont = controller.makeMove(mv);
                // Log("Scoring move: " + to_string(mv.row) + " " + to_string(mv.col));
                return {cont, mv};
            }
        }
        // else take safe (non-dangerous) move
        for (const Move &mv : validMoves) {
            if (!isDangerousMove(currBoard, mv)) {
                const bool cont = controller.makeMove(mv);
                // Log("Safe move: " + to_string(mv.row) + " " + to_string(mv.col));
                return {cont, mv};
            }
        }
    }

    // compute total score once
    int totalScore = 0;
    const auto &scores = currBoard.getScores();
    for (const auto &p : scores) totalScore += p.second;
    const bool scorelimit = totalScore > dimcon1;

    // No safe move → double-cross or sacrifice phase
    if (scorelimit) {
        bool hasCapturing = false;
        for (const Move &mv : validMoves) {
            if (currBoard.isCapturingMove(mv)) {
                hasCapturing = true;
                break;
            }
        }

        if (hasCapturing) {
            // simulate how many boxes can be captured in a chain
            Board sim = currBoard;
            int chainCount = 0;
            while (true) {
                const auto moves = sim.getValidMoves();
                bool captured = false;
                for (const Move &mv : moves) {
                    if (sim.isCapturingMove(mv)) {
                        const auto &grids = getCapturingGrids(sim, mv);
                        chainCount += static_cast<int>(grids.size());
                        sim.makeMove(mv, mySide);
                        captured = true;
                        break;
                    }
                }
                if (!captured) break;
            }

            // Double-Cross condition: trap when only 2 boxes remain
            if (chainCount == 2) {
                for (const Move &mv : validMoves) {
                    if (currBoard.isCapturingMove(mv)) {
                        Board test = currBoard;
                        test.makeMove(mv, mySide);

                        const auto nextMoves = test.getValidMoves();
                        for (const Move &m2 : nextMoves) {
                            if (test.isCapturingMove(m2)) {
                                const bool cont = controller.makeMove(m2);
                                // Log("Double-Cross trap move: " + to_string(m2.row) + " " + to_string(m2.col));
                                return {cont, m2};
                            }
                        }
                    }
                }
            } else {
                // no double-cross possible, just capture everything
                for (const Move &mv : validMoves) {
                    if (currBoard.isCapturingMove(mv)) {
                        const bool cont = controller.makeMove(mv);
                        // Log("Capturing (no trap): " + to_string(mv.row) + " " + to_string(mv.col));
                        return {cont, mv};
                    }
                }
            }
        }
    }

    // capture if possible
    for (const Move &mv : validMoves) {
        if (currBoard.isCapturingMove(mv)) {
            const bool cont = controller.makeMove(mv);
            // Log("Capturing (no trap): " + to_string(mv.row) + " " + to_string(mv.col));
            return {cont, mv};
        }
    }

    // No capturing move and no safe move — minimize opponent’s advantage
    if (true) {
        int bestIndex = 0;
        int minOppCapt = 100000;

        for (int i = 0, n = static_cast<int>(validMoves.size()); i < n; ++i) {
            int oppCapt = simulateOpponentCaptures(currBoard, validMoves[i], mySide, oppSide);
            if (oppCapt < minOppCapt) {
                minOppCapt = oppCapt;
                bestIndex = i;
            }
            if (oppCapt < 3) break; // early stop
        }

        const Move &bestMove = validMoves[bestIndex];
        const bool cont = controller.makeMove(bestMove);
        // Log("Sacrifice minimizing move: " + to_string(bestMove.row) + " " + to_string(bestMove.col) +
            // " (OpponentCaptures=" + to_string(minOppCapt) + ")");
        return {cont, bestMove};
    }

    // fallback: random move if no score limit
    const Move &mv = validMoves[rand() % validMoves.size()];
    const bool cont = controller.makeMove(mv);
    // Log("Random move (no score limit): " + to_string(mv.row) + " " + to_string(mv.col));
    return {cont, mv};
}

void Agent::run() {
    Board board = controller->getCurrentBoard();
    while (!board.isCompleted()) {
        while (true) {
            auto [cont, _] = makeMove(*controller);
            if (!cont) break;
            board = controller->getCurrentBoard();
            if (board.isCompleted()) break;
        }
        board = controller->getCurrentBoard();
        if (board.isCompleted()) break;
    }
}
