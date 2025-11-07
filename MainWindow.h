//
// Created by muhirwa gabo Oreste on 06/11/2025.
//

#ifndef CURVES3D_MAINWINDOW_H
#define CURVES3D_MAINWINDOW_H


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QVector3D>
#include <QDockWidget>

// Forward Declarations
class DrawingArea;
class PointModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addPointEntry();
    void removePointEntry();
    void updateModelFromUI();
    void handleCurveSelection(int index);
    void syncUIFromDrawingArea(const QList<QVector3D>& newPoints);

private:
    PointModel *m_pointModel;
    DrawingArea *drawingArea;

    QComboBox *curveDropdown;
    QPushButton *addPointButton;
    QScrollArea *scrollArea;
    QWidget *pointsContainer;
    QVBoxLayout *pointsLayout;

    QList<QLineEdit*> xFields;
    QList<QLineEdit*> yFields;
    QList<QLineEdit*> zFields; // NEW
    QList<QWidget*> pointRows;

    QDockWidget* createControlPanel();
    QWidget* createPointEntryWidget();
    void connectEntryFields(QLineEdit *xField, QLineEdit *yField, QLineEdit *zField);
};

#endif // MAINWINDOW_H


#endif //CURVES3D_MAINWINDOW_H