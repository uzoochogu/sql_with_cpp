#include <iostream>
#include <sqlite3.h>
#include <sqlcpp.h>
#include <cstdio>  //I will be change to std::format in C++20

constexpr const char *db_file = DB_PATH "/practice.db";
constexpr const char * db_file2 = DB_PATH "/scratch.db";

constexpr const char * sql_create_db1 =
        "DROP TABLE IF EXISTS students;"
        "BEGIN;"
        "CREATE TABLE IF NOT EXISTS students( student_id INTEGER, name VARCHAR(55), year INTEGER);"
        "INSERT INTO students VALUES (1, 'Emmanuel Ayomide', 2003);"
        "INSERT INTO students VALUES (2, 'Ibrahim Musa', 2002);"
        "INSERT INTO students VALUES (3, 'Emeka Frank', 2001);"
        "COMMIT;"
;



constexpr const char * sql_drop = "DROP TABLE IF EXISTS temp";
constexpr const char * sql_create = "CREATE TABLE IF NOT EXISTS temp ( a TEXT, b TEXT, c TEXT )";
constexpr const char * sql_insert = "INSERT INTO temp VALUES (?, ?, ?)";
constexpr const char * sql_begin = "BEGIN";
constexpr const char * sql_commit = "COMMIT";
constexpr const char * sql_select_all = "SELECT * FROM temp";
constexpr const char * sql_select_where = "SELECT * FROM temp WHERE a = ?";

constexpr const char * insert_strings[] = {
    "one", "two", "three",
    "two", "three", "four",
    "three", "four", "five",
    "four", "five", "six",
    "five", "six", "seven"
};

int sql_do(sqlite3 * db, const char * sql) 
{
    std::cout << "sql_do: " << sql << "\n";
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
    if(rc != SQLITE_OK) 
    {
        std::cout << sqlite3_errstr(rc);
        exit(1);
    }
    return sqlite3_changes(db);    //good practice to return this
}

//Utility to copy rows from one db table to another
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

int main()
{
    //practice.db
    sqlite3 *db = nullptr;
    sqlite3_stmt *stmt = nullptr;
    char *errmsg = nullptr;

    //open the database
    int rc = sqlite3_open(db_file, &db);

    //check that opening was successful
    if (rc == SQLITE_OK)
    {
        std::cout << "database practice.db open";
    }
    else
    {
        std::cout << sqlite3_errstr(rc);
        exit(1);
    }

    //Create a Table
    std::cout << "\nCreate the table\n";
    rc = sqlite3_exec(db, sql_create_db1, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK)
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
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::cout << "row " << ++row_count << ": ";
        //step through the columns 
        for (int i = 0; i < col_count; ++i)
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
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        std::cout << "row " << ++row_count << " :\n";
        for (int i = 0; i < col_count; ++i) 
        {
            std::cout << sqlite3_column_text(stmt, i);
            if(i < col_count - 1) std::cout <<  ", ";
            else std::cout <<"\n";
        }
    }
    std::cout << "\n\nOnly " << row_count << " row(s) found.\n";
    sqlite3_finalize(stmt);

    std::cout << "\nclose db\n";
    sqlite3_close(db);

    //reset handlers
    db = nullptr;
    stmt = nullptr;


    //Demonstrate a more advanced use of bind
    //scratch.db
    rc = sqlite3_open(db_file2, &db);

    if (rc == SQLITE_OK)
    {
        std::cout << "\n\ndatabase scatch.db open\n\n";
    }
    else
    {
        std::cout << "\n\nsqlite3_open returned an error (" << rc << ")\n";
        exit(1);
    }

    std::cout << "Drop the table:\n";
    sql_do(db, sql_drop);

    std::cout << "create the table:\n";
    sql_do(db, sql_create);

    std::cout << "begin transaction:\n";
    sql_do(db, sql_begin);

    // insert rows using prepared statement
    std::cout << "insert rows:\n";
    sqlite3_prepare_v2(db, sql_insert, -1, &stmt, nullptr);
    int num_params = sqlite3_bind_parameter_count(stmt);
    std::cout << "The statement " << sql_insert << " has " << num_params << " parameter(s)\n";
    int num_rows = sizeof(insert_strings) / sizeof(const char *) / num_params;      //obtains the number of batches of binds to do
    std::cout << "num rows is " << num_rows << "\n";

    //loop through and bind
    int istr_index = 0;
    int rows_inserted = 0;
    for (int rownum = 0; rownum < num_rows; ++rownum) 
    {
        for (int colnum = 0; colnum < num_params; ++colnum) 
        {
            // sqlite3_bind_text(stmt, param_idx, param_string, param_len (or -1 for strlen), destructor or constant);
            sqlite3_bind_text(stmt, colnum + 1, insert_strings[istr_index], -1, SQLITE_STATIC);
            ++istr_index;
        }
        sqlite3_step(stmt);     //processes the statement
        rows_inserted += sqlite3_changes(db);           //number of rows modified
        sqlite3_reset(stmt);    // reset the statement for next run
    }

    sqlite3_finalize(stmt);
    std::cout << "\ninserted " << rows_inserted << " rows \n";

    std::cout << "commit transaction:\n";
    sql_do(db, sql_commit);

    // fetch all rows
    std::cout << "\nfetch all rows:\n";
    sqlite3_prepare_v2(db, sql_select_all, -1, &stmt, nullptr);
    col_count = sqlite3_column_count(stmt);
    row_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        std::cout << "  row " << ++row_count << ": ";
        for (int i = 0; i < col_count; ++i) 
        {
            std::cout <<  sqlite3_column_text(stmt, i);
            if(i < col_count - 1) std::cout <<  ", ";
            else std::cout <<"\n";
        }
    }
    sqlite3_finalize(stmt);

    // find a row
    std::cout << "\nfind row with a='four'\n";
    param1 = "four";
    sqlite3_prepare_v2(db, sql_select_where, -1, &stmt, nullptr);
    std::cout << "The statement " << sql_select_where << " has " << sqlite3_bind_parameter_count(stmt) << " parameter(s)\n";

    // bind the string to the statement
    sqlite3_bind_text(stmt, 1, param1, -1, SQLITE_STATIC);

    // fetch result
    col_count = sqlite3_column_count(stmt);
    row_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        std::cout << "  row " << ++row_count << ": ";
        for (int i = 0; i < col_count; ++i) {
            std::cout <<  sqlite3_column_text(stmt, i);
            if(i < col_count - 1) std::cout <<  ", ";
            else std::cout <<"\n";
        }
    }
    sqlite3_finalize(stmt);

    std::cout << "close db\n";
    sqlite3_close(db);


    //Challenge 1: copy from rows from one table to another in practice.db
    db = nullptr;
    stmt = nullptr;
    sqlite3_stmt* stmt2 = nullptr;    //second statement 

    //open db
    rc = sqlite3_open(db_file, &db);
    

    //check that opening was successful, 
    if (rc == SQLITE_OK)
    {
        std::cout << "database practice.db open\n";
    }
    else
    {
        std::cout << sqlite3_errstr(rc) << "\n";
        exit(1);
    }

    //create second table
    sqlite3_exec(db, "DROP TABLE IF EXISTS students_copy;",nullptr, nullptr, nullptr);

    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS students_copy( student_id INTEGER, name VARCHAR(55), year INTEGER);",nullptr, nullptr, nullptr);

    //copy rows from one table to another within a db
    if(sql_db_table_copy_all(db, "students", "students_copy"))
    {
        std::cout << "\nCopying done!\n";
    }
    else std::cout << "\nThere was an error copying!\n";
    return 0;
}