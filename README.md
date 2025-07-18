# Slowfish Chess Engine

> This chess engine was a coding challenge I gave myself to improve upon my previous Java chess engine. It took a few days to complete, and I learned a whole bag of tricks you can use to make chess engines faster in the process (huge thanks to the Chess Programming Wiki, Sebastian Lague, and Bluefever Software). I might add more features in the future, but for now, it works well enough to play against and analyze positions. See if you can beat it!

## Overview
A C++ chess engine that supports the Universal Chess Interface (UCI) protocol. The engine features:

- **Board Representation**: It uses a 120 element integer array to simplify move generation. I'd eventually like to implement some kind of bitboard representation, but I haven't gotten around to it yet.
- **Evaluation Function**: It uses a handcrafted evaluation function that takes into account material, piece-square tables, and positional bonuses (e.g. passed/isolated pawns, open files, bishop pair), as well as adjustments for game phase for king safety, pawn structure, and mobility.
- **Transposition Table**: It uses a transposition table to store previously evaluated positions and their scores, allowing for faster lookups and reducing redundant calculations, since the same positions can often be reached through different move sequences in the evaluation tree.
- **Search Algorithm**: It uses alpha-beta pruning with quiescence search, PV scoring, killer/history heuristics, null move pruning, and heuristic move ordering. If you're interested, there's a great series of videos on these types of techniques by Sebastian Lague!
- **All One File**: The entire engine is contained in a single file, making it easy to compile and run, and to integrate into other projects or use with UCI-compatible chess GUIs. Though honestly, I just felt too lazy to organize it.

## Notes

The point of this entire engine was to improve my personal engine record, which I am happy to say I did! My old chess engine could search around 2000 nodes per second, which is terrible by modern chess computing standards. 

As a comparison, IBM's Deep Blue Supercomputer could evaluate around [700k nodes per second](https://web.archive.org/web/20081208081052/http://www.research.ibm.com/deepblue/watch/html/game1.log) during it's bout against Kasparov in 1997. At the start of the game, it took around 208 seconds to evaluate 137 million nodes. My chess engine, when comparing with a similar time metric of 141 seconds, evaluated nearly 600 million nodes at a speed of around 4 million nodes per second. That's over 4 times the speed of the chess engine that bested Kasparov.

That said, most modern engines (such as Stockfish, where my engine derives its name) can easily crush my engine any day of the weak. Where mine might take nearly 30 minutes for a depth of 15 (it scales pretty badly) stockfish can do it in 10ms. So there's certainly room to improve!

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
info depth 1 score cp 30 nodes 21 nps 0 time 0 pv e2e4
info depth 2 score cp 0 nodes 88 nps 0 time 0 pv e2e4 e7e5
info depth 3 score cp 25 nodes 674 nps 0 time 0 pv e2e4 d7d5 f1d3
info depth 4 score cp 0 nodes 3473 nps 3473000 time 1 pv e2e4 e7e5
info depth 5 score cp 25 nodes 8439 nps 2813000 time 3 pv e2e4 e7e5 d2d4 d7d5 c1e3
info depth 6 score cp 5 nodes 82074 nps 3568434 time 23 pv e2e4 e7e5 d2d4 b8c6 g1f3 f8d6
info depth 7 score cp 22 nodes 296772 nps 3956960 time 75 pv e2e4 d7d5 e4d5 d8d5 d2d4 e7e5 c1e3
info depth 8 score cp 5 nodes 2478674 nps 3744220 time 662 pv e2e4
info depth 9 score cp 22 nodes 5568379 nps 3877701 time 1436 pv e2e4 e7e6 d2d4 d7d5 b1c3 d5e4 c3e4 f8b4
info depth 10 score cp 12 nodes 48772131 nps 3960384 time 12315 pv e2e4
info depth 11 score cp 20 nodes 82102005 nps 3994259 time 20555 pv e2e4
info depth 12 score cp 10 nodes 576256706 nps 4070299 time 141576 pv e2e4
info depth 13 score cp 17 nodes 1189504554 nps 4086689 time 291068 pv e2e4
info depth 14 score cp 10 nodes 6902784793 nps 4114749 time 1677571 pv e2e4
bestmove e2e4

quit
```
