#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QChartView>
#include <QListWidget>
#include <QLineEdit>
#include "zoomablechartview.h"

struct LoopIteration{
    QString value;
    std::vector<bool> indices;
};

struct ColumnFilter{
    int column;
    QStringList allowedValues;
    QString query;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int argc, char *argv[],QWidget *parent = nullptr);
    ~MainWindow();

protected:
    enum ColumnType {COL_UNKNOWN,COL_STRING,COL_FLOAT,COL_INT};
    void setupMenus();
    void setupGUI();
    void closeEvent(QCloseEvent *event);
    void openFile();
    void openRecentFile();
    void readFile();
    void openTemplate();
    void openRecentTemplate();
    void saveTemplate();
    void readTemplate(const QString &fileName);
    bool readInCSV(const QString &fileName);
    bool readInSNP(const QString &fileName, int nrPorts=2);
    void buildTable();
    void updateSweepGUI();
    void updateSweeps(bool filterChecked=true);
    void plotSelected();
    void tabChanged(int index);
    void headerMenuRequested(QPoint pt);
    void addSweepVar();
    void deleteVar();
    void addPlotVar();
    void zoomAreaMode();
    void panMode();
    void zoomX();
    void zoomY();
    void zoomIn();
    void zoomOut();
    void zoomInX();
    void zoomOutX();
    void zoomInY();
    void zoomOutY();
    void zoomReset();
    void setLinLogY();
    void setLinLogX();
    void about();
    void addVerticalMarker();
    void addHorizontalMarker();
    void addMarkerA();
    void addMarkerB();
    void populateRecentFiles();
    void populateRecentTemplates();
    void filterToggled(bool checked);
    void regexToggled(bool checked);
    void filterCheckedToggled(bool checked);
    void filterPlotToggled(bool checked);
    void filterTextChanged(const QString &text);
    void columnShowAll();
    void columnShowNone();
    void columnFilter();
    void updateFilteredTable();
    void updateColBackground(int col,bool filtered=false);
    void filterRowsForColumnValues(ColumnFilter cf);
    void filterElementChanged(bool checked);
    bool parseQuery(const QString &text,const QString &data,const ColumnType col_type=COL_STRING);
    int determineOperator(const QString &text,QString &reference);
    void test();
    void copyCell();
    void copyHeader();
    void copyPlotToClipboard();
    void exportPlotImage();
    ColumnType getDataType(int column);
    bool isIntOnlyData(int column);
    bool isFloatOnlyData(int column);
    bool isPosFloatOnlyData(int column);
    int getIntegerWidth(int column);
    void showDecimal();
    void showBinary();
    void showHex();
    void convertDB20Float();
    void convertDB10Float();
    void convertFloatDB20();
    void convertFloatDB10();
    qlonglong convertStringToLong(QString text,bool &ok);
    int getIndex(const QString &name);
    bool hasColumnFilter(int column) const;
    int getColumnFilter(int column) const;
    QStringList getUniqueValues(const QString &var,const std::vector<bool> &indices);
    std::vector<bool> filterIndices(const QString &var,const QString &value,const std::vector<bool> &providedIndices);
    QStringList splitAtComma(const QString &line) const;
    QString unquote(const QString &text) const;

    QList<LoopIteration> groupBy(QStringList sweepVar,std::vector<bool> providedIndices=std::vector<bool>() );

private:
    QMenu *m_fileMenu;
    QMenu *m_plotMenu;
    QMenu *m_editMenu;
    QMenu *m_recentFilesMenu,*m_recentTemplatesMenu;

    QAction *m_openAct;
    QAction *m_exitAct;
    QAction *m_plotAct;
    QAction *m_logxAct,*m_logyAct;

    QTabWidget *tabWidget;
    QTableWidget *tableWidget;
    ZoomableChartView *chartView;

    QListWidget *lstSweeps;
    QListWidget *lstData;

    QToolButton *btFilter,*btFilterPlot,*btFilterChecked,*btRegExp;
    QLineEdit *leFilterText;

    QString m_fileName;

    QStringList m_recentFiles,m_recentTemplates;
    QChart::ChartTheme m_chartTheme;

    QStringList m_columns;
    QVector<QStringList> m_csv;
    QVector<ColumnType> m_columnType;
    QStringList m_sweeps,m_plotValues;

    QList<ColumnFilter> m_columnFilters;
    std::vector<bool>m_visibleRows;
    bool m_logx,m_logy;
};
#endif // MAINWINDOW_H
