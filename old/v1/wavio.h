/*wavio.h*/

//include guard
#ifndef INCLUDED_WAVIO
#define INCLUDED_WAVIO

//include headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//extern "C"
#ifdef __cplusplus
extern "C"
{
#endif 

//Format chunk
typedef struct{
    char chunkID[4]; /* "fmt " */
    long chunkSize; /* 16 */
    short waveFormatType; /* PCM: 1 */
    short channel; /* Mono: 1, Stereo: 2 */
    unsigned long samplesPerSec; /* Sampling Frequency */
    long bytesPerSec; /* blockSize * samplesPerSec */
    short blockSize; /* bytes * channel (8bit Mono: 1, 8bit Stereo: 2, 16bit Mono: 2, 16bit Stereo: 4, 24bit Mono: 3, 24bit Stereo: 6, 32bit Mono: 4, 32bit Stereo: 8)*/
    short bitsPerSample; /* Quantization bits */
} FMT;

//Data chunk
typedef struct{
    char chunkID[4]; /* "data" */
    long chunkSize; /* sizeof(data) */
    int *data; /* Data */
} DATA;

//RIFF chunk
typedef struct{
    char chunkID[4]; /* "RIFF" */
    long chunkSize; /* data.chunkSize + 36 */
    char formType[4]; /* "WAVE" */
    FMT fmt; /* Format chunk */
    DATA data; /* Data chunk */
} RIFF;

//PCM specification(fs, bits, and length per channel)
typedef struct{
    unsigned long fs; /* Sampling Frequency */
    short bits; /* Quantization bits */
    int length; /* The number of samples per channel */
} PCM_SPEC;

//NATIVE PCM Stereo([-(2^(n-1)), 2^(n-1)-1])
typedef struct{
    PCM_SPEC pcm_spec;
    int *data[2]; /* Stereo Sound data */
} STEREO_PCM_NATIVE;

//PCM Stereo([-1, 1])
typedef struct{
    PCM_SPEC pcm_spec;
    double *data[2]; /* Stereo Sound Data */
} STEREO_PCM;

//NATIVE PCM Mono([-(2^(n-1)), 2^(n-1)-1])
typedef struct{
    PCM_SPEC pcm_spec;
    int *data; /* Mono PCM data */
} MONO_PCM_NATIVE;

//PCM Mono([-1, 1])
typedef struct{
    PCM_SPEC pcm_spec;
    double *data; /* Mono PCM */
} MONO_PCM;

//PCM Information(Filename, Fs, bits, channel)
typedef struct{
    char *filename; /* Filename */
    unsigned long fs; /* Sampling frequency */
    short bits; /* Quantization bits */
    short channel; /* channels */
} PCMINFO;

void free_RIFF(RIFF *riff){
    //free RIFF data vector
    free(riff->data.data);

    //free RIFF struct
    free(riff);
}

void free_Stereo_Native(STEREO_PCM_NATIVE *stereo_pcm_native){
    //free STEREO_PCM_NATIVE data vector
    free(stereo_pcm_native->data[0]);
    free(stereo_pcm_native->data[1]);

    //free STEREO_PCM_NATIVE struct
    free(stereo_pcm_native);
}

void free_Stereo(STEREO_PCM *stereo_pcm){
    //free STEREO_PCM data vector
    free(stereo_pcm->data[0]);
    free(stereo_pcm->data[1]);

    //free STEREO_PCM struct
    free(stereo_pcm);
}

void free_Mono_Native(MONO_PCM_NATIVE *mono_pcm_native){
    //free MONO_PCM_NATIVE data vector
    free(mono_pcm_native->data);

    //free MONO_PCM_NATIVE struct
    free(mono_pcm_native);
}

void free_Mono(MONO_PCM *mono_pcm){
    //free MONO_PCM data vector
    free(mono_pcm->data);

    //free MONO_PCM struct
    free(mono_pcm);
}

//Read RIFF, fmt, and data chunks
void wavread_RIFF(RIFF *riff, char *filename){
    FILE *fp; /* File pointer */
    unsigned long int i = 0; /* for loop */
    int *init_data; /* For initialize the pointer */
    int x;
    short xx;

    //open the file
    fp = fopen(filename, "rb");

    //judge if the file equals to RIFF chunk
    fread(riff->chunkID, 1, 4, fp);

    //if the file doesn't have RIFF format
    if(strncmp(riff->chunkID, "RIFF", 4) != 0){
        //Throw the error and end the program.
        printf("Error!: The file does not have RIFF chunk.\n");
        free(riff);
        exit(1);
    }

    //Read each chunk
    fread(&riff->chunkSize, 4, 1, fp);
    fread(riff->formType, 1, 4, fp);

    //if the file is not WAV file.
    if(strncmp(riff->formType, "WAVE", 4) != 0){
        //Throw the error and end the program.
        printf("Error!: The file is not WAV file.\n");
        free(riff);
        exit(1);
    }

    //jump unnecessary chunks.
    fread(riff->fmt.chunkID, 1, 4, fp);
    while(strncmp(riff->fmt.chunkID, "fmt ", 4) != 0){
        fseek(fp, -2, SEEK_CUR);
        fread(riff->fmt.chunkID, 1, 4, fp);
    }

    //Read fmt chunk
    fread(&riff->fmt.chunkSize, 4, 1, fp);
    fread(&riff->fmt.waveFormatType, 2, 1, fp);
    fread(&riff->fmt.channel, 2, 1, fp);
    fread(&riff->fmt.samplesPerSec, 4, 1, fp);
    fread(&riff->fmt.bytesPerSec, 4, 1, fp);
    fread(&riff->fmt.blockSize, 2, 1, fp);
    fread(&riff->fmt.bitsPerSample, 2, 1, fp);

    //jump unnecessary chunks.
    fread(riff->data.chunkID, 1, 4, fp);
    while (strncmp(riff->data.chunkID, "data", 4) != 0){
        fseek(fp, -2, SEEK_CUR);
        fread(riff->data.chunkID, 1, 4, fp);
    }

    //Read data chunk
    fread(&riff->data.chunkSize, 4, 1, fp);

    //Define data vector
    riff->data.data = (int *)calloc(riff->data.chunkSize / (riff->fmt.bitsPerSample / 8), sizeof(int));
    init_data = &riff->data.data[0];

    switch (riff->fmt.bitsPerSample)
    {
        //8bit (unsigned)
        case 8:
            for(i = 0; i < riff->data.chunkSize; i++){
                xx = 0;
                fread(&xx, 1, 1, fp);
                *riff->data.data = xx;
                //printf("%d\n", *riff->data.data);
                riff->data.data++;
            }
            riff->data.data = init_data;
            break;
    
        //16bit (signed)
        case 16:
            for(i = 0; i < riff->data.chunkSize / 2; i++){
                xx = 0;
                fread(&xx, 2, 1, fp);
                *riff->data.data = xx;
                //printf("%d\n", *riff->data.data);
                riff->data.data++;
            }
            riff->data.data = init_data;
            break;

        //24bit (signed)
        case 24:
            for(i = 0; i < riff->data.chunkSize / 3; i++){
                x = 0;
                fread(&x, 3, 1, fp);

                if(x >= 0x800000){
                    x -= 0x1000000;
                }

                *riff->data.data = x;
                //printf("%d\n", *riff->data.data);
                riff->data.data++;
            }
            riff->data.data = init_data;
            break;

        //32bit (signed)
        case 32:
            for(i = 0; i < riff->data.chunkSize / 4; i++){
                x = 0;
                fread(&x, 4, 1, fp);
                *riff->data.data = x;
                //printf("%d\n", *riff->data.data);
                riff->data.data++;
            }
            riff->data.data = init_data;
            break;

        //Error
        default:
            printf("Error!: Inappropriate quantization bit number.\n");
            free_RIFF(riff);
            exit(1);
            break;
    }

    //Close file
    fclose(fp);
}

//Read PCMINFO
void getPCMINFO(PCMINFO *pcminfo, char *filename){
    //Define RIFF struct
    RIFF *riff = (RIFF *)malloc(sizeof(RIFF));

    //get RIFF struct data
    wavread_RIFF(riff, filename);

    //copy properties
    pcminfo->filename = filename;
    pcminfo->fs = riff->fmt.samplesPerSec;
    pcminfo->bits = riff->fmt.bitsPerSample;
    pcminfo->channel = riff->fmt.channel;

    //free RIFF struct
    free_RIFF(riff);
}

void wavread_Stereo_Native(STEREO_PCM_NATIVE *stereo_pcm_native, char *filename){
    //Define RIFF struct
    RIFF *riff = (RIFF *)malloc(sizeof(RIFF));

    unsigned long int i = 0; /* For loop */
    int *L, *R; /* For initialize the data vector */

    //get RIFF struct data
    wavread_RIFF(riff, filename);

    //copy pcm_spec
    stereo_pcm_native->pcm_spec.fs = riff->fmt.samplesPerSec;
    stereo_pcm_native->pcm_spec.bits = riff->fmt.bitsPerSample;
    stereo_pcm_native->pcm_spec.length = riff->data.chunkSize / (2 * (riff->fmt.bitsPerSample / 8));

    //initialize the data vector
    stereo_pcm_native->data[0] = (int *)calloc(stereo_pcm_native->pcm_spec.length, sizeof(int));
    stereo_pcm_native->data[1] = (int *)calloc(stereo_pcm_native->pcm_spec.length, sizeof(int));
    L = &stereo_pcm_native->data[0][0];
    R = &stereo_pcm_native->data[1][0];

    //copy data vector from riff
    for(i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i += 2){
        *stereo_pcm_native->data[0] = riff->data.data[i];
        *stereo_pcm_native->data[1] = riff->data.data[i+1];

        stereo_pcm_native->data[0]++;
        stereo_pcm_native->data[1]++;
    }

    //reset vector index
    stereo_pcm_native->data[0] = L;
    stereo_pcm_native->data[1] = R;

    //free RIFF struct
    free_RIFF(riff);
}

void wavread_Stereo(STEREO_PCM *stereo_pcm, char *filename){
    //Define RIFF struct
    RIFF *riff = (RIFF *)malloc(sizeof(RIFF));

    unsigned long int i = 0; /* For loop */
    double *L, *R; /* for initialize the pointer */

    //get RIFF struct data
    wavread_RIFF(riff, filename);

    //copy PCM properties
    stereo_pcm->pcm_spec.fs = riff->fmt.samplesPerSec;
    stereo_pcm->pcm_spec.bits = riff->fmt.bitsPerSample;
    stereo_pcm->pcm_spec.length = riff->data.chunkSize / (2 * (riff->fmt.bitsPerSample / 8));

    //initialize the data vector
    stereo_pcm->data[0] = (double *)calloc(stereo_pcm->pcm_spec.length, sizeof(double));
    stereo_pcm->data[1] = (double *)calloc(stereo_pcm->pcm_spec.length, sizeof(double));
    L = &stereo_pcm->data[0][0];
    R = &stereo_pcm->data[1][0];

    //copy data from riff
    //if 8bit, because its data have unsigned.
    if(riff->fmt.bitsPerSample == 8){
        for(i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i += 2){
            *stereo_pcm->data[0] = (double)(riff->data.data[i] - 128);
            if(*stereo_pcm->data[0] >= 0.0){
                *stereo_pcm->data[0] /= 127.0;
            }else{
                *stereo_pcm->data[0] /= 128.0;
            }

            *stereo_pcm->data[1] = (double)(riff->data.data[i+1] - 128);
            if(*stereo_pcm->data[1] >= 0.0){
                *stereo_pcm->data[1] /= 127.0;
            }else{
                *stereo_pcm->data[1] /= 128.0;
            }

            stereo_pcm->data[0]++;
            stereo_pcm->data[1]++;
        }
    }else{
        //16, 24, and 32bit
        for (i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i += 2){
            *stereo_pcm->data[0] = (double)riff->data.data[i];
            if(*stereo_pcm->data[0] >= 0){
                *stereo_pcm->data[0] /= (pow(2.0, riff->fmt.bitsPerSample - 1) - 1.0);
            }else{
                *stereo_pcm->data[0] /= pow(2.0, riff->fmt.bitsPerSample - 1);
            }

            *stereo_pcm->data[1] = (double)riff->data.data[i+1];
            if (*stereo_pcm->data[1] >= 0){
                *stereo_pcm->data[1] /= (pow(2.0, riff->fmt.bitsPerSample - 1) - 1.0);
            }else{
                *stereo_pcm->data[1] /= pow(2.0, riff->fmt.bitsPerSample - 1);
            }

            stereo_pcm->data[0]++;
            stereo_pcm->data[1]++;
        }
    }

    //reset vector index
    stereo_pcm->data[0] = L;
    stereo_pcm->data[1] = R;

    //free RIFF struct
    free_RIFF(riff);
}

void wavread_Mono_Native(MONO_PCM_NATIVE *mono_pcm_native, char *filename){
    //Define RIFF struct
    RIFF *riff = (RIFF *)malloc(sizeof(RIFF));
    
    unsigned long int i = 0; /* For loop */
    int *M; /* for intitialize the data vector pointer */

    //get RIFF struct
    wavread_RIFF(riff, filename);

    //copy pcm_spec from riff
    mono_pcm_native->pcm_spec.fs = riff->fmt.samplesPerSec;
    mono_pcm_native->pcm_spec.bits = riff->fmt.bitsPerSample;
    mono_pcm_native->pcm_spec.length = riff->data.chunkSize / (riff->fmt.bitsPerSample / 8);

    //initialize the data vector
    mono_pcm_native->data = (int *)calloc(mono_pcm_native->pcm_spec.length, sizeof(int));
    M = &mono_pcm_native->data[0];

    //copy data from riff data
    for(i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i++){
        *mono_pcm_native->data = riff->data.data[i];
        mono_pcm_native->data++;
    }

    //set the pointer to first
    mono_pcm_native->data = M;

    //free RIFF struct
    free_RIFF(riff);
}

void wavread_Mono(MONO_PCM *mono_pcm, char *filename){
    //Define RIFF struct
    RIFF *riff = (RIFF *)malloc(sizeof(RIFF));

    unsigned long int i = 0; /* For loop */
    double *M; /* for initialize the data vector pointer */

    //get RIFF struct
    wavread_RIFF(riff, filename);

    //copy PCM_SPEC from RIFF
    mono_pcm->pcm_spec.fs = riff->fmt.samplesPerSec;
    mono_pcm->pcm_spec.bits = riff->fmt.bitsPerSample;
    mono_pcm->pcm_spec.length = riff->data.chunkSize / (riff->fmt.bitsPerSample / 8);

    //initialize the data vector
    mono_pcm->data = (double *)calloc(mono_pcm->pcm_spec.length, sizeof(double));
    M = &mono_pcm->data[0];

    //copy data from riff
    if(riff->fmt.bitsPerSample == 8){
        for(i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i++){
            *mono_pcm->data = (double)(riff->data.data[i] - 128);
            if(*mono_pcm->data >= 0){
                *mono_pcm->data /= 127.0;
            }else{
                *mono_pcm->data /= 128.0;
            }

            mono_pcm->data++;
        }
    }else{
        for(i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i++){
            *mono_pcm->data = (double)riff->data.data[i] / pow(2.0, riff->fmt.bitsPerSample - 1);
            if(*mono_pcm->data >= 0){
                *mono_pcm->data /= (pow(2.0, riff->fmt.bitsPerSample - 1) - 1.0);
            }else{
                *mono_pcm->data /= pow(2.0, riff->fmt.bitsPerSample - 1);
            }

            mono_pcm->data++;
        }
    }

    //set the pointer to first
    mono_pcm->data = M;

    //free RIFF struct
    free_RIFF(riff);
}

//save WAV file from RIFF struct
void wavwrite_RIFF(RIFF *riff, char *filename){
    //variable
    FILE *fp; /* for write wav file */
    unsigned long int i = 0; /* for loop */
    int *init_data; /* for initialize data vector */

    //open file name with writing name
    fp = fopen(filename, "wb");

    //write each chunk
    fwrite(riff->chunkID, 1, 4, fp); /* "RIFF" */
    fwrite(&riff->chunkSize, 4, 1, fp);
    fwrite(riff->formType, 1, 4, fp); /* "WAVE" */
    fwrite(riff->fmt.chunkID, 1, 4, fp); /* "fmt " */
    fwrite(&riff->fmt.chunkSize, 4, 1, fp);
    fwrite(&riff->fmt.waveFormatType, 2, 1, fp); /* PCM: 1 */
    fwrite(&riff->fmt.channel, 2, 1, fp); /* Mono: 1, Stereo: 2 */
    fwrite(&riff->fmt.samplesPerSec, 4, 1, fp); /* Sampling frequency */
    fwrite(&riff->fmt.bytesPerSec, 4, 1, fp);
    fwrite(&riff->fmt.blockSize, 2, 1, fp);
    fwrite(&riff->fmt.bitsPerSample, 2, 1, fp); /* Quantization bit */
    fwrite(riff->data.chunkID, 1, 4, fp);
    fwrite(&riff->data.chunkSize, 4, 1, fp); /* data */

    //Define data vector
    init_data = &riff->data.data[0];

    //write data
    switch(riff->fmt.bitsPerSample){
        case 8:
            //8bit integer(unsigned)
            for(i = 0; i < riff->data.chunkSize; i++){
                //clipping
                if(riff->data.data[i] > 255){
                    riff->data.data[i] = 255;
                }else if(riff->data.data[i] < 0){
                    riff->data.data[i] = 0;
                }

                //write data
                fwrite(&riff->data.data[i], 1, 1, fp);
            }
            break;

        case 16:
            //16bit integer(signed)
            for(i = 0; i < riff->data.chunkSize / 2; i++){
                //clipping
                if(riff->data.data[i] > 32767){
                    riff->data.data[i] = 32767;
                }else if(riff->data.data[i] < -32768){
                    riff->data.data[i] = -32768;
                }

                //write data
                fwrite(&riff->data.data[i], 2, 1, fp);
            }
            break;

        case 24:
            //24bit integer(signed)
            for(i = 0; i < riff->data.chunkSize / 3; i++){
                //clipping
                if(riff->data.data[i] > 8388607){
                    riff->data.data[i] = 8388607;
                }else if(riff->data.data[i] < -8388608){
                    riff->data.data[i] = -8388608;
                }

                //write data
                fwrite(&riff->data.data[i], 3, 1, fp);
            }
            break;

        case 32:
            for(i = 0; i < riff->data.chunkSize / 4; i++){
                //clipping
                if(riff->data.data[i] > (long int)(pow(2.0, 31.0) - 1)){
                    riff->data.data[i] = (int)(pow(2.0, 31.0) - 1);
                }else if(riff->data.data[i] < (long int)(-1 * pow(2.0, 31.0))){
                    riff->data.data[i] = -1 * (int)(pow(2.0, 31.0));
                }

                //write data
                fwrite(&riff->data.data[i], 4, 1, fp);
            }
            break;

        default:
            printf("Error!: Inappropriate quantization bit number.\n");
            exit(1);
            break;
    }

    //set the pointer to first
    riff->data.data = init_data;

    //save WAV file
    fclose(fp);
}

//save WAV file from STEREO_PCM_NATIVE struct
void wavwrite_Stereo_Native(STEREO_PCM_NATIVE *stereo_pcm_native, char *filename){
    //variables
    RIFF *riff; /* for make RIFF struct */
    long int i; /* for loop */
    int *L, *R; /* for initialize the pointer */

    //Define RIFF struct
    riff = (RIFF *)malloc(sizeof(RIFF));

    //RIFF chunk ID ("RIFF")
    riff->chunkID[0] = 'R';
    riff->chunkID[1] = 'I';
    riff->chunkID[2] = 'F';
    riff->chunkID[3] = 'F';

    //RIFF format type ("WAVE")
    riff->formType[0] = 'W';
    riff->formType[1] = 'A';
    riff->formType[2] = 'V';
    riff->formType[3] = 'E';

    //fmt chunk ID ("fmt ")
    riff->fmt.chunkID[0] = 'f';
    riff->fmt.chunkID[1] = 'm';
    riff->fmt.chunkID[2] = 't';
    riff->fmt.chunkID[3] = ' ';

    //fmt chunksize (16)
    riff->fmt.chunkSize = 16;

    //fmt waveformattype (1: PCM)
    riff->fmt.waveFormatType = 1;

    //fmt channel (2: Stereo)
    riff->fmt.channel = 2;

    //fmt samplespersec (sampling frequency)
    riff->fmt.samplesPerSec = stereo_pcm_native->pcm_spec.fs;

    //fmt blocksize
    riff->fmt.blockSize = 2 * (stereo_pcm_native->pcm_spec.bits / 8);

    //fmt bytespersec
    riff->fmt.bytesPerSec = riff->fmt.blockSize * riff->fmt.samplesPerSec;

    //fmt bitspersample
    riff->fmt.bitsPerSample = stereo_pcm_native->pcm_spec.bits;

    //data chunkID
    riff->data.chunkID[0] = 'd';
    riff->data.chunkID[1] = 'a';
    riff->data.chunkID[2] = 't';
    riff->data.chunkID[3] = 'a';

    //calculate each chunksize
    riff->data.chunkSize = 2 * stereo_pcm_native->pcm_spec.length * (riff->fmt.bitsPerSample / 8);
    riff->chunkSize = riff->data.chunkSize + 36;

    //copy data and calculate data chunk size
    L = stereo_pcm_native->data[0]; //for initialize pointer
    R = stereo_pcm_native->data[1]; //for initialize pointer

    //define riff data vector
    riff->data.data = (int *)calloc(riff->data.chunkSize / (riff->fmt.bitsPerSample / 8), sizeof(int));

    for (i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i += 2){
        //copy data
        riff->data.data[i] = *stereo_pcm_native->data[0];
        riff->data.data[i + 1] = *stereo_pcm_native->data[1];

        //next pointer
        stereo_pcm_native->data[0]++;
        stereo_pcm_native->data[1]++;
    }

    //initialize the pointer
    stereo_pcm_native->data[0] = L;
    stereo_pcm_native->data[1] = R;

    //wavwrite from RIFF struct
    wavwrite_RIFF(riff, filename);

    //free RIFF struct
    free_RIFF(riff);
}

//save WAV file from STEREO_PCM struct
void wavwrite_Stereo(STEREO_PCM *stereo_pcm, char *filename){
    //variables
    RIFF *riff;    //riff struct
    long int i;    //for loop
    double *L, *R; //for initialize index

    //initialize riff struct
    riff = (RIFF *)malloc(sizeof(RIFF));

    //RIFF chunk ID ("RIFF")
    riff->chunkID[0] = 'R';
    riff->chunkID[1] = 'I';
    riff->chunkID[2] = 'F';
    riff->chunkID[3] = 'F';

    //RIFF format type ("WAVE")
    riff->formType[0] = 'W';
    riff->formType[1] = 'A';
    riff->formType[2] = 'V';
    riff->formType[3] = 'E';

    //fmt chunk ID ("fmt ")
    riff->fmt.chunkID[0] = 'f';
    riff->fmt.chunkID[1] = 'm';
    riff->fmt.chunkID[2] = 't';
    riff->fmt.chunkID[3] = ' ';

    //fmt chunk size (16)
    riff->fmt.chunkSize = 16;

    //fmt wavefmttype (1: PCM)
    riff->fmt.waveFormatType = 1;

    //fmt channel (2: Stereo)
    riff->fmt.channel = 2;

    //fmt samplespersec (sampling frequency)
    riff->fmt.samplesPerSec = stereo_pcm->pcm_spec.fs;

    //fmt blocksize
    riff->fmt.blockSize = 2 * (stereo_pcm->pcm_spec.bits / 8);

    //fmt bytespersec
    riff->fmt.bytesPerSec = riff->fmt.blockSize * riff->fmt.samplesPerSec;

    //fmt bitspersample (Quantization bits)
    riff->fmt.bitsPerSample = stereo_pcm->pcm_spec.bits;

    //data chunk ID ("data")
    riff->data.chunkID[0] = 'd';
    riff->data.chunkID[1] = 'a';
    riff->data.chunkID[2] = 't';
    riff->data.chunkID[3] = 'a';

    //data chunk size
    riff->data.chunkSize = 2 * stereo_pcm->pcm_spec.length * (riff->fmt.bitsPerSample / 8);
    riff->chunkSize = riff->data.chunkSize + 36;

    //convert float to integer and copy data
    L = stereo_pcm->data[0]; //for initialize pointer
    R = stereo_pcm->data[1]; //for initialize pointer

    //Define riff data vector
    riff->data.data = (int *)calloc(riff->data.chunkSize / (riff->fmt.bitsPerSample / 8), sizeof(int));

    //copy data
    if(stereo_pcm->pcm_spec.bits == 8){
        //8bit (unsigned)
        for(i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i += 2){

            //clipping
            if(*stereo_pcm->data[0] < -1.0){
                *stereo_pcm->data[0] = -1.0;
            }else if(*stereo_pcm->data[0] > 1.0){
                *stereo_pcm->data[0] = 1.0;
            }

            if (*stereo_pcm->data[1] < -1.0){
                *stereo_pcm->data[1] = -1.0;
            }else if(*stereo_pcm->data[1] > 1.0){
                *stereo_pcm->data[1] = 1.0;
            }

            //8bit (unsigned)
            riff->data.data[i] = (int)(floor(((*stereo_pcm->data[0] + 1.0) / 2.0) * (pow(2.0, stereo_pcm->pcm_spec.bits) - 1) + 0.5));
            riff->data.data[i + 1] = (int)(floor(((*stereo_pcm->data[1] + 1.0) / 2.0) * (pow(2.0, stereo_pcm->pcm_spec.bits) - 1) + 0.5));

            //for debug
            //printf("%lf -> %d, %lf -> %d\n", *stereo_pcm->data[0], riff->data.data[i], *stereo_pcm->data[1], riff->data.data[i + 1]);

            //next pointer
            stereo_pcm->data[0]++;
            stereo_pcm->data[1]++;
        }
    }else{
        //16, 24, and 32bit
        for(i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i += 2){

            //clipping
            if (*stereo_pcm->data[0] < -1.0){
                *stereo_pcm->data[0] = -1.0;
            }else if (*stereo_pcm->data[0] > 1.0){
                *stereo_pcm->data[0] = 1.0;
            }

            if (*stereo_pcm->data[1] < -1.0){
                *stereo_pcm->data[1] = -1.0;
            }else if (*stereo_pcm->data[1] > 1.0){
                *stereo_pcm->data[1] = 1.0;
            }

            //copy data
            riff->data.data[i] = (int)(floor(((*stereo_pcm->data[0] + 1.0) / 2.0) * (pow(2.0, (double)stereo_pcm->pcm_spec.bits) - 1.0) + 0.5) - pow(2.0, (double)stereo_pcm->pcm_spec.bits - 1.0));
            riff->data.data[i + 1] = (int)(floor(((*stereo_pcm->data[1] + 1.0) / 2.0) * (pow(2.0, (double)stereo_pcm->pcm_spec.bits) - 1.0) + 0.5) - pow(2.0, (double)stereo_pcm->pcm_spec.bits - 1.0));

            //for debug
            //printf("%lf -> %d, %lf -> %d\n", *stereo_pcm->data[0], riff->data.data[i], *stereo_pcm->data[1], riff->data.data[i+1]);

            //next pointer
            stereo_pcm->data[0]++;
            stereo_pcm->data[1]++;
        }
    }

    //initialize the pointer
    stereo_pcm->data[0] = L;
    stereo_pcm->data[1] = R;

    //wavwrite from riff struct
    wavwrite_RIFF(riff, filename);

    //free riff struct
    free_RIFF(riff);
}

//save WAV file from MONO_PCM_NATIVE struct
void wavwrite_Mono_Native(MONO_PCM_NATIVE *mono_pcm_native, char *filename){
    //variables
    RIFF *riff; //for write wav file from RIFF struct
    long int i; //for loop
    int *data;  //for initialize the pointer

    //initialize RIFF struct
    riff = (RIFF *)malloc(sizeof(RIFF));

    //RIFF chunk ID ("RIFF")
    riff->chunkID[0] = 'R';
    riff->chunkID[1] = 'I';
    riff->chunkID[2] = 'F';
    riff->chunkID[3] = 'F';

    //RIFF format type ("WAVE")
    riff->formType[0] = 'W';
    riff->formType[1] = 'A';
    riff->formType[2] = 'V';
    riff->formType[3] = 'E';

    //fmt chunk ID ("fmt ")
    riff->fmt.chunkID[0] = 'f';
    riff->fmt.chunkID[1] = 'm';
    riff->fmt.chunkID[2] = 't';
    riff->fmt.chunkID[3] = ' ';

    //fmt chunk size (16)
    riff->fmt.chunkSize = 16;

    //fmt wavefmttype (1: PCM)
    riff->fmt.waveFormatType = 1;

    //fmt channel (1: Mono)
    riff->fmt.channel = 1;

    //fmt samplespersec (sampling frequency)
    riff->fmt.samplesPerSec = mono_pcm_native->pcm_spec.fs;

    //fmt blocksize
    riff->fmt.blockSize = (mono_pcm_native->pcm_spec.bits / 8);

    //fmt bytespersec
    riff->fmt.bytesPerSec = riff->fmt.blockSize * riff->fmt.samplesPerSec;

    //fmt bitspersample(Quantization bit)
    riff->fmt.bitsPerSample = mono_pcm_native->pcm_spec.bits;

    //data chunk ID ("data")
    riff->data.chunkID[0] = 'd';
    riff->data.chunkID[1] = 'a';
    riff->data.chunkID[2] = 't';
    riff->data.chunkID[3] = 'a';

    //data chunk size
    riff->data.chunkSize = mono_pcm_native->pcm_spec.length * (riff->fmt.bitsPerSample / 8);
    riff->chunkSize = riff->data.chunkSize + 36;

    //set the pointer for initialize
    data = &mono_pcm_native->data[0];

    //define data vector
    riff->data.data = (int *)calloc(riff->data.chunkSize / (riff->fmt.bitsPerSample / 8), sizeof(int));

    for (i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i++){
        //copy data
        riff->data.data[i] = *mono_pcm_native->data;

        //next pointer
        mono_pcm_native->data++;
    }

    //reset pointer
    mono_pcm_native->data = data;

    //save wav file from riff struct
    wavwrite_RIFF(riff, filename);

    //free riff struct
    free_RIFF(riff);
}

//save WAV file from STEREO_PCM_NATIVE struct
void wavwrite_Mono(MONO_PCM *mono_pcm, char *filename){
    //variables
    RIFF *riff;   //for write wav file from RIFF struct
    long int i;   //for loop
    double *data; //for initialize the pointer

    //initialize RIFF struct
    riff = (RIFF *)malloc(sizeof(RIFF));

    //RIFF chunk ID ("RIFF")
    riff->chunkID[0] = 'R';
    riff->chunkID[1] = 'I';
    riff->chunkID[2] = 'F';
    riff->chunkID[3] = 'F';

    //RIFF format type ("WAVE")
    riff->formType[0] = 'W';
    riff->formType[1] = 'A';
    riff->formType[2] = 'V';
    riff->formType[3] = 'E';

    //fmt chunk ID ("fmt ")
    riff->fmt.chunkID[0] = 'f';
    riff->fmt.chunkID[1] = 'm';
    riff->fmt.chunkID[2] = 't';
    riff->fmt.chunkID[3] = ' ';

    //fmt chunk size (16)
    riff->fmt.chunkSize = 16;

    //fmt wavefmttype (1: PCM)
    riff->fmt.waveFormatType = 1;

    //fmt channel (1: Mono)
    riff->fmt.channel = 1;

    //fmt samplespersec (sampling frequency)
    riff->fmt.samplesPerSec = mono_pcm->pcm_spec.fs;

    //fmt blocksize
    riff->fmt.blockSize = (mono_pcm->pcm_spec.bits / 8);

    //fmt bytespersec
    riff->fmt.bytesPerSec = riff->fmt.blockSize * riff->fmt.samplesPerSec;

    //fmt bitspersample(Quantization bit)
    riff->fmt.bitsPerSample = mono_pcm->pcm_spec.bits;

    //data chunk ID ("data")
    riff->data.chunkID[0] = 'd';
    riff->data.chunkID[1] = 'a';
    riff->data.chunkID[2] = 't';
    riff->data.chunkID[3] = 'a';

    //data chunk size
    riff->data.chunkSize = mono_pcm->pcm_spec.length * (riff->fmt.bitsPerSample / 8);
    riff->chunkSize = riff->data.chunkSize + 36;

    //set the pointer for initialize
    data = &mono_pcm->data[0];

    //define data vector
    riff->data.data = (int *)calloc(riff->data.chunkSize / (riff->fmt.bitsPerSample / 8), sizeof(int));

    if(mono_pcm->pcm_spec.bits == 8){
        for(i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i++){
            //clipping
            if(*mono_pcm->data < -1.0){
                *mono_pcm->data = -1.0;
            }else if(*mono_pcm->data >1.0){
                *mono_pcm->data = 1.0;
            }

            //copy data
            riff->data.data[i] = (int)(floor(floor(((*mono_pcm->data + 1.0) / 2.0) * (pow(2.0, riff->fmt.bitsPerSample) - 1.0) + 0.5)));
        
            //next pointer
            mono_pcm->data++;
        }
    }else{
        for (i = 0; i < riff->data.chunkSize / (riff->fmt.bitsPerSample / 8); i++){
            //clipping
            if (*mono_pcm->data < -1.0){
                *mono_pcm->data = -1.0;
            }else if (*mono_pcm->data > 1.0){
                *mono_pcm->data = 1.0;
            }

            //copy data
            riff->data.data[i] = (int)(floor(((*mono_pcm->data + 1.0) / 2.0) * (pow(2.0, riff->fmt.bitsPerSample) - 1.0) + 0.5) - (pow(2.0, riff->fmt.bitsPerSample - 1.0)));
        
            //next pointer
            mono_pcm->data++;
        }
    }

    //reset pointer
    mono_pcm->data = data;

    //save wav file from riff struct
    wavwrite_RIFF(riff, filename);

    //free riff struct
    free_RIFF(riff);
}

//close extern "C"
#ifdef __cplusplus
}
#endif

//close include guard
#endif