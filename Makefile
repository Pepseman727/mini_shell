build: exe

exe: main.c
	gcc main.c -o build/test_app

clean:
	rm -f build/* 2> /dev/null 