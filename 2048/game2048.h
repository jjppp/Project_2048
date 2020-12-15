#ifndef GAME2048_H
#define GAME2048_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Game2048; }
QT_END_NAMESPACE

class Game2048 : public QMainWindow
{
    Q_OBJECT

public:
    Game2048(QWidget *parent = nullptr);
    ~Game2048();
    void keyPressEvent(QKeyEvent* event);

private:
    Ui::Game2048 *ui;

//    void keyReleaseEvent(QKeyEvent* event);
};
#endif // GAME2048_H
