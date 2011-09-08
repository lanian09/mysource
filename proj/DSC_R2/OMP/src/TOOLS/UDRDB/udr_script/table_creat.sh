#!/bin/sh
mysql -h localhost -u udrdb -pudrdb -D UDRINFO < acct_table.sql
