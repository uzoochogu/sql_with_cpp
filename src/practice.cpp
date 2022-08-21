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

    //check that opening was successful
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

    //Using prepared Statements
    std::cout << "Prepared Statements:\n";
    sqlite3_prepare_v2(db, "SELECT * FROM students", -1, &stmt, nullptr);

    std::cout << "Fetch Rows:\n";
    int col_count = sqlite3_column_count(stmt);
    int row_count = 0;

    //step through the rows
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::cout << "row " << ++row_count << ": ";
        //step through the columns 
        for(int i = 0; i < col_count; ++i)
        {
            std::cout << sqlite3_column_text(stmt, i);
            if(i < col_count - 1) std::cout << ", ";
        }
        std::cout << "\n";
        
    }

    //Always finalize!
    std::cout << "Statement Finalized";
    sqlite3_finalize(stmt);


    //Using Prepared statements and Bind
    
    // find a row
    std::cout << "\nfind row with name='Emeka Frank'";
    const char * sql_prepare = "SELECT * FROM students WHERE name = ?";
    const char * param1 = "Emeka Frank";
    sqlite3_prepare_v2(db, sql_prepare, -1, &stmt, nullptr);
    std::cout << "\nstatement: " << sql_prepare << "\n";
    std::cout << "the statement has " << sqlite3_bind_parameter_count(stmt) << " parameter(s)\n";

    // bind the string to the statement
    // sqlite3_bind_text(stmt, param_position, param_string, param_length (or -1 for strlen), destructor or constant);
    sqlite3_bind_text(stmt, 1, param1, -1, SQLITE_STATIC);
    col_count = sqlite3_column_count(stmt);
    row_count = 0;
    while(sqlite3_step(stmt) == SQLITE_ROW) 
    {
        std::cout << "row " << ++row_count << " : ";
        for(int i = 0; i < col_count; ++i) {
            std::cout << sqlite3_column_text(stmt, i);
            if(i < col_count - 1) std::cout <<  ", ";
            else std::cout <<"\n";
        }
    }
    sqlite3_finalize(stmt);

    //Drop the table
    std::cout << "drop table\n";
    rc = sqlite3_exec(db, "DROP TABLE IF EXISTS students", nullptr, nullptr, nullptr);
    if(rc != SQLITE_OK) {
        puts(sqlite3_errmsg(db));
        return 1;
    }

    std::cout << "\nclose db\n";
    sqlite3_close(db);
    return 0;
}