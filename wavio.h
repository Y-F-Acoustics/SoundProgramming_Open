/*wavio.h (Beta)*/

//include guard
#ifndef INCLUDED_WAVIO
#define INCLUDED_WAVIO

#include <stdint.h>

//extern "C"
#ifdef __cplusplus
extern "C"
{
#endif

//Format chunk
typedef struct{
    char chunkID[4]; /* "fmt " */
    int64_t chunkSize; /* 16 */
    int16_t waveFormatType; /* PCM: 1 */
    int16_t channel; /* Mono: 1, Stereo: 2 */
    uint64_t samplesPerSec; /* Sampling Frequency */
    int64_t bytesPerSec; /* blockSize * samplesPerSec */
    int16_t blockSize; /* bytes * channel (8bit Mono: 1, 8bit Stereo: 2, 16bit Mono: 2, 16bit Stereo: 4, 24bit Mono: 3, 24bit Stereo: 6, 32bit Mono: 4, 32bit Stereo: 8)*/
    int16_t bitsPerSample; /* Quantization bits */
} FMT;

//Data chunk
typedef struct{
    char chunkID[4]; /* "data" */
    uint32_t chunkSize; /* sizeof(data) */
    int32_t *data; /* Data */
} DATA;

//RIFF chunk
typedef struct{
    char chunkID[4]; /* "RIFF" */
    int64_t chunkSize; /* data.chunkSize + 36 */
    char formType[4]; /* "WAVE" */
    FMT fmt; /* Format chunk */
    DATA data; /* Data chunk */
} RIFF;

//PCM specification(fs, bits, and length per channel)
typedef struct{
    unsigned long fs; /* Sampling Frequency */
    int16_t bits; /* Quantization bits */
    int32_t length; /* The number of samples per channel */
} PCM_SPEC;

//NATIVE PCM Stereo([-(2^(n-1)), 2^(n-1)-1])
typedef struct{
    PCM_SPEC pcm_spec;
    int32_t *data[2]; /* Stereo Sound data */
} STEREO_PCM_NATIVE;

//PCM Stereo([-1, 1])
typedef struct{
    PCM_SPEC pcm_spec;
    double *data[2]; /* Stereo Sound Data */
} STEREO_PCM;

//NATIVE PCM Mono([-(2^(n-1)), 2^(n-1)-1])
typedef struct{
    PCM_SPEC pcm_spec;
    int32_t *data; /* Mono PCM data */
} MONO_PCM_NATIVE;

//PCM Mono([-1, 1])
typedef struct{
    PCM_SPEC pcm_spec;
    double *data; /* Mono PCM */
} MONO_PCM;

//PCM Information(Filename, Fs, bits, channel)
typedef struct{
    char *filename; /* Filename */
    uint64_t fs; /* Sampling frequency */
    int16_t bits; /* Quantization bits */
    int16_t channel; /* channels */
} PCMINFO;

//Prototype declaration for wavio.c
/* using RIFF struct */ 
RIFF *alloc_RIFF(void);
void free_RIFF(RIFF *riff);
void wavread_RIFF(RIFF *riff, char *filename);
void wavwrite_RIFF(RIFF *riff, char *filename);

/* using MONO_PCM_NATIVE struct*/
MONO_PCM_NATIVE *alloc_Mono_Native(void);
void free_Mono_Native(MONO_PCM_NATIVE *mono_pcm_native);
void wavread_Mono_Native(MONO_PCM_NATIVE *mono_pcm_native, char *filename);
void wavwrite_Mono_Native(MONO_PCM_NATIVE *mono_pcm_native, char *filename);

/* using MONO_PCM struct */
MONO_PCM *alloc_Mono(void);
void free_Mono(MONO_PCM *mono_pcm);
void wavread_Mono(MONO_PCM *mono_pcm, char *filename);
void wavwrite_Mono(MONO_PCM *mono_pcm, char *filename);

/* using STEREO_PCM_NATIVE struct */
STEREO_PCM_NATIVE *alloc_Stereo_Native(void);
void free_Stereo_Native(STEREO_PCM_NATIVE *stereo_pcm_native);
void wavread_Stereo_Native(STEREO_PCM_NATIVE *stereo_pcm_native, char *filename);
void wavwrite_Stereo_Native(STEREO_PCM_NATIVE *stereo_pcm_native, char *filename);

/* using STEREO_PCM struct */
STEREO_PCM *alloc_Stereo(void);
void free_Stereo(STEREO_PCM *stereo_pcm);
void wavread_Stereo(STEREO_PCM *stereo_pcm, char *filename);
void wavwrite_Stereo(STEREO_PCM *stereo_pcm, char *filename);

/* others */
void getPCMINFO(PCMINFO *pcminfo, char *filename);


#ifdef __cplusplus
}
#endif

//close include guard
#endif