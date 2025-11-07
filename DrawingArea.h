//
// Created by muhirwa gabo Oreste on 06/11/2025.
//

#ifndef CURVES3D_DRAWINGAREA_H
#define CURVES3D_DRAWINGAREA_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QVector3D>
#include <QVector>
#include <QMatrix4x4>
#include <QString>

class DrawingArea : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit DrawingArea(QWidget *parent = nullptr);

public slots:
    void setCurrentCurveType(const QString &type);
    void updateCurve(const QList<QVector3D>& points);

    signals:
        void controlPointsMoved(const QList<QVector3D>& newPoints);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QString m_currentCurveType;
    QList<QVector3D> m_controlPoints;
    QVector<QVector3D> m_calculatedCurvePoints;

    // --- 3D Camera/View State ---
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;

    QPoint m_lastMousePos;
    qreal m_rotationX = 3.0;
    qreal m_rotationY = -45.0;
    qreal m_zoomDistance = -300.0;

    // --- Dragging State Variables ---
    int m_draggingPointIndex = -1;

    // --- Modern OpenGL Members ---
    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_curveVbo;
    QOpenGLBuffer m_pointsVbo;

    // Shader Locations
    int m_posAttr;
    int m_matrixUniform;
    int m_colorUniform;

    // --- Private Methods ---
    void calculateAndStoreCurve();
    void initializeShaders();
    void setupVBOs();
    void drawAxes();
    void drawCurve();
    void drawPoints();
    void drawGrid();
};




#endif //CURVES3D_DRAWINGAREA_H