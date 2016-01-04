test_assign2: test_assign2_1.c buffer_mgr.c buffer_mgr.h buffer_mgr_stat.c buffer_mgr_stat.h storage_mgr.c storage_mgr.h dt.h test_helper.h
	gcc -I. -c -o storage_mgr.o storage_mgr.c
	gcc -I. -c -o buffer_mgr.o buffer_mgr.c
	gcc -I. -c -o buffer_mgr_stat.o buffer_mgr_stat.c
	gcc -I. -c -o test_assign2_1.o test_assign2_1.c
	gcc -I. -c -o dberror.o dberror.c
	gcc -I. -o test_assign2 test_assign2_1.o storage_mgr.o dberror.o buffer_mgr.o buffer_mgr_stat.o 

clean:
	rm -rf *.o test_assign2

all: test_assign1
 
