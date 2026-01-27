OUT := bin
GCC := gcc -pedantic -Wall -Wextra

debug: out
	$(GCC) *.c -o $(OUT)/shiteshell -g

release: out
	$(GCC) *.c -o $(OUT)/shiteshell -O2

.PHONY: out
out:
	-mkdir $(OUT)

# this prevents `make` getting confused with files named `clean`
.PHONY: clean
clean:
# little '-' ignores errors from `rm`
	-rm -rf $(OUT)
