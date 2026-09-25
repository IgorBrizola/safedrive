# make        -> compila ./safedrive
# make run    -> compila e executa
# make zip    -> gera entrega.zip com a versão comentada (tem nomes e matrículas)
# make clean  -> apaga o executável e o zip
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

safedrive: safedrive.c
	$(CC) $(CFLAGS) -o $@ $<

run: safedrive
	./safedrive

zip:
	zip -j entrega.zip comentado/safedrive_comentado.c

clean:
	rm -f safedrive safedrive.exe entrega.zip

.PHONY: run zip clean
