//
// Created by root on 1/21/18.
//

#include "DataStore.hpp"
#include "FileLog.hpp"
#include "Err.hpp"

int DataStore::Put(std::string tableName, std::string key, std::string value)
{
    std::map<std::string, DataTable*>::iterator iter;
    DataTable *table = NULL;
    int err = 0;

    iter = mStore.find(tableName);
    if (mStore.end() == iter) {
        err = Err::E_TABLE_NOT_EXIST;
    } else {
        table = iter->second;
        err = table->DoPut(key, value);
    }

    return err;
}

int DataStore::Add(std::string tableName, std::string key, std::string value)
{
    std::map<std::string, DataTable*>::iterator iter;
    DataTable *table = NULL;
    int err = 0;

    iter = mStore.find(tableName);
    if (mStore.end() == iter) {
        err = Err::E_TABLE_NOT_EXIST;
    } else {
        table = iter->second;
        err = table->DoAdd(key, value);
    }

    return err;
}

int DataStore::Del(std::string tableName, std::string key)
{
    std::map<std::string, DataTable*>::iterator iter;
    DataTable *table = NULL;
    int err = 0;

    iter = mStore.find(tableName);
    if (mStore.end() == iter) {
        err = Err::E_TABLE_NOT_EXIST;
    } else {
        table = iter->second;
        err = table->DoDel(key);
    }

    return err;
}

int DataStore::Get(std::string tableName, std::string key, std::string& value)
{
    std::map<std::string, DataTable*>::iterator iter;
    DataTable *table = NULL;
    int err = 0;

    iter = mStore.find(tableName);
    if (mStore.end() == iter) {
        err = Err::E_TABLE_NOT_EXIST;
    } else {
        table = iter->second;
        err = table->DoGet(key, value);
    }

    return err;
}

int DataStore::CreateTable(std::string tableName)
{
    std::map<std::string, DataTable*>::iterator iter;
    DataTable *table = NULL;
    int err = 0;

    iter = mStore.find(tableName);
    if (mStore.end() != iter) {
        err = Err::E_TABLE_EXIST;
    } else {
        table = new SimpleDataTable(tableName);
        mStore.insert(std::make_pair(tableName, table));
    }

    return err;
}

int DataStore::DropTable(std::string tableName)
{
    std::map<std::string, DataTable*>::iterator iter;

    iter = mStore.find(tableName);
    if (mStore.end() != iter) {
        mStore.erase(tableName);
    }

    return 0;
}

int SimpleDataTable::DoAdd(std::string key, std::string value)
{
    std::map<std::string, std::string>::iterator iter;
    int err = 0;

    iter = mTable.find(key);
    if (iter != mTable.end()) {
        err = Err::E_KEY_EXIST;
    } else {
        mTable.insert(std::make_pair(key, value));
    }

    return err;
}

int SimpleDataTable::DoPut(std::string key, std::string value)
{
    std::map<std::string, std::string>::iterator iter;
    int err = 0;

    iter = mTable.find(key);
    if (iter == mTable.end()) {
        mTable.insert(std::make_pair(key, value));
    } else {
        iter->second = value;
    }

    return err;
}

int SimpleDataTable::DoDel(std::string key)
{
    std::map<std::string, std::string>::iterator iter;
    int err = 0;

    iter = mTable.find(key);
    if (iter != mTable.end()) {
        mTable.erase(key);
    }

    return err;
}

int SimpleDataTable::DoGet(std::string key, std::string &value)
{
    std::map<std::string, std::string>::iterator iter;
    int err = 0;

    iter = mTable.find(key);
    if (iter != mTable.end()) {
        value = iter->second;
    } else {
        err = Err::E_KEY_NOT_EXIST;
    }

    return err;
}

