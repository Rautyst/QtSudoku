#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QMainWindow>
#include <QStackedWidget>
#include <QGridLayout>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QSpacerItem>
#include <QIntValidator>
#include <QTimer>
#include <QLabel>
#include <QFile>

class CellBtn : public QPushButton
{
    Q_OBJECT
public:
    CellBtn(QWidget* parent);

    int GetDigit() const;
    bool IsLocked() const;
    void SetDigit(int digit);
    void Lock();
    void Open();

private slots:
    void ChangeDigit();

private:
    void mousePressEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void UpdateColor();

    int _digit;
    bool _is_open;
};

class Sudoku : public QWidget
{
    Q_OBJECT
public:
    Sudoku(QWidget* parent);

    static bool IsSandboxMode();
public slots:
    void Generate(int open_slots_count);
private slots:
    void Solve();
    void Help();
    void ClickedReturnBtn();
    void Check();

    void Update();
signals:
    void ReturnToMenu();
private:
    std::pair<int,int> FindError();

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;

    QPushButton* _check;
    QPushButton* _solve;
    QPushButton* _return;
    QPushButton* _help;
    CellBtn* _cells[9][9];

    QTimer* _timer;
    uint16_t _seconds;
    QLabel* _timer_lbl;

    static inline bool _sandbox_mode = false;
    int _open_slots_count;
};

class Menu : public QWidget
{
    Q_OBJECT
public:
    Menu(QWidget* parent);
private:
    QPushButton* _play;
    QPushButton* _exit;
    QLineEdit* _setting;
private slots:
    void ClickedPlayBtn();
    void ClickedExitBtn();
signals:
    void Play(int setting);
    void Close();
};

class SdkWindow : public QMainWindow
{
    Q_OBJECT
public:
    SdkWindow();
private:
    Menu* _m;
    Sudoku* _sdk;
    QStackedWidget* _main_widget;
private slots:
    void gotoMenu();
    void gotoSudoku(int setting);
    void ClickedExitBtn();
signals:
    void Close();
};
