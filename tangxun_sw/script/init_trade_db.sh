#!/bin/sh

if [ $# -eq 0 ]
then
    echo "Specify database name as first argument"
    exit
fi

DATABASE=$1
TABLE_POOL=pool
TABLE_RECORD=record
SQL=/usr/bin/sqlite3

echo "==================Dropping table $TABLE_POOL=================="
$SQL $DATABASE "DROP TABLE $TABLE_POOL;"

echo "==================Creating table $TABLE_POOL=================="
$SQL $DATABASE "CREATE TABLE $TABLE_POOL(stock TEXT, model TEXT, modelParam TEXT, addDate TEXT,\
 startDateOfDaySummary TEXT, numOfDaySummary INTEGER, op TEXT, opRemark TEXT, tradeDate TEXT,\
 position TEXT, volume INTEGER, price REAL, PRIMARY KEY(stock, model));"

echo "==================Adding parameters to $TABLE_POOL================"

echo "==================Updating parameters of $TABLE_POOL============="



echo "==================Dropping table $TABLE_RECORD=================="
$SQL $DATABASE "DROP TABLE $TABLE_RECORD;"

echo "==================Creating table $TABLE_RECORD=================="
$SQL $DATABASE "CREATE TABLE $TABLE_RECORD(stock TEXT, model TEXT, modelParam TEXT, addDate TEXT,\
 op TEXT, opRemark TEXT, tradeDate TEXT, position TEXT, volume INTEGER, price REAL);"

echo "==================Adding parameters to $TABLE_RECORD================"

echo "==================Updating parameters of $TABLE_RECORD============="


