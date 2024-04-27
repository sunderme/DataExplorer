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
 * \brief Max
 * \param a
 * \param b
 * \return
 */
template <typename T>
int compare (T const& a, T const& b) {
    int result=0;
    if(a>b) result=1;
    if(a<b) result=-1;
    return result;
}
/*!
 * \brief construct GUI
 * Read in settings, build menu&GUI
 * Interpret CLI options
 * \param argc
 * \param argv
 * \param parent
 */
MainWindow::MainWindow(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent),m_logx(false),m_logy(false)
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
    m_reloadAct = new QAction(tr("&Reload"), this);
    m_reloadAct->setShortcut(Qt::Key_F5);
    connect(m_reloadAct, &QAction::triggered, this, &MainWindow::reloadFile);
    m_fileMenu->addAction(m_reloadAct);
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

    m_logyAct=new QAction(tr("Log Y"),this);
    //act->setIcon(QIcon(":/icons/zoom-select-x.svg"));
    m_logyAct->setShortcut(Qt::Key_L);
    connect(m_logyAct,&QAction::triggered,this,&MainWindow::setLinLogY);
    m_plotMenu->addAction(m_logyAct);
    m_logxAct=new QAction(tr("Log X"),this);
    //act->setIcon(QIcon(":/icons/zoom-select-x.svg"));
    m_logxAct->setShortcut(Qt::ShiftModifier | Qt::Key_L);
    connect(m_logxAct,&QAction::triggered,this,&MainWindow::setLinLogX);
    m_plotMenu->addAction(m_logxAct);

    act=new QAction(tr("delete"),this);
    act->setIcon(QIcon(":/icons/delete.svg"));
    act->setShortcut(QKeySequence::Delete);
    m_plotMenu->addAction(act);
    plotToolBar->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::deleteVar);

    m_plotTypeMenu=new QMenu(tr("plot type"));
    m_plotTypeActionGroup=new QActionGroup(this);
    m_plotTypeActionGroup->setExclusive(true);
    act=new QAction(tr("line series"),this);
    act->setCheckable(true);
    act->setChecked(true);
    connect(act,&QAction::triggered,this,&MainWindow::plotStyleChanged);
    m_plotTypeMenu->addAction(act);
    m_plotTypeActionGroup->addAction(act);
    m_lineSeriesAveragedAct=new QAction(tr("line series averaged"),this);
    m_lineSeriesAveragedAct->setCheckable(true);
    connect(m_lineSeriesAveragedAct,&QAction::triggered,this,&MainWindow::plotStyleChanged);
    m_plotTypeActionGroup->addAction(m_lineSeriesAveragedAct);
    m_plotTypeMenu->addAction(m_lineSeriesAveragedAct);
    m_plotMenu->addMenu(m_plotTypeMenu);

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
    btRegExp=new QToolButton;
    btRegExp->setCheckable(true);
    btRegExp->setText(".*");
    //btRegExp->setIcon(QIcon(":/icons/view-filter.svg"));
    connect(btRegExp,&QAbstractButton::toggled,this,&MainWindow::regexToggled);
    hLayout2->addWidget(btRegExp);
    hLayout2->addSpacing(1);
    mainLayout->addLayout(hLayout2);
    mainLayout->addWidget(tableWidget,3);
    wgt->setLayout(mainLayout);

    chartView = new ZoomableChartView();
    chartView->chart()->setTheme(m_chartTheme);
    connect(chartView,&ZoomableChartView::changeLinLogY,this,&MainWindow::setLinLogY);
    connect(chartView,&ZoomableChartView::changeLinLogX,this,&MainWindow::setLinLogX);
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
 * \brief reload previously opened file from disk
 */
void MainWindow::reloadFile()
{
    if(m_fileName.isEmpty()) return;
    const QList<ColumnFilter> store_columnFilters=m_columnFilters;
    const auto store_sweeps=m_sweeps;
    const auto store_plotValues=m_plotValues;
    readFile();
    m_columnFilters=store_columnFilters;
    for(const ColumnFilter &cf:m_columnFilters){
        const int column=cf.column;
        updateColBackground(column,true);
    }
    updateFilteredTable();
    m_sweeps=store_sweeps;
    m_plotValues=store_plotValues;
    updateSweepGUI();
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

    // add new template to recent templates
    m_recentTemplates.removeOne(fileName);
    m_recentTemplates.prepend(fileName);
    populateRecentTemplates();
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
                    found=true;
                }
                continue;
            }
            if(fileName.endsWith(".vcsv")) break; // special treatment for VCSV
            m_columns=splitAtComma(line);
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
                data[1]<<unquote(line);
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

                data[i].append(unquote(elements[i]));
            }
        }
        if(errorOccured){
            return false;
        }
        m_csv=data;
        m_columnType=QVector<ColumnType>(data.size(),COL_UNKNOWN);
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
        m_columnType=QVector<ColumnType>(data.size(),COL_FLOAT); // assuming normal SP-file
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
    QStringList vars=m_sweeps;
    QString xn;
    if(!vars.isEmpty()){
        xn=vars.takeLast();
    }
    int index_x=getIndex(xn);

    bool multiPlot=m_plotValues.size()>1;
    chartView->clear();
    for(const QString &yn:m_plotValues){
        addSeriesToChart(index_x,vars,yn,multiPlot);
    }

    chartView->setTitle("Line chart");
    if(tabWidget->currentIndex()!=1){
        tabWidget->setCurrentIndex(1); // plot tab
    }
    // recreate lin/log
    if(m_logx){
        chartView->setLogX(true);
    }
    if(m_logy){
        chartView->setLogY(true);
    }
    chartView->updateMarker();
}
/*!
 * \brief plot series
 * if x values contain strings, use a bar instead of a line chart
 * \param index_x
 * \param vars
 * \param yn
 * \param multiPlot
 */
void MainWindow::addSeriesToChart(const int index_x,const QStringList &vars,const QString &yn,bool multiPlot){
    bool discretePoints=false; //(m_columnType[index_x]!=COL_INT) && (m_columnType[index_x]!=COL_FLOAT); for now, detection not done generally
    if(discretePoints){
        addBarSeriesToChart(index_x,vars,yn,multiPlot);
    }else{
        addLineSeriesToChart(index_x,vars,yn,multiPlot);
    }
}
/*!
 * \brief add LineSeries To Chart
 * Assume x is number
 * \param index_x column of x (sweep) value
 * \param vars group vars
 * \param yn plot value
 * \param multiPlot if multiple values will be plotted
 */
void MainWindow::addLineSeriesToChart(const int index_x, const QStringList &vars, const QString &yn, bool multiPlot)
{
    int index_y=getIndex(yn);
    if(index_y<0) return;
    bool averaging=m_lineSeriesAveragedAct->isChecked();
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
        QList<QPointF> points=getPoints(index_x,index_y,lit);
        if(averaging){
            points=averagePointSeries(points);
        }
        series->append(points);
        chartView->addSeries(series);
    }
}

QList<QPointF> MainWindow::getPoints(const int index_x,const int index_y,const LoopIteration &lit)
{
    QList<QPointF> series;
    qreal cnt=0;
    for(std::size_t i=0;i<lit.indices.size();++i){
        if(lit.indices[i]){
            bool ok_x,ok_y;
            qreal x;
            if(index_x<0){
                x=cnt;
                cnt+=1;
                ok_x=true;
            }else{
                x=m_csv[index_x].value(i).toDouble(&ok_x);
            }

            qreal y=m_csv[index_y].value(i).toDouble(&ok_y);
            if(ok_x && ok_y){
                QPointF pt(x,y);
                series.append(pt);
            }
        }
    }
    return series;
}
/*!
 * \brief calculate average of y values with same x values
 * Series need to be sorted
 * \param points
 */
QList<QPointF> MainWindow::averagePointSeries(const QList<QPointF> &points)
{
    QList<QPointF> avg;
    int n=0;
    QPointF resultingPoint;
    for(const auto &pt:points){
        if(n==0){
            resultingPoint=pt;
            ++n;
        }else{
            if(pt.x()==resultingPoint.x()){
                resultingPoint.setY(resultingPoint.y()+pt.y());
                ++n;
            }else{
                resultingPoint.setY(resultingPoint.y()/n);
                avg<<resultingPoint;
                n=0;
            }
        }
    }
    return avg;
}

void MainWindow::addBarSeriesToChart(const int index_x, const QStringList &vars, const QString &yn, bool multiPlot)
{
    // TO BE IMPLEMENTED
    return;
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
    bool addSeparator=false;
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
        addSeparator=true;
    }
    if(isPosFloatOnlyData(column)){
        act=new QAction(tr("float -> dB20"), this);
        act->setData(column);
        connect(act,&QAction::triggered,this,&MainWindow::convertFloatDB20);
        menu->addAction(act);
        act=new QAction(tr("float -> dB10"), this);
        act->setData(column);
        connect(act,&QAction::triggered,this,&MainWindow::convertFloatDB10);
        menu->addAction(act);
        addSeparator=true;
    }
    if(isFloatOnlyData(column)){
        act=new QAction(tr("dB20 -> float"), this);
        act->setData(column);
        connect(act,&QAction::triggered,this,&MainWindow::convertDB20Float);
        menu->addAction(act);
        act=new QAction(tr("dB10 -> float"), this);
        act->setData(column);
        connect(act,&QAction::triggered,this,&MainWindow::convertDB10Float);
        menu->addAction(act);
        addSeparator=true;
    }
    if(addSeparator){
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
    act=new QAction(tr("filter"), this);
    act->setData(column);
    connect(act,&QAction::triggered,this,&MainWindow::columnFilter);
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
 * \brief set Y-axis log/lin
 */
void MainWindow::setLinLogY()
{
    m_logy=!m_logy;
    chartView->setLogY(m_logy);
    m_logyAct->setText(m_logy ? tr("Lin Y") : tr("Log Y"));
}
/*!
 * \brief set X-axis log/lin
 */
void MainWindow::setLinLogX()
{
    m_logx=!m_logx;
    chartView->setLogX(m_logx);
    m_logxAct->setText(m_logx ? tr("Lin X") : tr("Log X"));
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
 * \brief use regexp as filter text
 * \param checked
 */
void MainWindow::regexToggled(bool )
{
    // filter columns
    filterTextChanged(leFilterText->text());
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
    bool useRegex=btRegExp->isChecked();
    if(btFilter->isChecked()){
        for(int i=0;i<m_columns.size();++i){
            bool show=false;
            if(useRegex){
                show=m_columns.value(i).contains(QRegularExpression(text));
            }else{
                show=m_columns.value(i).contains(text, Qt::CaseInsensitive);
            }
            if(show){
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
void MainWindow::updateColBackgroundOff(int col){
    QTableWidgetItem *item=tableWidget->horizontalHeaderItem(col);
    item->setBackground(Qt::red);
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
    }
    updateColBackgroundOff(column);
    updateFilteredTable();
}
/*!
 * \brief filter row which meet column condition
 * Condition is provided as simple query (>,<,= &/|)
 */
void MainWindow::columnFilter()
{
    // ask for filter as text
    bool ok;
    QString text=tr("use ><=&|,");

    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    int cfi=getColumnFilter(column);
    if(cfi>=0){
        ColumnFilter &cf=m_columnFilters[cfi];
        text=cf.query;
        if(text.isEmpty())
            text=tr("use ><=&|contains");
    }
    text = QInputDialog::getText(this, tr("Column filter"),
                                             tr("Query:"), QLineEdit::Normal,
                                             text, &ok);
    if (!ok)
        return; // no filter set

    if(cfi>=0){
        ColumnFilter &cf=m_columnFilters[cfi];
        cf.query=text;
    }else{
        ColumnFilter cf;
        cf.column=column;
        cf.query=text;
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
    ColumnType col_type=COL_STRING;
    if(!cf.query.isEmpty()){
        col_type=getDataType(column);
    }
    QStringList &colVals=m_csv[column];
    for(int i=0;i<colVals.size();++i){
        if(m_visibleRows[i]){
            if(cf.query.isEmpty()){
                if(!cf.allowedValues.contains(colVals[i])){
                    m_visibleRows[i]=false;
                }
            }else{
                m_visibleRows[i]=parseQuery(cf.query,colVals[i],col_type);
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
        if(m_columnFilters[cfi].allowedValues.size()==1){
            updateColBackground(column,true);
        }
    }else{
        m_columnFilters[cfi].allowedValues.removeOne(value);
    }
    if(m_columnFilters[cfi].allowedValues.isEmpty()){
        updateColBackgroundOff(column);
    }
    updateFilteredTable();
}
/*!
 * \brief interpret query to check if data is valid or not
 * \param text
 * \param data
 * \return
 */
bool MainWindow::parseQuery(const QString &text, const QString &data,const ColumnType col_type)
{
    QList<Query> queries;
    for(int start=0;start<text.length();){
        int end=text.indexOf('&',start);
        int end_=text.indexOf('|',start);
        bool andOperator=true;
        if(end_>=0 && (end_<end || end<0) ){
            end=end_;
            andOperator=false;
        }
        if(end>=0){
            Query q{text.mid(start,end-start).trimmed(),andOperator};
            queries<<q;
            start=end+1;
            continue;
        }
        Query q{text.mid(start).trimmed(),andOperator};
        queries<<q;
        break;
    }

    bool res=true;
    bool useAnd=true;
    for(auto &qu:queries){
        QString q=qu.query;
        QString reference;
        int operatorType=determineOperator(q,reference);
        if(operatorType<-10) continue; // unknown operator
        bool res_query=true;
        if(operatorType>=10){
            if(operatorType==10){
                // contains ...
                res_query=data.contains(reference);
            }
            if(operatorType==11){
                // !contains ...
                res_query=!data.contains(reference);
            }
            if(operatorType==12){
                // regex ...
                res_query=data.contains(QRegularExpression(reference));
            }
            if(operatorType==13){
                // !regex ...
                res_query=!data.contains(QRegularExpression(reference));
            }
        }else{
            int result;
            if(col_type==COL_STRING){
                result=compare(data,reference);
            }
            if(col_type==COL_FLOAT){
                double number=data.toDouble();
                double ref=reference.toDouble();
                result=compare(number,ref);
            }
            if(col_type==COL_INT){
                bool ok;
                qulonglong number=convertStringToLong(data,ok);
                qulonglong ref=convertStringToLong(reference,ok);
                result=compare(number,ref);
            }
            // determine if compare was correct
            if(result==0){
                if(abs(operatorType)>1){
                    res_query=false; // equale but should be >/<
                }
            }else{
                // result 1 or -1
                if(operatorType!=-3 && (operatorType==0 || (operatorType/result)<0)){
                    res_query=false; // opposite i.e. >/>= but compare less
                }
            }
        }
        if(useAnd){
            res=res&res_query;
        }else{
            res=res|res_query;
        }
        useAnd=qu.connectAnd;
    }
    return res;
}
/*!
 * \brief detremine operator and the corresponding reference
 * e.g. ">=0" -> ">="=2 , "0"
 * \param text
 * \param reference
 * \return opType (2 >=,1 >,0 =,-1 <=, -2 <,-3 !=, -1000 uknown, 10 contains,11 !contains)
 */
int MainWindow::determineOperator(const QString &text, QString &reference)
{
    if(text.startsWith(">=")){
        reference=text.mid(2);
        return 1;
    }
    if(text.startsWith("<=")){
        reference=text.mid(2);
        return -1;
    }
    if(text.startsWith("!=")){
        reference=text.mid(2);
        return -3;
    }
    if(text.startsWith('>')){
        reference=text.mid(1);
        return 2;
    }
    if(text.startsWith('<')){
        reference=text.mid(1);
        return -2;
    }
    if(text.startsWith('=')){
        reference=text.mid(1);
        return 0;
    }
    if(text.startsWith("contains ")){
        reference=text.mid(9);
        return 10;
    }
    if(text.startsWith("!contains ")){
        reference=text.mid(10);
        return 11;
    }
    if(text.startsWith("regex ")){
        reference=text.mid(6);
        return 12;
    }
    if(text.startsWith("!regex ")){
        reference=text.mid(7);
        return 13;
    }
    return -1000; // unknown
}
/*!
 * \brief called when plot style is changed
 * replot if plot is visible
 */
void MainWindow::plotStyleChanged()
{
    if(tabWidget->currentIndex()==1){
        plotSelected();
    }
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
        QString txt;
        int row=-1;
        for(auto item:items){
            if(txt.isEmpty()){
                txt=item->text();
            }else{
                if(row!=item->row()){
                    txt.append("\n");
                    txt.append(item->text());
                }else{
                    txt.append("\t");
                    txt.append(item->text());
                }
            }
            row=item->row();
        }

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
 * \brief check what data type one column consists of
 * String, int or float.
 * Result is cached as it does not change.
 * \param column
 * \return
 */
MainWindow::ColumnType MainWindow::getDataType(int column)
{
    bool ok=true;
    if(m_columnType[column]==COL_UNKNOWN){
        ColumnType result=COL_INT; // int -> float -> string
        QRegularExpression reFloat("^\\s*[+-]?\\d+(\\.\\d+)?(e[+-]?\\d+)?$");
        QRegularExpression reInt("^[+-]?\\d+$");
        for(qsizetype row=0;row<m_csv[column].count();++row){
            QString cell=m_csv[column].value(row).simplified().toLower();
            if(cell.startsWith("0x")){
                cell.mid(2).toULongLong(&ok);
                if(ok) continue;
                result=COL_STRING;
                break;
            }
            if(cell.startsWith("0b")){
                cell.mid(2).toULongLong(&ok);
                if(ok) continue;
                result=COL_STRING;
                break;
            }
            if(result==COL_INT){
                QRegularExpressionMatch match = reInt.match(cell);
                ok = match.hasMatch();
                if(ok) continue;
                result=COL_FLOAT;
            }
            QRegularExpressionMatch match = reFloat.match(cell);
            ok = match.hasMatch();
            if(ok) continue;
            result=COL_STRING;
            break;
        }
        m_columnType[column]=result;
    }
    return m_columnType[column];
}
/*!
 * \brief check if data consists only of ints
 * \param column
 * \return
 */
bool MainWindow::isIntOnlyData(int column)
{
    ColumnType colType=getDataType(column);
    return colType==COL_INT;
}
/*!
 * \brief check if data consists only of floats
 * Determined by numbers containing [-]\d.\d[E[-]\d]
 * \param column
 * \return
 */
bool MainWindow::isFloatOnlyData(int column)
{
    ColumnType colType=getDataType(column);
    return colType==COL_FLOAT || colType==COL_INT;
}
/*!
 * \brief check if data consists only of positive floats
 * \param column
 * \return
 */
bool MainWindow::isPosFloatOnlyData(int column)
{
    bool ok=true;
    QRegularExpression re("^\\d+(\\.\\d+)?(E[-]?\\d+)?");
    for(qsizetype row=0;row<m_csv[column].count();++row){
        QString cell=m_csv[column].value(row);
        QRegularExpressionMatch match = re.match(cell);
        ok = match.hasMatch();
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
 * \brief convert column in table as float from dB20
 */
void MainWindow::convertDB20Float()
{
    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    bool ok;
    for(qsizetype row=0;row<m_csv[column].count();++row){
        QString cell=m_csv[column].value(row);
        double value=cell.toDouble(&ok);
        if(!ok) break;
        value=pow(10,value/20);
        QTableWidgetItem *item=tableWidget->item(row,column);
        cell=QString("%1").arg(value);
        m_csv[column][row]=cell;
        item->setText(cell);
    }
    tableWidget->resizeColumnToContents(column);
}
/*!
 * \brief convert column in table as float from dB10
 */
void MainWindow::convertDB10Float()
{
    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    bool ok;
    for(qsizetype row=0;row<m_csv[column].count();++row){
        QString cell=m_csv[column].value(row);
        double value=cell.toDouble(&ok);
        if(!ok) break;
        value=pow(10,value/10);
        QTableWidgetItem *item=tableWidget->item(row,column);
        cell=QString("%1").arg(value);
        m_csv[column][row]=cell;
        item->setText(cell);
    }
    tableWidget->resizeColumnToContents(column);
}
/*!
 * \brief convert column in table as dB20 from pos. float
 */
void MainWindow::convertFloatDB20()
{
    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    bool ok;
    for(qsizetype row=0;row<m_csv[column].count();++row){
        QString cell=m_csv[column].value(row);
        double value=cell.toDouble(&ok);
        if(!ok) break;
        value=log10(value)*20;
        QTableWidgetItem *item=tableWidget->item(row,column);
        cell=QString("%1").arg(value);
        m_csv[column][row]=cell;
        item->setText(cell);
    }
    tableWidget->resizeColumnToContents(column);
}
/*!
 * \brief convert column in table as dB10 from pos. float
 */
void MainWindow::convertFloatDB10()
{
    QAction *act=qobject_cast<QAction*>(sender());
    int column=act->data().toInt();
    bool ok;
    for(qsizetype row=0;row<m_csv[column].count();++row){
        QString cell=m_csv[column].value(row);
        double value=cell.toDouble(&ok);
        if(!ok) break;
        value=log10(value)*10;
        QTableWidgetItem *item=tableWidget->item(row,column);
        cell=QString("%1").arg(value);
        m_csv[column][row]=cell;
        item->setText(cell);
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
qlonglong MainWindow::convertStringToLong(QString text, bool &ok)
{
    qlonglong value;
    if(text.startsWith("0b")){
        value=text.mid(2).toLongLong(&ok,2);
    }else{
        if(text.startsWith("0x")){
            value=text.mid(2).toLongLong(&ok,16);
        }else{
            value=text.toLongLong(&ok);
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
 * \brief remove quotes (") around text
 * \param text
 * \return text without quotes
 */
QString MainWindow::unquote(const QString &text) const
{
    QString result=text;
    if(text.startsWith("\"") and text.endsWith("\"")){
        result=text.mid(1,text.length()-2);
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
avg plot
histogram
fix marker position after replot
improve display a/b numbers (eng format, covering)
Unit tests
chart style in config
edit trace name/title
*/

