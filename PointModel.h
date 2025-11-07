//
// Created by muhirwa gabo Oreste on 06/11/2025.
//

#ifndef CURVES3D_POINTMODEL_H
#define CURVES3D_POINTMODEL_H


#include <QObject>
#include <QVector3D> // The 3D vector type
#include <QList>

class PointModel : public QObject
{
    Q_OBJECT

public:
    explicit PointModel(QObject *parent = nullptr);

    QList<QVector3D> getControlPoints() const;
    void setControlPoints(const QList<QVector3D>& points);

    signals:
        void pointsChanged(const QList<QVector3D>& points);

private:
    QList<QVector3D> m_controlPoints;
};




#endif //CURVES3D_POINTMODEL_H