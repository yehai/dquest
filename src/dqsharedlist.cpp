#include "dqsharedlist.h"
#include <QSharedData>
#include <QList>
#include <QSharedPointer>

class DQSharedListPriv : public QSharedData {
public:
    DQSharedListPriv() {
        metaInfo = 0;
    }

    ~DQSharedListPriv() {
        clear();
    }

    void clear(){
        /*
        foreach (DQAbstractModel*model, list){
            delete model;
        }
        */
        list.clear();
//        metaInfo = 0;
    }

    QList < QSharedPointer<DQAbstractModel> > list;
    DQModelMetaInfo *metaInfo;
};

DQSharedList::DQSharedList() : data(new DQSharedListPriv){
}

DQSharedList::DQSharedList(const DQSharedList &rhs) : data(rhs.data)
{
}

DQSharedList &DQSharedList::operator=(const DQSharedList &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

DQSharedList::~DQSharedList()
{
}

bool DQSharedList::isEmpty() const{
    return data->list.isEmpty();
}

int DQSharedList::size() const{
    return data->list.size();
}

DQAbstractModel* DQSharedList::at(int i) const{
    return data->list.value(i).data();
}

DQAbstractModel* DQSharedList::last() const{
    return data->list.last().data();
}

bool DQSharedList::append(DQAbstractModel* model){
    if (data->metaInfo &&
        model->metaInfo() != data->metaInfo) {
        return false;
    }

    QSharedPointer<DQAbstractModel>  ptr(model);
    data->list << ptr;
    return true;
}

bool DQSharedList::append(DQSharedList& other){
    if (data->metaInfo &&
        other.metaInfo() != data->metaInfo){
        return false;
    }

    data->list.append(other.data->list);
    return true;
}

void DQSharedList::clear() {
    data->clear();
}

void DQSharedList::removeAt(int index) {
//    DQAbstractModel *model = data->list.value(index).data();
    data->list.removeAt(index);
//    delete model;
}

bool DQSharedList::save(bool forceInsert,bool forceAllField) {
    int n = size();
    bool res = true;

    for (int i = 0 ; i < n ;i++){
        DQAbstractModel* model = at(i);
        if (!model->save(forceInsert,forceAllField)) {
            res = false;
        }
    }

    return res;
}

DQModelMetaInfo* DQSharedList::metaInfo(){
    return data->metaInfo;
}

void DQSharedList::setMetaInfo(DQModelMetaInfo* metaInfo){
    data->metaInfo = metaInfo;
}
