#include "sudoku.h"

CellBtn::CellBtn(QWidget* parent) :
    QPushButton("0",parent),
    _digit{0},
    _is_open{true}
{
    connect(this,&CellBtn::clicked,this,&CellBtn::ChangeDigit);
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
        setEnabled(false);
        UpdateColor();
    }
}

void CellBtn::Open()
{
    if (not _is_open)
    {
        _is_open = true;
        setEnabled(true);
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
        this->setStyleSheet("background-color:"+QColor(255,100,100).name()+";color:"+QColor(255,255,255).name());
        return;
    }
    if (_digit == 0)
    {
        this->setStyleSheet("background-color:"+QColor(255,255,255).name());
        return;
    }

    this->setStyleSheet("background-color:"+QColor(213/(_digit+1),25*(_digit+1),133).name());
}

Sudoku::Sudoku(QWidget* parent) :
    QWidget(parent),
    _check{new QPushButton("Check",this)},
    _solve{new QPushButton("Get Solve",this)},
    _return{new QPushButton("Return to Menu",this)}
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
    connect(_solve ,&QPushButton::clicked,this,&Sudoku::Solve);
    connect(_check ,&QPushButton::clicked,this,&Sudoku::Check);
    connect(_return,&QPushButton::clicked,this,&Sudoku::ClickedReturnBtn);
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
            sdk[i][j] = new Cell;
        }
    }

    for (uint8_t row = 0; row < 9; )
    {
        for (uint8_t column = 0; column < 9; )
        {
            for (uint8_t tmp_column = 0; tmp_column < column; tmp_column += 1)
            {
                sdk[row][column]->RemoveFD(sdk[row][tmp_column]->GetDigit());
            }
            for (uint8_t tmp_row = 0; tmp_row < row; tmp_row += 1)
            {
                sdk[row][column]->RemoveFD(sdk[tmp_row][column]->GetDigit());
            }

            for (uint8_t tmp_row = (row / 3) * 3; tmp_row < row; tmp_row += 1)
            {
                for (uint8_t tmp_column = (column / 3) * 3; tmp_column < column; tmp_column += 1)
                {
                    sdk[row][column]->RemoveFD(sdk[tmp_row][tmp_column]->GetDigit());
                }
            }

            {
                int8_t tmp_row = row / 3 * 3;
                int8_t tmp_column = column / 3 * 3;
                while ((tmp_row != row) or (tmp_column != column))
                {
                    sdk[row][column]->RemoveFD(sdk[tmp_row][tmp_column]->GetDigit());
                    tmp_column += 1;
                    if (tmp_column >= column / 3 * 3 + 3)
                    {
                        tmp_row += 1;
                        tmp_column = column / 3 * 3;
                    }
                }
            }


            if (not sdk[row][column]->GenerateDigit())
            {
                sdk[row][column]->Reset();
                if (column == 0)
                {
                    row -= 1;
                    column = 8;
                }
                else column -= 1;

                continue;
            }
            column += 1;
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
    for (uint8_t row = 0; row < 9; row += 1)
    {
        for (uint8_t column = 0; column < 9; column += 1)
        {

            bool t = true;
            for (uint8_t i = 0; i < open_slots_count; i += 1)
            {
                if (opened[column + (row * 9)])
                {
                    _cells[row][column]->SetDigit(sdk[row][column]->GetDigit());
                    _cells[row][column]->Lock();
                    t = false;
                    break;
                }
            }
            if (t)
            {
                _cells[row][column]->SetDigit(0);
                _cells[row][column]->Open();
            }
            delete sdk[row][column];
        }
        delete[] sdk[row];
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
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::FlatCap));
    painter.drawLine(0,(height()-_return->height())/3,width(),(height()-_return->height())/3);
    painter.drawLine(0,(height()-_return->height())/3*2,width(),(height()-_return->height())/3*2);
    painter.drawLine(width()/3,0,width()/3,height());
    painter.drawLine(width()/3*2,0,width()/3*2,height());
}

Menu::Menu(QWidget *parent) :
    QWidget(parent),
    _play{new QPushButton("Play!",this)},
    _exit{new QPushButton("Exit",this)},
    _setting{new QLineEdit(this)}
{
    QGridLayout* main_layout = new QGridLayout(this);
    main_layout->addWidget(_play,   1,1,1,1);
    main_layout->addWidget(_exit,   1,2,1,1);
    main_layout->addWidget(_setting,1,3,1,1);
    _setting->setValidator(new QIntValidator(0, 100, this) );
    _setting->setAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignVCenter);
    connect(_play,&QPushButton::clicked,this,&Menu::ClickedPlayBtn);
    connect(_exit,&QPushButton::clicked,this,&Menu::ClickedExitBtn);
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

SdkWindow::SdkWindow() :
    _m{new Menu(this)},
    _sdk{new Sudoku(this)},
    _main_widget{new QStackedWidget(this)}
{
    _main_widget->addWidget(_m);
    _main_widget->addWidget(_sdk);
    _main_widget->setCurrentWidget(_m);
    connect(_m,&Menu::Play,this,&SdkWindow::gotoSudoku);
    connect(_m,&Menu::Close,this,&SdkWindow::ClickedExitBtn);
    connect(_sdk,&Sudoku::ReturnToMenu,this,&SdkWindow::gotoMenu);
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
