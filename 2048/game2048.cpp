#include "game2048.h"
#include "ui_game2048.h"

Game2048::Game2048(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game2048)
{
    ui->setupUi(this);
}

Game2048::~Game2048()
{
    delete ui;
}

