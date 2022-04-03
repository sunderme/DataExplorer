#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QChartView>
#include <QListWidget>
#include "zoomablechartview.h"

struct loopIteration{
    QString value;
    QList<int> indices;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void setupMenus();
    void setupGUI();
    void openFile();
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
    void test();
    int getIndex(const QString &name);
    QStringList getUniqueValues(const QString &var,const QList<int> &indices);
    QList<int> filterIndices(const QString &var,const QString &value,const QList<int> &providedIndices);

    QList<loopIteration> groupBy(QStringList sweepVar,QList<int> providedIndices=QList<int>() );

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

    QString fileName;

    QStringList columns;
    QList<QStringList> csv;
    QStringList sweeps,plotValues;
};
#endif // MAINWINDOW_H
