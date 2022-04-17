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
    void saveTemplate();
    void readInCSV(const QString &fileName);
    void buildTable();
    void updateSweepGUI();
    void updateSweeps();
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
    void zoomReset();
    void addVerticalMarker();
    void populateRecentFiles();
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
    int getIndex(const QString &name);
    bool hasColumnFilter(int column) const;
    int getColumnFilter(int column) const;
    QStringList getUniqueValues(const QString &var,const std::vector<bool> &indices);
    std::vector<bool> filterIndices(const QString &var,const QString &value,const std::vector<bool> &providedIndices);

    QList<LoopIteration> groupBy(QStringList sweepVar,std::vector<bool> providedIndices=std::vector<bool>() );

private:
    QMenu *fileMenu;
    QMenu *plotMenu;
    QMenu *recentFilesMenu;

    QAction *openAct;
    QAction *exitAct;
    QAction *plotAct;

    QTabWidget *tabWidget;
    QTableWidget *tableWidget;
    ZoomableChartView *chartView;

    QListWidget *lstSweeps;
    QListWidget *lstData;

    QToolButton *btFilter,*btFilterPlot,*btFilterChecked;
    QLineEdit *leFilterText;

    QString fileName;

    QStringList recentFiles;

    QStringList columns;
    QVector<QStringList> csv;
    QStringList sweeps,plotValues;

    QList<ColumnFilter> columnFilters;
    std::vector<bool>visibleRows;
};
#endif // MAINWINDOW_H
