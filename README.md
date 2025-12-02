# Dots and Boxes
<p align="center">
   <img src="https://github.com/user-attachments/assets/06374eae-f277-4361-9df5-b77a5f2e9803" alt="animated"/>
</p>
A Dots and Boxes game framework where you can play against an AI agents in C++ or Python or build one and play against it or fight the bots in a web interface.

## Agent 1
This agent uses a purely random strategy. At every turn, it retrieves the list of all currently valid moves from the board and simply picks one random move from that list using rand() % validMoves.size(). The agent does not evaluate the board, does not look ahead, and does not avoid giving the opponent boxes. It only plays whatever legal move is randomly selected.

## Agent 2
This new agent is still simple, but it is smarter than the previous fully-random agent. The improvement is that it now prioritizes capturing moves. Before choosing a random move, it checks all valid moves and selects any move that will complete a box. Only if no capturing move exists does it fall back to choosing a random move.

## Agent 3
This agent follows a simple but effective Dot-and-Boxes strategy: it first checks all available moves and immediately takes any capturing move that completes a box, ensuring it never misses a scoring opportunity. If no box can be completed, it then looks for a safe move, defined as any line that does not give an adjacent square its third side—avoiding moves that would set up the opponent for an easy capture. To do this, it counts how many sides each nearby box already has and labels a move as dangerous if it would create a 3-sided box. Only when no safe moves remain does it fall back to less optimal moves. Overall, the agent actively tries to maximize its own captures while minimizing chances for the opponent, making it significantly more strategic than a random or purely greedy agent.

## Agent 4
This agent adds a third and much stronger layer of decision-making, making it significantly more strategic than the earlier versions. It first takes any available capturing move, guaranteeing it never misses free points. If no captures exist, it tries up to 200 random moves to find a safe move—one that does not create a 3-sided box and therefore avoids giving the opponent an easy score. When even that fails, the agent enters a full look-ahead simulation: for every possible move it can make, it simulates the resulting board and then estimates how many boxes the opponent would be able to capture in a forced chain. It picks the move that leads to the fewest opponent captures, effectively minimizing damage in bad positions. Overall, this bot combines greedy scoring, risk avoidance, and forward simulation to choose moves that both maximize its own scoring opportunities and reduce the opponent’s future gains.

## Agent 5
This advanced agent is built around a deeper understanding of Dots and Boxes strategy, combining tactical safety checks with dynamic, simulation-based decision making. It begins by analyzing the board to identify “dangerous moves”—those that create a 3-sided box and are likely to hand free points to the opponent. Using the countSides and isDangerousMove helpers, the agent prioritizes safe, non-damaging moves whenever possible. It also incorporates a time-based decision mode: when the remaining time is low, it immediately focuses on fast decisions such as instant captures or random safe moves to avoid timeouts. This split between quick-play behavior and full-strength strategic play makes the agent robust in time-constrained scenarios.

When time allows, the agent shifts into a more sophisticated strategic posture. It evaluates whether the game has entered a “sacrifice” or “double-cross” phase, where safe moves no longer exist and controlled sacrifices become crucial. The agent estimates the current scoring dynamics, detects whether long capture chains or double-cross opportunities exist, and selectively captures or delays captures to force the opponent into unfavorable sequences. When no good tactical option is available, it simulates the opponent’s future capture chains for every possible move and chooses the one that minimizes their eventual score. By combining safety checks, capture-chain logic, double-cross conditions, and opponent-simulation minimization, this model plays with a balanced mix of aggression, caution, and long-term planning.

(incase an agent is not showing up, clear build directory of that agent and build as explained in **build c++ agent**
## Prerequisites

- **Python 3.9+** with pip
- **CMake 3.10+** (for C++ agents)
- **C++ compiler** with C++14 support (g++, clang, or MSVC)

## Install Python Dependencies

```bash
pip install fastapi uvicorn pydantic
```

## Launch Game Server

```bash
python3 ui.py
```

The server will start on `http://localhost:8000`. Open this URL in your web browser to access the game interface.


### Creating More Agents
The server **automatically recognizes** any new agents you create! Simply:
- Copy an existing agent folder (e.g., `cp -r python_agent python_agent2`)
- The new agent will appear in the dropdown automatically
- You can create as many agents as you want (python_agent2, cpp_agent2, cpp_agent3, etc.)

## How to Play

1. **Start the server:** Run `python3 ui.py`
2. **Open browser:** Navigate to `http://localhost:8000`
3. **Select agents:** Choose Player 1 and Player 2 from the dropdowns
4. **Configure board:** Set board dimensions (rows/cols) and initial random lines
5. **Start game:** Click "Start New Game"
6. **Watch or play:** 
   - If playing as **Human**, click on the board to draw lines
   - If watching **bots**, they will play automatically
7. **View results:** See final scores and time taken for each player

## Game Rules - Dots and Boxes

- Players take turns drawing lines between adjacent dots
- When a player completes a box (all 4 sides):
  - They capture that box and score **+1 point**
  - They get **another turn immediately**
- Game ends when all boxes are captured
- **Player with most boxes wins**

### Move Format
- **Horizontal line:** `row col true`
- **Vertical line:** `row col false`

---

## Building C++ Agents

```bash
cd cpp_agent/build
cmake ..
cmake --build . --config Release
cd ../..
```


---

## Advanced: Creating Your Own Agent

To create your own agent, edit the `make_move` function (Python) or `makeMove` function (C++) in the appropriate file inside the `submission/` folder:

- **Python:** Edit `python_agent/submission/agent.py`
- **C++:** Edit `cpp_agent/src/submission/agent.cpp`

**Note:** For C++ agents, the UI automatically rebuilds your code when you start a game, so you don't need to manually run build commands!

### Debugging Your Agent

Both Python and C++ agents have access to a logger for debugging:

**Python:**
```python
from custom_logger import logger

def make_move(controller: Controller) -> Tuple[bool, Move]:
    log('Hi from make_move function')
    time_ms = controller.get_time_ms()
    log(f'Time remaining: {time_ms} ms')
    # Your code here
```

**C++:**
```cpp
#include "customLogger.h"

std::pair<bool, Move> makeMove(Controller &controller) {
    Log("Hi from makeMove function");
    Log(controller.getTimeMs());
    // Your code here
}
```

Logger output appears in the terminal where you ran `python3 ui.py`.

### Creating Multiple Bots (Optional)

If you want to create multiple bots, simply duplicate the agent folder:

**Python:**
```bash
cp -r python_agent python_agent2
# Edit python_agent2/submission/agent.py
```

**C++:**
```bash
cp -r cpp_agent cpp_agent2
# Edit cpp_agent2/src/submission/agent.cpp
# remember to delete the build folder in case you copied that as well
```

The server will automatically detect, build (for C++), and list your new agents!

---

## Troubleshooting

### "Agent did not respond" error
- **C++ agents:** Verify the agent is built: `ls cpp_agent/build/agent`
- **Python agents:** Test standalone: `python3 -m python_agent`
- Check for infinite loops or crashes in your code

### Build errors (C++)
```bash
# Clean and rebuild
cd cpp_agent/build
rm -rf *
cmake ..
cmake --build . --config Release
```

### Port already in use
```bash
# Kill existing server (macOS/Linux)
lsof -ti:8000 | xargs kill -9

# Or use a different port in ui.py
```

### Time limit exceeded
- For testing your code with the UI, we have set the time limit to be 60 seconds per move
- Optimize your search algorithm

---

## Configuration Options

Edit `ui.py` to customize:

```python
TIME_LIMIT_SECS = 60  # Total time per move (seconds)
```

---

## Agent Communication Protocol

Agents communicate with the server via stdin/stdout using these commands:

| Command | Description |
|---------|-------------|
| `!REQ_PLAYER_NUM` | Request player ID (1 or 2) |
| `!REQ_BOARD` | Request current board state |
| `!REQ_TIME` | Request remaining time in milliseconds |
| `!REQ_MOVES` | Request opponent's moves |
| `!SENDING_MOVES` | Send your moves to server |

The controller handles this automatically - you don't need to implement the protocol yourself.

---

