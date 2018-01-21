//
// Created by root on 1/21/18.
//

#ifndef PROJECT_DATASTORE_HPP
#define PROJECT_DATASTORE_HPP

#include <map>
#include <string>

class DataTable
{
public:
    DataTable(std::string name):mTableName(name)
    {}
    ~DataTable()
    {}

public:
    virtual int DoPut(std::string key, std::string value) = 0;
    virtual int DoAdd(std::string key, std::string value) = 0;
    virtual int DoDel(std::string key) = 0;
    virtual int DoGet(std::string key, std::string& value) = 0;

private:
    std::string mTableName;
};

class SimpleDataTable : public DataTable
{
public:
    SimpleDataTable(std::string name): DataTable(name)
    {}
    ~SimpleDataTable()
    {}

public:
    virtual int DoPut(std::string key, std::string value);
    virtual int DoAdd(std::string key, std::string value);
    virtual int DoDel(std::string key);
    virtual int DoGet(std::string key, std::string& value);

private:
    std::map<std::string, std::string> mTable;
};

class DataStore
{
public:
    DataStore()
    {
    }
    ~DataStore()
    {
    }

public:
    int Put(std::string tableName, std::string key, std::string value);
    int Add(std::string tableName, std::string key, std::string value);
    int Get(std::string tableName, std::string key, std::string& value);
    int Del(std::string tableName, std::string key);

    int CreateTable(std::string tableName);
    int DropTable(std::string tableName);

private:
    /* table <key, value> */
    std::map<std::string, DataTable*> mStore;
};

#endif //PROJECT_DATASTORE_HPP


