// Tic Tac Toe Game using C++ (C++17)
// Sanjivani University - Career Craft Academy powered by HCL Tech
// Build:  g++ -std=c++17 tictactoe.cpp -o tictactoe
// Run:    ./tictactoe   (Windows: tictactoe.exe)

#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
using namespace std;

// ---------- Input helper: returns -1 for non-numeric input ----------
int readInt() {
    int value;
    if (cin >> value) {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return value;
    }
    if (cin.eof()) throw runtime_error("Input closed.");
    cin.clear();                                               // clear fail flag
    cin.ignore(numeric_limits<streamsize>::max(), '\n');       // discard bad line
    return -1;
}


// ---------- Input helper: asks for a valid player name ----------
string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return s;
}

string readName(const string& prompt, const string& taken = "") {
    while (true) {
        cout << prompt;
        string s;
        if (!getline(cin, s)) throw runtime_error("Input closed.");
        size_t a = s.find_first_not_of(" \t\r");
        if (a == string::npos) { cout << "Name cannot be empty.\n"; continue; }
        size_t b = s.find_last_not_of(" \t\r");
        s = s.substr(a, b - a + 1);
        if (s.size() > 20)                { cout << "Name too long (max 20 characters).\n"; continue; }
        if (toLower(s) == "computer")     { cout << "That name is reserved. Choose another.\n"; continue; }
        if (!taken.empty() && toLower(s) == toLower(taken)) {
            cout << "That name is already used by Player 1. Choose another.\n";
            continue;
        }
        return s;
    }
}

// ---------- Board ----------
class Board {
    char cells[3][3];
public:
    Board() { reset(); }

    void reset() {
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++) cells[r][c] = ' ';
    }

    bool isEmpty(int pos) const {
        return cells[(pos - 1) / 3][(pos - 1) % 3] == ' ';
    }

    void placeMark(int pos, char mark) {
        if (pos < 1 || pos > 9)
            throw out_of_range("Enter a number between 1 and 9.");
        if (!isEmpty(pos))
            throw invalid_argument("That cell is already taken.");
        cells[(pos - 1) / 3][(pos - 1) % 3] = mark;
    }

    void clearCell(int pos) { cells[(pos - 1) / 3][(pos - 1) % 3] = ' '; }

    bool isFull() const {
        for (int pos = 1; pos <= 9; pos++)
            if (isEmpty(pos)) return false;
        return true;
    }

    // Returns 'X' or 'O' if someone has won, otherwise ' '
    char getWinner() const {
        for (int i = 0; i < 3; i++) {
            if (cells[i][0] != ' ' && cells[i][0] == cells[i][1] && cells[i][1] == cells[i][2])
                return cells[i][0];                       // row
            if (cells[0][i] != ' ' && cells[0][i] == cells[1][i] && cells[1][i] == cells[2][i])
                return cells[0][i];                       // column
        }
        if (cells[1][1] != ' ' &&
            ((cells[0][0] == cells[1][1] && cells[1][1] == cells[2][2]) ||
             (cells[0][2] == cells[1][1] && cells[1][1] == cells[2][0])))
            return cells[1][1];                           // diagonals
        return ' ';
    }

    void display() const {
        cout << "\n";
        for (int r = 0; r < 3; r++) {
            cout << "     ";
            for (int c = 0; c < 3; c++) {
                char ch = cells[r][c];
                if (ch == ' ') ch = static_cast<char>('1' + r * 3 + c);
                cout << " " << ch << " ";
                if (c < 2) cout << "|";
            }
            cout << "\n";
            if (r < 2) cout << "     ---+---+---\n";
        }
        cout << "\n";
    }
};

// ---------- Player hierarchy (run-time polymorphism) ----------
class Player {
protected:
    string name;
    char mark;
public:
    Player(string n, char m) : name(n), mark(m) {}
    virtual ~Player() {}
    string getName() const { return name; }
    char getMark() const { return mark; }
    virtual int chooseMove(Board& board) = 0;   // pure virtual
};

class HumanPlayer : public Player {
public:
    HumanPlayer(string n, char m) : Player(n, m) {}
    int chooseMove(Board&) override {
        cout << name << " (" << mark << "), enter cell number (1-9): ";
        return readInt();
    }
};

class ComputerPlayer : public Player {
    char opp;

    int minimax(Board& board, bool computerTurn, int depth) {
        char w = board.getWinner();
        if (w == mark) return 10 - depth;
        if (w == opp)  return depth - 10;
        if (board.isFull()) return 0;

        int best = computerTurn ? -1000 : 1000;
        for (int pos = 1; pos <= 9; pos++) {
            if (!board.isEmpty(pos)) continue;
            board.placeMark(pos, computerTurn ? mark : opp);
            int score = minimax(board, !computerTurn, depth + 1);
            board.clearCell(pos);                         // undo the move
            if (computerTurn) { if (score > best) best = score; }
            else              { if (score < best) best = score; }
        }
        return best;
    }
public:
    ComputerPlayer(string n, char m) : Player(n, m), opp(m == 'X' ? 'O' : 'X') {}

    int chooseMove(Board& board) override {
        int bestPos = -1, bestScore = -1000;
        for (int pos = 1; pos <= 9; pos++) {
            if (!board.isEmpty(pos)) continue;
            board.placeMark(pos, mark);
            int score = minimax(board, false, 1);
            board.clearCell(pos);
            if (score > bestScore) { bestScore = score; bestPos = pos; }
        }
        cout << name << " (" << mark << ") plays cell " << bestPos << "\n";
        return bestPos;
    }
};

// ---------- Scoreboard with file handling ----------
struct Scores {
    map<string, int> wins;   // player name -> number of wins
    int draws = 0;
};

class ScoreFile {
    string filename;
public:
    explicit ScoreFile(string f = "scores.txt") : filename(f) {}

    // File format: first line = draws, then one "wins name" line per player
    Scores load() const {
        Scores s;
        ifstream in(filename);
        if (!in) return s;
        in >> s.draws;
        in.ignore(numeric_limits<streamsize>::max(), '\n');
        int w;
        while (in >> w) {
            string name;
            getline(in, name);
            size_t a = name.find_first_not_of(" \t\r");
            if (a == string::npos) continue;
            size_t b = name.find_last_not_of(" \t\r");
            s.wins[name.substr(a, b - a + 1)] = w;
        }
        return s;
    }

    void save(const Scores& s) const {
        ofstream out(filename);
        if (!out) { cout << "Warning: could not save scores.\n"; return; }
        out << s.draws << "\n";
        for (const auto& entry : s.wins)
            out << entry.second << " " << entry.first << "\n";
    }
};

// ---------- Game controller ----------
class Game {
    Board board;
    ScoreFile file;
    Scores scores;

    // Plays one round; returns the winner's mark, or ' ' for a draw
    char playRound(Player& p1, Player& p2) {
        board.reset();
        Player* current = &p1;
        while (true) {
            board.display();
            while (true) {
                try {
                    int pos = current->chooseMove(board);
                    board.placeMark(pos, current->getMark());
                    break;
                } catch (const logic_error& e) {
                    cout << "Invalid move: " << e.what() << "\n";
                }
            }
            char w = board.getWinner();
            if (w != ' ') { board.display(); return w; }
            if (board.isFull()) { board.display(); return ' '; }
            current = (current == &p1) ? &p2 : &p1;
        }
    }

    void playSession(bool vsComputer) {
        unique_ptr<Player> p1, p2;
        if (vsComputer) {
            string n = readName("Enter your name: ");
            p1 = make_unique<HumanPlayer>(n, 'X');
            p2 = make_unique<ComputerPlayer>("Computer", 'O');
        } else {
            string n1 = readName("Enter Player 1 name (X): ");
            string n2 = readName("Enter Player 2 name (O): ", n1);
            p1 = make_unique<HumanPlayer>(n1, 'X');
            p2 = make_unique<HumanPlayer>(n2, 'O');
        }
        // make sure both players appear on the scoreboard, even with 0 wins
        scores.wins.emplace(p1->getName(), 0);
        scores.wins.emplace(p2->getName(), 0);

        int again = 1;
        while (again == 1) {
            char w = playRound(*p1, *p2);
            if (w == ' ') {
                cout << "It's a draw!\n";
                scores.draws++;
            } else {
                Player& winner = (w == p1->getMark()) ? *p1 : *p2;
                cout << winner.getName() << " wins!\n";
                scores.wins[winner.getName()]++;
            }
            file.save(scores);

            cout << "Play again? (1 = Yes, 0 = No): ";
            again = readInt();
            while (again != 0 && again != 1) {
                cout << "Please enter 1 or 0: ";
                again = readInt();
            }
        }
    }

    void showScores() const {
        cout << "\n===== SCOREBOARD =====\n";
        if (scores.wins.empty() && scores.draws == 0) {
            cout << "No games played yet.\n";
            return;
        }
        cout << left << setw(22) << "Player" << "Wins\n"
             << "---------------------------\n";
        for (const auto& entry : scores.wins)
            cout << left << setw(22) << entry.first << entry.second << "\n";
        cout << "---------------------------\n"
             << left << setw(22) << "Draws" << scores.draws << "\n";
    }

public:
    Game() { scores = file.load(); }

    void run() {
        int choice = 0;
        while (choice != 4) {
            cout << "\n===== TIC TAC TOE =====\n"
                 << "1. Player vs Player\n"
                 << "2. Player vs Computer\n"
                 << "3. View Scoreboard\n"
                 << "4. Exit\n"
                 << "Enter your choice: ";
            choice = readInt();
            switch (choice) {
                case 1: playSession(false); break;
                case 2: playSession(true);  break;
                case 3: showScores();       break;
                case 4: cout << "Thanks for playing. Goodbye!\n"; break;
                default: cout << "Invalid choice. Please enter 1-4.\n";
            }
        }
    }
};

int main() {
    try {
        Game game;
        game.run();
    } catch (const exception& e) {
        cout << "\nProgram ended: " << e.what() << "\n";
    }
    return 0;
}
