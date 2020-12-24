#include <QApplication>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QObject>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QPropertyAnimation>
#include <bits/stdc++.h>
#include <unistd.h>
#include <game2048.h>

const int MAX_SIZE = 10;
const int board_size = 4;
const int tile_size = 150;

struct pos {
    int x, y;

    pos operator + (pos b) const {
        return pos(x + b.x, y + b.y);
    }

    pos(int x = 0, int y = 0) {
        this->x = x; this->y = y;
    }

    bool isValidPosition() { // to check whether the position is valid or not
        return (x > 0 && x <= board_size && y > 0 && y <= board_size);
    }
} ;

const pos LEFT = pos(0, -1);
const pos RIGHT = pos(0, 1);
const pos UP = pos(-1, 0);
const pos DOWN = pos(1, 0);

int p[2][MAX_SIZE];
int R[32769], G[32769], B[32769];

Game2048* Frame;
QStatusBar* Bar;

int randInt(int l, int r) { // get a random integer in interval [l,r]
    return l + (1LL * rand() * rand()) % (r - l + 1);
}

QPushButton* newButton(QString text, int font_size = 50) {
    QPushButton* button = new QPushButton(Frame);
    QFont font;
    font.setFamily("consolas");
    font.setPointSize(font_size);
    button->setFont(font);
    button->setText(text);
    button->show();
    button->setFocusPolicy(Qt:: NoFocus);
    return button;
}

class Tile {
    private:
        int value;
        pos position;
        QPushButton* button;
        QPropertyAnimation* animation;

    public:

        Tile(int value = 2, pos position = pos(0, 0), bool zoom = true) {
            this->value = value;
            this->position = position;
            this->button = newButton(QString:: number(this->getValue()));
            this->animation = nullptr;
            if (zoom) {
                this->addAnimation(QRect(30 + (this->position.y - 1 + 0.5) * tile_size, 50 + (this->position.x - 1 + 0.5) * tile_size, 0, 0), QRect(30 + (this->position.y - 1) * tile_size, 50 + (this->position.x - 1) * tile_size, tile_size, tile_size), 300);
            } else {
                this->button->setGeometry(QRect(30 + (this->position.y - 1) * tile_size, 50 + (this->position.x - 1) * tile_size, tile_size, tile_size));
            }
            color();
       }

        ~Tile() {
            this->button->hide();
            delete this->button;
            this->button = nullptr;
            if (this->animation != nullptr) {
                delete this->animation;
                this->animation = nullptr;
            }
        }

        int getValue() {
            return this->value;
        }

        QRect getGeometry() {
            return this->button->geometry();
        }

        void hide() {
            delete this->button;
        }

        void show(bool zoom = false) {
            this->button = newButton(QString:: number(this->getValue()));
            if (zoom) {
                this->addAnimation(QRect(30 + (this->position.y - 1 + 0.5) * tile_size, 50 + (this->position.x - 1 + 0.5) * tile_size, 0, 0), QRect(30 + (this->position.y - 1) * tile_size, 50 + (this->position.x - 1) * tile_size, tile_size, tile_size), 300);
            } else {
                this->button->setGeometry(QRect(30 + (this->position.y - 1) * tile_size, 50 + (this->position.x - 1) * tile_size, tile_size, tile_size));
            }
            color();
        }

        bool isStopped() {
            if (this->animation == nullptr) return true;
            if (this->animation->state() == QPropertyAnimation:: Stopped) {
                delete this->animation;
                this->animation = nullptr;
                return true;
            }
            return false;
        }

        void color() {
            QString r = QString:: number(R[this->value]);
            QString g = QString:: number(G[this->value]);
            QString b = QString:: number(B[this->value]);
            this->button->setStyleSheet("background-color: rgb(" + r + "," + g + "," + b + ")");
        }

        void mulTwo() {
            this->value <<= 1;
            this->button->setText(QString:: number(this->value));
            this->color();
        }

        void addAnimation(QRect startValue, QRect endValue, int duration) {
            delete this->animation;
            this->animation = new QPropertyAnimation(this->button, "geometry");
            this->animation->setDuration(duration);
            this->animation->setStartValue(startValue);
            this->animation->setEndValue(endValue);
            this->animation->setEasingCurve(QEasingCurve:: OutQuad);
            this->animation->start();
        }

        bool move(Tile* (*board)[MAX_SIZE], pos direction) {
            bool successfully_moved = false;
            pos current_pos = this->position;
            for (pos next_pos = current_pos + direction; next_pos.isValidPosition(); current_pos = next_pos, next_pos = next_pos + direction) {
                if (board[next_pos.x][next_pos.y] == nullptr) {
                    std:: swap(board[next_pos.x][next_pos.y], board[current_pos.x][current_pos.y]);
                    successfully_moved = true;
                } else break;
            }
            this->addAnimation(this->button->geometry(), QRect(30 + (current_pos.y - 1) * tile_size, 50 + (current_pos.x - 1) * tile_size, tile_size, tile_size), 180);
            this->position = current_pos;
            return successfully_moved;
        }
} ;

class GameState {
    private:
        std:: queue <Tile*> delete_later;
        Tile* board[MAX_SIZE][MAX_SIZE];
        QLabel* label;
        int current_score;

    public:
        GameState() {
            this->current_score = 0;
            for (int i = 0; i < MAX_SIZE; ++ i) {
                for (int j = 0; j < MAX_SIZE; ++ j) {
                    this->board[i][j] = nullptr;
                }
            }
            this->generateTile();
            this->generateTile();
            label = new QLabel(Frame);
            QFont font;
            font.setFamily("consolas");
            font.setPointSize(50);
            label->setFont(font);
            label->show();
            label->setGeometry(30, 680, 280, 80);
            label->setNum(0);
        }

        GameState(const GameState &B) {
            this->current_score = B.current_score;
            for (int i = 0; i < MAX_SIZE; ++ i) {
                for (int j = 0; j < MAX_SIZE; ++ j) {
                    if (B.board[i][j] != nullptr) {
                        this->board[i][j] = new Tile(B.board[i][j]->getValue(), pos(i, j), false);
                    } else {
                        this->board[i][j] = nullptr;
                    }
                }
            }
            label = new QLabel(Frame);
            QFont font;
            font.setFamily("consolas");
            font.setPointSize(50);
            label->setFont(font);
            label->show();
            label->setGeometry(30, 680, 280, 80);
            label->setNum(B.current_score);
        }

        ~GameState() {
            for (int i = 0; i < MAX_SIZE; ++ i) {
                for (int j = 0; j < MAX_SIZE; ++ j) {
                    delete this->board[i][j];
                    this->board[i][j] = nullptr;
                }
            }
            delete this->label;
            this->label = nullptr;
            for (; !delete_later.empty(); ) {
                auto dump = delete_later.front();
                delete dump;
                delete_later.pop();
            }
        }

        void hide() {
            for (int i = 1; i <= board_size; ++ i) {
                for (int j = 1; j <= board_size; ++ j) {
                    if (this->board[i][j] != nullptr) {
                        this->board[i][j]->hide();
                    }
                }
            }
            this->label->hide();
        }

        void show() {
            for (int i = 1; i <= board_size; ++ i) {
                for (int j = 1; j <= board_size; ++ j) {
                    if (this->board[i][j] != nullptr) {
                        this->board[i][j]->show();
                    }
                }
            }
            this->label->show();
        }

        void generateTile() {
            std:: vector<pos> unused;
            for (int i = 1; i <= board_size; ++ i) {
                for (int j = 1; j <= board_size; ++ j) {
                    if (board[i][j] == nullptr) {
                        unused.push_back(pos(i, j));
                    }
                }
            }
            if (unused.size() != 0) {
                pos tar = unused[randInt(0, (int)unused.size() - 1)];
                board[tar.x][tar.y] = new Tile(randInt(1, 2) * 2, tar);
            }
        }

        bool isLost() {
            for (int i = 1; i <= board_size; ++ i) {
                for (int j = 1; j <= board_size; ++ j) {
                    if (board[i][j] == nullptr) {
                        return false;
                    }
                }
            }
            for (int i = 1; i <= board_size; ++ i) {
                for (int j = 1; j <= board_size; ++ j) {
                    if (i > 1 && board[i][j]->getValue() == board[i - 1][j]->getValue()) return false;
                    if (j > 1 && board[i][j]->getValue() == board[i][j - 1]->getValue()) return false;
                }
            }
            return true;
        }

        void gameOver() {
            QMessageBox:: information(NULL, "You Lose!", "You Lose!", QMessageBox::Yes, QMessageBox::Yes);
        }

        bool move(int* pi, int* pj, int type) {
            bool successfully_moved = false;
            pos directions[] = {LEFT, RIGHT, UP, DOWN};
            for (int i = 1; i <= board_size; ++ i) {
                for (int j = 1; j <= board_size; ++ j) {
                    if (board[pi[i]][pj[j]] != nullptr) {
                        successfully_moved |= board[pi[i]][pj[j]]->move(this->board, directions[type]);
                    }
                }
            }
            return successfully_moved;
        }

        bool merge(int *pi, int *pj, int type) {
            for (Tile* dump; !delete_later.empty(); ) {
                dump = delete_later.front();
                if (dump->isStopped()) {
                    delete dump;
                    dump = nullptr;
                    delete_later.pop();
                } else break;
            }
            bool successfully_merged = false;
            successfully_merged |= move(pi, pj, type);
            for (int i = 1; i <= board_size; ++ i) {
                for (int j = 1; j <= board_size; ++ j) {
                    int x = pi[i], px = pi[i - (type > 1)], y = pj[j], py = pj[j - (type <= 1)];
                    if (pos(px, py).isValidPosition() && board[x][y] != nullptr && board[px][py] != nullptr) {
                        if (board[x][y]->getValue() == board[px][py]->getValue()) {
                            if (type <= 1) {
                                board[x][y]->addAnimation(board[x][y]->getGeometry(), QRect(board[px][py]->getGeometry().x(), board[px][py]->getGeometry().y(), 0, tile_size), 100);
                            } else {
                                board[x][y]->addAnimation(board[x][y]->getGeometry(), QRect(board[px][py]->getGeometry().x(), board[px][py]->getGeometry().y(), tile_size, 0), 100);
                            }
                            delete_later.push(board[x][y]);
                            board[x][y] = nullptr;
                            board[px][py]->mulTwo();
                            current_score += board[px][py]->getValue();
                            successfully_merged = true;
                        }
                    }
                }
            }
            successfully_merged |= move(pi, pj, type);
            label->setNum(current_score);
            return successfully_merged;
        }

        void saveGame(std:: string path = "sav.dat") {
            std:: fstream file;
            file.open(path, std:: ios:: out | std:: ios:: trunc);
            if (file.fail()) {
                std:: cout << "failed!" << std:: endl;
                return ;
            }
            for (int i = 1; i <= board_size; ++ i) {
                for (int j = 1; j <= board_size; ++ j) {
                    int tmp = 0;
                    if (board[i][j] != nullptr) {
                        tmp = board[i][j]->getValue();
                    }
                    file << tmp << " ";
                }
            }
            file << current_score;
            file.close();
        }

        void loadGame(std:: string path = "sav.dat") {
            for (; !delete_later.empty(); ) {
                auto dump = delete_later.front();
                delete_later.pop();
                delete dump;
            }
            std:: fstream file;
            file.open(path, std:: ios:: in);
            for (int i = 1; i <= board_size; ++ i) {
                for (int j = 1; j <= board_size; ++ j) {
                    int value;
                    file >> value;
                    if (board[i][j] != nullptr) {
                        delete board[i][j];
                    }
                    board[i][j] = nullptr;
                    if (value != 0) {
                        board[i][j] = new Tile(value, pos(i, j));
                    }
                }
            }
            file >> current_score;
            file.close();
            this->label->setNum(current_score);
        }
} ;

std:: stack <GameState*> stack;

GameState* current_game_state;

class MyButtons : public QPushButton {
    private:
        unsigned type;

    public:
        MyButtons(QWidget* parent = nullptr, QString text = "", unsigned type = 1) : QPushButton(parent) {
            this->type = type;
            if (type == 1) {
                connect(this, &QPushButton:: clicked, this, &MyButtons:: saveGame);
                this->setGeometry(400, 680, 110, 80);
                this->setText(text);
            } else if (type == 0) {
                connect(this, &QPushButton:: clicked, this, &MyButtons:: loadGame);
                this->setGeometry(520, 680, 110, 80);
                this->setText(text);
            } else {
                connect(this, &QPushButton:: clicked, this, &MyButtons:: backStep);
                this->setGeometry(320, 690, 60, 60);

                this->setIconSize(QSize(48, 48));
                this->setIcon(QIcon("backstep.png"));
            }
            QFont font;
            font.setFamily("consolas");
            font.setPointSize(25);
            this->setFont(font);
            this->show();
            this->setFocusPolicy(Qt:: NoFocus);
            this->setCursor(QCursor(Qt::PointingHandCursor));
        }

        ~MyButtons() {
            if (this->type == 1) {
                disconnect(this, &QPushButton:: clicked, this, &MyButtons:: saveGame);
            } else if (this->type == 0) {
                disconnect(this, &QPushButton:: clicked, this, &MyButtons:: loadGame);
            } else {
                disconnect(this, &QPushButton:: clicked, this, &MyButtons:: backStep);
            }
        }

    private slots:
        void saveGame() {
            current_game_state->saveGame();
        }

        void loadGame() {
            current_game_state->loadGame();
        }

        void backStep() {
            if (stack.size() > 1) {
                delete current_game_state;
                stack.pop();
                current_game_state = stack.top();
                current_game_state->show();
            }
        }
};

MyButtons *saveButton, *loadButton, *backButton;

void pre() { // Some preparations including setting colors and iteration orders
    for (int i = 0; i <= board_size + 1; ++ i) {
        p[0][i] = i;
        p[1][board_size + 1 - i] = p[0][i];
    }
    srand((int) time(NULL));
    for (int i = 2, j = 1; i <= 8192; i <<= 1, ++ j) {
        R[i] = G[i] = B[i] = 250 - (j - 1) * 9;
    }
    saveButton = new MyButtons(Frame, "Save", 1);
    loadButton = new MyButtons(Frame, "Load", 0);
    backButton = new MyButtons(Frame, "Back", 2);
}

void test(std:: string path) {
    current_game_state->loadGame(path);
}

void Game2048:: keyPressEvent(QKeyEvent* event) { // React differently according to the key pressed
    GameState* tmp_game_state = current_game_state;
    tmp_game_state->hide();
    current_game_state = new GameState(*tmp_game_state);
    int flag = 0;
    auto Key = event->key();
    if (Key == Qt:: Key_Up) {
        flag = current_game_state->merge(p[0], p[0], 2);
    } else if (Key == Qt:: Key_Left) {
        flag = current_game_state->merge(p[0], p[0], 0);
    } else if (Key == Qt:: Key_Right) {
        flag = current_game_state->merge(p[0], p[1], 1);
    } else if (Key == Qt:: Key_Down) {
        flag = current_game_state->merge(p[1], p[0], 3);
    }
    if (flag == 1) {
        current_game_state->generateTile();
        stack.push(current_game_state);
    } else {
        delete current_game_state;
        current_game_state = tmp_game_state;
        current_game_state->show();
    }
    if (current_game_state->isLost()) {
        current_game_state->gameOver();
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Game2048 Window;
    Frame = &Window;
    Frame->show();
    pre();
    current_game_state = new GameState();
    stack.push(current_game_state);
    test("../Tests/test06");
    return a.exec();
}
