#include "coretests.h"

CoreTests::CoreTests(QObject *parent) : QObject(parent)
{
}

void CoreTests::initTestCase()
{
}

void CoreTests::cleanupTestCase()
{
}

void CoreTests::metaInfo(){
    // Test create()
    DQModelMetaInfo* metaInfo4 = dqMetaInfo<Model4>();

    DQAbstractModel *model = metaInfo4->create();
    QVERIFY(model);

    QVERIFY(model->metaInfo() == metaInfo4);

    delete model;

    // Test initalData()
    DQModelMetaInfo* metaInfo2 = dqMetaInfo<Model2>();

    DQList<Model2> initialData = metaInfo2->initialData();

    QVERIFY(initialData.size() == 5);
    QVERIFY(initialData.at(0)->key == "initial0");
    QVERIFY(initialData.at(4)->key == "initial4");

}

void CoreTests::sqliteColumnConstraint(){
    Model1 model;
    DQClause clause = model.id.clause();
    DQSqliteStatement sql;

    QVERIFY(sql.columnConstraint(clause) == "PRIMARY KEY AUTOINCREMENT");

    //    clause = DQClause(DQClause::PRIMARY_KEY) | clause;
    //    QVERIFY(sql.columnConstraint(clause) == "PRIMARY KEY AUTOINCREMENT");

    DQModelMetaInfoField f1("id",0,QVariant::Invalid,
                            DQClause(DQClause::PRIMARY_KEY));
    clause = f1.clause;
    QVERIFY(sql.columnConstraint(clause) == "PRIMARY KEY AUTOINCREMENT");

    DQModelMetaInfo *info = dqMetaInfo<Model1>();
    const DQModelMetaInfoField *f3 = info->at(0);

    clause = f3->clause;
    QVERIFY(sql.columnConstraint(clause) == "PRIMARY KEY AUTOINCREMENT");

}

void CoreTests::sqlCreateTable()
{
    DQSqliteStatement sql;

    qDebug() << sql.createTableIfNotExists<Model1>();

}

void CoreTests::index() {
    DQIndex<Model1> index("index1");
    index << "key";

    DQSqliteStatement sql;

    QString cmd = sql.createIndexIfNotExists(index);
    QVERIFY(cmd == "CREATE INDEX IF NOT EXISTS index1 on model1 (key);");
}

void CoreTests::clause()
{
    DQClause c1;

    QVERIFY(!c1.testFlag(DQClause::DEFAULT));

    DQClause unique = DQUnique;
    QVERIFY(unique.testFlag(DQClause::UNIQUE));
    QVERIFY(!unique.testFlag(DQClause::NOT_NULL));

    DQClause notNull = DQNotNull;
    QVERIFY(!notNull.testFlag(DQClause::UNIQUE));
    QVERIFY(notNull.testFlag(DQClause::NOT_NULL));

    c1 = unique | notNull;
    QVERIFY(c1.testFlag(DQClause::UNIQUE));
    QVERIFY(c1.testFlag(DQClause::NOT_NULL));
    QVERIFY(unique.testFlag(DQClause::UNIQUE));
    QVERIFY(!unique.testFlag(DQClause::NOT_NULL));
    QVERIFY(!notNull.testFlag(DQClause::UNIQUE));
    QVERIFY(notNull.testFlag(DQClause::NOT_NULL));

    DQPrimaryKey key;
    c1 = key.clause();
    QVERIFY(c1.testFlag(DQClause::PRIMARY_KEY));

    DQClause c2;
    c2 = c1 | c2;
    QVERIFY(c2.testFlag(DQClause::PRIMARY_KEY));

    QVariant v1 = qVariantFromValue( (void*) dqMetaInfo<Model4>());
    c2 = DQClause(DQClause::FOREIGN_KEY , v1 );
    QVariant v2 = c2.flag(DQClause::FOREIGN_KEY);


}

void CoreTests::where(){

    QStringList input;

    input << "key = "
          << " key = "
          << " key=";

    foreach (QString i , input){
        DQWhere where(i,3);
//        QVERIFY(where.left() == "key");
        QVERIFY(where.toString() == "key = 3");
    }

    DQWhere price("price");
    QVariant v = "price";
    QVERIFY(v.type() == QVariant::String);

    // The data field is not stored in QString.
    QVERIFY(price.left().type() != QVariant::String);

    DQWhere filter = price > 3;
    QVERIFY(filter.toString() == "price > 3");

    DQWhere qty("qty");

    filter = (price <= 10) &&  (qty > 100);
    QVERIFY(filter.toString() == "( price <= 10 ) and ( qty > 100 )");

    filter = price.equal(qty);
    QVERIFY(filter.toString() == "price = qty");

    filter = price.notEqual(qty);
    QVERIFY(filter.toString() == "price <> qty");

}

void CoreTests::expression(){
    DQWhere where = DQWhere("key = ","test") && DQWhere("length > ", 5);
    DQExpression expression(where);

    qDebug() << expression.string();
    QVERIFY(expression.string() == "(key = :arg0) and (length > :arg1)");

}


void CoreTests::mode1l(){
    Model1 model;
    DQModel *m = new Model1();
    DQModelMetaInfo* info1 = model.metaInfo();
    DQModelMetaInfo* info2 = dqMetaInfo<Model1>();
    DQModelMetaInfo* info3 = m->metaInfo();

    QVERIFY(info1 == info3);

//    QEXPECT_FAIL("","Should be working after refactored",Continue);
    QVERIFY(info1 == info2);

    delete m;

}

void CoreTests::model1_accessFields(){

    DQModelMetaInfo* info = dqMetaInfo<Model1>();
    Model1 *model = new Model1();

    QVERIFY(info->fieldNameList().size() == 3);

    QVariant v("test");

    QVERIFY( info->setValue(model,"key",v) );

    qDebug() << model->key.get();
    QVERIFY(model->key.get() == v);

    QVERIFY(info->value(model,"key") == v);

    delete model;
}

void CoreTests::model1_equalTo_model2(){
    DQModelMetaInfo* info1 = dqMetaInfo<Model1>();
    DQModelMetaInfo* info2 = dqMetaInfo<Model2>();

    QVERIFY(info1 != info2);

    QStringList f1 = info1->fieldNameList();
    QStringList f2 = info2->fieldNameList();
    QVERIFY(f1.size() == 3); // should "id , key , value"
    QVERIFY(f1.size() == f2.size());
    for (int i = 0 ; i < f1.size() ;i++){
        QVERIFY(f1.at(i) == f2.at(i));
    }

}

void CoreTests::model2(){
    DQModelMetaInfo* info1 = dqMetaInfo<Model2>();
    Model2 *model = new Model2();

    DQModelMetaInfo* info2 = model->metaInfo();
    QVERIFY(info2!=0);
    QVERIFY(info1 == info2);

    QVERIFY(dqModelTableName<Model2>() == "model2");
    QVERIFY(info1->name() == "model2");

    QVERIFY(info1->fieldNameList().size() == 3);

    QVariant v("test");

    QVERIFY( info1->setValue(model,"key",v) );

    qDebug() << model->key.get();
    QVERIFY(model->key.get() == v);

    QVERIFY(info1->value(model,"key") == v);

    delete model;
}

void CoreTests::model3(){
    QVERIFY(dqMetaInfo<Model3>() == 0);
    QVERIFY(DQ_MODEL_NAME(Model3) == "");
    QVERIFY(dqModelTableName<Model3>() == "") ;
}

void CoreTests::model4() {
    DQModelMetaInfo* info = dqMetaInfo<Model4>();

    QVERIFY(info->fieldNameList().size() == 5);

}

void CoreTests::model5() {
    DQSqliteStatement statement;
    QString sql = statement.createTableIfNotExists<Model5>();

    QString answer = "CREATE TABLE IF NOT EXISTS model5  (\n"
            "id INTEGER PRIMARY KEY AUTOINCREMENT\n"
            ");" ;
    // All the fields in model5 should be unsupported

    QCOMPARE(sql,answer);

}

void CoreTests::queryrules(){
    DQQuery<Model1> query;
    query = query.filter(DQWhere("key","=","test")).limit(1);

    DQQueryRules rules;
    rules = query;

    QVERIFY(rules.limit() == 1);
    QVERIFY(rules.metaInfo() == dqMetaInfo<Model1>() );

}

void CoreTests::dqList(){
    DQList<Model2> list;
    Model2 item1;
    Model2 item2;
    Model2 item3;

    item1.key = "test1";
    item2.key = "test2";
    item3.key = "test4";
    QVERIFY(list.append(item1));
    QVERIFY(list.append(item2));

    QVERIFY(list.size() == 2);

    QVERIFY(list.at(0)-> key == "test1");
    QVERIFY(list.at(1)-> key == "test2");

    DQList<Model2> list2;
    QVERIFY(list2.size() == 0);
    list2 = list;
    QVERIFY(list.size() == 2);
    list.append(item3);

    QVERIFY(list.size() == 3);
    QVERIFY(list2.size() == 3);

    list.removeAt(0);
    QVERIFY(list.size() == 2);
    QVERIFY(list2.size() == 2);

    list.clear();
    QVERIFY(list.size() == 0);
    QVERIFY(list2.size() == 0);

    // Prove that DQShareList and DQList are exchangable.
    DQSharedList list3;
    list2.append(item1);list2.append(item2);
    list3 = list2;
    QVERIFY(list2.size() == list3.size());

    list2 = list3;
    QVERIFY(list2.size() == 2);
}

void CoreTests::stringlistField(){
    DQField<QStringList> field;

    QVERIFY(field.type() == (QVariant::Type) qMetaTypeId<QStringList>());
    QFETCH(QStringList,list);
    QFETCH(QString,str);

    field = list;
    QVERIFY(field.get() == list);

//    qDebug() << field.get(true);
    QVERIFY(field.get(true) == str);
    QVERIFY(field.get() == list);

    field.set(str);
//    qDebug() << field.get();
    QVERIFY(field.get() == list);
}

void CoreTests::stringlistField_data() {
    QTest::addColumn<QStringList>("list");
    QTest::addColumn<QString>("str");

    QStringList list;
    QString str;

    list.clear();
    list << "a" << "b" << "c";
    str = "a & b & c";
    QTest::newRow("basic") << list << str;

    list.clear();
    list << "&" << "\"" << "\"abc\"";
    str = "&amp; & &quot; & &quot;abc&quot;";
    QTest::newRow("special char") << list << str;

    list.clear();
    list << "&quot;" << "&amp;" << "&&quot;amp;";
    str = "&amp;quot; & &amp;amp; & &amp;&amp;quot;amp;";
    QTest::newRow("compex") << list << str;

}

void CoreTests::stream() {
    HealthCheck record;
    DQStream stream(&record);

    QVERIFY(stream.model() == &record);
    QVERIFY(stream.currentField() == 0);

    stream << "Tester 1" << 179 << 120.5 << QDateTime::currentDateTime();

    QVERIFY(record.name == "Tester 1");
    QVERIFY(record.height == 179);
    QVERIFY(record.weight == 120.5);
    QString name;
    int height;
    double weight;
    QDateTime recordDate;

    QVERIFY(stream.currentField() == 0);

    stream >> name
           >> height
           >> weight
           >> recordDate;

    QVERIFY(record.name == name);
    QVERIFY(record.height == height);
    QVERIFY(record.weight == weight);
    QVERIFY(record.recordDate == recordDate);
}

void CoreTests::listWriter() {
    DQList<HealthCheck> list;
    DQListWriter writer(&list);

    QVERIFY(list.size() == 0);

    writer << "Tester 1" << 179 << 120.5 << QDateTime::currentDateTime();
    QVERIFY(list.size() == 1);
    writer << "Tester 2" << 160 << 80 << QDateTime::currentDateTime()
           << "Tester 3" << 120 << 60 << QDateTime::currentDateTime();
    QVERIFY(list.size() == 3);

    writer << "Tester 4" << 130 << writer.next()
           << "Tester 5" << 160 << writer.next();
    QVERIFY(list.size() == 5);

}
