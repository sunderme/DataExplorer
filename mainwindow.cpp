#include "mainwindow.h"

#include <QMenuBar>
#include <QFileDialog>
#include <QTextStream>
#include <QtCharts>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupMenus();
    setupGUI();
    resize(800,600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    openAct = new QAction(tr("&Open"), this);
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAct);
    exitAct = new QAction(tr("&Quit"), this);
    connect(exitAct, &QAction::triggered, this, &MainWindow::close);
    fileMenu->addAction(exitAct);

    plotMenu = menuBar()->addMenu(tr("&Plot"));
    plotAct = new QAction(tr("&Plot"), this);
    connect(plotAct, &QAction::triggered, this, &MainWindow::plotSelected);
    plotMenu->addAction(plotAct);
}

void MainWindow::setupGUI()
{
    tableWidget = new QTableWidget;
    chartView = new QChartView();
    chartView->setRubberBand(QChartView::RectangleRubberBand);

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->addTab(tableWidget,tr("CSV"));
    tabWidget->addTab(chartView,tr("Plots"));

    setCentralWidget(tabWidget);
}

void MainWindow::openFile()
{
    fileName = QFileDialog::getOpenFileName(this,
        tr("Open CSV"), "", tr("CSV Files (*.csv)"));
    if(fileName.isEmpty()) return;
    readInCSV(fileName);
    buildTable();
}

void MainWindow::readInCSV(const QString &fileName)
{
    QFile data(fileName);
    if (data.open(QFile::ReadOnly)){
        QTextStream stream(&data);
        QString line;
        // first line is column names
        stream.readLineInto(&line);
        columns=line.split(',');
        QList<QStringList> data(columns.size());
        while (stream.readLineInto(&line)) {
            QStringList elements=line.split(',');
            for(int i=0;i<elements.size();++i){
                data[i].append(elements[i]);
            }
        }
        csv=data;
    }

}

void MainWindow::buildTable()
{
    tableWidget->clear();
    if(csv.isEmpty()) return;
    tableWidget->setRowCount(csv[0].size());
    tableWidget->setColumnCount(columns.size());
    tableWidget->setHorizontalHeaderLabels(columns);
    for(int i=0;i<columns.size();++i){
        for(int row=0;row<csv[i].size();++row){
            QTableWidgetItem *newItem = new QTableWidgetItem(csv[i].value(row));
            tableWidget->setItem(row, i, newItem);
        }
    }
}

void MainWindow::plotSelected()
{
    QLineSeries *series = new QLineSeries();
    for(int row=0;row<csv[0].size();++row){
        bool ok_x,ok_y;
        qreal x=csv[1].value(row).toDouble(&ok_x);
        qreal y=csv[2].value(row).toDouble(&ok_y);
        if(ok_x && ok_y){
            QPointF pt(x,y);
            series->append(pt);
        }
    }
    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("Simple line chart example");

    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
}

