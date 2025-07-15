# Slowfish Chess Engine

> This chess engine was a coding challenge I gave myself to improve upon my previous Java chess engine. It took a few days to complete, and I learned a whole bag of tricks you can use to make chess engines faster in the process (huge thanks to the Chess Programming Wiki, Sebastian Lague, and Bluefever Software). I might add more features in the future, but for now, it works well enough to play against and analyze positions. See if you can beat it!

## Overview
A C++ chess engine that supports the Universal Chess Interface (UCI) protocol. The engine features:

- **Board Representation**: It uses a 120 element integer array to simplify move generation. I'd eventually like to implement some kind of bitboard representation, but I haven't gotten around to it yet.
- **Evaluation Function**: It uses a handcrafted evaluation function that takes into account material, piece-square tables, and positional bonuses (e.g. passed/isolated pawns, open files, bishop pair), as well as adjustments for game phase for king safety, pawn structure, and mobility.
- **Transposition Table**: It uses a transposition table to store previously evaluated positions and their scores, allowing for faster lookups and reducing redundant calculations, since the same positions can often be reached through different move sequences in the evaluation tree.
- **Search Algorithm**: It uses alpha-beta pruning with quiescence search, PV scoring, killer/history heuristics, null move pruning, and heuristic move ordering. If you're interested, there's a great series of videos on these types of techniques by Sebastian Lague!
- **All One File**: The entire engine is contained in a single file, making it easy to compile and run, and to integrate into other projects or use with UCI-compatible chess GUIs. Though honestly, I just felt too lazy to organize it.

## UCI Interface

The engine supports the Universal Chess Interface (UCI) protocol, allowing it to work with any UCI-compatible chess GUI or interface.

### Supported UCI Commands

#### Engine Identification
- `uci` - Initialize UCI mode
- `isready` - Check if engine is ready
- `quit` - Exit the engine

#### Game Control
- `ucinewgame` - Start a new game
- `position startpos` - Set position to starting position
- `position fen <fenstring>` - Set position from FEN string
- `position startpos moves <move1> <move2> ...` - Set position and play moves
- `position fen <fenstring> moves <move1> <move2> ...` - Set FEN position and play moves

#### Search Control
- `go movetime <ms>` - Search for a number of milliseconds
- `go depth <depth>` - Search up to the specified depth
- `go nodes <nodes>` - Search a specified number of nodes
- `stop` - Stop current search as soon as possible

### Move Format

Moves are in long UCI algebraic notation:
- `e2e4` - Pawn from e2 to e4
- `e1g1` - White kingside castling
- `e8c8` - Black queenside castling
- `e7e8q` - Pawn promotion to queen


## Testing Positions

Here are some positions I used to test the engine:

* `7r/p3ppk1/3p4/2p1P1Kp/2Pb4/3P1QPq/PP5P/R6R b - - 0 1` - Mate in 2 (d4e3).
* `rn3rk1/p5pp/2p5/3Ppb2/2q5/1Q6/PPPB2PP/R3K1NR b - - 0 1` - Mate in 3 (c4f1).
* `3r2k1/p4ppp/b1pb1Q2/q7/8/1B3p2/PBPPNP1P/1R2K1R1 b - - 1 0` - Mate in 4 (a5d2).
* `R4r1k/6pp/2pq4/2n2b2/2Q1pP1b/1r2P2B/NP5P/2B2KNR b - - 1 24` - Mate in 12 (f5h3).
* `1r4k1/3n1r1p/R3p1p1/2p5/2Q1N3/1q4PP/1b2PPB1/3R2K1 w - - 0 1` - Best move is queen to b3 (c4b3).
* `8/8/7k/8/8/8/5q2/3B2RK b - - 0 1` - Best move is king to h7, winning a piece (h6h7).

## Compilation

```bash
g++ -O3 -march=native -mtune=native -flto -funroll-loops -ffast-math src/slowfish.cpp -o slowfish.exe
```

## Example UCI Usage

```
uci
id name slowfish
uciok

isready
readyok

position startpos

go movetime 1000
info depth 1 score cp 30 nodes 21 nps 0 time 0 pv d2d4
info depth 2 score cp 0 nodes 89 nps 0 time 0 pv d2d4 d7d5
info depth 3 score cp 25 nodes 637 nps 637000 time 1 pv d2d4 d7d5 c1e3
info depth 4 score cp 0 nodes 3834 nps 1917000 time 2 pv d2d4 d7d5 c1e3 c8e6
info depth 5 score cp 25 nodes 11867 nps 2966750 time 4 pv e2e4 e7e5 d2d4 d7d5 c1e3
info depth 6 score cp 5 nodes 81217 nps 3384041 time 6 pv e2e4 e7e5 d2d4 b8c6 g1f3 f8d6
info depth 7 score cp 22 nodes 247051 nps 3743196 time 21 pv e2e4 d7d5 e4d5 d8d5 d2d4 e7e5 c1e3
info depth 8 score cp 5 nodes 2286791 nps 3786077 time 64 pv e2e4 e7e5 g1f3 g8f6 b1c3 b8c6 d2d4
info depth 9 score cp 22 nodes 5660667 nps 3952979 time 206 pv e2e4
info depth 10 score cp 12 nodes 41294543 nps 3909727 time 992 pv e2e4
bestmove e2e4

quit
```