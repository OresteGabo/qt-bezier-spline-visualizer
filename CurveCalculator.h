//
// Created by muhirwa gabo Oreste on 06/11/2025.
//

#ifndef CURVES3D_CURVECALCULATOR_H
#define CURVES3D_CURVECALCULATOR_H


#include <QVector3D>
#include <QList>
#include <QVector>

class CurveCalculator
{
public:
    static constexpr int CURVE_DETAIL = 100;

    // --- Core Curve Algorithms (Use QVector3D) ---
    static QVector<QVector3D> calculateBezier_DeCasteljau(const QList<QVector3D>& controlPoints);

    static QVector<QVector3D> calculateHermite_Matrix(const QVector3D& p1, const QVector3D& p4,
                                                    const QVector3D& r1, const QVector3D& r4);

    static QVector<QVector3D> calculateBSpline(const QList<QVector3D>& controlPoints, int degree);

    // --- Helper for Hermite/Catmull-Rom ---
    static QVector<QVector3D> calculateCatmullRomSegment(const QVector3D& p0, const QVector3D& p1,
                                                       const QVector3D& p2, const QVector3D& p3);
private:
    // Helper for De Casteljau (3D vector math works identically)
    static QVector3D deCasteljau(const QList<QVector3D>& controlPoints, qreal t);
};



#endif //CURVES3D_CURVECALCULATOR_H