build:
	gcc sb/sb.c util.c exporter.c goi.c main.c -o goi
	gcc -pthread sb/sb.c util.c exporter.c goi_threads.c main.c -o goi_threads
	gcc -fopenmp sb/sb.c util.c exporter.c goi_omp.c main.c -o goi_omp
clean:
	rm -f *.out *.gch
