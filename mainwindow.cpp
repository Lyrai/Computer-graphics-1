#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphview.h"
#include "expressionparser.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
{
    ui->setupUi(this);
    view = new GraphView(scene);
    ui->xFrom->setText(QString::number(view->left()));
    ui->xTo->setText(QString::number(view->right()));
    //view->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_function_textChanged()
{
    ExpressionParser parser;
    view->setExpr(parser.parse(ui->function->toPlainText()));
}


void MainWindow::on_xFrom_textChanged()
{
    view->setLeft(ui->xFrom->toPlainText().toDouble());
}


void MainWindow::on_xTo_textChanged()
{
    view->setRight(ui->xTo->toPlainText().toDouble());
}


void MainWindow::on_pushButton_clicked()
{
    if(view->isVisible())
        view->close();

    view->show();
}

