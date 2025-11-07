//
// Created by muhirwa gabo Oreste on 06/11/2025.
//

#include "CurveCalculator.h"

#include <QtMath>
#include <QDebug>

// --- Helper: De Casteljau for a single t (3D) ---
QVector3D CurveCalculator::deCasteljau(const QList<QVector3D>& controlPoints, qreal t)
{
    if (controlPoints.isEmpty()) {
        return QVector3D(0, 0, 0);
    }

    QVector<QVector3D> points = controlPoints.toVector();
    int n = points.size() - 1;

    for (int r = 1; r <= n; ++r) {
        for (int i = 0; i <= n - r; ++i) {
            // Vector interpolation: Q_i^r = (1-t) * Q_i^{r-1} + t * Q_{i+1}^{r-1}
            points[i] = (1.0 - t) * points[i] + t * points[i+1];
        }
    }
    return points[0];
}

// --- 1. Bézier Curve (3D) ---
QVector<QVector3D> CurveCalculator::calculateBezier_DeCasteljau(const QList<QVector3D>& controlPoints)
{
    QVector<QVector3D> calculatedPoints;

    if (controlPoints.size() < 2) return controlPoints.toVector();

    qreal numSteps = static_cast<qreal>(CURVE_DETAIL);

    for (int i = 0; i <= numSteps; ++i) {
        qreal t = static_cast<qreal>(i) / numSteps;
        QVector3D curvePoint = deCasteljau(controlPoints, t);
        calculatedPoints.append(curvePoint);
    }
    return calculatedPoints;
}

// --- 2. Hermite Curve (3D) ---
QVector<QVector3D> CurveCalculator::calculateHermite_Matrix(const QVector3D& p1, const QVector3D& p4,
                                                        const QVector3D& r1, const QVector3D& r4)
{
    QVector<QVector3D> calculatedPoints;
    qreal numSteps = static_cast<qreal>(CURVE_DETAIL);

    for (int i = 0; i <= numSteps; ++i) {
        qreal t = static_cast<qreal>(i) / numSteps;
        qreal t2 = t * t;
        qreal t3 = t2 * t;

        // Hermite Blending Functions
        qreal h1 = 2.0 * t3 - 3.0 * t2 + 1.0;
        qreal h2 = -2.0 * t3 + 3.0 * t2;
        qreal h3 = t3 - 2.0 * t2 + t;
        qreal h4 = t3 - t2;

        // Vector calculation: Q(t) = P1*H1 + P4*H2 + R1*H3 + R4*H4
        QVector3D q_t = p1 * h1 + p4 * h2 + r1 * h3 + r4 * h4;

        calculatedPoints.append(q_t);
    }
    return calculatedPoints;
}

// --- Helper: Catmull-Rom Segment (3D) ---
QVector<QVector3D> CurveCalculator::calculateCatmullRomSegment(const QVector3D& p0, const QVector3D& p1,
                                                           const QVector3D& p2, const QVector3D& p3)
{
    qreal tau = 0.5;

    // Tangent R1 = (P2 - P0) * tau
    QVector3D r1 = (p2 - p0) * tau;
    // Tangent R2 = (P3 - P1) * tau
    QVector3D r2 = (p3 - p1) * tau;

    // Use Hermite core: P1->p1, P4->p2, R1->r1, R4->r2
    return calculateHermite_Matrix(p1, p2, r1, r2);
}

// --- 3. B-Spline Curve (3D) ---
QVector<QVector3D> CurveCalculator::calculateBSpline(const QList<QVector3D>& controlPoints, int degree)
{
    QVector<QVector3D> calculatedPoints;

    if (controlPoints.size() < 4) {
        return controlPoints.toVector();
    }

    qreal numSteps = static_cast<qreal>(CURVE_DETAIL);

    for (int i = 3; i < controlPoints.size(); ++i) {
        const QVector3D& p0 = controlPoints[i-3];
        const QVector3D& p1 = controlPoints[i-2];
        const QVector3D& p2 = controlPoints[i-1];
        const QVector3D& p3 = controlPoints[i];

        for (int j = 0; j <= numSteps; ++j) {
            qreal t = static_cast<qreal>(j) / numSteps;
            qreal t2 = t * t;
            qreal t3 = t2 * t;

            // Uniform Cubic B-Spline Blending Functions
            qreal b0 = (-t3 + 3.0 * t2 - 3.0 * t + 1.0) / 6.0;
            qreal b1 = (3.0 * t3 - 6.0 * t2 + 4.0) / 6.0;
            qreal b2 = (-3.0 * t3 + 3.0 * t2 + 3.0 * t + 1.0) / 6.0;
            qreal b3 = t3 / 6.0;

            // Vector calculation: Q(t) = P0*B0 + P1*B1 + P2*B2 + P3*B3
            QVector3D q_t = p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;

            calculatedPoints.append(q_t);
        }
    }
    return calculatedPoints;
}