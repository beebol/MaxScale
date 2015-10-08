/**
 * @file bug620.cpp bug620 regression case ("enable_root_user=true generates errors to error log")
 *
 * - Maxscale.cnf contains RWSplit router definition with enable_root_user=true
 * - GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' IDENTIFIED BY 'skysqlroot';
 * - try to coinnect using 'root' user and execute some query
 * - errors are not expected in the log. All Maxscale services should be alive.
 */


#include <my_config.h>
#include <iostream>
#include <unistd.h>
#include "testconnections.h"

using namespace std;

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    int global_result = 0;

    Test->read_env();
    Test->print_env();

    Test->connect_maxscale();

    printf("Creating 'root'@'%%'\n");
    //global_result += execute_query(Test->conn_rwsplit, (char *) "CREATE USER 'root'@'%'; SET PASSWORD FOR 'root'@'%' = PASSWORD('skysqlroot');");

    global_result += execute_query(Test->conn_rwsplit, (char *) "GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' IDENTIFIED BY 'skysqlroot';");
    sleep(10);

    MYSQL * conn;

    printf("Connecting using 'root'@'%%'\n");
    conn = open_conn(Test->rwsplit_port, Test->maxscale_IP, (char *) "root", (char *)  "skysqlroot", Test->ssl);
    if (conn == NULL) {
        printf("Connection using 'root' user failed\n");
        global_result++;
    } else {

        printf("Simple query...\n");
        global_result += execute_query(conn, (char *) "SELECT * from mysql.user");
        mysql_close(conn);
    }

    printf("Dropping 'root'@'%%'\n");
    global_result += execute_query(Test->conn_rwsplit, (char *) "DROP USER 'root'@'%';");

    Test->close_maxscale_connections();

    global_result +=Test->check_log_err((char *) "Warning: Failed to add user skysql", FALSE);
    global_result +=Test->check_log_err((char *) "Error : getaddrinfo failed", FALSE);
    global_result +=Test->check_log_err((char *) "Error : Couldn't find suitable Master", FALSE);

    global_result +=Test->check_maxscale_alive();
    Test->copy_all_logs(); return(global_result);
}
