release: exe

debug: exe_dbg


exe: main.c
	gcc main.c -O3 -o build/test_app
exe_dbg: main.c
	gcc main.c -o build/test_app_db -Wall
clean:
	rm -rf build/* 2> /dev/null 