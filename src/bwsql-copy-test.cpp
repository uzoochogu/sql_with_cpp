/** 
 * This program copies from one table to another within a db using the BWSQL wrapper 
 * on top of sqlite3
 * 
 * 
*/ 

#include <BWSQL.h>
#include <iostream>

constexpr const char * db_file = DB_PATH "/scratch.db";

constexpr const char * sql_create = "CREATE TABLE IF NOT EXISTS temp"
                                    "( id INTEGER PRIMARY KEY, a TEXT, b TEXT, c TEXT )";
constexpr const char * sql_drop =   "DROP TABLE IF EXISTS temp";
constexpr const char * sql_begin = "BEGIN";
constexpr const char * sql_commit = "COMMIT";
constexpr const char * sql_insert = "INSERT INTO temp (a,b,c) VALUES (?, ?, ?)";
constexpr const char * sql_select_all = "SELECT * FROM temp";
constexpr const char * sql_select_where = "SELECT * FROM temp WHERE a = ?";
constexpr const char * sql_select_count = "SELECT COUNT(*) FROM temp";


// print all rows from a prepared statement
void disp_rows(bw::BWSQL & db) {
    for(const char ** row = db.fetch_row(); row; row = db.fetch_row()) {
        for(int i = 0; i < db.num_sql_columns(); ++i) {
            printf("%s%s", row[i], (i < db.num_sql_columns() - 1) ? ", " : "\n");
        }
    }
}

int main()
{
    bw::BWSQL db(db_file);

    


    return 0;
}