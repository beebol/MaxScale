/**
 * @file bug653.cpp  regression case for bug 653 ("Memory corruption when users with long hostnames that can no the resolved are loaded into MaxScale")
 *
 * - CREATE USER 'user_with_very_long_hostname'@'very_long_hostname_that_can_not_be_resolved_and_it_probably_caused_crash.com.net.org' IDENTIFIED BY 'old';
 * - try to connect using user 'user_with_very_long_hostname'
 * - DROP USER 'user_with_very_long_hostname'@'very_long_hostname_that_can_not_be_resolved_and_it_probably_caused_crash.com.net.org'
 * - check MaxScale is alive
 */

#include <my_config.h>
#include <iostream>
#include "testconnections.h"

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    int global_result = 0;

    Test->read_env();
    Test->print_env();

    Test->connect_maxscale();

    printf("Creating user with old style password\n");
    global_result += execute_query(Test->conn_rwsplit, (char *) "CREATE USER 'user_long_host11'@'very_long_hostname_that_probably_caused_crashhh.com.net.org' IDENTIFIED BY 'old'; GRANT ALL PRIVILEGES ON *.* TO 'user_long_hoste'@'very_long_hostname_that_probably_caused_crashhh.com.net.org' WITH GRANT OPTION;");
    sleep(10);

    printf("Trying to connect using user with old style password\n");
    MYSQL * conn = open_conn(Test->rwsplit_port, Test->maxscale_IP, (char *) "user_long_host11", (char *)  "old", Test->ssl);

    if ( conn == NULL) {
        printf("Connections is not open as expected\n");
    } else {
        printf("Connections is open for the user with bad host. FAILED!\n");
        global_result++;
        mysql_close(conn);
    }

    global_result += execute_query(Test->conn_rwsplit, (char *) "DROP USER 'user_long_host11'@'very_long_hostname_that_probably_caused_crashhh.com.net.org'");
    Test->close_maxscale_connections();


    global_result +=Test->check_maxscale_alive();

    Test->copy_all_logs(); return(global_result);
}
