//
// Created by muhirwa gabo Oreste on 06/11/2025.
//

#include "MainWindow.h"
#include "DrawingArea.h"
#include "PointModel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QDoubleValidator>
#include <QScrollBar>
#include <cstdlib> // For qrand in initialization

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("3D Curves & Algorithms Viewer");
    resize(1000, 700);

    m_pointModel = new PointModel(this);
    drawingArea = new DrawingArea;

    connect(m_pointModel, &PointModel::pointsChanged, drawingArea, &DrawingArea::updateCurve);
    connect(drawingArea, &DrawingArea::controlPointsMoved, this, &MainWindow::syncUIFromDrawingArea);

    // 1. Set the central widget (the Drawing Area takes the full space)
    setCentralWidget(drawingArea);

    // 2. Create the Dock Widget (the Control Panel)
    QDockWidget *controlDock = createControlPanel();

    // Make the Dock Widget collapsible and add it to the left side
    addDockWidget(Qt::LeftDockWidgetArea, controlDock); // <-- USE QDockWidget

    // Ensure the Central Widget (DrawingArea) is not covered by the dock when maximized
    drawingArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    handleCurveSelection(curveDropdown->currentIndex());
    // Initial data setup
    for (int i = 0; i < 4; ++i) { addPointEntry(); }
    updateModelFromUI();
}

MainWindow::~MainWindow() {}

// --- Setup Methods ---

QDockWidget* MainWindow::createControlPanel()
{
    // ... (Control panel setup)
    QWidget *panelContent = new QWidget;
    panelContent->setMinimumWidth(280); // Set a minimum width for the inner content

    QVBoxLayout *vLayout = new QVBoxLayout(panelContent);
    vLayout->setContentsMargins(5, 5, 5, 5);


    //QWidget *panel = new QWidget;
    //panel->setFixedWidth(300);

    //QVBoxLayout *vLayout = new QVBoxLayout(panel);
    vLayout->setContentsMargins(5, 5, 5, 5);

    vLayout->addWidget(new QLabel("Curve Type:"));
    curveDropdown = new QComboBox;
    curveDropdown->addItem("Bézier Curve (De Casteljau)");
    curveDropdown->addItem("Hermite Curve (Matricielle)");
    curveDropdown->addItem("B-Spline Curve");
    vLayout->addWidget(curveDropdown);
    connect(curveDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::handleCurveSelection);

    vLayout->addSpacing(15);
    vLayout->addWidget(new QLabel("Control Points (X, Y, Z Coords):"));
    vLayout->addSpacing(5);

    // Header for coordinates
    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addSpacing(25);
    headerLayout->addWidget(new QLabel("X"));
    headerLayout->addWidget(new QLabel("Y"));
    headerLayout->addWidget(new QLabel("Z"));
    headerLayout->addStretch(1);
    vLayout->addLayout(headerLayout);

    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    pointsContainer = new QWidget;
    pointsLayout = new QVBoxLayout(pointsContainer);
    pointsLayout->setAlignment(Qt::AlignTop);
    pointsLayout->setSpacing(5);
    pointsLayout->setContentsMargins(0, 0, 0, 0);
    pointsLayout->addStretch(1);

    scrollArea->setWidget(pointsContainer);
    vLayout->addWidget(scrollArea);

    addPointButton = new QPushButton("Add Point (+)");
    connect(addPointButton, &QPushButton::clicked, this, &MainWindow::addPointEntry);
    vLayout->addWidget(addPointButton);

    // --- End existing control setup ---

    // Create the QDockWidget and set the content
    QDockWidget *dock = new QDockWidget("Control Panel", this);
    dock->setWidget(panelContent);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);

    return dock;
}

QWidget* MainWindow::createPointEntryWidget()
{
    QWidget *widget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    hLayout->setContentsMargins(0, 0, 0, 0);

    int count = pointRows.size();

    QLabel *label = new QLabel(QString("P%1:").arg(count));
    label->setFixedWidth(25);
    hLayout->addWidget(label);

    QDoubleValidator *validator = new QDoubleValidator(-999.0, 999.0, 1, this);

    // X Field
    QLineEdit *xField = new QLineEdit;
    xField->setPlaceholderText("X");
    xField->setFixedWidth(60);
    xField->setValidator(validator);
    xField->setText(QString::number(-50.0 + count * 30));
    hLayout->addWidget(xField);

    // Y Field
    QLineEdit *yField = new QLineEdit;
    yField->setPlaceholderText("Y");
    yField->setFixedWidth(60);
    yField->setValidator(validator);
    yField->setText(QString::number(rand() % 40 - 20)); // Random Y for spread
    hLayout->addWidget(yField);

    // Z Field (NEW)
    QLineEdit *zField = new QLineEdit;
    zField->setPlaceholderText("Z");
    zField->setFixedWidth(60);
    zField->setValidator(validator);
    zField->setText(QString::number(rand() % 40 - 20)); // Random Z for depth
    hLayout->addWidget(zField);

    // Remove Button
    QPushButton *removeButton = new QPushButton("-");
    removeButton->setFixedSize(20, 20);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removePointEntry);
    hLayout->addWidget(removeButton);

    hLayout->addStretch(1);

    xFields.append(xField);
    yFields.append(yField);
    zFields.append(zField);
    connectEntryFields(xField, yField, zField);
    pointRows.append(widget);

    return widget;
}

void MainWindow::connectEntryFields(QLineEdit *xField, QLineEdit *yField, QLineEdit *zField)
{
    connect(xField, &QLineEdit::textEdited, this, &MainWindow::updateModelFromUI);
    connect(yField, &QLineEdit::textEdited, this, &MainWindow::updateModelFromUI);
    connect(zField, &QLineEdit::textEdited, this, &MainWindow::updateModelFromUI);
}

void MainWindow::addPointEntry()
{
    QWidget *row = createPointEntryWidget();
    // Insert the new point entry before the stretch added earlier
    pointsLayout->insertWidget(pointRows.size() - 1, row);

    // Renumbering P# labels is omitted for simplicity, but is recommended.
}

void MainWindow::removePointEntry()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int index = -1;
    for (int i = 0; i < pointRows.size(); ++i) {
        if (pointRows[i] == button->parentWidget()) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        QWidget *rowWidget = pointRows.takeAt(index);
        pointsLayout->removeWidget(rowWidget);
        delete rowWidget;

        // Remove associated QLineEdits
        xFields.removeAt(index);
        yFields.removeAt(index);
        zFields.removeAt(index);

        updateModelFromUI();
    }
}


void MainWindow::updateModelFromUI()
{
    QList<QVector3D> newPoints;

    for (int i = 0; i < xFields.size(); ++i) {
        bool xOk, yOk, zOk;
        qreal x = xFields[i]->text().toDouble(&xOk);
        qreal y = yFields[i]->text().toDouble(&yOk);
        qreal z = zFields[i]->text().toDouble(&zOk);

        if (xOk && yOk && zOk) {
            newPoints.append(QVector3D(x, y, z));
        }
    }

    m_pointModel->setControlPoints(newPoints);
}

void MainWindow::syncUIFromDrawingArea(const QList<QVector3D>& newPoints)
{
    m_pointModel->setControlPoints(newPoints);

    for (int i = 0; i < newPoints.size(); ++i) {
        if (i < xFields.size()) {
            xFields[i]->blockSignals(true);
            yFields[i]->blockSignals(true);
            zFields[i]->blockSignals(true);

            xFields[i]->setText(QString::number(newPoints[i].x(), 'f', 1));
            yFields[i]->setText(QString::number(newPoints[i].y(), 'f', 1));
            zFields[i]->setText(QString::number(newPoints[i].z(), 'f', 1));

            xFields[i]->blockSignals(false);
            yFields[i]->blockSignals(false);
            zFields[i]->blockSignals(false);
        }
    }
}

void MainWindow::handleCurveSelection(int index)
{
    QString type = curveDropdown->itemText(index);
    drawingArea->setCurrentCurveType(type);
    updateModelFromUI();
}