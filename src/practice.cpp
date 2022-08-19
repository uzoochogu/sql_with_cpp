#include <cstdio>
#include <iostream>
#include <sqlite3.h>
#include <sqlcpp.h>

constexpr const char *db_file = DB_PATH "/practice.db";

constexpr const char * sql_create =
        "DROP TABLE IF EXISTS students;"
        "BEGIN;"
        "CREATE TABLE IF NOT EXISTS students( student_id INTEGER, name VARCHAR(55), year INTEGER);"
        "INSERT INTO students VALUES (1, 'Emmanuel Ayomide', 2003);"
        "INSERT INTO students VALUES (2, 'Ibrahim Musa', 2002);"
        "INSERT INTO students VALUES (3, 'Emeka Frank', 2003);"
        "COMMIT;"
;

int main()
{
    sqlite3 *db = nullptr;
    sqlite3_stmt *stmt = nullptr;
    char *errmsg = nullptr;

    //open the database
    int rc = sqlite3_open(db_file, &db);

    //check that it is open
    if(rc == SQLITE_OK)
    {
        std::cout << "database open";
    }
    else
    {
        std::cout << sqlite3_errstr(rc);
        exit(1);
    }

    //Create a Table
    std::cout << "\nCreate the table\n";
    rc = sqlite3_exec(db, sql_create, nullptr, nullptr, &errmsg);
    if(rc != SQLITE_OK)
    {
        std::cout << errmsg;
        sqlite3_free(errmsg);
        return 1;
    }

    std::cout << "close db\n";
    sqlite3_close(db);
    return 0;
}