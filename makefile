.PHONY: clean pguard
pguard:
	gcc -o pguard src/process_guard.c src/util.c -lpthread
clean:
	rm -f pguard

