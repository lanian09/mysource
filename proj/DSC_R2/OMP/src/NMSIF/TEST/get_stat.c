#include <stdio.h>
#include <time.h>
#include <mysql.h>

MYSQL	sql, *conn;

struct	tm	*now ();

char	*get_this_time ();


main ()
{
	init ();

	get_stat ();
}


init ()
{
	mysql_init (&sql);

	if ((conn = mysql_real_connect (&sql, "localhost", "root", "",
									//"BSDM", 0, 0, 0)) == NULL) {
									"DSCM", 0, 0, 0)) == NULL) {
		printf ("[*] fail mysql_connect \n\n");
		exit (1);
	}
}

get_stat ()
{
	int			ret, i;
	char		query[1024];
	MYSQL_ROW	row;
	MYSQL_RES	*result;

	i = 0;
	sprintf (query, "SELECT * FROM %s where (stat_date='%s')", 
			"fault_5minute_statistics", get_this_time ());

printf ("111 query=%s\n", query);
	if (mysql_query (conn, query) != 0) {
		printf ("[*] mysql_query : %s\n", mysql_error (conn));
		exit (1);
	}

	result = mysql_store_result (conn);

printf ("[*] result=0x%x, conn=0x%x\n", result, conn);

	while ((row = mysql_fetch_row (result)) != NULL) {

		memset (query, 0, 1024);
		sprintf (query, "SELECT * FROM %s where (stat_date='%s')", 
				"fault_5minute_statistics", get_this_time ());

printf ("[*] query=%s\n", query);

		ret = test_mysql_select_query (query);

		if (ret == 1) {
			printf ("[%02d] %s %s %s %s %s %s %s %s\n", i, row[0],
				row[1], row[2], row[3], row[4], row[5], row[6], row[7],
				row[8], row[9], row[10], row[11], row[12], row[13], row[14]);
		}
		i++;
	}
	printf ("loop=%d\n", i);
	mysql_free_result (result);

	return 1;
}


test_mysql_select_query (char *query)
{
	int			t_ret=0;
	MYSQL_RES	*t_res;
	MYSQL_ROW	t_row;

	if (mysql_query (conn, query) != 0) {
		printf ("[*] mysql_query fail :%s\n", mysql_error(conn));
		return -1;
	}

	t_ret = 0;
	t_res = mysql_store_result (conn);
	while ((t_row = mysql_fetch_row (t_res)) != NULL) {
		t_ret = 1;	break;
	}
	mysql_free_result (t_res);

	return t_ret;
}



char *get_this_time ()
{
	static char	ttime[40];
	//time_t		now;
	struct tm	*ntime;

#if 0
	now = time(0);

	memset (ttime, 0, 40);

	if ((ntime = (struct tm*)localtime((time_t*)&now)) == NULL)
		strcpy (ttime, "");

	else strftime (ttime, 32, "%Y-%m-%d %H:%M", ntime);
#endif
	
	ntime = (struct tm *)now ();

#if 0
	sprintf (ttime, "%d-%02d-%02d %02d:%02d:00", ntime->tm_year+1900,
			ntime->tm_mon+1, ntime->tm_mday, ntime->tm_hour, 
			get_abs_min (ntime->tm_min));
#endif

	sprintf (ttime, "%d-%02d-%02d %02d:00", ntime->tm_year+1900,
			ntime->tm_mon+1, ntime->tm_mday, ntime->tm_hour);

	printf ("time=%s\n", ttime);

	return ttime;
}


get_abs_min (int r_min)
{
	static int	ret_min=0;

	if (!(r_min%5))
		ret_min = r_min;
	else ret_min = r_min - (r_min%5);

	return ret_min;
}


struct tm *now ()
{
	struct tm	*tim;

	time_t	oclock = time (NULL);
	tim = localtime (&oclock);

	return tim;
}
