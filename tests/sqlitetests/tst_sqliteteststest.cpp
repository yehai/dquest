#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>

#include <QSqlError>
#include <dqconnection.h>
#include <dqsqlitestatement.h>
#include <dqquery.h>
#include <dqsql.h>
#include <dqlistwriter.h>

#include "model1.h"
#include "model2.h"
#include "model3.h"
#include "model4.h"
#include "model5.h"
#include "config.h"
#include "misc.h"

class SqlitetestsTest : public QObject
{
    Q_OBJECT

public:
    SqlitetestsTest();

    /// Verify the create table stmt
    void verifyCreateTable();

    /// Test the basic operation of DQForeignKey
    void foreignKey();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void insertInto();

    /// Test DQModel::save()
    void dqModelSave();

    /// Test DQQuery::deletFrom without filter
    /**
      It will clear all the record made by previous operation.
     */
    void deleteAll();

    /// Insert pre-defined records for each model. They may needed for following tests
    /**
      @todo Should be part of initTestCase
     */
    void prepareInitRecords();

    void select();

    void queryAll();
    void querySelect();

    /// Test can it load model through foreign key
    void foreignKeyLoad();

    /// Test Model4 access
    void model4();

    /// Test date time access
    void datetime();

    /// Verify the save and load for specific type
    void checkTypeSaveAndLoad();

    void queryOrderBy();

private:
    DQConnection connect;
    QSqlDatabase db;
};

SqlitetestsTest::SqlitetestsTest()
{
}

void SqlitetestsTest::initTestCase()
{
    verifyCreateTable();
    foreignKey();

    DQConnection defaultConnection = DQConnection::defaultConnection();

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName( "tests.db" );

    QVERIFY( db.open() );

    QVERIFY( !defaultConnection.isOpen());
    QVERIFY (connect.open(db) );

    QVERIFY(defaultConnection.isOpen()); // connect become default connection

    DQSql sql = connect.sql();


    QVERIFY( sql.createTableIfNotExists<Model1>() );

    QVERIFY( sql.exists(dqMetaInfo<Model1>() ) );

    QVERIFY( sql.dropTable(dqMetaInfo<Model1>()) );

    QVERIFY( !sql.exists(dqMetaInfo<Model1>() ) );

    QVERIFY( sql.createTableIfNotExists<Model1>() );

    QVERIFY ( connect.addModel<Model1>() );
    QVERIFY ( connect.addModel<Model2>() );
    QVERIFY (!connect.addModel<Model1>() );
    QVERIFY (!connect.addModel<Model3>() );
    QVERIFY ( connect.addModel<Model4>() );
    QVERIFY ( connect.addModel<User>() );
    QVERIFY ( connect.addModel<Config>() );
    QVERIFY ( connect.addModel<ExamResult>() );
    QVERIFY ( connect.addModel<AllType>() );
    QVERIFY ( connect.addModel<HealthCheck>());

    QVERIFY( connect.dropTables() );

    QVERIFY( connect.createTables() ); // recreate table

    DQIndex<Model1> index1("index1");
    index1 << "value";

    QVERIFY(sql.createIndexIfNotExists(index1));

}

void SqlitetestsTest::cleanupTestCase()
{
    connect.close();
}

void SqlitetestsTest::verifyCreateTable(){
    DQSqliteStatement statement;
    QString sql = statement.createTableIfNotExists<Model1>();

    QString answer = "CREATE TABLE IF NOT EXISTS model1  (\n"
    "id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
    "key TEXT NOT NULL,\n"
    "value TEXT \n"
    ");";

    QCOMPARE(sql,answer);

    QString sql2 = statement.createTableIfNotExists<Model2>();
    sql2.replace("model2","model1");
    QCOMPARE(sql,sql2);

    sql = statement.createTableIfNotExists<AllType>();

    QStringList lines = sql.split("\n");

    int n = 8; // no. of field
    QVERIFY(lines.size()  == n +3);

}

void SqlitetestsTest::foreignKey() {
    Model2 model;

    QVERIFY (model.metaInfo()->foreignKeyNameList().size() == 0 );

    DQForeignKey<Model2> key;


    model.key = "test key";
    model.value = "test value";

    key = model;

    QVERIFY(key->key() == "test key");

    DQSqliteStatement statement;
    QString sql = statement.createTableIfNotExists<Config>();


    QString answer = "CREATE TABLE IF NOT EXISTS config  (\n"
    "id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
    "key TEXT ,\n"
    "value TEXT ,\n"
    "uid INTEGER NOT NULL,\n"
    "FOREIGN KEY(uid) REFERENCES user(id)\n"
    ");" ;

    QCOMPARE(sql,answer);

    Config config;
    QVERIFY (config.metaInfo()->foreignKeyNameList().size() == 1 );

}


void SqlitetestsTest::insertInto(){
    DQSqliteStatement statement;
    DQModelMetaInfo *info = dqMetaInfo<Model2>();
    QString sql = statement.insertInto(info,info->fieldNameList());

    QVERIFY( sql == "INSERT INTO model2 (id,key,value) values (:id,:key,:value);" );

    sql = statement.replaceInto(info,info->fieldNameList());
    QVERIFY( sql == "REPLACE INTO model2 (id,key,value) values (:id,:key,:value);" );

    DQConnection connection = DQConnection::defaultConnection();
    QVERIFY (connection.isOpen() );


}

void SqlitetestsTest::dqModelSave(){
    Model1 model1;

    QVERIFY(model1.id->isNull());

    model1.key = "save1";
    model1.value = "value1";

    QVERIFY ( model1.save() );
    QVERIFY (!model1.id->isNull());

    QVariant id = model1.id();

    QVERIFY ( model1.save() );
    QVERIFY ( model1.id() == id);

    QVERIFY ( model1.save(true) ); // Force insert
    QVERIFY ( model1.id() != id); // ID should be changed.
}

void SqlitetestsTest::deleteAll() {
    DQQuery<Model1> query;

    QVERIFY(query.remove());

    int count = query.count();
    QVERIFY(count == 0);

    Model1 model1;
    model1.key = "temp";
    model1.value = "temp value";
    model1.save();

    query = DQQuery<Model1>();
    count = query.count();
    QVERIFY(count == 1);

    count = query.count(); // call it twice
    QVERIFY(count == 1);

    QVERIFY(!model1.id->isNull());
    QVERIFY(model1.remove());
    QVERIFY(model1.id->isNull());
    QVERIFY(query.count() == 0);
    QVERIFY(model1.save());
    QVERIFY(query.count() == 1);

    int id = model1.id().toInt();
    Model1 model1b;
    QVERIFY(model1b.load( DQWhere("id = ",id) ));
//    QVERIFY(model1.id() == model1b.id() );
    QVERIFY(model1.id == model1b.id );

    query = DQQuery<Model1>().filter(DQWhere("key = " ,"temp")).limit(1);
    QEXPECT_FAIL("","Normally sqlite do not support limit in delete from , unless it is build with special flag",Continue);
    QVERIFY(query.remove());

    qDebug() << "Failed sql : " << query.lastQuery().lastQuery();

    query = DQQuery<Model1>().filter(DQWhere("key" , "=" , "temp"));
    QVERIFY(query.remove());

    query = DQQuery<Model1>();

    count = query.count();
    QVERIFY(count == 0);


}

void SqlitetestsTest::prepareInitRecords() {
    Model1 model1;
    model1.key = "config1";
    model1.value ="value1";
    QVERIFY ( model1.save(true) );

    model1.key = "config2";
    model1.value ="value2";
    QVERIFY ( model1.save(true) );

    model1.key = "config3";
    model1.value ="value3";
    QVERIFY ( model1.save(true) );

    Model2 model2;
    model2.key = "config1";
    model2.value = "value1";
    QVERIFY(model2.save());

    model2.key = "config2";
    model2.value = "value2";
    QVERIFY(model2.save(true));

    User user;
    user.name = "Ben Lau";
    user.userId = "benlau";

    QVERIFY(!user.save()); // passwd is missed

    user.passwd = "123456";
    QVERIFY (!user.save()); // passwd is too short
    user.passwd = "12345678";
    QVERIFY (user.save());

    Config config;
    config.key = "config1";
    config.value = "value1";
    QVERIFY(!config.save()); // config.uid can not be null

    QVERIFY(config.id->isNull());

    config.uid = user;

    QVERIFY(config.save());

    ExamResult result;
    result.uid = user;
    result.subject = "English";
    result.mark = 50;
    QVERIFY(result.save());

    result.subject = "Maths";
    result.mark = 80;
    QVERIFY(result.save(true));
    DQQuery<ExamResult> examResultQuery;
    QVariant total = examResultQuery.filter(DQWhere("uid = " , user.id )).call("sum",QStringList("mark"));
    QVERIFY(total == 130);
    QVariant avg = examResultQuery.filter(DQWhere("uid = " , user.id() )).call("avg",QStringList("mark"));
    QVERIFY(avg == 65);

}

void SqlitetestsTest::select()
{
    Model1 model1a,model1b;

    DQQuery<Model1> query1 = DQQuery<Model1>().filter(DQWhere("key","=","config1")).limit(1) ;

    DQSqliteStatement statement;
    QString sql = statement.select(query1);

    qDebug() << sql;

    QVERIFY (query1.exec() );
    QVERIFY (query1.next() );

    QVERIFY( query1.recordTo(model1a)) ;

    QVERIFY( !model1a.id->isNull());
    QVERIFY( model1a.key() == "config1");
    QVERIFY( model1a.value() == "value1");

    model1b = query1.record();

    QVERIFY( !model1b.id->isNull());
    QVERIFY( model1b.key() == "config1");
    QVERIFY( model1b.value() == "value1");

    QVERIFY( !query1.next());

}

void SqlitetestsTest::queryAll(){
    DQQuery<Model2> query;

    DQList<Model2> record = query.all();

    QVERIFY(record.size() == 7); // 5 initial record + 2 newly inserted record
    QVERIFY(record.at(0)->key() == "initial0");
    QVERIFY(record.at(0)->value() == "value0");
    QVERIFY(record.at(1)->key() == "initial1");
    QVERIFY(record.at(1)->value() == "value1");

    // Alernative way

    record = Model2::objects().all();
    QVERIFY(record.size() == 7);

}

void SqlitetestsTest::querySelect() {
    DQQuery<Model2> query;

    DQList<Model2> record = query.select("key").all();

    QVERIFY(record.size() == 7); // 5 initial record + 2 newly inserted record
    QVERIFY(record.at(0)->key() == "initial0");
    QVERIFY(record.at(0)->value().isNull()); // It is not loaded
    QVERIFY(record.at(1)->key() == "initial1");
    QVERIFY(record.at(1)->value().isNull()); // It is not loaded

    // Alernative way

    record = Model2::objects().all();
    QVERIFY(record.size() == 7);

}

void SqlitetestsTest::foreignKeyLoad() {
    DQQuery<Config> query = DQQuery<Config>().filter(DQWhere("key=","config1")).limit(1);

    QVERIFY(query.exec());
    QVERIFY(query.next());

    Config config = query.record();
    QVERIFY(config.key() == "config1");

    QVERIFY(!config.uid.isLoaded());
    QVERIFY(config.uid->name() == "Ben Lau");
    QVERIFY(config.uid.isLoaded());

}

void SqlitetestsTest::model4() {
    Model4 item1,item2;

    item1.key = "test";
    item1.value = "test";
    QVERIFY (item1.save());

//    qDebug() << item1.lastQuery().lastQuery();
    qDebug() << &item1;

    QVERIFY (item2.load(DQWhere("key = ","test")));

    QVERIFY(item2.help == "...");

}

void SqlitetestsTest::datetime() {
    User user1;

    user1.userId = "tester";
    user1.name = "tester";
    user1.passwd = "12345678";

    QDateTime datetime = QDateTime::currentDateTime().toUTC();

    user1.lastLoginTime = datetime;

    QVERIFY(user1.clean());
    QVERIFY(user1.save());

    User user2;
    QVERIFY(user2.load(DQWhere("userId=","tester")));

    QVERIFY(user2.lastLoginTime == datetime);

    QVERIFY(!user2.creationTime->isNull());
}

void SqlitetestsTest::checkTypeSaveAndLoad(){

    QStringList sl;
    sl << "a" << "b" << "c";

    AllType type1,type2;
    type1.d = "1.0";
    QByteArray data(100,'0');
    type1.data = data;
    type1.b = true;
    type1.sl = sl;

    QVERIFY(type1.save());

    QVERIFY(type2.load(DQWhere("id=",type1.id) ) );

    QVERIFY(type2.data == data);
    QVERIFY(type2.d == "1.0");
    QVERIFY(type2.b == true);
    qDebug() << type2.sl;
    QVERIFY(type2.sl == sl);

    // try again

    type1.b = false;

    QVERIFY(type1.save());

    QVERIFY(type2.load(DQWhere("id=",type1.id) ) );

    QVERIFY(type2.b == false);

}

void SqlitetestsTest::queryOrderBy(){
    DQQuery<HealthCheck> query;

    QVERIFY(query.remove());

    DQList<HealthCheck> records;

    DQListWriter writer(&records);

    writer << "tester 1" << 160 << 120 << writer.next()
           << "tester 2" << 120 << 170 << writer.next()
           << "tester 3" << 140 << 110 << writer.next();

    writer.close();
    QVERIFY(records.save());

    DQList<HealthCheck> result = query.all();
    QVERIFY(result.size() == 3);

    result = query.orderBy("height asc").all();

    QVERIFY(result.size() == 3);

    QVERIFY(result.at(0)->name == "tester 2");
    QVERIFY(result.at(2)->name == "tester 1");

    result = query.orderBy("height desc").all();
    QVERIFY(result.at(0)->name == "tester 1");
    QVERIFY(result.at(2)->name == "tester 2");

    result = query.orderBy("weight desc").all();
    QVERIFY(result.at(0)->name == "tester 2");
    QVERIFY(result.at(2)->name == "tester 3");

    result = query.orderBy("weight asc").all();
    QVERIFY(result.at(0)->name == "tester 3");
    QVERIFY(result.at(2)->name == "tester 2");

}


QTEST_MAIN(SqlitetestsTest);

#include "tst_sqliteteststest.moc"
