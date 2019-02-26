#!/bin/sh

if [ $# -eq 0 ]
then
    echo "Specify database name as first argument"
    exit
fi

DATABASE=$1
TABLE=env
SQL=/usr/bin/sqlite3

echo "==================Dropping table=================="
$SQL $DATABASE "DROP TABLE $TABLE;"

echo "==================Creating table=================="
$SQL $DATABASE "CREATE TABLE $TABLE(key TEXT PRIMARY KEY, value TEXT);"

echo "==================Adding parameters=================="
$SQL $DATABASE "INSERT INTO $TABLE (key,value) VALUES('DataPath','');"
$SQL $DATABASE "INSERT INTO $TABLE (key,value) VALUES('DaysForStockInPool','');"
$SQL $DATABASE "INSERT INTO $TABLE (key,value) VALUES('MaxStockInPool','');"

echo "==================Updating parameters=================="
$SQL $DATABASE "UPDATE $TABLE SET value='/home/minz/stock' WHERE key='DataPath';"
$SQL $DATABASE "UPDATE $TABLE SET value='0' WHERE key='DaysForStockInPool';"
$SQL $DATABASE "UPDATE $TABLE SET value='30' WHERE key='MaxStockInPool';"





