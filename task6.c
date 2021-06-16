#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h> 
#include <stdbool.h>


int copy_read_write(char* fd_from,char* fd_to);
int copy_mmap(char* fd_from, char* fd_to);
void help_function();


void help_function(){


	printf("The COPY program.\n");
	printf("Usage:\n");
	printf("\tcopy [-h]\n");
	printf("\tcopy [-m] <file_name> <new_file_name>\n");
	printf("\tcopy <file_name> <new_file_name>\n\n\n");
	printf("Without option -m, program uses read() and write() functions to copy file contents.\n");
	printf("If the option -m is given, both files are mapped to memory regions with mmap() and are copied the file with memcpy() instead.\n\n");
}

int main (int argc, char **argv){

	int c;
	bool is_mmap = false, is_help = false;

	while((c = getopt (argc, argv, "mh")) != -1){
		switch (c){
			case 'm':
				is_mmap = true;
				break;
			case 'h':
				is_help = true;
				break;
			default:
				printf("\nUnknown flags passed!\n");
		}
	}

	if(argc == 1){

		help_function();
	}



	char* file_from = argv[optind];
	char* file_to = argv[optind+1];


	printf("file one: %s      file two: %s ",file_from, file_to);
	


	if (is_mmap){

		printf("\nCopying  one file into another using mmap() and memcpy() functions.\n\n");
		copy_mmap(file_from, file_to);
		return 0;
	}
	else{

		printf("\nCopying one file  into another using read() and write() functions.\n\n");
		copy_read_write(file_from, file_to);
		return 0;

	}


	if(is_help){

		help_function();
	
	}

	return 0;
}




int copy_read_write(char* file_from,char* file_to){

	
	int fd_from, fd_to;

	if ((fd_from = open(file_from, O_RDONLY)) < 0){

		printf("\nERROR: Can't open %s for reading!\n", file_from);
		return 1;
	}

	if ((fd_to = open(file_to, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0){

		printf("\nERROR: Can't open %s for writing!\n", file_to);
		return 1;
	}

	size_t copied_bytes;
	char buf[512*1024]; 


	while ((copied_bytes = read(fd_from, &buf, sizeof(buf))) > 0){

		if (write(fd_to, &buf, copied_bytes) < 0){

			printf("\nERROR: Error writing into output file!\n");
		}
	}
	
	if (copied_bytes < 0){

		printf("\nERROR: Error reading input file!\n");
	}


	if (close(fd_from)){

		fprintf(stderr,"\nERROR: Can't close %s!\n", file_from);
	}
	if (close(fd_to)){

		fprintf(stderr,"\nERROR: Can't close %s!\n", file_to);
	}


}


int copy_mmap(char* file_from,char* file_to){


	int fd_from, fd_to;

	if ((fd_from = open(file_from, O_RDONLY)) < 0){

		printf("\nERROR: Can't open %s for reading!\n", file_from);
		return 1;
	}

	if ((fd_to = open(file_to, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0){

		printf("\nERROR: Can't open %s for writing!\n", file_to);
		return 1;
	}

	
	void *file_from_map, *file_to_map;

	struct stat fileStat;

	if(fstat(fd_from,&fileStat) < 0){

		printf("\nERROR: fstat on the source file failed!\n");
		return 1;
	}

	int fd_from_size = fileStat.st_size;

	ftruncate(fd_to,fd_from_size);

	file_from_map = mmap(0, fileStat.st_size, PROT_READ, MAP_PRIVATE, fd_from, 0);

	if (file_from_map == MAP_FAILED){

		printf("\nERROR: mmap for the source file failed!\n");
	}

	file_to_map = mmap(0, fd_from_size, PROT_WRITE, MAP_SHARED, fd_to, 0);

	if (file_to_map  == MAP_FAILED){
		
		printf("\nERROR: mmap for the output file failed!\n");
	}

	if (memcpy(file_to_map, file_from_map, fd_from_size) == NULL){

		printf("\nERROR: memcpy() function failed!\n");
	}

	if (munmap(file_from_map, fd_from_size)){

		printf("\nERROR: munmap for the input file failed!\n");
	}

	if (munmap(file_to_map, fd_from_size)){

		printf("\nERROR: munmap for the output file failed!\n");
	}

	if (close(fd_from)){

		fprintf(stderr,"\nERROR: Can't close %s!\n", file_from);
	}
	if (close(fd_to)){

		fprintf(stderr,"\nERROR: Can't close %s!\n", file_to);
	}
}
