build:
	gcc sb/sb.c util.c exporter.c goi.c main.c -o goi
	gcc -pthread sb/sb.c util.c exporter.c goi_threads.c main.c -o goi_threads
	gcc -fopenmp sb/sb.c util.c exporter.c goi_omps.c main.c -o goi_omps
clean:
	rm -f *.out *.gch
