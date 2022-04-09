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
    void openFile();
    void readFile();
    void openTemplate();
    void saveTemplate();
    void readInCSV(const QString &fileName);
    void buildTable();
    void updateSweepGUI();
    void updateSweeps();
    void plotSelected();
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
    void filterToggled(bool checked);
    void filterTextChanged(const QString &text);
    void columnShowAll();
    void columnShowNone();
    void updateFilteredTable();
    void filterRowsForColumnValues(ColumnFilter cf);
    void filterElementChanged(bool checked);
    void legendMarkerClicked();
    void legendMarkerHovered(bool hover);
    void setSeriesVisible(QAbstractSeries *series, bool visible = true);
    void seriesAdded(QAbstractSeries *series);
    void seriesRemoved(QAbstractSeries *series);
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

    QAction *openAct;
    QAction *exitAct;
    QAction *plotAct;

    QTableWidget *tableWidget;
    ZoomableChartView *chartView;

    QListWidget *lstSweeps;
    QListWidget *lstData;

    QToolButton *btFilter;
    QLineEdit *leFilterText;

    QString fileName;

    QStringList columns;
    QList<QStringList> csv;
    QStringList sweeps,plotValues;

    QList<ColumnFilter> columnFilters;
    std::vector<bool>visibleRows;
};
#endif // MAINWINDOW_H
