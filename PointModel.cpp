//
// Created by muhirwa gabo Oreste on 06/11/2025.
//

#include "PointModel.h"
#include <QDebug>

PointModel::PointModel(QObject *parent)
    : QObject(parent) {}

QList<QVector3D> PointModel::getControlPoints() const
{
    return m_controlPoints;
}

void PointModel::setControlPoints(const QList<QVector3D>& points)
{
    if (m_controlPoints != points) {
        m_controlPoints = points;
        emit pointsChanged(m_controlPoints);

        // Debugging 3D points
        qDebug() << "PointModel updated. Total points:" << m_controlPoints.size();
        for (const QVector3D& p : m_controlPoints) {
            qDebug() << "  (" << p.x() << ", " << p.y() << ", " << p.z() << ")";
        }
    }
}