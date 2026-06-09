#include<stdio.h>
#include<stdlib.h>
#include<string.h>




typedef struct{
	int times;
	int len;
}tag_t;


void print_buff(char* buff,int len){
	for(int i=0;i<len;i++){
		printf("%c",buff[i]);
	}
}
#define debug(...) fprintf(stderr,__VA_ARGS__);
int find_pattern(tag_t* tag,char* data,int len){
	for(int index=0;index<len;index++){
		char* buff=data+index;
		int pattern_len=1;
		int buff_len=(len-index);
		int len_half=buff_len/2;
		char found=0;
		while(pattern_len<=len_half){
			if(memcmp(buff,buff+pattern_len,pattern_len)!=0){
				pattern_len++;
			}
			else{
				found=1;
				break;
			}
		}
		if(!found){
			continue;
		}
		int pattern_times=0;
		int offset=0;
		while(offset<buff_len){
			if(memcmp(buff,buff+offset,pattern_len)==0){
				offset=offset+pattern_len;
				pattern_times++;
			}else{
				break;
			}
		}
		tag->times=pattern_times;
		tag->len=pattern_len;
		return index;
	}
	tag->times=1;
	tag->len=len;
	return 0;

}


void decompress(){

	FILE* infile=fopen("compressed.dat","rb");
	int buff_size=4096;
	char* buff=malloc(buff_size);
	while(1){
		tag_t tag;
		fread(&tag,sizeof(tag_t),1,infile);
		if(tag.len<=0 && tag.times<=0){
			break;
		}
		debug("READ: len=%d times=%d\n",tag.len,tag.times);
		if(buff_size<=tag.len){
			while(buff_size<=tag.len){
				buff_size*=2;
			}
			char* tmp=realloc(buff,buff_size);
			if(tmp==NULL){
				perror("realloc()");
				return;
			}
			buff=tmp;
		}
		fread(buff,tag.len,1,infile);
		for(int i=0;i<tag.times;i++){
			print_buff(buff,tag.len);
		}

	}
	free(buff);
	buff=NULL;
}

void compress(char* data,int len){
	FILE* outfile=fopen("compressed.dat","wb");
	if(outfile==NULL){
		perror("fopen()");
		return;
	}
	tag_t mark;
	int index=0;
	while(index<len){
		char* start=data+index;
		int len_actual=len-index;
		int pos=find_pattern(&mark,start,len_actual);
		if(pos!=0){
			tag_t remain;
			remain.len=pos;
			remain.times=1;
			debug("WRITE: len=%d times=%d\n",remain.len,remain.times);
			fwrite(&remain,sizeof(tag_t),1,outfile);
			fwrite(start,pos,1,outfile);
			index+=pos;
		}
		debug("WRITE: len=%d times=%d\n",mark.len,mark.times);
		fwrite(&mark,sizeof(tag_t),1,outfile);
		fwrite(start+pos,mark.len,1,outfile);
		index+=(mark.times*mark.len);
	}
	tag_t finisher={0,0};
	fwrite(&finisher,sizeof(tag_t),1,outfile);
	fclose(outfile);
}

char* read_file(char* file_name,int* len){
	int size=4096;
	char* buff=malloc(size);
	if(buff==NULL){
		perror("malloc");
		exit(1);
	}
	FILE* file=fopen(file_name,"rb");
	if(file==NULL){
		perror("fopen");
		exit(1);
	}
	int index=0;
	for(;;index++){
		char ch=fgetc(file);
		if(ch==EOF){
			debug("Stopped at: %d\n",index);
			break;
		}
		while(size<index){
			size*=2;
			char* tmp=realloc(buff,size);
			if(tmp==NULL){
				perror("realloc()");
				exit(1);
			}
			buff=tmp;
		}
		buff[index]=ch;
	}
	*len=index;
	fclose(file);
	return buff;
}




int main(){
	int len;

	char* data=read_file("input.txt",&len);
	compress(data,len);
	decompress();
	free(data);
	data=NULL;
	return 0;
}

