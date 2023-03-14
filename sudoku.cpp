#include "sudoku.h"

CellBtn::CellBtn(QWidget* parent) : QPushButton("0",parent), _digit(0),_is_open(true)
{
    connect(this,SIGNAL(clicked()),this,SLOT(ChangeDigit()));
    this->setStyleSheet("background-color:"+QColor(255,255,255).name());
}

uint8_t CellBtn::GetDigit() const
{
    return _digit;
}

bool CellBtn::IsLocked() const
{
    return !_is_open;
}

void CellBtn::SetDigit(const uint8_t digit)
{
    _digit = digit;
    setText(QString::number(_digit));
    UpdateColor();
}

void CellBtn::Lock()
{
    if (_is_open)
    {
        _is_open = false;
        disconnect(this,SIGNAL(clicked()),this,SLOT(ChangeDigit()));
        UpdateColor();
    }
}

void CellBtn::Open()
{
    if (not _is_open)
    {
        _is_open = true;
        connect(this,SIGNAL(clicked()),this,SLOT(ChangeDigit()));
        UpdateColor();
    }
}

void CellBtn::ChangeDigit()
{
    _digit += 1;
    if (_digit == 10)
    {
        _digit = 0;
    }
    setText(QString::number(_digit));
    UpdateColor();
}

void CellBtn::resizeEvent(QResizeEvent*)
{
    QFont tmp;
    tmp.setPixelSize(this->height()/2);
    this->setFont(tmp);
}

void CellBtn::UpdateColor()
{
    if (IsLocked())
    {
        this->setStyleSheet("background-color:"+QColor(255,100,100).name());
        return;
    }
    if (_digit == 0)
    {
        this->setStyleSheet("background-color:"+QColor(255,255,255).name());
        return;
    }

    this->setStyleSheet("background-color:"+QColor(213/(_digit+1),25*(_digit+1),133).name());
}

Sudoku::Sudoku(QWidget* parent) : QWidget(parent)
{
    QGridLayout* main_layout = new QGridLayout(this);
    for (uint8_t i = 0; i < 9; i+=1)
    {
        for (uint8_t j = 0; j < 9; j+=1)
        {
            _cells[i][j] = new CellBtn(this);
            _cells[i][j]->setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Preferred );
            main_layout->addWidget(_cells[i][j],i + i/3,j + j/3);
        }
    }
    main_layout->addItem(new QSpacerItem(30,30),3,3);
    main_layout->addItem(new QSpacerItem(30,30),7,7);
    _check  = new QPushButton(this);
    _solve  = new QPushButton(this);
    _return = new QPushButton(this);
    _check ->setText("Check");
    _solve ->setText("Get Solve");
    _return->setText("Return to Menu");
    connect(_solve ,SIGNAL(clicked()),this,SLOT(Solve()));
    connect(_check ,SIGNAL(clicked()),this,SLOT(Check()));
    connect(_return,SIGNAL(clicked()),this,SLOT(ClickedReturnBtn()));
    main_layout->addWidget(_solve ,13,0,1,3);
    main_layout->addWidget(_return,13,8,1,3);
    main_layout->addWidget(_check ,13,4,1,3);

    this->setLayout(main_layout);
    setWindowTitle("Sudoku");
}

// Данный класс нужен только для того, чтобы создавать и решать судоку
class Cell
{
private:
    bool* _free_digits;
    uint8_t _digit; // 0 - нет числа
public:
    Cell(const uint8_t digit = 0)
    {
        _digit = digit;
        _free_digits = new bool[9];
        for (uint8_t i = 0; i < 9; i += 1) _free_digits[i] = true; // изначально все свободны
    }

    ~Cell()
    {
        delete[] _free_digits;
    }

    void Reset()
    {
        for (uint8_t i = 0; i < 9; i += 1) _free_digits[i] = true; // изначально все свободны
        _digit = 0;
    }

    uint8_t GetDigit()
    {
        return _digit;
    }

    void RemoveFD(uint8_t digit)
    {
        _free_digits[digit - 1] = false;
    }

    bool GenerateDigit()
    {
        uint8_t fd_count = 0;
        for (uint8_t i = 0; i < 9; i += 1) fd_count += _free_digits[i];

        if (fd_count == 0) return false;

        uint8_t tmp = rand() % fd_count;
        uint8_t true_num = 0;
        while (true)
        {
            if (_free_digits[true_num])
            {
                if (tmp == 0) break;
                tmp -= 1;
            }
            true_num += 1;
        }

        _digit = true_num + 1;
        _free_digits[true_num] = false;
        return true;
    }

    void SetDigit(uint8_t digit)
    {
        _digit = digit;
    }
};

void Sudoku::Generate(uint8_t open_slots_count)
{
    Cell*** sdk = new Cell**[9];
    for (uint8_t i = 0; i < 9; i += 1)
    {
        sdk[i] = new Cell*[9] ;
        for (uint8_t j = 0; j < 9; j += 1)
        {
            sdk[i][j] = new Cell{9};
        }
    }

    for (uint8_t row = 0; row < 9; )
    {
        for (uint8_t line = 0; line < 9; )
        {
            for (uint8_t tmp_line = 0; tmp_line < line; tmp_line += 1)
            {
                sdk[row][line]->RemoveFD(sdk[row][tmp_line]->GetDigit());
            }
            for (uint8_t tmp_row = 0; tmp_row < row; tmp_row += 1)
            {
                sdk[row][line]->RemoveFD(sdk[tmp_row][line]->GetDigit());
            }

            for (uint8_t tmp_row = (row / 3) * 3; tmp_row < row; tmp_row += 1)
            {
                for (uint8_t tmp_line = (line / 3) * 3; tmp_line < line; tmp_line += 1)
                {
                    sdk[row][line]->RemoveFD(sdk[tmp_row][tmp_line]->GetDigit());
                }
            }

            {
                int8_t tmp_row = row / 3 * 3;
                int8_t tmp_line = line / 3 * 3;
                while ((tmp_row != row) or (tmp_line != line))
                {
                    sdk[row][line]->RemoveFD(sdk[tmp_row][tmp_line]->GetDigit());
                    tmp_line += 1;
                    if (tmp_line >= line / 3 * 3 + 3)
                    {
                        tmp_row += 1;
                        tmp_line = line / 3 * 3;
                    }
                }
            }


            if (not sdk[row][line]->GenerateDigit())
            {
                sdk[row][line]->Reset();
                if (line == 0)
                {
                    row -= 1;
                    line = 8;
                }
                else line -= 1;

                continue;
            }
            line += 1;
        }
        row += 1;
    }

    bool* opened = new bool[9 * 9];
    for (uint8_t i = 0; i < 9 * 9; i += 1)
    {
        opened[i] = false;
    }

    for (uint8_t i = 0; i < open_slots_count; i += 1)
    {
        uint8_t tmp = rand() % (9 * 9 - i);
        uint8_t true_num = 0;
        while (true)
        {
            if (not opened[true_num])
            {
                if (tmp == 0)
                {
                    break;
                }
                tmp -= 1;
            }
            true_num += 1;
        }
        opened[true_num] = true;
    }
    for (uint8_t line = 0; line < 9; line += 1)
    {
        for (uint8_t row = 0; row < 9; row += 1)
        {

            bool t = true;
            for (uint8_t i = 0; i < open_slots_count; i += 1)
            {
                if (opened[row + (line * 9)])
                {
                    _cells[row][line]->SetDigit(sdk[row][line]->GetDigit());
                    _cells[row][line]->Lock();
                    t = false;
                    break;
                }
            }
            if (t)
            {
                _cells[row][line]->SetDigit(0);
                _cells[row][line]->Open();
            }

        }
    }
    delete[] opened;
    delete[] sdk;
}

void Sudoku::Check()
{
    bool** lines = new bool*[9];
    bool** rows = new bool*[9];
    bool** squares = new bool* [9];
    for (uint8_t i = 0; i < 9; i+=1)
    {
        lines[i] = new bool[9];
        rows[i] = new bool[9];
        squares[i] = new bool[9];
        for (uint8_t j = 0; j < 9; j += 1)
        {
            lines[i][j] = true;
            rows[i][j] = true;
            squares[i][j] = true;
        }
    }

    bool flag = true;
    for (uint8_t line = 0; (line < 9) and flag; line += 1)
    {
        for (uint8_t row = 0; (row < 9) and flag; row += 1)
        {
            if (_cells[row][line]->GetDigit() == 0)
            {
                _check->setStyleSheet("background-color: red;");
                flag = false;
                break;
            }

            if ((lines[line][_cells[row][line]->GetDigit()-1]) and (rows[row][_cells[row][line]->GetDigit()-1])
                    and (squares[row / 3 + line / 3 * 3][_cells[row][line]->GetDigit() - 1]))
            {
                lines[line][_cells[row][line]->GetDigit() - 1] = false;
                rows[row][_cells[row][line]->GetDigit() - 1] = false;
                squares[row / 3 + line / 3 * 3][_cells[row][line]->GetDigit() - 1] = false;
            }
            else
            {
                _check->setStyleSheet("background-color: red;");
                flag = false;
                break;
            }
        }
        if (not flag) break;
    }
    if (flag)
    {
        _check->setStyleSheet("background-color: green;");
    }

    for (uint8_t i = 0; i < 9; i += 1)
    {
        delete[] lines[i];
        delete[] rows[i];
    }
    delete[] lines;
    delete[] rows;
}

void Sudoku::Solve()
{
    _solve->setText("u dirty cheater /(0\\_/0)\\");

    Cell*** sdk = new Cell * *[9];
    for (uint8_t i = 0; i < 9; i += 1) sdk[i] = new Cell * [9];
    for (uint8_t i = 0; i < 9; i += 1)
    {
        for (uint8_t j = 0; j < 9; j += 1)
        {
            sdk[i][j] = new Cell{ 9 };
        }
    }

    for (uint8_t row = 0; row < 9; )
    {
        for (uint8_t line = 0; line < 9; )
        {

            if (_cells[row][line]->IsLocked())
            {
                sdk[row][line]->SetDigit(_cells[row][line]->GetDigit());
                line += 1;
                continue;
            }
            for (uint8_t tl = 0; tl < line; tl += 1)
            {
                sdk[row][line]->RemoveFD(sdk[row][tl]->GetDigit());
            }
            for (uint8_t tr = 0; tr < row; tr += 1)
            {
                sdk[row][line]->RemoveFD(sdk[tr][line]->GetDigit());
            }
            for (uint8_t tl = line+1; tl < 9; tl += 1)
            {
                if (_cells[row][tl]->IsLocked())
                {
                    sdk[row][line]->RemoveFD(_cells[row][tl]->GetDigit());
                }
            }
            for (uint8_t tr = row+1; tr < 9; tr += 1)
            {
                if (_cells[tr][line]->IsLocked())
                {
                    sdk[row][line]->RemoveFD(_cells[tr][line]->GetDigit());
                }
            }

            {
                int8_t tr = row / 3 * 3;
                int8_t tl = line / 3 * 3;
                while ((tr != row) or (tl != line))
                {
                    sdk[row][line]->RemoveFD(sdk[tr][tl]->GetDigit());
                    tl += 1;
                    if (tl >= line / 3 * 3 + 3)
                    {
                        tr += 1;
                        tl = line / 3 * 3;
                    }
                }
            }

            if (not sdk[row][line]->GenerateDigit())
            {
                sdk[row][line]->Reset();

                if (line == 0)
                {
                    row -= 1;
                    line = 9 -1;
                }
                else line -= 1;

                while (_cells[row][line]->IsLocked())
                {

                    if (line == 0)
                    {
                        row -= 1;
                        line = 9 - 1;
                    }
                    else line -= 1;

                }

                continue;
            }
            line += 1;
        }
        row += 1;
    }

    for (uint8_t row = 0; row < 9; row += 1)
    {
        for (uint8_t line = 0; line < 9; line += 1)
        {
            _cells[row][line]->SetDigit(sdk[row][line]->GetDigit());
            delete sdk[row][line];
        }
    }
    for (uint8_t i = 0; i < 9; i += 1) delete[] sdk[i];
    delete[] sdk;
}

void Sudoku::ClickedReturnBtn()
{
    emit ReturnToMenu();
}

void Sudoku::paintEvent(QPaintEvent*)
{
    QPainter painter(this); // Создаём объект отрисовщика
    painter.setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::FlatCap));
    painter.drawLine(0,(height()-_return->height())/3,width(),(height()-_return->height())/3);
    painter.drawLine(0,(height()-_return->height())/3*2,width(),(height()-_return->height())/3*2);
    painter.drawLine(width()/3,0,width()/3,height());
    painter.drawLine(width()/3*2,0,width()/3*2,height());
}

Menu::Menu(QWidget *parent) : QWidget(parent)
{
    QGridLayout* main_layout = new QGridLayout(this);
    _play    = new QPushButton(this);
    _exit    = new QPushButton(this);
    _setting = new QLineEdit  (this);
    main_layout->addWidget(_play,   1,1,1,1);
    main_layout->addWidget(_exit,   1,2,1,1);
    main_layout->addWidget(_setting,1,3,1,1);
    _play   ->setText("Play!");
    _exit   ->setText("Exit");
    _setting->setValidator(new QIntValidator(0, 100, this) );
    _setting->setAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignVCenter);
    connect(_play,SIGNAL(clicked()),this,SLOT(ClickedPlayBtn()));
    connect(_exit,SIGNAL(clicked()),this,SLOT(ClickedExitBtn()));
    this->setLayout(main_layout);
}

void Menu::ClickedPlayBtn()
{
    uint8_t setting = _setting->text().toInt();
    if (setting > 81)
    {
        qWarning() << ">81!? srsly?";
        return;
    }
    emit Play(setting);
}

void Menu::ClickedExitBtn()
{
    emit Close();
}

SdkWindow::SdkWindow()
{
    _m           = new Menu(this);
    _sdk         = new Sudoku(this);
    _main_widget = new QStackedWidget(this);
    _main_widget->addWidget(_m);
    _main_widget->addWidget(_sdk);
    _main_widget->setCurrentWidget(_m);
    connect(_m,SIGNAL(Play(uint8_t)),this,SLOT(gotoSudoku(uint8_t)));
    connect(_m,SIGNAL(Close()),this,SLOT(ClickedExitBtn()));
    connect(_sdk,SIGNAL(ReturnToMenu()),this,SLOT(gotoMenu()));
    setCentralWidget(_main_widget);
    this->setMinimumSize(400,400);
    this->resize(400,400);
}

void SdkWindow::gotoMenu()
{
    _main_widget->setCurrentWidget(_m);
}

void SdkWindow::gotoSudoku(uint8_t setting)
{
    _sdk->Generate(setting);
    _main_widget->setCurrentWidget(_sdk);
}

void SdkWindow::ClickedExitBtn()
{
    emit Close();
}
