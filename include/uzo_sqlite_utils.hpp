#include <iostream>
#include <sqlite3.h>
#include <sqlcpp.h>
#include <cstdio>  //I will be change to std::format in C++20

namespace  uzo 
{
    /**
     * @brief Utility to copy rows from one db table to another
     * 
     * @param db db remote pointer
     * @param table1 name of table1
     * @param table2 name of table2
     * @return true copy successful
     * @return false copy unsuccessful
     */
    bool sql_db_table_copy_all(sqlite3* db, char* table1, char* table2)
    {
    constexpr const int table_num = 2;
    sqlite3_stmt* stmt1 = nullptr;
    sqlite3_stmt* stmt2 = nullptr;
    int col_count2{0};

    char* sql_check = 
    "SELECT count(*) FROM sqlite_master WHERE type='table'"
    " AND  name = ?";

    //check that the two tables exists in the DB
    char * tables[] = {table1, table2};
    int num_rows = sizeof(tables) / sizeof(char *);
    for(int i = 0; i < num_rows; i++)
    {
        sqlite3_prepare_v2(db, sql_check, -1, &stmt1, nullptr);
        sqlite3_bind_text(stmt1, 1, tables[i], -1, SQLITE_STATIC);
        sqlite3_step(stmt1);
        if(sqlite3_column_int(stmt1, 0) == 0)
        {
            std::cout << tables[i] << " table does not exist!\n";
            return false;
        }
        sqlite3_reset(stmt1);
    }
    sqlite3_finalize(stmt1);

    //get table 2 column count 
    char* longer_table_name = (strlen(table1) > strlen(table2) ) ? table1: table2;  // buffer created once, use longest table name
    size_t nbytes = snprintf(nullptr, 0, "SELECT * FROM %s", longer_table_name) + 1; // +1 for the '\0'
    char* sql_select_all = new char[nbytes]; //use only as much as is required 
    snprintf(sql_select_all, nbytes, "SELECT * FROM %s", table2); //Check and Allocate idiom - Coined by me, better than sprintf_s, sprintf
    sqlite3_prepare_v2(db, sql_select_all,-1, &stmt1, nullptr);
    col_count2 = sqlite3_column_count(stmt1);
    sqlite3_finalize(stmt1);

    //Select all rows in first table
    stmt1 = nullptr;
    nbytes = snprintf(nullptr, 0, "SELECT * FROM %s", table2) + 1;
    snprintf(sql_select_all, nbytes, "SELECT * FROM %s", table1);
    sqlite3_prepare_v2(db, sql_select_all, -1, &stmt1, nullptr);

    //start transaction
    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr); //Good practice

    //format insert statement for second table
    nbytes = snprintf(nullptr, 0, "INSERT INTO %s VALUES (", table2) + 1; 
    char* sql_insert = new char[nbytes]; 
    snprintf(sql_insert, nbytes, "INSERT INTO %s VALUES (", table2);
    std::string buf{sql_insert};
    for(int i = 0; i < col_count2; i++)
    {
        buf.append("?");
        if( i < col_count2 - 1) buf.append(", ");
        else buf.append(")");
    }
    sqlite3_prepare_v2(db, buf.c_str(), -1, &stmt2, nullptr);

    //Traverse and copy
    while(sqlite3_step(stmt1) == SQLITE_ROW)
    {
        for(int colnum = 0;  colnum < col_count2; ++colnum) //insert only as much table2 can take
        {
            //copy the data
            buf.assign(reinterpret_cast<const char*>(sqlite3_column_text(stmt1, colnum)));
            sqlite3_bind_text(stmt2, colnum + 1, buf.c_str(), -1, SQLITE_TRANSIENT);  //copy string object prior to return of bind func
        }
        sqlite3_step(stmt2);
        sqlite3_reset(stmt2);
    }
    sqlite3_finalize(stmt1);
    sqlite3_finalize(stmt2);

    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    return true;
    }

}
