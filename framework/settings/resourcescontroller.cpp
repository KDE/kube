#include "resourcescontroller.h"

ResourcesController::ResourcesController(QObject *parent) : QObject(parent), m_model(new ResourceListModel())
{

}


ResourceListModel* ResourcesController::model() const
{
    return m_model.data();
}
