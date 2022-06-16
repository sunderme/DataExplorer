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
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int argc, char *argv[],QWidget *parent = nullptr);
    ~MainWindow();

protected:
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
    void about();
    void addVerticalMarker();
    void addHorizontalMarker();
    void addMarkerA();
    void addMarkerB();
    void populateRecentFiles();
    void populateRecentTemplates();
    void filterToggled(bool checked);
    void filterCheckedToggled(bool checked);
    void filterPlotToggled(bool checked);
    void filterTextChanged(const QString &text);
    void columnShowAll();
    void columnShowNone();
    void updateFilteredTable();
    void updateColBackground(int col,bool filtered=false);
    void filterRowsForColumnValues(ColumnFilter cf);
    void filterElementChanged(bool checked);
    void test();
    void copyCell();
    void copyHeader();
    void copyPlotToClipboard();
    void exportPlotImage();
    bool isIntOnlyData(int column);
    int getIntegerWidth(int column);
    void showDecimal();
    void showBinary();
    void showHex();
    qlonglong convertStringToLong(QString text,bool &ok);
    int getIndex(const QString &name);
    bool hasColumnFilter(int column) const;
    int getColumnFilter(int column) const;
    QStringList getUniqueValues(const QString &var,const std::vector<bool> &indices);
    std::vector<bool> filterIndices(const QString &var,const QString &value,const std::vector<bool> &providedIndices);

    QList<LoopIteration> groupBy(QStringList sweepVar,std::vector<bool> providedIndices=std::vector<bool>() );

private:
    QMenu *m_fileMenu;
    QMenu *m_plotMenu;
    QMenu *m_editMenu;
    QMenu *m_recentFilesMenu,*m_recentTemplatesMenu;

    QAction *m_openAct;
    QAction *m_exitAct;
    QAction *m_plotAct;

    QTabWidget *tabWidget;
    QTableWidget *tableWidget;
    ZoomableChartView *chartView;

    QListWidget *lstSweeps;
    QListWidget *lstData;

    QToolButton *btFilter,*btFilterPlot,*btFilterChecked;
    QLineEdit *leFilterText;

    QString m_fileName;

    QStringList m_recentFiles,m_recentTemplates;
    QChart::ChartTheme m_chartTheme;

    QStringList m_columns;
    QVector<QStringList> m_csv;
    QStringList m_sweeps,m_plotValues;

    QList<ColumnFilter> m_columnFilters;
    std::vector<bool>m_visibleRows;
};
#endif // MAINWINDOW_H
