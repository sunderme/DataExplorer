#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QChartView>

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
    void plotSelected();

private:
    QMenu *fileMenu;
    QMenu *plotMenu;

    QAction *openAct;
    QAction *exitAct;
    QAction *plotAct;

    QTableWidget *tableWidget;
    QChartView *chartView;

    QString fileName;

    QStringList columns;
    QList<QStringList> csv;
};
#endif // MAINWINDOW_H
