#pragma once

#include "resourcelistmodel.h"

#include <QObject>
#include <QScopedPointer>

class ResourcesController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (ResourceListModel *model READ model CONSTANT)

public:
    explicit ResourcesController(QObject *parent = Q_NULLPTR);

    ResourceListModel *model() const;

private:
    QScopedPointer<ResourceListModel> m_model;
};
