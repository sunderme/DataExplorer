#include "mainwindow.h"

#include <QMenuBar>
#include <QFileDialog>
#include <QTextStream>
#include <QtCharts>
#include <set>
#include "zoomablechart.h"

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
    QAction *loadTemplateAct=new QAction(tr("&Open Template"), this);
    connect(loadTemplateAct, &QAction::triggered, this, &MainWindow::openTemplate);
    fileMenu->addAction(loadTemplateAct);
    QAction *saveTemplateAct=new QAction(tr("&Save Template"), this);
    connect(saveTemplateAct, &QAction::triggered, this, &MainWindow::saveTemplate);
    fileMenu->addAction(saveTemplateAct);
    exitAct = new QAction(tr("&Quit"), this);
    connect(exitAct, &QAction::triggered, this, &MainWindow::close);
    fileMenu->addAction(exitAct);

    QToolBar *plotToolBar = addToolBar(tr("Plot"));
    plotMenu = menuBar()->addMenu(tr("&Plot"));
    plotAct = new QAction(tr("&Plot"), this);
    plotAct->setIcon(QIcon(":/icons/labplot-xy-curve-segments.svg"));
    plotToolBar->addAction(plotAct);
    connect(plotAct, &QAction::triggered, this, &MainWindow::plotSelected);
    plotMenu->addAction(plotAct);
    QAction *act=new QAction(tr("Zoom area"),this);
    act->setIcon(QIcon(":/icons/zoom.svg"));
    act->setShortcut(Qt::Key_Z);
    connect(act,&QAction::triggered,this,&MainWindow::zoomAreaMode);
    plotMenu->addAction(act);
    plotToolBar->addAction(act);
    act=new QAction(tr("Zoom X"),this);
    act->setIcon(QIcon(":/icons/zoom-select-x.svg"));
    act->setShortcut(Qt::Key_X);
    connect(act,&QAction::triggered,this,&MainWindow::zoomX);
    plotMenu->addAction(act);
    plotToolBar->addAction(act);
    act=new QAction(tr("Zoom Y"),this);
    act->setIcon(QIcon(":/icons/zoom-select-y.svg"));
    act->setShortcut(Qt::Key_Y);
    connect(act,&QAction::triggered,this,&MainWindow::zoomY);
    plotMenu->addAction(act);
    plotToolBar->addAction(act);
    act=new QAction(tr("Zoom in"),this);
    act->setIcon(QIcon(":/icons/zoomin.svg"));
    act->setShortcut(Qt::ShiftModifier|Qt::Key_Z);
    connect(act,&QAction::triggered,this,&MainWindow::zoomIn);
    plotMenu->addAction(act);
    act=new QAction(tr("Zoom out"),this);
    act->setIcon(QIcon(":/icons/zoomout.svg"));
    act->setShortcut(Qt::ControlModifier|Qt::Key_Z);
    connect(act,&QAction::triggered,this,&MainWindow::zoomOut);
    plotMenu->addAction(act);
    act=new QAction(tr("Zoom fit"),this);
    act->setIcon(QIcon(":/icons/zoom-fit-best.svg"));
    act->setShortcut(Qt::Key_F);
    connect(act,&QAction::triggered,this,&MainWindow::zoomReset);
    plotMenu->addAction(act);
    plotToolBar->addAction(act);
    act=new QAction(tr("Pan"),this);
    act->setIcon(QIcon(":/icons/transform-move.svg"));
    act->setShortcut(Qt::Key_P);
    connect(act,&QAction::triggered,this,&MainWindow::panMode);
    plotMenu->addAction(act);
    plotToolBar->addAction(act);

    act=new QAction("delete",this);
    act->setShortcut(QKeySequence::Delete);
    plotMenu->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::deleteVar);

    QAction *testAction=new QAction("test",this);
    connect(testAction, &QAction::triggered, this, &MainWindow::test);
    plotMenu->addAction(testAction);
}

void MainWindow::setupGUI()
{
    tableWidget = new QTableWidget;
    QWidget *wgt= new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;
    lstSweeps = new QListWidget;
    lstSweeps->setSelectionMode(QAbstractItemView::SingleSelection);
    lstSweeps->setDragEnabled(true);
    lstSweeps->setDefaultDropAction(Qt::MoveAction);
    lstSweeps->setAcceptDrops(true);
    lstSweeps->setDropIndicatorShown(true);
    lstData = new QListWidget;
    hLayout->addWidget(lstSweeps);
    hLayout->addWidget(lstData);
    mainLayout->addLayout(hLayout,1);
    QHBoxLayout *hLayout2 = new QHBoxLayout;
    btFilter=new QToolButton;
    btFilter->setCheckable(true);
    btFilter->setText("Filter");
    btFilter->setIcon(QIcon(":/icons/view-filter.svg"));
    connect(btFilter,&QAbstractButton::toggled,this,&MainWindow::filterToggled);
    hLayout2->addWidget(btFilter);
    leFilterText = new QLineEdit;
    connect(leFilterText,&QLineEdit::textEdited,this,&MainWindow::filterTextChanged);
    hLayout2->addWidget(leFilterText);
    hLayout2->addSpacing(1);
    mainLayout->addLayout(hLayout2);
    mainLayout->addWidget(tableWidget,3);
    wgt->setLayout(mainLayout);

    chartView = new ZoomableChartView();
    chartView->setZoomMode(ZoomableChartView::RectangleZoom);

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->addTab(wgt,tr("CSV"));
    tabWidget->addTab(chartView,tr("Plots"));

    tableWidget->horizontalHeader()-> setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableWidget->horizontalHeader(),&QAbstractItemView::customContextMenuRequested,this,&MainWindow::headerMenuRequested);

    setCentralWidget(tabWidget);
}

void MainWindow::openFile()
{
    fileName = QFileDialog::getOpenFileName(this,
        tr("Open CSV"), "", tr("CSV Files (*.csv)"));
    if(fileName.isEmpty()) return;
    readInCSV(fileName);
    buildTable();
    sweeps.clear();
    plotValues.clear();
    if(columns.contains('x')){
        // add x to sweeps
        sweeps<<"x";
    }
    if(columns.contains('y')){
        // add y to plot values
        plotValues<<"y";
    }
    updateSweepGUI();
}

void MainWindow::openTemplate()
{
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open template"), "", tr("Template File (*.deTemplate)"));
    if(fileName.isEmpty()) return;
    QFile loadFile(fileName);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }
    QByteArray data = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    QJsonObject jo=loadDoc.object();
    QJsonArray ja=jo["sweeps"].toArray();
    sweeps.clear();
    for(int i = 0; i < ja.size(); ++i) {
        sweeps<<ja[i].toString();
    }
    ja=jo["plots"].toArray();
    plotValues.clear();
    for(int i = 0; i < ja.size(); ++i) {
        plotValues<<ja[i].toString();
    }
    updateSweepGUI();
}
/*!
 * \brief save sweeps/plots as template
 * Format is json
 */
void MainWindow::saveTemplate()
{
    fileName = QFileDialog::getSaveFileName(this,
        tr("Save template"), "", tr("Template File (*.deTemplate)"));
    if(fileName.isEmpty()) return;
    QFile saveFile(fileName);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }

    QJsonObject jo;
    jo["sweeps"]=QJsonArray::fromStringList(sweeps);
    jo["plots"]=QJsonArray::fromStringList(plotValues);

    QJsonDocument saveDoc(jo);
    saveFile.write(saveDoc.toJson());
}

void MainWindow::readInCSV(const QString &fileName)
{
    QFile data(fileName);
    if (data.open(QFile::ReadOnly)){
        QTextStream stream(&data);
        QString line;
        // first line with commas is column names
        while(stream.readLineInto(&line)){
            if(line.startsWith('!') || line.isEmpty())
                continue;
            columns=line.split(',');
            if(columns.size()>1)
                break;
        }
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
    tableWidget->resizeColumnsToContents();
}

void MainWindow::updateSweepGUI()
{
    lstSweeps->clear();
    lstData->clear();
    foreach(const QString &elem,sweeps){
        new QListWidgetItem(elem,lstSweeps);
    }
    foreach(const QString &elem,plotValues){
        new QListWidgetItem(elem,lstData);
    }
}

void MainWindow::updateSweeps()
{
    sweeps.clear();
    plotValues.clear();
    for(int i=0;i<lstSweeps->count();++i){
        sweeps<<lstSweeps->item(i)->text();
    }
    for(int i=0;i<lstData->count();++i){
        plotValues<<lstData->item(i)->text();
    }
}

void MainWindow::plotSelected()
{
    updateSweeps();
    if(plotValues.isEmpty()) return;
    if(sweeps.isEmpty()) return;
    QStringList vars=sweeps;
    QString xn=vars.takeLast();
    int index_x=getIndex(xn);
    QString yn=plotValues.last();
    int index_y=getIndex(yn);

    QList<loopIteration> lits=groupBy(vars);
    ZoomableChart *chart = new ZoomableChart();
    foreach(loopIteration lit,lits){
        QLineSeries *series = new QLineSeries();
        if(!lit.value.isEmpty()){
            series->setName(lit.value.left(lit.value.size()-1));
        }
        for(int i:lit.indices){
            bool ok_x,ok_y;
            qreal x=csv[index_x].value(i).toDouble(&ok_x);
            qreal y=csv[index_y].value(i).toDouble(&ok_y);
            if(ok_x && ok_y){
                QPointF pt(x,y);
                series->append(pt);
            }
        }
        chart->addSeries(series);
    }
    if(lits.size()<2){
        chart->legend()->hide();
    }
    chart->createDefaultAxes();
    chart->setTitle("Simple line chart example");

    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::headerMenuRequested(QPoint pt)
{
    int column=tableWidget->horizontalHeader()->logicalIndexAt(pt);

    QMenu *menu=new QMenu(this);
    QAction *act=new QAction(tr("add as sweep var"), this);
    act->setData(columns[column]);
    connect(act,&QAction::triggered,this,&MainWindow::addSweepVar);
    menu->addAction(act);
    act=new QAction(tr("add as plot var"), this);
    act->setData(columns[column]);
    connect(act,&QAction::triggered,this,&MainWindow::addPlotVar);
    menu->addAction(act);
    menu->popup(tableWidget->horizontalHeader()->viewport()->mapToGlobal(pt));
}

void MainWindow::addSweepVar()
{
    QAction *act=qobject_cast<QAction*>(sender());
    QString var=act->data().toString();
    sweeps.prepend(var);
    updateSweepGUI();
}
/*!
 * \brief remove var from sweepvar/plotvar, depending which one is focused
 */
void MainWindow::deleteVar()
{
    // delete pressed
    if(lstSweeps->hasFocus()){
        auto lst=lstSweeps->selectedItems();
        for(auto elem:lst){
            QString var=elem->text();
            sweeps.removeAll(var);
            delete elem;
        }
    }
    if(lstData->hasFocus()){
        auto lst=lstData->selectedItems();
        for(auto elem:lst){
            QString var=elem->text();
            plotValues.removeAll(var);
            delete elem;
        }
    }
}

void MainWindow::addPlotVar()
{
    QAction *act=qobject_cast<QAction*>(sender());
    QString var=act->data().toString();
    plotValues.prepend(var);
    updateSweepGUI();
}

void MainWindow::zoomAreaMode()
{
    chartView->setZoomMode(ZoomableChartView::RectangleZoom);
}

void MainWindow::panMode()
{
    chartView->setZoomMode(ZoomableChartView::Pan);
}

void MainWindow::zoomX()
{
    chartView->setZoomMode(ZoomableChartView::HorizontalZoom);
}

void MainWindow::zoomY()
{
    chartView->setZoomMode(ZoomableChartView::VerticalZoom);
}

void MainWindow::zoomIn()
{
    chartView->chart()->zoomIn();
}

void MainWindow::zoomOut()
{
    chartView->chart()->zoomOut();
}

void MainWindow::zoomReset()
{
    chartView->chart()->zoomReset();
}

void MainWindow::filterToggled(bool checked)
{
    // filter columns
    for(int i=0;i<columns.size();++i){
        if(!checked){
            tableWidget->showColumn(i);
        }else{
            if(columns.value(i).contains(leFilterText->text())){
                tableWidget->showColumn(i);
            }else{
                tableWidget->hideColumn(i);
            }
        }
    }
}

void MainWindow::filterTextChanged(const QString &text)
{
    if(btFilter->isChecked()){
        for(int i=0;i<columns.size();++i){
            if(columns.value(i).contains(text)){
                tableWidget->showColumn(i);
            }else{
                tableWidget->hideColumn(i);
            }
        }
    }
}

QDebug operator<< (QDebug d, const QList<loopIteration>& dt) {
    foreach(const loopIteration &lit,dt){
        d << lit.value << '/' << lit.indices;
    }
    return d;
}

void MainWindow::test()
{
    if(csv.isEmpty()) return;
    QList<int> providedIndices(csv[0].size());
    for(int i=0;i<csv[0].size();++i){
        providedIndices[i]=i;
    }
    QStringList vals=getUniqueValues("x",providedIndices);
    qDebug()<<"x"<<vals;
    vals=getUniqueValues("s",providedIndices);
    qDebug()<<"s"<<vals;
    QList<int> indices=filterIndices("s","2",providedIndices);
    qDebug()<<"s indices"<<indices;
    indices=filterIndices("x","0.2",providedIndices);
    qDebug()<<"x inices"<<indices;
    QStringList vars;
    vars<<"s";
    QList<loopIteration> lits=groupBy(vars);
    qDebug()<<"by s:"<<lits;
    vars.clear();
    vars<<"s"<<"h";
    lits=groupBy(vars);
    qDebug()<<"by s,h:"<<lits;
    vars.clear();
    lits=groupBy(vars);
    qDebug()<<"by none:"<<lits;
}

int MainWindow::getIndex(const QString &name)
{
    int result=columns.indexOf(name);
    return result;
}
/*!
 * \brief get Unique Values for given var in list of indices
 * \param var variable name
 * \param indices
 * \return list of values
 */
QStringList MainWindow::getUniqueValues(const QString &var, const QList<int> &indices)
{
    int index=getIndex(var);
    QStringList result;
    for(int i:indices){
        result<<csv[index][i];
    }
    result.removeDuplicates();
    return result;
}
/*!
 * \brief filter providedIndices for all values where var contains value
 * \param var
 * \param value
 * \param providedIndices
 * \return
 */
QList<int> MainWindow::filterIndices(const QString &var, const QString &value, const QList<int> &providedIndices)
{
    int index=getIndex(var);
    QList<int> result;
    foreach(int i,providedIndices){
        if(csv[index][i]==value){
            result<<i;
        }
    }
    return result;
}
/*!
 * \brief like groupBy in pandas.
 * Produces list of indices which belong to one sweep iteration
 * \param sweepVar, last is x axxis
 * \return list of list of indices
 */
QList<loopIteration> MainWindow::groupBy(QStringList sweepVar,QList<int> providedIndices)
{
    QList<loopIteration> result;
    if(providedIndices.isEmpty()){
        // fill from 0 to size(csv)-1
        int sz=csv[0].size();
        providedIndices.resize(sz);
        for(int i=0;i<csv[0].size();++i){
            providedIndices[i]=i;
        }
    }
    if(!sweepVar.isEmpty()){
        QString var=sweepVar.takeFirst();
        QStringList values=getUniqueValues(var,providedIndices);
        for(const QString &value:values){
            QList<int> indices=filterIndices(var,value,providedIndices);
            QList<loopIteration>groupedResult=groupBy(sweepVar,indices);
            for(loopIteration &lit:groupedResult){
                lit.value.prepend(var+"="+value+";");
            }
            result.append(groupedResult);
        }
    }else{
        loopIteration lit;
        lit.indices=providedIndices;
        result<<lit;
    }
    return result;
}

/* TODO
Unit tests
filter
zoom/toolbar
cursor in plot
*/
