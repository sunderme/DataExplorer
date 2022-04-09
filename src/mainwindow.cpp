#include "mainwindow.h"

#include <QMenuBar>
#include <QFileDialog>
#include <QTextStream>
#include <QtCharts>
#include <QtGlobal>
#include <QSettings>
#include <set>
#include "zoomablechart.h"

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent)
{
    QSettings settings("DataExplorer","DataExplorer");
    recentFiles=settings.value("recentFiles").toStringList();
    setupMenus();
    setupGUI();

    if(settings.contains("geometry")){
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
    }else{
        resize(800,600);
    }

    if(argc>1){
        // assume last argument as filename
        // maybe more elaborate later
        fileName=QString(argv[argc-1]);
        readFile();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("DataExplorer", "DataExplorer");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("recentFiles",recentFiles);
    event->accept();
}


MainWindow::~MainWindow()
{
}

void MainWindow::setupMenus()
{
    recentFilesMenu=new QMenu(tr("Open recent files..."));
    fileMenu = menuBar()->addMenu(tr("&File"));
    openAct = new QAction(tr("&Open"), this);
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAct);
    populateRecentFiles();
    fileMenu->addMenu(recentFilesMenu);
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
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    act->setShortcut(Qt::ShiftModifier|Qt::Key_Z);
#else
    act->setShortcut(Qt::ShiftModifier+Qt::Key_Z);
#endif
    connect(act,&QAction::triggered,this,&MainWindow::zoomIn);
    plotMenu->addAction(act);
    act=new QAction(tr("Zoom out"),this);
    act->setIcon(QIcon(":/icons/zoomout.svg"));
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    act->setShortcut(Qt::ControlModifier|Qt::Key_Z);
#else
    act->setShortcut(Qt::ControlModifier+Qt::Key_Z);
#endif
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
/*!
 * \brief call file dialog and then open file
 */
void MainWindow::openFile()
{
    fileName = QFileDialog::getOpenFileName(this,
        tr("Open CSV"), "", tr("CSV Files (*.csv)"));
    if(fileName.isEmpty()) return;
    readFile();
    recentFiles.removeOne(fileName);
    recentFiles.prepend(fileName);
    populateRecentFiles();
}
/*!
 * \brief open file via recent menu
 */
void MainWindow::openRecentFile()
{
    QAction *act=qobject_cast<QAction*>(sender());
    fileName=act->text();
    if(QFileInfo(fileName).exists())
        readFile();
}
/*!
 * \brief read in csv file and update GUI
 */
void MainWindow::readFile()
{
    if(fileName.isEmpty()) return;
    readInCSV(fileName);
    buildTable();
    sweeps.clear();
    plotValues.clear();
    if(columns.contains("x")){
        // add x to sweeps
        sweeps<<"x";
    }
    if(columns.contains("y")){
        // add y to plot values
        plotValues<<"y";
    }
    updateSweepGUI();
}
/*!
 * \brief open Template which contains the sweeps/plots settings
 */
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
/*!
 * \brief read in CSV
 * Assumes comma separated values
 * First line with commas is assumed to be header line
 * \param fileName
 */
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
        QVector<QStringList> data(columns.size());
        while (stream.readLineInto(&line)) {
            QStringList elements=line.split(',');
            for(int i=0;i<elements.size();++i){
                data[i].append(elements[i]);
            }
        }
        csv=data;
        columnFilters.clear();
    }
}
/*!
 * \brief popalte table widget with present data
 */
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
/*!
 * \brief update Sweep/plotvar list widget
 */
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
/*!
 * \brief update internal sweep structure from GUI
 */
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
/*!
 * \brief plot data with selected sweeps
 * Only last plot var is plotted.
 */
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

    QList<LoopIteration> lits=groupBy(vars,visibleRows);
    ZoomableChart *chart = new ZoomableChart();
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    foreach(LoopIteration lit,lits){
        QLineSeries *series = new QLineSeries();
        if(!lit.value.isEmpty()){
            series->setName(lit.value.left(lit.value.size()-1));
        }
        for(std::size_t i=0;i<lit.indices.size();++i){
            if(lit.indices[i]){
                bool ok_x,ok_y;
                qreal x=csv[index_x].value(i).toDouble(&ok_x);
                qreal y=csv[index_y].value(i).toDouble(&ok_y);
                if(ok_x && ok_y){
                    QPointF pt(x,y);
                    series->append(pt);
                }
            }
        }
        chart->addSeries(series);
        seriesAdded(series);
    }
    if(lits.size()<2){
        chart->legend()->hide();
    }
    chart->createDefaultAxes();
    chart->setTitle("Line chart");


}
/*!
 * \brief show context menu on table header
 * \param pt
 */
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
    menu->addSeparator();
    act=new QAction(tr("show all"), this);
    act->setData(column);
    connect(act,&QAction::triggered,this,&MainWindow::columnShowAll);
    menu->addAction(act);
    act=new QAction(tr("show none"), this);
    act->setData(column);
    connect(act,&QAction::triggered,this,&MainWindow::columnShowNone);
    menu->addAction(act);
    QStringList lst=csv[column];
    lst.removeDuplicates();
    if(lst.size()<20){
        int cfi=getColumnFilter(column);
        for(const QString &elem:lst){
            act=new QAction(elem, this);
            act->setCheckable(true);
            bool check=true;
            if(cfi>=0){
                if(!columnFilters[cfi].allowedValues.contains(elem))
                    check=false;
            }
            act->setChecked(check);
            act->setData(column);
            connect(act,&QAction::toggled,this,&MainWindow::filterElementChanged);
            menu->addAction(act);
        }
    }

    menu->popup(tableWidget->horizontalHeader()->viewport()->mapToGlobal(pt));
}
/*!
 * \brief add Sweep Var
 */
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
/*!
 * \brief add Plot Var
 */
void MainWindow::addPlotVar()
{
    QAction *act=qobject_cast<QAction*>(sender());
    QString var=act->data().toString();
    plotValues.prepend(var);
    updateSweepGUI();
}
/*!
 * \brief set zoomAreaMode
 */
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

void MainWindow::populateRecentFiles()
{
    recentFilesMenu->clear();
    for(const QString &elem:recentFiles){
        QAction *act=new QAction(elem);
        connect(act,&QAction::triggered,this,&MainWindow::openRecentFile);
        recentFilesMenu->addAction(act);
    }
}
/*!
 * \brief filter button toggled
 * \param checked
 */
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
/*!
 * \brief filter text was changed
 * Filtering is updated if turned on
 * \param text
 */
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

void MainWindow::columnShowAll()
{
    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    int cfi=getColumnFilter(column);
    if(cfi>=0){
        columnFilters.removeAt(cfi);
        updateFilteredTable();
        updateColBackground(column,false);
    }
}

void MainWindow::updateColBackground(int col,bool filtered){
    // color filter columns
    for(int row=0;row<tableWidget->rowCount();++row){
        QTableWidgetItem *item=tableWidget->item(row,col);
        if(filtered)
            item->setBackground(Qt::cyan);
        else
            item->setBackground(Qt::white);
    }
}

void MainWindow::columnShowNone()
{
    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    int cfi=getColumnFilter(column);
    if(cfi>=0){
        columnFilters[cfi].allowedValues.clear();
    }else{
        ColumnFilter cf;
        cf.column=column;
        columnFilters.append(cf);
        updateColBackground(column,true);
    }
    updateFilteredTable();
}

void MainWindow::updateFilteredTable()
{
    int sz=csv[0].size();
    visibleRows.resize(sz);
    std::fill(visibleRows.begin(),visibleRows.end(),true);
    QList<int> colsFiltered;
    for(const ColumnFilter &cf:columnFilters){
        filterRowsForColumnValues(cf);
        colsFiltered.append(cf.column);
    }
    for(int i=0;i<tableWidget->rowCount();++i){
        bool hide = !visibleRows.at(i);
        tableWidget->setRowHidden(i,hide);
    }

}

void MainWindow::filterRowsForColumnValues(ColumnFilter cf)
{
    int column=cf.column;
    QStringList &colVals=csv[column];
    for(int i=0;i<colVals.size();++i){
        if(visibleRows[i]){
            if(!cf.allowedValues.contains(colVals[i])){
                visibleRows[i]=false;
            }
        }
    }
}

void MainWindow::filterElementChanged(bool checked)
{
    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    QString value=act->text();
    int cfi=getColumnFilter(column);
    if(cfi<0){
        ColumnFilter cf;
        cf.column=column;
        QStringList lst=csv[column];
        lst.removeDuplicates();
        cf.allowedValues=lst;
        columnFilters.append(cf);
        cfi=columnFilters.size()-1;
        updateColBackground(cfi,true);
    }
    if(checked){
        columnFilters[cfi].allowedValues.append(value);
        //TODO remove filter if all is allowed
    }else{
        columnFilters[cfi].allowedValues.removeOne(value);
    }
    updateFilteredTable();
}
/*!
 * \brief operator << for debug QList<loopIteration>
 * \param d
 * \param dt
 * \return
 */
QDebug operator<< (QDebug d, const QList<LoopIteration>& dt) {
    foreach(const LoopIteration &lit,dt){
        d << lit.value << '/' << lit.indices;
    }
    return d;
}
/*!
 * \brief simple unit test until better solution
 */
void MainWindow::test()
{
    if(csv.isEmpty()) return;
    std::vector<bool> providedIndices(csv[0].size());
    for(int i=0;i<csv[0].size();++i){
        providedIndices[i]=true;
    }
    QStringList vals=getUniqueValues("x",providedIndices);
    qDebug()<<"x"<<vals;
    vals=getUniqueValues("s",providedIndices);
    qDebug()<<"s"<<vals;
    std::vector<bool> indices=filterIndices("s","2",providedIndices);
    qDebug()<<"s indices"<<indices;
    indices=filterIndices("x","0.2",providedIndices);
    qDebug()<<"x inices"<<indices;
    QStringList vars;
    vars<<"s";
    QList<LoopIteration> lits=groupBy(vars);
    qDebug()<<"by s:"<<lits;
    vars.clear();
    vars<<"s"<<"h";
    lits=groupBy(vars);
    qDebug()<<"by s,h:"<<lits;
    vars.clear();
    lits=groupBy(vars);
    qDebug()<<"by none:"<<lits;
}
/*!
 * \brief get column number from header name
 * \param name
 * \return
 */
int MainWindow::getIndex(const QString &name)
{
    int result=columns.indexOf(name);
    return result;
}
/*!
 * \brief check if specific column has a filter
 * \param column
 * \return
 */
bool MainWindow::hasColumnFilter(int column) const
{
    for(const ColumnFilter& cf:columnFilters){
        if(cf.column==column)
            return true;
    }
    return false;
}
/*!
 * \brief return column filter for given column
 * \param column
 * \return
 */
int MainWindow::getColumnFilter(int column) const
{
    for(int i=0;i<columnFilters.size();++i){
        const ColumnFilter& cf=columnFilters[i];
        if(cf.column==column)
            return i;
    }
    return -1;
}
/*!
 * \brief get Unique Values for given var in list of indices
 * \param var variable name
 * \param indices
 * \return list of values
 */
QStringList MainWindow::getUniqueValues(const QString &var, const std::vector<bool> &indices)
{
    int index=getIndex(var);
    QStringList result;
    for(std::size_t i=0;i<indices.size();++i){
        if(indices[i]){
            result<<csv[index][i];
        }
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
std::vector<bool> MainWindow::filterIndices(const QString &var, const QString &value, const std::vector<bool> &providedIndices)
{
    int index=getIndex(var);
    std::vector<bool> result=providedIndices;
    for(std::size_t i=0;i<result.size();++i){
        if(csv[index][i]!=value){
            result[i]=false;
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
QList<LoopIteration> MainWindow::groupBy(QStringList sweepVar,std::vector<bool> providedIndices)
{
    QList<LoopIteration> result;
    if(providedIndices.size()==0){
        // fill from 0 to size(csv)-1
        int sz=csv[0].size();
        providedIndices.resize(sz);
        for(int i=0;i<csv[0].size();++i){
            providedIndices[i]=true;
        }
    }
    if(!sweepVar.isEmpty()){
        QString var=sweepVar.takeFirst();
        QStringList values=getUniqueValues(var,providedIndices);
        for(const QString &value:values){
            std::vector<bool> indices=filterIndices(var,value,providedIndices);
            QList<LoopIteration>groupedResult=groupBy(sweepVar,indices);
            for(LoopIteration &lit:groupedResult){
                lit.value.prepend(var+"="+value+";");
            }
            result.append(groupedResult);
        }
    }else{
        LoopIteration lit;
        lit.indices=providedIndices;
        result<<lit;
    }
    return result;
}
/*!
 * \brief change series visibility when legendMarker is clicked
 */
void MainWindow::legendMarkerClicked()
{
    auto* marker = qobject_cast<QLegendMarker*>(sender());
    Q_ASSERT(marker);

    // Toggle visibility of series
    setSeriesVisible(marker->series(), !marker->series()->isVisible());
}
/*!
 * \brief highlight series when hovering over legen marker
 * Not working properly !!!
 * \param hover
 *
 */
void MainWindow::legendMarkerHovered(bool hover)
{
    auto* marker = qobject_cast<QLegendMarker*>(sender());
    Q_ASSERT(marker);

    QFont font = marker->font();
    font.setBold(hover);
    marker->setFont(font);

    if (marker->series()->type() == QAbstractSeries::SeriesTypeLine) {
        auto series = qobject_cast<QLineSeries*>(marker->series());
        auto pen = series->pen();
        pen.setWidth(hover ? (pen.width() * 2) : (pen.width() / 2));
        series->setPen(pen);
    }
}
/*!
 * \brief set series in chart visible
 * \param series
 * \param visible
 */
void MainWindow::setSeriesVisible(QAbstractSeries *series, bool visible)
{
    ZoomableChart *m_chart=qobject_cast<ZoomableChart *>(chartView->chart());
    if(!m_chart) return;
    series->setVisible(visible);
    for (auto *marker : m_chart->legend()->markers(series)) {
        // Turn legend marker back to visible, since hiding series also hides the marker
        // and we don't want it to happen now.
        marker->setVisible(true);

        // Dim the marker, if series is not visible
        qreal alpha = visible ? 1.0 : 0.5;
        QColor color;
        QBrush brush = marker->labelBrush();
        color = brush.color();
        color.setAlphaF(alpha);
        brush.setColor(color);
        marker->setLabelBrush(brush);

        brush = marker->brush();
        color = brush.color();
        color.setAlphaF(alpha);
        brush.setColor(color);
        marker->setBrush(brush);

        QPen pen = marker->pen();
        color = pen.color();
        color.setAlphaF(alpha);
        pen.setColor(color);
        marker->setPen(pen);
    }

    for (QAbstractAxis *axis : m_chart->axes(Qt::Vertical)) {
        bool hideAxis = true;
        for (auto *series : m_chart->series()) {
            for (const auto *attachedAxis : series->attachedAxes()) {
                if (series->isVisible() && attachedAxis == axis) {
                    hideAxis = false;
                    break;
                }
            }
            if (!hideAxis)
                break;
        }
        axis->setVisible(!hideAxis);
    }
}
/*!
 * \brief slot for updating legend markers
 * Only directly used here.
 * \param series
 */
void MainWindow::seriesAdded(QAbstractSeries *series)
{
    // Connect all markers to handler
    ZoomableChart *m_chart=qobject_cast<ZoomableChart *>(chartView->chart());
    if(!m_chart) return;
    const auto markers = m_chart->legend()->markers(series);
    for (auto marker : markers) {
        QObject::connect(marker, &QLegendMarker::clicked,
                         this, &MainWindow::legendMarkerClicked);
        QObject::connect(marker, &QLegendMarker::hovered,
                         this, &MainWindow::legendMarkerHovered);
    }
}
/*!
 * \brief remove signal/slot connect when series is removed
 * Basically unused.
 * \param series
 */
void MainWindow::seriesRemoved(QAbstractSeries *series)
{
    // Connect all markers to handler
    ZoomableChart *m_chart=qobject_cast<ZoomableChart *>(chartView->chart());
    if(!m_chart) return;
    const auto markers = m_chart->legend()->markers(series);
    for (auto marker : markers) {
        QObject::disconnect(marker, &QLegendMarker::clicked,
                            this, &MainWindow::legendMarkerClicked);
        QObject::disconnect(marker, &QLegendMarker::hovered,
                            this, &MainWindow::legendMarkerHovered);
    }
}

/* TODO
Unit tests
recent files
better filter selector
cursor in plot
*/

