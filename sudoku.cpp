#include "sudoku.h"

CellBtn::CellBtn(QWidget* parent) :
    QPushButton("0",parent),
    _digit{0},
    _is_open{true}
{
    connect(this,&CellBtn::clicked,this,&CellBtn::ChangeDigit);
    this->setStyleSheet("background-color:"+QColor(255,255,255).name());
}

int CellBtn::GetDigit() const
{
    return _digit;
}

bool CellBtn::IsLocked() const
{
    return !_is_open;
}

void CellBtn::SetDigit(const int digit)
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
    _check {new QPushButton("Check",this)},
    _solve {new QPushButton("Get Solve",this)},
    _return{new QPushButton("Return to Menu",this)},
    _help{new QPushButton("Help",this)},
    _timer{new QTimer(this)},
    _seconds{0},
    _timer_lbl{new QLabel("0 second later",this)}
{
    QGridLayout* main_layout = new QGridLayout(this);
    for (int i = 0; i < 9; i+=1)
    {
        for (int j = 0; j < 9; j+=1)
        {
            _cells[i][j] = new CellBtn(this);
            _cells[i][j]->setSizePolicy(QSizePolicy::Expanding , QSizePolicy::Expanding);
            main_layout->addWidget(_cells[i][j],i + i/3,j + j/3);
        }
    }
    main_layout->addItem(new QSpacerItem(30,30),3,3);
    main_layout->addItem(new QSpacerItem(30,30),7,7);
    main_layout->addItem(new QSpacerItem(0,30),13,0);
    connect(_solve ,&QPushButton::clicked,this,&Sudoku::Solve);
    connect(_check ,&QPushButton::clicked,this,&Sudoku::Check);
    connect(_return,&QPushButton::clicked,this,&Sudoku::ClickedReturnBtn);
    main_layout->addWidget(_solve ,14,0,1,3);
    main_layout->addWidget(_help ,14,4,1,3);
    main_layout->addWidget(_return,14,8,1,3);
    main_layout->addWidget(_check ,15,8,1,3);
    main_layout->addWidget(_timer_lbl,15,0,1,7);

    _check->setSizePolicy(QSizePolicy::Expanding , QSizePolicy::Expanding);

    _timer_lbl->setAlignment(Qt::Alignment::enum_type::AlignCenter);

    _timer->setTimerType(Qt::TimerType::VeryCoarseTimer);
    connect(_timer,&QTimer::timeout,this,&Sudoku::Update);
    _timer->start(1000);

    this->setLayout(main_layout);
    setWindowTitle("Sudoku");
}

// Данный класс нужен только для того, чтобы создавать и решать судоку
class Cell
{
private:
    bool _free_digits[9];
    int _digit; // 0 - нет числа
public:
    Cell(const int digit = 0)
    {
        _digit = digit;
        for (int i = 0; i < 9; i += 1) _free_digits[i] = true; // изначально все свободны
    }

    void Reset()
    {
        for (int i = 0; i < 9; i += 1) _free_digits[i] = true; // изначально все свободны
        _digit = 0;
    }

    int GetDigit()
    {
        return _digit;
    }

    void RemoveFD(int digit)
    {
        _free_digits[digit - 1] = false;
    }

    bool GenerateDigit()
    {
        int fd_count = 0;
        for (int i = 0; i < 9; i += 1) fd_count += _free_digits[i];

        if (fd_count == 0) return false;

        int tmp = rand() % fd_count;
        int true_num = 0;
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

    void SetDigit(int digit)
    {
        _digit = digit;
    }
};

void Sudoku::Generate(int open_slots_count)
{
    Cell sdk[9][9];

    for (int row = 0; row < 9; )
    {
        for (int column = 0; column < 9; )
        {
            for (int tmp_column = 0; tmp_column < column; tmp_column += 1)
            {
                sdk[row][column].RemoveFD(sdk[row][tmp_column].GetDigit());
            }
            for (int tmp_row = 0; tmp_row < row; tmp_row += 1)
            {
                sdk[row][column].RemoveFD(sdk[tmp_row][column].GetDigit());
            }

            {
                int tmp_row = row / 3 * 3;
                int tmp_column = column / 3 * 3;

                while (not ((tmp_row == row) && (tmp_column == column)))
                {
                    sdk[row][column].RemoveFD(sdk[tmp_row][tmp_column].GetDigit());
                    tmp_column += 1;
                    if (tmp_column >= column / 3 * 3 + 3)
                    {
                        tmp_row += 1;
                        tmp_column = column / 3 * 3;
                    }
                }
            }


            if (not sdk[row][column].GenerateDigit())
            {
                if ((row == 0) and (column == 0)) exit(-1);
                sdk[row][column].Reset();
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

    bool opened[9 * 9];
    for (int i = 0; i < 9 * 9; i += 1)
    {
        opened[i] = false;
    }

    for (int i = 0; i < open_slots_count; i += 1)
    {
        int tmp = rand() % (9 * 9 - i);
        int true_num = 0;
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
    for (int row = 0; row < 9; row += 1)
    {
        for (int column = 0; column < 9; column += 1)
        {

            bool t = true;
            for (int i = 0; i < open_slots_count; i += 1)
            {
                if (opened[column + (row * 9)])
                {
                    _cells[row][column]->SetDigit(sdk[row][column].GetDigit());
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
        }
    }
}

void Sudoku::Solve()
{
    _solve->setText("u dirty cheater /(0\\_/0)\\");

    Cell sdk[9][9];

    for (int row = 0; row < 9; )
    {
        for (int column = 0; column < 9; )
        {
            qDebug() << column << ' ' << row;

            if (_cells[row][column]->IsLocked())
            {
                sdk[row][column].SetDigit(_cells[row][column]->GetDigit());
                column += 1;
                continue;
            }
            for (int tmp_column = 0; tmp_column < column; tmp_column += 1)
            {
                sdk[row][column].RemoveFD(sdk[row][tmp_column].GetDigit());
            }
            for (int tmp_row = 0; tmp_row < row; tmp_row += 1)
            {
                sdk[row][column].RemoveFD(sdk[tmp_row][column].GetDigit());
            }
            for (int tmp_column = column+1; tmp_column < 9; tmp_column += 1)
            {
                if (_cells[row][tmp_column]->IsLocked())
                {
                    sdk[row][column].RemoveFD(_cells[row][tmp_column]->GetDigit());
                }
            }
            for (int tmp_row = row+1; tmp_row < 9; tmp_row += 1)
            {
                if (_cells[tmp_row][column]->IsLocked())
                {
                    sdk[row][column].RemoveFD(_cells[tmp_row][column]->GetDigit());
                }
            }

            {
                int tmp_row = row / 3 * 3;
                int tmp_column = column / 3 * 3;
                while (not ((tmp_row == row) and (tmp_column == column)))
                {
                    sdk[row][column].RemoveFD(sdk[tmp_row][tmp_column].GetDigit());
                    tmp_column += 1;
                    if (tmp_column >= column / 3 * 3 + 3)
                    {
                        tmp_row += 1;
                        tmp_column = column / 3 * 3;
                    }
                }
            }

            {
                int tmp_row = row / 3 * 3 + 2;
                int tmp_column = column / 3 * 3 + 2;
                while (not ((tmp_row == row) and (tmp_column == column)))
                {
                    if (_cells[tmp_row][tmp_column]->IsLocked())
                    {
                        sdk[row][column].RemoveFD(sdk[tmp_row][tmp_column].GetDigit());
                    }
                    tmp_column -= 1;
                    if (tmp_column < column / 3 * 3)
                    {
                        tmp_row -= 1;
                        tmp_column = column / 3 * 3 + 2;
                    }
                }
            }

            if (not sdk[row][column].GenerateDigit())
            {
                sdk[row][column].Reset();

                if (column == 0)
                {
                    row -= 1;
                    column = 9 -1;
                }
                else column -= 1;

                if (row<0)
                {
                    _timer_lbl->setText("there are no solutions");
                    _timer_lbl->setStyleSheet("color: red;");
                    return;
                }

                while (_cells[row][column]->IsLocked())
                {

                    if (column == 0)
                    {
                        row -= 1;
                        column = 9 - 1;
                    }
                    else column -= 1;

                    if (row<0)
                    {
                        _timer_lbl->setText("there are no solutions");
                        _timer_lbl->setStyleSheet("color: red;");
                        return;
                    }
                }

                continue;
            }
            column += 1;
        }
        row += 1;
    }

    for (int row = 0; row < 9; row += 1)
    {
        for (int column = 0; column < 9; column += 1)
        {
            _cells[row][column]->SetDigit(sdk[row][column].GetDigit());
        }
    }
}

void Sudoku::Help()
{
    auto err = FindError();
    qDebug() << err;
    if (err == std::make_pair<int,int>(-1,-1))
    {
        _check->setStyleSheet("background-color: green;");
    }
    else
    {
        _cells[err.first][err.second]->setStyleSheet(_cells[err.first][err.second]->styleSheet() + "border: 15px solid rgb(0,200,250);");
    }
}

void Sudoku::ClickedReturnBtn()
{
    emit ReturnToMenu();
}

void Sudoku::Check()
{
    if (FindError() == std::make_pair<int,int>(-1,-1)) {
         _check->setStyleSheet("background-color: red;");
    }
    else _check->setStyleSheet("background-color: green;");
}

void Sudoku::Update()
{
    _seconds += 1;
    _timer_lbl->setText(QString::number(_seconds) + " second later");
    if (_check->styleSheet() == "background-color: red;") _check->setStyleSheet("");
}

std::pair<int, int> Sudoku::FindError()
{
    bool columns [9][9];
    bool rows [9][9];;
    bool squares [9][9];
    for (int i = 0; i < 9; i+=1)
    {
        for (int j = 0; j < 9; j += 1)
        {
            columns[i][j] = true;
            rows[i][j] = true;
            squares[i][j] = true;
        }
    }

    for (int column = 0; column < 9; column += 1)
    {
        for (int row = 0; row < 9; row += 1)
        {
            if (_cells[row][column]->GetDigit() == 0)
            {
                return {column,row};
            }

            if ((columns[column][_cells[row][column]->GetDigit()-1]) and (rows[row][_cells[row][column]->GetDigit()-1])
                    and (squares[row / 3 + column / 3 * 3][_cells[row][column]->GetDigit() - 1]))
            {
                columns[column][_cells[row][column]->GetDigit() - 1] = false;
                rows[row][_cells[row][column]->GetDigit() - 1] = false;
                squares[row / 3 + column / 3 * 3][_cells[row][column]->GetDigit() - 1] = false;
            }
            else
            {
                return {column,row};
            }
        }
    }

    return {-1,-1};
}

void Sudoku::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    QFont tmp;
    tmp.setPixelSize(_timer_lbl->height()/2);
    _timer_lbl->setFont(tmp);
}

void Sudoku::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::FlatCap));
    painter.setRenderHint(QPainter::Antialiasing);
    int sdk_table_height = (height()-_return->height()-_check->height()-30);
    painter.drawLine(0,sdk_table_height/3,width(),sdk_table_height/3);
    painter.drawLine(0,sdk_table_height/3*2,width(),sdk_table_height/3*2);
    painter.drawLine(0,sdk_table_height,width(),sdk_table_height);
    painter.drawLine(width()/3,0,width()/3,sdk_table_height);
    painter.drawLine(width()/3*2,0,width()/3*2,sdk_table_height);
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
    int setting = _setting->text().toInt();
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

void SdkWindow::gotoSudoku(int setting)
{
    _sdk->Generate(setting);
    _main_widget->setCurrentWidget(_sdk);
}

void SdkWindow::ClickedExitBtn()
{
    emit Close();
}
