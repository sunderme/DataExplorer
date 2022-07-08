/****************************************************************************
**
** Copyright (C) 2022 Jan Sundermeyer
**
** License: GLP v3
**
****************************************************************************/

#include "mainwindow.h"

#include <QMenuBar>
#include <QFileDialog>
#include <QTextStream>
#include <QtCharts>
#include <QtGlobal>
#include <QSettings>
#include <set>
#include "zoomablechart.h"

/*!
 * \brief construct GUI
 * Read in settings, build menu&GUI
 * Interpret CLI options
 * \param argc
 * \param argv
 * \param parent
 */
MainWindow::MainWindow(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent)
{
    QSettings settings("DataExplorer","DataExplorer");
    m_recentFiles=settings.value("recentFiles").toStringList();
    m_recentTemplates=settings.value("recentTemplates").toStringList();
    m_chartTheme=static_cast<QChart::ChartTheme>(settings.value("chartTheme",QChart::ChartThemeLight).toInt());
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
        m_fileName=QString(argv[argc-1]);
        readFile();
    }
}
/*!
 * \brief write settings on closeEvent
 * \param event
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("DataExplorer", "DataExplorer");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("recentFiles",m_recentFiles);
    settings.setValue("recentTemplates",m_recentTemplates);
    settings.setValue("chartTheme",m_chartTheme);
    event->accept();
}


MainWindow::~MainWindow()
{
}
/*!
 * \brief define all menus
 */
void MainWindow::setupMenus()
{
    m_recentFilesMenu=new QMenu(tr("Open recent files..."));
    m_recentTemplatesMenu=new QMenu(tr("Open recent templates..."));
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_openAct = new QAction(tr("&Open"), this);
    connect(m_openAct, &QAction::triggered, this, &MainWindow::openFile);
    m_fileMenu->addAction(m_openAct);
    populateRecentFiles();
    m_fileMenu->addMenu(m_recentFilesMenu);
    QAction *loadTemplateAct=new QAction(tr("&Open Template"), this);
    connect(loadTemplateAct, &QAction::triggered, this, &MainWindow::openTemplate);
    m_fileMenu->addAction(loadTemplateAct);
    populateRecentTemplates();
    m_fileMenu->addMenu(m_recentTemplatesMenu);
    QAction *saveTemplateAct=new QAction(tr("&Save Template"), this);
    connect(saveTemplateAct, &QAction::triggered, this, &MainWindow::saveTemplate);
    m_fileMenu->addAction(saveTemplateAct);
    QAction *act=new QAction(tr("Export Plot as Image"),this);
    connect(act,&QAction::triggered,this,&MainWindow::exportPlotImage);
    m_fileMenu->addAction(act);

    m_exitAct = new QAction(tr("&Quit"), this);
    connect(m_exitAct, &QAction::triggered, this, &MainWindow::close);
    m_fileMenu->addAction(m_exitAct);

    m_editMenu = menuBar()->addMenu(tr("&Edit"));

    QAction *copyAction=new QAction(tr("Copy cell content"),this);
    connect(copyAction, &QAction::triggered, this, &MainWindow::copyCell);
    m_editMenu->addAction(copyAction);
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    copyAction->setShortcut(Qt::ControlModifier|Qt::Key_C);
#else
    copyAction->setShortcut(Qt::ControlModifier+Qt::Key_C);
#endif


    QAction *copyHAction=new QAction(tr("Copy header"),this);
    connect(copyHAction, &QAction::triggered, this, &MainWindow::copyHeader);
    m_editMenu->addAction(copyHAction);
    copyHAction->setShortcut(Qt::Key_C);

    QToolBar *plotToolBar = addToolBar(tr("Plot"));
    m_plotMenu = menuBar()->addMenu(tr("&Plot"));
    m_plotAct = new QAction(tr("&Plot"), this);
    m_plotAct->setIcon(QIcon(":/icons/labplot-xy-curve-segments.svg"));
    plotToolBar->addAction(m_plotAct);
    connect(m_plotAct, &QAction::triggered, this, &MainWindow::plotSelected);
    m_plotMenu->addAction(m_plotAct);

    act=new QAction(tr("Copy plot to clipboard"),this);
    act->setIcon(QIcon(":/icons/edit-copy.svg"));
    connect(act,&QAction::triggered,this,&MainWindow::copyPlotToClipboard);
    m_plotMenu->addAction(act);
    plotToolBar->addAction(act);
    act=new QAction(tr("Zoom area"),this);
    act->setIcon(QIcon(":/icons/zoom.svg"));
    act->setShortcut(Qt::Key_Z);
    connect(act,&QAction::triggered,this,&MainWindow::zoomAreaMode);
    m_plotMenu->addAction(act);
    plotToolBar->addAction(act);
    act=new QAction(tr("Zoom X"),this);
    act->setIcon(QIcon(":/icons/zoom-select-x.svg"));
    act->setShortcut(Qt::Key_X);
    connect(act,&QAction::triggered,this,&MainWindow::zoomX);
    m_plotMenu->addAction(act);
    plotToolBar->addAction(act);
    act=new QAction(tr("Zoom Y"),this);
    act->setIcon(QIcon(":/icons/zoom-select-y.svg"));
    act->setShortcut(Qt::Key_Y);
    connect(act,&QAction::triggered,this,&MainWindow::zoomY);
    m_plotMenu->addAction(act);
    plotToolBar->addAction(act);
    act=new QAction(tr("Zoom in"),this);
    act->setIcon(QIcon(":/icons/zoomin.svg"));
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    act->setShortcut(Qt::ShiftModifier|Qt::Key_Z);
#else
    act->setShortcut(Qt::ShiftModifier+Qt::Key_Z);
#endif
    connect(act,&QAction::triggered,this,&MainWindow::zoomIn);
    m_plotMenu->addAction(act);
    act=new QAction(tr("Zoom out"),this);
    act->setIcon(QIcon(":/icons/zoomout.svg"));
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    act->setShortcut(Qt::ControlModifier|Qt::Key_Z);
#else
    act->setShortcut(Qt::ControlModifier+Qt::Key_Z);
#endif
    connect(act,&QAction::triggered,this,&MainWindow::zoomOut);
    m_plotMenu->addAction(act);
    act=new QAction(tr("Zoom in X"),this);
    act->setIcon(QIcon(":/icons/zoomin.svg"));
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    act->setShortcut(Qt::ShiftModifier|Qt::Key_X);
#else
    act->setShortcut(Qt::ShiftModifier+Qt::Key_X);
#endif
    connect(act,&QAction::triggered,this,&MainWindow::zoomInX);
    m_plotMenu->addAction(act);
    act=new QAction(tr("Zoom out"),this);
    act->setIcon(QIcon(":/icons/zoomout.svg"));
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    act->setShortcut(Qt::ControlModifier|Qt::Key_X);
#else
    act->setShortcut(Qt::ControlModifier+Qt::Key_X);
#endif
    connect(act,&QAction::triggered,this,&MainWindow::zoomOutX);
    m_plotMenu->addAction(act);

    act=new QAction(tr("Zoom in Y"),this);
    act->setIcon(QIcon(":/icons/zoomin.svg"));
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    act->setShortcut(Qt::ShiftModifier|Qt::Key_Y);
#else
    act->setShortcut(Qt::ShiftModifier+Qt::Key_Y);
#endif
    connect(act,&QAction::triggered,this,&MainWindow::zoomInY);
    m_plotMenu->addAction(act);
    act=new QAction(tr("Zoom out Y"),this);
    act->setIcon(QIcon(":/icons/zoomout.svg"));
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    act->setShortcut(Qt::ControlModifier|Qt::Key_Y);
#else
    act->setShortcut(Qt::ControlModifier+Qt::Key_Y);
#endif
    connect(act,&QAction::triggered,this,&MainWindow::zoomOutY);
    m_plotMenu->addAction(act);

    act=new QAction(tr("Zoom fit"),this);
    act->setIcon(QIcon(":/icons/zoom-fit-best.svg"));
    act->setShortcut(Qt::Key_F);
    connect(act,&QAction::triggered,this,&MainWindow::zoomReset);
    m_plotMenu->addAction(act);
    plotToolBar->addAction(act);
    act=new QAction(tr("Pan"),this);
    act->setIcon(QIcon(":/icons/transform-move.svg"));
    act->setShortcut(Qt::Key_P);
    connect(act,&QAction::triggered,this,&MainWindow::panMode);
    m_plotMenu->addAction(act);
    plotToolBar->addAction(act);

    act=new QAction(tr("Vertical Marker"),this);
    act->setIcon(QIcon(":/icons/zoom-select-y.svg"));
    act->setShortcut(Qt::Key_V);
    connect(act,&QAction::triggered,this,&MainWindow::addVerticalMarker);
    m_plotMenu->addAction(act);
    act=new QAction(tr("Horizontal Marker"),this);
    act->setIcon(QIcon(":/icons/zoom-select-x.svg"));
    act->setShortcut(Qt::Key_H);
    connect(act,&QAction::triggered,this,&MainWindow::addHorizontalMarker);
    m_plotMenu->addAction(act);
    act=new QAction(tr("Set Marker A"),this);
    act->setIcon(QIcon(":/icons/zoom-select-x.svg"));
    act->setShortcut(Qt::Key_A);
    connect(act,&QAction::triggered,this,&MainWindow::addMarkerA);
    m_plotMenu->addAction(act);
    act=new QAction(tr("Set Marker B"),this);
    act->setIcon(QIcon(":/icons/zoom-select-x.svg"));
    act->setShortcut(Qt::Key_B);
    connect(act,&QAction::triggered,this,&MainWindow::addMarkerB);
    m_plotMenu->addAction(act);

    act=new QAction(tr("delete"),this);
    act->setIcon(QIcon(":/icons/delete.svg"));
    act->setShortcut(QKeySequence::Delete);
    m_plotMenu->addAction(act);
    plotToolBar->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::deleteVar);

    QAction *testAction=new QAction("test",this);
    connect(testAction, &QAction::triggered, this, &MainWindow::test);
    m_plotMenu->addAction(testAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    act=new QAction(tr("About"),this);
    helpMenu->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::about);
}
/*!
 * \brief generate GUI
 */
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
    lstData->setDragEnabled(true);
    lstData->setDefaultDropAction(Qt::MoveAction);
    lstData->setSelectionMode(QAbstractItemView::SingleSelection);
    lstData->setAcceptDrops(true);
    lstData->setDropIndicatorShown(true);
    hLayout->addWidget(lstSweeps);
    hLayout->addWidget(lstData);
    mainLayout->addLayout(hLayout,1);
    QHBoxLayout *hLayout2 = new QHBoxLayout;
    btFilterPlot=new QToolButton;
    btFilterPlot->setCheckable(true);
    btFilterPlot->setText("Filter to plot");
    btFilterPlot->setIcon(QIcon(":/icons/view-filter-plot.svg"));
    connect(btFilterPlot,&QAbstractButton::toggled,this,&MainWindow::filterPlotToggled);
    hLayout2->addWidget(btFilterPlot);
    /*
    btFilterChecked=new QToolButton;
    btFilterChecked->setCheckable(true);
    btFilterChecked->setText("Filter to checked headers");
    btFilterChecked->setIcon(QIcon(":/icons/view-filter-checked.svg"));
    connect(btFilterChecked,&QAbstractButton::toggled,this,&MainWindow::filterCheckedToggled);
    hLayout2->addWidget(btFilterChecked);
    */
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
    chartView->chart()->setTheme(m_chartTheme);
    //chartView->setZoomMode(ZoomableChartView::RectangleZoom);
    //chartView->setMouseTracking(true);

    tabWidget = new QTabWidget;
    tabWidget->addTab(wgt,tr("CSV"));
    tabWidget->addTab(chartView,tr("Plots"));
    connect(tabWidget,&QTabWidget::currentChanged,this,&MainWindow::tabChanged);

    tableWidget->horizontalHeader()-> setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableWidget->horizontalHeader(),&QAbstractItemView::customContextMenuRequested,this,&MainWindow::headerMenuRequested);

    setCentralWidget(tabWidget);
    this->setMouseTracking(true);
}
/*!
 * \brief call file dialog and then open file
 */
void MainWindow::openFile()
{
    m_fileName = QFileDialog::getOpenFileName(this,
        tr("Open CSV"), "", tr("CSV Files (*.csv);;VCSV Files (*.vcsv);;S2P Files (*.s2p);;S3P Files (*.s3p);;S4P Files (*.s4p)"));
    if(m_fileName.isEmpty()) return;
    readFile();
    m_recentFiles.removeOne(m_fileName);
    m_recentFiles.prepend(m_fileName);
    populateRecentFiles();
}
/*!
 * \brief open file via recent menu
 */
void MainWindow::openRecentFile()
{
    QAction *act=qobject_cast<QAction*>(sender());
    m_fileName=act->text();
    m_recentFiles.removeOne(m_fileName);
    if(QFileInfo::exists(m_fileName)){
        readFile();
        m_recentFiles.prepend(m_fileName);
    }
    populateRecentFiles();
}
/*!
 * \brief open template via recent menu
 */
void MainWindow::openRecentTemplate()
{
    QAction *act=qobject_cast<QAction*>(sender());
    QString fileName=act->text();
    m_recentTemplates.removeOne(fileName);
    if(QFileInfo::exists(fileName)){
        readTemplate(fileName);
        m_recentTemplates.prepend(fileName);
    }
    populateRecentTemplates();
}
/*!
 * \brief read in csv file and update GUI
 */
void MainWindow::readFile()
{
    if(m_fileName.isEmpty()) return;
    bool ok;
    if(m_fileName.endsWith(".s2p")){
        ok=readInSNP(m_fileName,2);
    }else{
        if(m_fileName.endsWith(".s3p")){
            ok=readInSNP(m_fileName,3);
        }else{
            if(m_fileName.endsWith(".s4p")){
                ok=readInSNP(m_fileName,4);
            }else{
                ok=readInCSV(m_fileName);
            }
        }
    }
    if(!ok) return;
    buildTable();
    m_sweeps.clear();
    m_plotValues.clear();
    if(m_columns.size()==2){
        //assume first x, second y
        m_sweeps<<m_columns[0];
        m_plotValues<<m_columns[1];
    }else{
        if(m_columns.contains("x")){
            // add x to sweeps
            m_sweeps<<"x";
        }
        if(m_columns.contains("y")){
            // add y to plot values
            m_plotValues<<"y";
        }
    }
    updateSweepGUI();
    // set window title to filename
    setWindowFilePath(m_fileName);
}
/*!
 * \brief open Template which contains the sweeps/plots settings
 */
void MainWindow::openTemplate()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open template"), "", tr("Template File (*.deTemplate)"));
    readTemplate(fileName);
    m_recentTemplates.removeOne(fileName);
    m_recentTemplates.prepend(fileName);
    populateRecentTemplates();
}
/*!
 * \brief save sweeps/plots as template
 * Format is json
 */
void MainWindow::saveTemplate()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save template"), m_fileName+".deTemplate", tr("Template File (*.deTemplate)"));
    if(fileName.isEmpty()) return;
    QFile saveFile(fileName);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }

    QJsonObject jo;
    jo["sweeps"]=QJsonArray::fromStringList(m_sweeps);
    jo["plots"]=QJsonArray::fromStringList(m_plotValues);
    // columnFilter
    QJsonArray jFilters;
    for(int i=0;i<m_columnFilters.size();++i){
        const ColumnFilter& cf=m_columnFilters[i];
        QJsonObject jCF;
        jCF["name"]=m_columns.value(cf.column);
        jCF["values"]=QJsonArray::fromStringList(cf.allowedValues);
        jFilters.append(jCF);
    }
    jo["filters"]=jFilters;

    QJsonDocument saveDoc(jo);
    saveFile.write(saveDoc.toJson());
}
/*!
 * \brief read in template
 * \param fileName
 */
void MainWindow::readTemplate(const QString &fileName)
{
    if(fileName.isEmpty()) return;
    QFile loadFile(fileName);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }
    QByteArray data = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    QJsonObject jo=loadDoc.object();

    // handle variables (sweep/plot)
    QJsonArray ja=jo["sweeps"].toArray();
    m_sweeps.clear();
    for(int i = 0; i < ja.size(); ++i) {
        m_sweeps<<ja[i].toString();
    }
    ja=jo["plots"].toArray();
    m_plotValues.clear();
    for(int i = 0; i < ja.size(); ++i) {
        m_plotValues<<ja[i].toString();
    }
    // handle filters
    // reset old filter
    for(const ColumnFilter &cf:m_columnFilters){
        updateColBackground(cf.column,false);
    }
    m_columnFilters.clear();
    // load new filter
    ja=jo["filters"].toArray();
    for(int i = 0; i < ja.size(); ++i) {
        ColumnFilter cf;
        QJsonObject jCF=ja[i].toObject();
        QString columnName=jCF["name"].toString();
        cf.column=getIndex(columnName);
        if(cf.column<0) continue; // name not present in current data
        std::vector<bool> providedIndices(m_csv[0].size());
        for(int i=0;i<m_csv[0].size();++i){
            providedIndices[i]=true;
        }
        QStringList presentValues=getUniqueValues(columnName,providedIndices);
        presentValues.sort();
        QJsonArray jValues=jCF["values"].toArray();
        for(int k=0;k<jValues.size();++k){
            QString val=jValues[k].toString();
            if(std::binary_search(presentValues.constBegin(),presentValues.constEnd(),val)){
                cf.allowedValues<<val;
            }
        }
        m_columnFilters.append(cf);
        updateColBackground(cf.column,true);
    }
    updateSweepGUI();
    updateFilteredTable();
}
/*!
 * \brief read in CSV
 * Assumes comma separated values
 * First line with commas is assumed to be header line
 * \param fileName
 * \return operation successful
 */
bool MainWindow::readInCSV(const QString &fileName)
{
    QFile dataFile(fileName);
    if (dataFile.open(QFile::ReadOnly)){
        QTextStream stream(&dataFile);
        QString line;
        // first line with commas is column names
        QString rest; // for VCSV
        bool found=false;
        while(stream.readLineInto(&line)){
            if(line.startsWith('!') || line.isEmpty())
                continue;
            if(line.startsWith(';') || line.isEmpty()){
                // VCSV
                if(fileName.endsWith(".vcsv")){
                    // try to interpret vcsv as header
                    m_columns=rest.split(','); // comment before last contains column names  ?
                    rest=line.mid(1);
                }
                continue;
            }
            if(fileName.endsWith(".vcsv")) break; // special treatment for VCSV
            m_columns=line.split(',');
            if(m_columns.size()>1){
                found=true;
                break;
            }
        }
        if(!found){
            // single column
            // repeat read
            stream.seek(0);
            m_columns[0]="x";
            m_columns<<"y";
        }
        int l=0;
        QVector<QStringList> data(m_columns.size());
        bool errorOccured=false;
        while (stream.readLineInto(&line)) {
            QStringList elements=splitAtComma(line);
            if(!found){
                // special treatment single column
                data[0]<<QString("%1").arg(l++);
                data[1]<<line;
                continue;
            }
            if(elements.size()!=m_columns.size() ){ //
                // columns estimate wrong but ignore empty lines or lines without comma (e.g. END at end of csv)
                if(!line.isEmpty() && elements.size()>1){
                    errorOccured=true;
                    QErrorMessage *msg=new QErrorMessage(this);
                    msg->showMessage(tr("CSV read in failed!\nColumns don't match."));
                    msg->exec();
                    delete msg;
                }
                break;
            }
            for(int i=0;i<elements.size();++i){
                data[i].append(elements[i]);
            }
        }
        if(errorOccured){
            return false;
        }
        m_csv=data;
        m_columnFilters.clear();
        return true;
    }
    return false;
}
/*!
 * \brief read touchstone file
 * \param fileName
 * \return success
 */
bool MainWindow::readInSNP(const QString &fileName,int nrPorts)
{
    QFile dataFile(fileName);
    if (dataFile.open(QFile::ReadOnly)){
        QTextStream stream(&dataFile);
        QString line;
        // first line with commas is column names
        QString prevLine;
        while(stream.readLineInto(&line)){
            if(line.startsWith('!') || line.isEmpty() || line.startsWith('[')){
                prevLine=line;
                continue;
            }
            if(line.startsWith('#')){
                // get data infor
                prevLine=line;
                continue;
            }
            break;
        }
        if(prevLine.startsWith("!")){
            //assume port definition
            prevLine=prevLine.mid(1);
            m_columns=prevLine.split(QRegularExpression("\\s+"),Qt::SkipEmptyParts);
        }
        if(m_columns.size()!=nrPorts*nrPorts*2+1){
            // set port names to default
            m_columns.resize(nrPorts*nrPorts*2+1);
            m_columns[0]="freq";
            for(int i=0;i<nrPorts;++i){
                for(int j=0;j<nrPorts;++j){
                    m_columns[1+2*i*nrPorts+2*j]=QString("dBS%1%2").arg(i+1).arg(j+1);
                    m_columns[1+2*i*nrPorts+2*j+1]=QString("angS%1%2").arg(i+1).arg(j+1);
                }
            }
        }
        QVector<QStringList> data(nrPorts*nrPorts*2+1);
        bool errorOccured=false;
        int lineCount=0;
        int offset=0;
        while (!line.isEmpty()) {
            QStringList elements=line.split(QRegularExpression("\\s+"),Qt::SkipEmptyParts);
            for(int i=0;i<elements.size();++i){
                data[i+offset].append(elements[i]);
            }
            if(nrPorts>2){
                ++lineCount;
                if(lineCount>=nrPorts){
                    offset=0;
                    lineCount=0;
                }else{
                    offset+=elements.size();
                }
            }
            if(!stream.readLineInto(&line))
                break;
        }
        if(errorOccured){
            return false;
        }
        m_csv=data;
        m_columnFilters.clear();
        return true;
    }
    return false;
}
/*!
 * \brief popalte table widget with present data
 */
void MainWindow::buildTable()
{
    tableWidget->clear();
    if(m_csv.isEmpty()) return;
    tableWidget->setRowCount(m_csv[0].size());
    tableWidget->setColumnCount(m_columns.size());
    //tableWidget->setHorizontalHeaderLabels(columns);
    for(int i=0;i<m_columns.size();++i){
        QTableWidgetItem *hdr=new QTableWidgetItem(m_columns[i]);
        hdr->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
        hdr->setCheckState(Qt::Unchecked);
        tableWidget->setHorizontalHeaderItem(i,hdr);
        for(int row=0;row<m_csv[i].size();++row){
            QTableWidgetItem *newItem = new QTableWidgetItem(m_csv[i].value(row));

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
    foreach(const QString &elem,m_sweeps){
        QListWidgetItem *item=new QListWidgetItem(elem,lstSweeps);
        item->setCheckState(Qt::Checked);
    }
    foreach(const QString &elem,m_plotValues){
        QListWidgetItem *item=new QListWidgetItem(elem,lstData);
        item->setCheckState(Qt::Checked);
    }
}
/*!
 * \brief update internal sweep structure from GUI
 */
void MainWindow::updateSweeps(bool filterChecked)
{
    m_sweeps.clear();
    m_plotValues.clear();
    for(int i=0;i<lstSweeps->count();++i){
        if(filterChecked && lstSweeps->item(i)->checkState()!=Qt::Checked)
            continue;
        m_sweeps<<lstSweeps->item(i)->text();
    }
    for(int i=0;i<lstData->count();++i){
        if(filterChecked && lstData->item(i)->checkState()!=Qt::Checked)
            continue;
        m_plotValues<<lstData->item(i)->text();
    }
}
/*!
 * \brief plot data with selected sweeps
 * Only last plot var is plotted.
 */
void MainWindow::plotSelected()
{
    updateSweeps();
    if(m_plotValues.isEmpty()) return;
    if(m_sweeps.isEmpty()) return;
    QStringList vars=m_sweeps;
    QString xn=vars.takeLast();
    int index_x=getIndex(xn);
    if(index_x<0) return;
    bool multiPlot=m_plotValues.size()>1;
    chartView->clear();
    for(const QString &yn:m_plotValues){
        int index_y=getIndex(yn);
        if(index_y<0) continue;

        QList<LoopIteration> lits=groupBy(vars,m_visibleRows);
        foreach(LoopIteration lit,lits){
            QLineSeries *series = new QLineSeries();
            if(!lit.value.isEmpty()){
                QString name=lit.value.left(lit.value.size()-1);
                if(multiPlot)
                    name=yn+":"+name;
                series->setName(name);
            }else{
                series->setName(yn);
            }
            for(std::size_t i=0;i<lit.indices.size();++i){
                if(lit.indices[i]){
                    bool ok_x,ok_y;
                    qreal x=m_csv[index_x].value(i).toDouble(&ok_x);
                    qreal y=m_csv[index_y].value(i).toDouble(&ok_y);
                    if(ok_x && ok_y){
                        QPointF pt(x,y);
                        series->append(pt);
                    }
                }
            }
            chartView->addSeries(series);
        }
    }


    chartView->setTitle("Line chart");
    if(tabWidget->currentIndex()!=1){
        tabWidget->setCurrentIndex(1); // plot tab
    }

}
/*!
 * \brief plot if changed to plot tab
 * \param index
 */
void MainWindow::tabChanged(int index)
{
    if(index==1){
        plotSelected();
    }
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
    act->setData(m_columns[column]);
    connect(act,&QAction::triggered,this,&MainWindow::addSweepVar);
    menu->addAction(act);
    act=new QAction(tr("add as plot var"), this);
    act->setData(m_columns[column]);
    connect(act,&QAction::triggered,this,&MainWindow::addPlotVar);
    menu->addAction(act);
    menu->addSeparator();
    if(isIntOnlyData(column)){
        act=new QAction(tr("show as hex"), this);
        act->setData(column);
        connect(act,&QAction::triggered,this,&MainWindow::showHex);
        menu->addAction(act);
        act=new QAction(tr("show as binary"), this);
        act->setData(column);
        connect(act,&QAction::triggered,this,&MainWindow::showBinary);
        menu->addAction(act);
        act=new QAction(tr("show as decimal"), this);
        act->setData(column);
        connect(act,&QAction::triggered,this,&MainWindow::showDecimal);
        menu->addAction(act);
        menu->addSeparator();
    }
    act=new QAction(tr("copy header"), this);
    act->setData(column);
    connect(act,&QAction::triggered,this,&MainWindow::copyHeader);
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
    QStringList lst=m_csv[column];
    lst.removeDuplicates();
    if(lst.size()<20){
        int cfi=getColumnFilter(column);
        for(const QString &elem:lst){
            act=new QAction(elem, this);
            act->setCheckable(true);
            bool check=true;
            if(cfi>=0){
                if(!m_columnFilters[cfi].allowedValues.contains(elem))
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
    m_sweeps.prepend(var);
    QListWidgetItem *item=new QListWidgetItem(var);
    item->setCheckState(Qt::Checked);
    lstSweeps->insertItem(0,item);
}
/*!
 * \brief remove var from sweepvar/plotvar, depending which one is focused
 */
void MainWindow::deleteVar()
{
    // delete pressed
    if(tabWidget->currentIndex()>0){
        // plot visible
        if(chartView->deleteSelectedMarker()){
            return;
        }
        if(chartView->deleteSelectedSeries()){
            return;
        }
        return;
    }
    if(lstSweeps->hasFocus()){
        auto lst=lstSweeps->selectedItems();
        for(auto elem:lst){
            QString var=elem->text();
            m_sweeps.removeAll(var);
            delete elem;
        }
    }
    if(lstData->hasFocus()){
        auto lst=lstData->selectedItems();
        for(auto elem:lst){
            QString var=elem->text();
            m_plotValues.removeAll(var);
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
    m_plotValues.prepend(var);
    QListWidgetItem *item=new QListWidgetItem(var);
    item->setCheckState(Qt::Checked);
    lstData->insertItem(0,item);
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
    chartView->zoom(1.4);
}

void MainWindow::zoomOut()
{
    chartView->zoom(0.7);
}

void MainWindow::zoomInX()
{
    chartView->zoom(1.2,ZoomableChartView::ZoomDirection::X);
}

void MainWindow::zoomOutX()
{
    chartView->zoom(0.8,ZoomableChartView::ZoomDirection::X);
}

void MainWindow::zoomInY()
{
    chartView->zoom(1.2,ZoomableChartView::ZoomDirection::Y);
}

void MainWindow::zoomOutY()
{
    chartView->zoom(0.8,ZoomableChartView::ZoomDirection::Y);
}

void MainWindow::zoomReset()
{
    chartView->zoomReset();
}
/*!
 * \brief show about message
 */

void MainWindow::about()
{
    QMessageBox::about(this, tr("About DataExplorer"),
                           tr("Version %1\n"
                               "Written by Jan Sundermeyer (C) 2022\n"
                              "Data browsing and plotting app.").arg(DE_VERSION));
}

void MainWindow::addVerticalMarker()
{
    chartView->addVerticalMarker();
}

void MainWindow::addHorizontalMarker()
{
    chartView->addHorizontalMarker();
}

void MainWindow::addMarkerA()
{
    chartView->addMarker(false); // marker A
}

void MainWindow::addMarkerB()
{
    chartView->addMarker(true); // marker A
}


void MainWindow::populateRecentFiles()
{
    m_recentFilesMenu->clear();
    for(const QString &elem:m_recentFiles){
        QAction *act=new QAction(elem);
        connect(act,&QAction::triggered,this,&MainWindow::openRecentFile);
        m_recentFilesMenu->addAction(act);
    }
}

void MainWindow::populateRecentTemplates()
{
    m_recentTemplatesMenu->clear();
    for(const QString &elem:m_recentTemplates){
        QAction *act=new QAction(elem);
        connect(act,&QAction::triggered,this,&MainWindow::openRecentTemplate);
        m_recentTemplatesMenu->addAction(act);
    }
}
/*!
 * \brief filter button toggled
 * \param checked
 */
void MainWindow::filterToggled(bool checked)
{
    if(checked)
        btFilterPlot->setChecked(false);
    // filter columns
    if(!checked){
        for(int i=0;i<m_columns.size();++i){
            tableWidget->showColumn(i);
        }
    }else{
        filterTextChanged(leFilterText->text());
    }
}
/*!
 * \brief filter to only columns which are checked on header
 * \param checked
 */
void MainWindow::filterCheckedToggled(bool checked)
{
    Q_UNUSED(checked);
}
/*!
 * \brief filter to only columns selected for plotting (sweep or plot)
 * \param checked
 */
void MainWindow::filterPlotToggled(bool checked)
{
    if(checked)
        btFilter->setChecked(false);
    updateSweeps(false);
    // filter columns
    for(int i=0;i<m_columns.size();++i){
        if(!checked){
            tableWidget->showColumn(i);
        }else{
            QString text=m_columns.value(i);
            if(m_sweeps.contains(text) || m_plotValues.contains(text) || hasColumnFilter(i)){
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
    if(!btFilter->isChecked()){
        btFilter->setChecked(true);
    }
    if(btFilter->isChecked()){
        for(int i=0;i<m_columns.size();++i){
            if(m_columns.value(i).contains(text, Qt::CaseInsensitive)){
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
        m_columnFilters.removeAt(cfi);
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
        else{
            QColor color=QApplication::palette().color(QPalette::Base);
            item->setBackground(color);
        }
    }
    // color header
    QTableWidgetItem *item=tableWidget->horizontalHeaderItem(col);
    if(filtered){
        QColor color=QApplication::palette().color(QPalette::Midlight);
        item->setBackground(color);
    }else{
        QColor color=QApplication::palette().color(QPalette::Button);
        item->setBackground(color);
    }
}

void MainWindow::columnShowNone()
{
    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    int cfi=getColumnFilter(column);
    if(cfi>=0){
        m_columnFilters[cfi].allowedValues.clear();
    }else{
        ColumnFilter cf;
        cf.column=column;
        m_columnFilters.append(cf);
        updateColBackground(column,true);
    }
    updateFilteredTable();
}

void MainWindow::updateFilteredTable()
{
    if(m_csv.isEmpty()) return;
    int sz=m_csv[0].size();
    m_visibleRows.resize(sz);
    std::fill(m_visibleRows.begin(),m_visibleRows.end(),true);
    QList<int> colsFiltered;
    for(const ColumnFilter &cf:m_columnFilters){
        filterRowsForColumnValues(cf);
        colsFiltered.append(cf.column);
    }
    for(int i=0;i<tableWidget->rowCount();++i){
        bool hide = !m_visibleRows.at(i);
        tableWidget->setRowHidden(i,hide);
    }

}

void MainWindow::filterRowsForColumnValues(ColumnFilter cf)
{
    int column=cf.column;
    QStringList &colVals=m_csv[column];
    for(int i=0;i<colVals.size();++i){
        if(m_visibleRows[i]){
            if(!cf.allowedValues.contains(colVals[i])){
                m_visibleRows[i]=false;
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
        QStringList lst=m_csv[column];
        lst.removeDuplicates();
        cf.allowedValues=lst;
        m_columnFilters.append(cf);
        cfi=m_columnFilters.size()-1;
        updateColBackground(column,true);
    }
    if(checked){
        m_columnFilters[cfi].allowedValues.append(value);
        //remove filter if all is allowed
        QStringList lst=m_csv[column];
        lst.removeDuplicates();
        if(lst.size()==m_columnFilters[cfi].allowedValues.size()){
            // assume identical
            updateColBackground(column,false);
            m_columnFilters.takeAt(cfi);
        }
    }else{
        m_columnFilters[cfi].allowedValues.removeOne(value);
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
    if(m_csv.isEmpty()) return;
    std::vector<bool> providedIndices(m_csv[0].size());
    for(int i=0;i<m_csv[0].size();++i){
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
 * \brief copy content of cell to clipboard
 * For now, copies only first cell
 */
void MainWindow::copyCell()
{
    auto items=tableWidget->selectedItems();
    if(!items.isEmpty()){
        QString txt=items[0]->text();
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(txt);
    }
}
/*!
 * \brief copy header text to clipboard
 */
void MainWindow::copyHeader()
{
    if(tabWidget->currentIndex()>0) return; // plot is visible, no table action
    QAction *act=qobject_cast<QAction*>(sender());
    bool ok;
    int col=act->data().toInt(&ok);
    if(!ok){
        auto items=tableWidget->selectedItems();
        if(!items.isEmpty()){
            col=tableWidget->column(items[0]);
        }else{
            col=-1;
        }
    }
    if(col>=0){
        QString txt=m_columns.value(col);
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(txt);
    }
}
/*!
 * \brief render plot to image and copy it to clipboard
 */
void MainWindow::copyPlotToClipboard()
{
    if(tabWidget->isTabVisible(1)){
        // plot is visivle
        QGraphicsScene *scene=chartView->scene();
        QClipboard *clipboard = QGuiApplication::clipboard();
        QRectF rect=chartView->frameRect();
        int w=rect.width();
        int h=rect.height();
        if(w<2048){
            // force higher resolution
            w=4096;
            h=4096.*rect.height()/rect.width();
        }
        QPixmap pixmap(w,h);
        pixmap.fill();
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        scene->render(&painter,QRectF(),rect);
        painter.end();
        clipboard->setPixmap(pixmap);
    }
}
/*!
 * \brief render plot to image and save it as file
 */
void MainWindow::exportPlotImage()
{
    if(tabWidget->isTabVisible(1)){
        // plot is visible
        QString fileName = QFileDialog::getSaveFileName(this,
            tr("Export image"), m_fileName+".png", tr("PNG File (*.png)"));
        if(fileName.isEmpty()) return;
        QFile saveFile(fileName);
        if (!saveFile.open(QIODevice::WriteOnly)) {
            qWarning("Couldn't open save file.");
            return;
        }
        QGraphicsScene *scene=chartView->scene();
        QRectF rect=chartView->frameRect();
        int w=rect.width();
        int h=rect.height();
        if(w<2048){
            // force higher resolution
            w=4096;
            h=4096.*rect.height()/rect.width();
        }
        QPixmap pixmap(w,h);
        pixmap.fill();
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        scene->render(&painter,QRectF(),rect);
        painter.end();

        pixmap.save(fileName,"PNG");
    }
}
/*!
 * \brief check if data consists only of ints
 * \param column
 * \return
 */
bool MainWindow::isIntOnlyData(int column)
{
    bool ok=true;
    for(qsizetype row=0;row<m_csv[column].count();++row){
        QString cell=m_csv[column].value(row);
        if(cell.startsWith("0b")){
            cell.mid(2).toULongLong(&ok,2);
        }else{
            if(cell.startsWith("0x")){
                cell.mid(2).toULongLong(&ok);
            }else{
                cell.toULongLong(&ok);
            }
        }
        if(!ok) break;
    }
    return ok;
}
/*!
 * \brief get maximum number of bits on all integer number in column
 * Maybe fail for negative numbers !!!!
 * \param column
 * \return
 */
int MainWindow::getIntegerWidth(int column)
{
    int bits=0;
    bool negative=false;
    for(qsizetype row=0;row<m_csv[column].count();++row){
        QString cell=m_csv[column].value(row);
        bool ok;
        qlonglong value=convertStringToLong(cell,ok);
        if(!ok) break;
        if(value<0){
            // wild guessing from here
            value=-value;
            negative=true;
        }
        int k;
        for(k=0;value!=0;++k){
            value=value>>1;
        }
        if(k>bits) bits=k;
    }
    return !negative ? bits : bits+1;
}
/*!
 * \brief show column in table as decimal coding
 */
void MainWindow::showDecimal()
{
    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    bool ok;
    for(qsizetype row=0;row<m_csv[column].count();++row){
        QString cell=m_csv[column].value(row);
        qulonglong value=convertStringToLong(cell,ok);
        if(!ok) break;
        QTableWidgetItem *item=tableWidget->item(row,column);
        item->setText(QString("%1").arg(value,0));
    }
    tableWidget->resizeColumnToContents(column);
}
/*!
 * \brief show column in table as binary coding
 */
void MainWindow::showBinary()
{
    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    int bits=getIntegerWidth(column);
    bool ok;
    for(qsizetype row=0;row<m_csv[column].count();++row){
        QString cell=m_csv[column].value(row);
        qulonglong value=convertStringToLong(cell,ok);
        if(value<0){
            // 2er complement
            value=(1<<bits)+value;
        }
        QTableWidgetItem *item=tableWidget->item(row,column);
        item->setText(QString("0b%1").arg(value,bits,2,QChar('0')));
        if(!ok) break;
    }
    tableWidget->resizeColumnToContents(column);
}
/*!
 * \brief show column in table as hex coding
 */
void MainWindow::showHex()
{
    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    int bits=getIntegerWidth(column);
    int digits=bits/4 + (bits%4==0 ? 0 : 1);
    bool ok;
    for(qsizetype row=0;row<m_csv[column].count();++row){
        QString cell=m_csv[column].value(row);
        qulonglong value=convertStringToLong(cell,ok);
        if(!ok) break;
        if(value<0){
            // 2er complement
            value=(1<<bits)+value;
        }
        QTableWidgetItem *item=tableWidget->item(row,column);
        item->setText(QString("0x%1").arg(value,digits,16,QChar('0')));
    }
    tableWidget->resizeColumnToContents(column);
}
/*!
 * \brief convert String to long
 * Can handle 0x and 0b formats
 * \param text
 * \param ok
 * \return
 */
qulonglong MainWindow::convertStringToLong(QString text, bool &ok)
{
    qulonglong value;
    if(text.startsWith("0b")){
        value=text.mid(2).toULongLong(&ok,2);
    }else{
        if(text.startsWith("0x")){
            value=text.mid(2).toULongLong(&ok,16);
        }else{
            value=text.toULongLong(&ok);
        }
    }
    return value;
}
/*!
 * \brief get column number from header name
 * \param name
 * \return
 */
int MainWindow::getIndex(const QString &name)
{
    int result=m_columns.indexOf(name);
    return result;
}
/*!
 * \brief check if specific column has a filter
 * \param column
 * \return
 */
bool MainWindow::hasColumnFilter(int column) const
{
    for(const ColumnFilter& cf:m_columnFilters){
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
    for(int i=0;i<m_columnFilters.size();++i){
        const ColumnFilter& cf=m_columnFilters[i];
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
    if(index<0) return result;
    for(std::size_t i=0;i<indices.size();++i){
        if(indices[i]){
            result<<m_csv[index][i];
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
        if(m_csv[index][i]!=value){
            result[i]=false;
        }
    }
    return result;
}
/*!
 * \brief split line at commas but handle quotes correctly
 * \param line
 * \return split line
 */
QStringList MainWindow::splitAtComma(const QString &line) const
{
    if(line.contains("\"")){
        QStringList result;
        qsizetype pos=-1;
        qsizetype oldPos=0;
        qsizetype posQuote=line.indexOf("\"");
        qsizetype posQuote2=line.indexOf("\"",posQuote+1);
        while((pos=line.indexOf(",",pos+1))>=0){
            if(pos>posQuote && posQuote2>pos){
                continue; // skip through quote
            }
            posQuote=line.indexOf("\"",pos);
            posQuote2=line.indexOf("\"",posQuote+1);
            result<<line.mid(oldPos,pos-oldPos);
            oldPos=pos+1;
        }
        result<<line.mid(oldPos);
        return result;
    }else{
        return line.split(",");
    }
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
        int sz=m_csv[0].size();
        providedIndices.resize(sz);
        for(int i=0;i<m_csv[0].size();++i){
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

/* TODO
Unit tests
chart style in config
touchstone
log axis
import vcsv
edit trace name/title
*/

