/* GPU-based image processing: sequential reference workflow. Build your own parallel version.
 * Usage: reference OUTPUT_DIRECTORY INPUT1.ppm [INPUT2.ppm ...]
 * Create OUTPUT_DIRECTORY first. Input/output use binary RGB PPM (P6).
 * PROJECT_NUMBER chooses only the serial recipe below. No parallel API is used. */
#include "studio.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PROJECT_NUMBER 7
static int token(FILE *file,char *out,size_t capacity){
    int c;do{c=fgetc(file);if(c=='#')while(c!='\n'&&c!=EOF)c=fgetc(file);}while(c!=EOF&&isspace(c));
    size_t n=0;while(c!=EOF&&!isspace(c)){if(n+1>=capacity)return 0;out[n++]=(char)c;c=fgetc(file);}out[n]=0;
    if(c=='\r'){int next=fgetc(file);if(next!='\n'&&next!=EOF)ungetc(next,file);}return n>0;
}
static int load(const char *path,StudioImage *image){
    FILE *f=fopen(path,"rb");if(!f)return 1;char word[32];int w,h,status=1;
    if(!token(f,word,32)||strcmp(word,"P6"))goto done;
    if(!token(f,word,32))goto done;w=atoi(word);
    if(!token(f,word,32))goto done;h=atoi(word);
    if(!token(f,word,32)||strcmp(word,"255"))goto done;
    {StudioOptions defaults=studio_options_default();int ow,oh;
     if(studio_target_dimensions(w,h,&defaults,&ow,&oh))goto done;}
    image->width=w;image->height=h;image->pixels=(unsigned char*)malloc((size_t)w*h*3);
    if(image->pixels&&fread(image->pixels,1,(size_t)w*h*3,f)==(size_t)w*h*3)status=0;
done:fclose(f);if(status)studio_image_free(image);return status;
}
static int save(const char *path,const StudioImage *image){
    FILE *f=fopen(path,"wb");if(!f)return 1;
    fprintf(f,"P6\n%d %d\n255\n",image->width,image->height);
    size_t bytes=(size_t)image->width*image->height*3;
    int failed=fwrite(image->pixels,1,bytes,f)!=bytes;failed|=fclose(f)!=0;return failed;
}
int main(int argc,char **argv){
    if(argc<3){fprintf(stderr,"Usage: reference OUTPUT_DIRECTORY INPUT1.ppm [INPUT2.ppm ...]\n");return 1;}
    int outputs=0;double begin=studio_clock_ms();
    /* One source image at a time; one variant at a time; no worker threads. */
    for(int input_index=2;input_index<argc;++input_index){
        StudioImage source={0};if(load(argv[input_index],&source)){fprintf(stderr,"Cannot load %s\n",argv[input_index]);return 1;}
        int count=PROJECT_NUMBER==4?12:1;
        for(int variant=0;variant<count;++variant){
            StudioOptions recipe=studio_options_default();
            if(PROJECT_NUMBER==2)recipe.resize_width=640; /* bilinear reference */
            else if(PROJECT_NUMBER==3)recipe.filters[0]=4; /* box blur; use 2 for sharpen */
            else if(PROJECT_NUMBER==4){
                const int widths[3]={160,320,640};recipe.resize_width=widths[variant%3];
                recipe.brightness=(variant/3-2)*10; /* 12 distinct deterministic recipes */
            }else if(PROJECT_NUMBER==7){
                recipe.brightness=10;recipe.interpolation=0;
                /* Test the other CPU baselines separately: resize_width=640, or filters[0]=4. */
            }else{
                recipe.resize_width=320;recipe.brightness=10;recipe.filters[0]=4;
                /* Change the target/recipe to match your recorded experiment. */
            }
            /* Sequential pipeline: load above -> resize/color/filter here -> save below. */
            StudioImage result={0};int status=studio_image_process(source.pixels,source.width,source.height,&recipe,1,&result);
            if(status){studio_image_free(&source);return status;}
            char destination[4096];int length=snprintf(destination,sizeof(destination),"%s/out-%03d-%03d.ppm",argv[1],input_index-1,variant+1);
            if(length<0||(size_t)length>=sizeof(destination)||save(destination,&result)){
                studio_image_free(&result);studio_image_free(&source);return 1;
            }
            studio_image_free(&result);++outputs;
        }
        studio_image_free(&source);
    }
    double seconds=(studio_clock_ms()-begin)/1000.0;
    /* Diagnostic printing is outside the measured load-to-save interval. */
    printf("Sequential full program: %.6f s; %d outputs; %.3f outputs/s\n",seconds,outputs,seconds>0?outputs/seconds:0);
    return 0;
}
