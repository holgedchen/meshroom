#include "NodeModel.hpp"
#include <QQmlEngine>
#include <QJsonObject>

namespace nodeeditor
{

NodeModel::NodeModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int NodeModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return _nodes.count();
}

QVariant NodeModel::data(const QModelIndex& index, int role) const
{
    if(index.row() < 0 || index.row() >= _nodes.count())
        return QVariant();
    Node* node = _nodes[index.row()];
    switch(role)
    {
        case NameRole:
            return node->name();
        case InputsRole:
            return QVariant::fromValue(node->inputs());
        case OutputsRole:
            return QVariant::fromValue(node->outputs());
        case XRole:
            return node->x();
        case YRole:
            return node->y();
        case ModelDataRole:
            return QVariant::fromValue(node);
        default:
            return QVariant();
    }
}

bool NodeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(index.row() < 0 || index.row() >= _nodes.count())
        return false;
    Node* node = _nodes[index.row()];
    switch(role)
    {
        case XRole:
            node->setX(value.toInt());
            break;
        case YRole:
            node->setY(value.toInt());
            break;
        default:
            return false;
    }
    Q_EMIT dataChanged(index, index);
    return true;
}

Node* NodeModel::get(const QString& name)
{
    QListIterator<Node*> it(_nodes);
    while(it.hasNext())
    {
        Node* s = it.next();
        if(s->name() == name)
            return s;
    }
    return nullptr;
}

QHash<int, QByteArray> NodeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[InputsRole] = "inputs";
    roles[OutputsRole] = "outputs";
    roles[XRole] = "x";
    roles[YRole] = "y";
    roles[ModelDataRole] = "modelData";
    return roles;
}

void NodeModel::addNode(Node* node)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    // prevent items to be garbage collected in JS
    QQmlEngine::setObjectOwnership(node, QQmlEngine::CppOwnership);
    node->setParent(this);

    _nodes << node;
    endInsertRows();
    Q_EMIT countChanged(rowCount());
}

void NodeModel::addNode(const QJsonObject& descriptor)
{
    Node* node = new Node();
    node->deserializeFromJSON(descriptor);
    addNode(node);
}

QVariantMap NodeModel::get(int row) const
{
    QHash<int, QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> i(names);
    QVariantMap result;
    while(i.hasNext())
    {
        i.next();
        QModelIndex idx = index(row, 0);
        QVariant data = idx.data(i.key());
        result[i.value()] = data;
    }
    return result;
}

QJsonArray NodeModel::serializeToJSON() const
{
    QJsonArray array;
    for(auto n : _nodes)
        array.append(n->serializeToJSON());
    return array;
}

void NodeModel::deserializeFromJSON(const QJsonArray& array)
{
    for(auto n : array)
        addNode(n.toObject());
}

} // namespace