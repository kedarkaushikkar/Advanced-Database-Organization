test_assign3: test_assign3_1.c record_mgr.c record_mgr.h expr.c expr.h tables.h test_expr.c rm_serializer.c buffer_mgr.c buffer_mgr.h buffer_mgr_stat.c buffer_mgr_stat.h storage_mgr.c storage_mgr.h dt.h test_helper.h dberror.c dberror.h
	gcc -I. -c -o storage_mgr.o storage_mgr.c
	gcc -I. -c -o buffer_mgr.o buffer_mgr.c
	gcc -I. -c -o buffer_mgr_stat.o buffer_mgr_stat.c
	gcc -I. -c -o test_assign3_1.o test_assign3_1.c
	gcc -I. -c -o dberror.o dberror.c
	gcc -I. -c -o expr.o expr.c
	gcc -I. -c -o record_mgr.o record_mgr.c
	gcc -I. -c -o rm_serializer.o rm_serializer.c
	gcc -I. -c -o test_expr.o test_expr.c
	gcc -I. -o test_assign3 test_assign3_1.o storage_mgr.o dberror.o buffer_mgr.o buffer_mgr_stat.o expr.o record_mgr.o rm_serializer.o test_expr.o

clean:
	rm -rf *.o test_assign3

all: test_assign3
