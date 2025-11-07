//
// Created by muhirwa gabo Oreste on 06/11/2025.
//

#include "DrawingArea.h"
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QtMath>
#include <QColor>

#include "CurveCalculator.h"
#include <QMouseEvent>
#include <QOpenGLVertexArrayObject>
#include <QScreen>
#include <QVector4D>

// --- Shaders ---
const char *vertexShaderSource =
    "attribute vec3 position;\n"
    "uniform mat4 matrix;\n"
    "void main() {\n"
    "    gl_Position = matrix * vec4(position, 1.0);\n"
    "}\n";

const char *fragmentShaderSource =
    "uniform vec4 color;\n"
    "void main() {\n"
    "    gl_FragColor = color;\n"
    "}\n";

// --- Class Implementation ---

DrawingArea::DrawingArea(QWidget *parent)
    : QOpenGLWidget(parent), m_currentCurveType("Bézier Curve (De Casteljau)")
{
    setMouseTracking(true);
}

void DrawingArea::setCurrentCurveType(const QString &type)
{
    if (m_currentCurveType != type) {
        m_currentCurveType = type;
        calculateAndStoreCurve();
        update();
    }
}

void DrawingArea::updateCurve(const QList<QVector3D>& points)
{
    m_controlPoints = points;
    calculateAndStoreCurve();
    update();
}

void DrawingArea::calculateAndStoreCurve()
{
    m_calculatedCurvePoints.clear();
    const QList<QVector3D>& points = m_controlPoints;

    if (points.isEmpty()) return;

    if (m_currentCurveType == "Bézier Curve (De Casteljau)") {
        m_calculatedCurvePoints = CurveCalculator::calculateBezier_DeCasteljau(points);
    }
    else if (m_currentCurveType == "B-Spline Curve") {
        m_calculatedCurvePoints = CurveCalculator::calculateBSpline(points, 3);
    }
    else if (m_currentCurveType == "Hermite Curve (Matricielle)") {
        if (points.size() < 2) {
            m_calculatedCurvePoints = points.toVector();
            return;
        }

        // Apply endpoint padding for Catmull-Rom to start/end at P0/PN
        QList<QVector3D> tempPoints = points;
        tempPoints.prepend(points.first());
        tempPoints.append(points.last());

        if (tempPoints.size() < 4) {
             m_calculatedCurvePoints = points.toVector();
             return;
        }

        for (int i = 1; i < tempPoints.size() - 2; ++i) {
            QVector<QVector3D> segment = CurveCalculator::calculateCatmullRomSegment(
                tempPoints[i-1], tempPoints[i], tempPoints[i+1], tempPoints[i+2]
            );

            if (!m_calculatedCurvePoints.isEmpty() && i != 1) {
                segment.removeFirst();
            }
            m_calculatedCurvePoints.append(segment);
        }
    }
    // VBOs must be updated after calculation
    setupVBOs();
}

// --- Shader and VBO Management ---

void DrawingArea::initializeShaders()
{
    if (!m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) close();
    if (!m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) close();
    if (!m_program.link()) close();

    m_posAttr = m_program.attributeLocation("position");
    m_matrixUniform = m_program.uniformLocation("matrix");
    m_colorUniform = m_program.uniformLocation("color");
}

void DrawingArea::setupVBOs()
{
    // Set up Curve VBO
    if (m_curveVbo.isCreated()) m_curveVbo.destroy();
    if (m_calculatedCurvePoints.size() > 1) {
        m_curveVbo.create();
        m_curveVbo.bind();
        m_curveVbo.allocate(m_calculatedCurvePoints.constData(), m_calculatedCurvePoints.size() * sizeof(QVector3D));
        m_curveVbo.release();
    }

    // Set up Points VBO (for control points)
    if (m_pointsVbo.isCreated()) m_pointsVbo.destroy();
    if (!m_controlPoints.isEmpty()) {
        m_pointsVbo.create();
        m_pointsVbo.bind();
        QVector<QVector3D> pointVector = m_controlPoints.toVector();
        m_pointsVbo.allocate(pointVector.constData(), pointVector.size() * sizeof(QVector3D));
        m_pointsVbo.release();
    }
}

// --- OpenGL Overrides ---

void DrawingArea::initializeGL()
{
    initializeOpenGLFunctions();
    initializeShaders();

    //glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
}

void DrawingArea::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, (float)w / (float)h, 0.1f, 1000.0f);
}

void DrawingArea::drawAxes()
{
    QVector<QVector3D> axesData = {
        QVector3D(0.0f, 0.0f, 0.0f), QVector3D(30.0f, 0.0f, 0.0f), // X (Red)
        QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 30.0f, 0.0f), // Y (Green)
        QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 30.0f)  // Z (Blue)
    };

    QOpenGLBuffer axesVbo(QOpenGLBuffer::VertexBuffer);
    axesVbo.create();
    axesVbo.bind();
    axesVbo.allocate(axesData.constData(), axesData.size() * sizeof(QVector3D));

    m_program.bind();
    QMatrix4x4 combined = m_projection * m_view;
    m_program.setUniformValue(m_matrixUniform, combined);

    m_program.enableAttributeArray(m_posAttr);
    m_program.setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 3, 0);

    glLineWidth(2.0f);

    // X-Axis (Red)
    m_program.setUniformValue(m_colorUniform, QVector4D(1.0f, 0.0f, 0.0f, 1.0f));
    glDrawArrays(GL_LINES, 0, 2);

    // Y-Axis (Green)
    m_program.setUniformValue(m_colorUniform, QVector4D(0.0f, 1.0f, 0.0f, 1.0f));
    glDrawArrays(GL_LINES, 2, 2);

    // Z-Axis (Blue)
    m_program.setUniformValue(m_colorUniform, QVector4D(0.0f, 0.0f, 1.0f, 1.0f));
    glDrawArrays(GL_LINES, 4, 2);

    m_program.disableAttributeArray(m_posAttr);
    axesVbo.release();
    axesVbo.destroy();
    m_program.release();
}

void DrawingArea::drawCurve()
{
    if (m_curveVbo.isCreated() && m_calculatedCurvePoints.size() > 1) {

        m_program.bind();
        QMatrix4x4 combined = m_projection * m_view;
        m_program.setUniformValue(m_matrixUniform, combined);

        // Draw Control Polygon (Gray Line)
        m_pointsVbo.bind();
        m_program.enableAttributeArray(m_posAttr);
        m_program.setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 3, 0);

        m_program.setUniformValue(m_colorUniform, QVector4D(0.6f, 0.6f, 0.6f, 1.0f));
        glLineWidth(1.0f);
        glDrawArrays(GL_LINE_STRIP, 0, m_controlPoints.size());
        m_pointsVbo.release();

        // Draw Calculated Curve (Blue Line)
        m_curveVbo.bind();
        m_program.enableAttributeArray(m_posAttr);
        m_program.setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 3, 0);

        m_program.setUniformValue(m_colorUniform, QVector4D(0.0f, 0.0f, 1.0f, 1.0f));
        glLineWidth(3.0f);
        glDrawArrays(GL_LINE_STRIP, 0, m_calculatedCurvePoints.size());

        m_program.disableAttributeArray(m_posAttr);
        m_curveVbo.release();
        m_program.release();
    }
}

void DrawingArea::drawPoints()
{
    if (m_pointsVbo.isCreated() && !m_controlPoints.isEmpty()) {
        m_program.bind();
        QMatrix4x4 combined = m_projection * m_view;
        m_program.setUniformValue(m_matrixUniform, combined);

        m_pointsVbo.bind();
        m_program.enableAttributeArray(m_posAttr);
        m_program.setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 3, 0);

        // --- Ensure Point Size is Set for Visibility ---
        glPointSize(10.0f); // <-- This sets the size of the dots

        // Draw each point individually to set color based on dragging state
        for (int i = 0; i < m_controlPoints.size(); ++i) {
            if (i == m_draggingPointIndex) {
                // Orange for the currently dragged point
                m_program.setUniformValue(m_colorUniform, QVector4D(1.0f, 0.5f, 0.0f, 1.0f));
            } else {
                // Red for all other control points
                m_program.setUniformValue(m_colorUniform, QVector4D(1.0f, 0.0f, 0.0f, 1.0f));
            }
            glDrawArrays(GL_POINTS, i, 1);
        }

        m_program.disableAttributeArray(m_posAttr);
        m_pointsVbo.release();
        m_program.release();
    }
}

void DrawingArea::paintGL()
{
    // 1. Update Camera
    m_view.setToIdentity();
    m_view.translate(0.0, 0.0, m_zoomDistance);
    m_view.rotate(m_rotationX, 1, 0, 0);
    m_view.rotate(m_rotationY, 0, 1, 0);

    // 2. Clear Scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    setupVBOs();

    // 3. Draw Grid (Floor) <-- NEW CALL
    drawGrid();

    // 4. Draw Elements
    drawAxes();
    drawCurve();
    drawPoints();
}

// --- Mouse Events for Camera Control and Dragging ---

void DrawingArea::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->pos();

    if (event->button() == Qt::LeftButton) {
        // Simplified 3D Point Selection (2D hit test on projected 3D points)
        QMatrix4x4 combined = m_projection * m_view;

        for (int i = 0; i < m_controlPoints.size(); ++i) {
            QVector4D p4D(m_controlPoints[i], 1.0f);
            QVector4D projected = combined * p4D;

            if (projected.w() <= 0) continue; // Skip points behind the camera

            QPointF projected2D(projected.x() / projected.w(), projected.y() / projected.w());

            QPointF pixelPos( (projected2D.x() + 1.0) * width() / 2.0,
                              (1.0 - projected2D.y()) * height() / 2.0);

            QPointF diff = pixelPos - event->pos();
            qreal distance = qSqrt(diff.x() * diff.x() + diff.y() * diff.y());

            const qreal HIT_RADIUS = 10.0;
            if (distance <= HIT_RADIUS) {
                m_draggingPointIndex = i;
                update();
                return;
            }
        }
    }
}

void DrawingArea::mouseMoveEvent(QMouseEvent *event)
{
    qreal dx = event->position().x() - m_lastMousePos.x();
    qreal dy = event->position().y() - m_lastMousePos.y();

    if (m_draggingPointIndex != -1) {
        // Simplified 3D Dragging: Move on XY Plane relative to camera view
        const qreal DRAG_SENSITIVITY = 0.5;

        QVector3D currentPoint = m_controlPoints[m_draggingPointIndex];
        currentPoint.setX(currentPoint.x() + dx * DRAG_SENSITIVITY);
        currentPoint.setY(currentPoint.y() - dy * DRAG_SENSITIVITY);

        m_controlPoints[m_draggingPointIndex] = currentPoint;

        // Re-calculate and redraw
        setupVBOs();
        calculateAndStoreCurve();
        update();
        emit controlPointsMoved(m_controlPoints);
    }
    else if (event->buttons() & Qt::RightButton) {
        // Camera Rotation (Orbit)
        m_rotationX += dy * 0.5;
        m_rotationY += dx * 0.5;
        update();
    }
    else if (event->buttons() & Qt::MiddleButton) { // <-- NEW: Panning (Translate)
        // --- Camera Panning (Middle Click Drag) ---
        // Panning sensitivity should scale with distance (zoom)
        const qreal PAN_SENSITIVITY = 0.1;

        m_view.translate(dx * PAN_SENSITIVITY, -dy * PAN_SENSITIVITY, 0.0);
        update();
    }
    m_lastMousePos = event->pos();
}

void DrawingArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_draggingPointIndex != -1) {
        m_draggingPointIndex = -1;
        update();
        emit controlPointsMoved(m_controlPoints);
    }
    QWidget::mouseReleaseEvent(event);
}

void DrawingArea::wheelEvent(QWheelEvent *event)
{
    // The delta() is usually 120 (scroll up) or -120 (scroll down)
    qreal numDegrees = event->angleDelta().y() / 8.0;
    qreal numSteps = numDegrees / 15.0; // Converts to +/- 1, 2, 3...

    const qreal ZOOM_INCREMENT = 20.0;

    // Decrease m_zoomDistance (make it more negative) to zoom out.
    // Increase m_zoomDistance (make it less negative/closer to zero) to zoom in.
    m_zoomDistance += numSteps * ZOOM_INCREMENT;

    // Optional: Clamp the zoom distance to prevent going too far in or out
    if (m_zoomDistance > -50.0) m_zoomDistance = -50.0;
    if (m_zoomDistance < -1000.0) m_zoomDistance = -1000.0;

    update(); // Request a redraw with the new distance
    event->accept();
}

void DrawingArea::drawGrid()
{
    QVector<QVector3D> gridData;
    const int GRID_SIZE = 100; // Total extent in one direction (e.g., -100 to +100)
    const int GRID_SPACING = 20; // Distance between lines (e.g., one tile size)
    const float HALF_SIZE = (float)GRID_SIZE / 2.0f;

    // Generate lines parallel to the Z-axis (varying X)
    for (int i = -GRID_SIZE / 2; i <= GRID_SIZE / 2; ++i) {
        float x = (float)i * GRID_SPACING;
        gridData.append(QVector3D(x, 0.0f, -HALF_SIZE * GRID_SPACING));
        gridData.append(QVector3D(x, 0.0f, HALF_SIZE * GRID_SPACING));
    }

    // Generate lines parallel to the X-axis (varying Z)
    for (int i = -GRID_SIZE / 2; i <= GRID_SIZE / 2; ++i) {
        float z = (float)i * GRID_SPACING;
        gridData.append(QVector3D(-HALF_SIZE * GRID_SPACING, 0.0f, z));
        gridData.append(QVector3D(HALF_SIZE * GRID_SPACING, 0.0f, z));
    }

    // --- Draw using the Shader Program ---

    QOpenGLBuffer gridVbo(QOpenGLBuffer::VertexBuffer);
    gridVbo.create();
    gridVbo.bind();
    gridVbo.allocate(gridData.constData(), gridData.size() * sizeof(QVector3D));

    m_program.bind();
    QMatrix4x4 combined = m_projection * m_view;
    m_program.setUniformValue(m_matrixUniform, combined);

    m_program.enableAttributeArray(m_posAttr);
    m_program.setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 3, 0);

    glLineWidth(1.0f);

    // Set grid color (Darker gray/cyan for visibility against axes)
    m_program.setUniformValue(m_colorUniform, QVector4D(0.3f, 0.3f, 0.4f, 1.0f));

    // Draw all generated lines
    glDrawArrays(GL_LINES, 0, gridData.size());

    m_program.disableAttributeArray(m_posAttr);
    gridVbo.release();
    gridVbo.destroy();
    m_program.release();
}

