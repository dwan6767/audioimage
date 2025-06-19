#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sndfile.h>

#define SAMPLE_RATE 8000
#define TONE_DURATION_SEC 0.003f
#define TONE_MIN_FREQ 400.0f
#define TONE_MAX_FREQ 2000.0f
#define TONE_STEP_HZ 10.0f
#define TONES_PER_ROW 100  // set this to your image width

// Goertzel algorithm to detect power at target frequency
double goertzel(const float *data, int N, double target_freq) {
    double k = (int)(0.5 + ((N * target_freq) / SAMPLE_RATE));
    double w = (2.0 * M_PI * k) / N;
    double cos_w = cos(w);
    double coeff = 2.0 * cos_w;
    double q0 = 0.0, q1 = 0.0, q2 = 0.0;
    for (int i = 0; i < N; i++) {
        q0 = coeff * q1 - q2 + data[i];
        q2 = q1;
        q1 = q0;
    }
    return q1*q1 + q2*q2 - q1*q2*coeff;
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s input.wav\n", argv[0]);
        return 1;
    }

    // Open input WAV
    SF_INFO sfinfo;
    SNDFILE *infile = sf_open(argv[1], SFM_READ, &sfinfo);
    if (!infile) {
        fprintf(stderr, "Error opening file '%s'\n", argv[1]);
        return 1;
    }
    if (sfinfo.channels != 1) {
        fprintf(stderr, "Only mono WAV supported. Channels = %d\n", sfinfo.channels);
        sf_close(infile);
        return 1;
    }

    int samples_per_tone = (int)(SAMPLE_RATE * TONE_DURATION_SEC);
    int total_samples = sfinfo.frames;
    int total_tones = total_samples / samples_per_tone;

    int img_width = TONES_PER_ROW;
    int img_height = (total_tones + img_width - 1) / img_width;

    // Allocate buffers
    float *buffer = malloc(samples_per_tone * sizeof(float));
    unsigned char *image = malloc(img_width * img_height * sizeof(unsigned char));
    if (!buffer || !image) {
        fprintf(stderr, "Memory allocation failed\n");
        sf_close(infile);
        return 1;
    }

    // Process each tone chunk
    for (int i = 0; i < total_tones; i++) {
        sf_readf_float(infile, buffer, samples_per_tone);
        // Detect frequency with max power
        double best_mag = 0.0;
        double best_freq = TONE_MIN_FREQ;
        for (double f = TONE_MIN_FREQ; f <= TONE_MAX_FREQ; f += TONE_STEP_HZ) {
            double mag = goertzel(buffer, samples_per_tone, f);
            if (mag > best_mag) {
                best_mag = mag;
                best_freq = f;
            }
        }
        // Map freq to brightness
        double norm = (best_freq - TONE_MIN_FREQ) / (TONE_MAX_FREQ - TONE_MIN_FREQ);
        if (norm < 0.0) norm = 0.0;
        if (norm > 1.0) norm = 1.0;
        image[i] = (unsigned char)(norm * 255.0);
    }

    // Clean up WAV
    sf_close(infile);
    free(buffer);

    // Write PNG
    char outname[64];
    snprintf(outname, sizeof(outname), "decoded.png");
    if (!stbi_write_png(outname, img_width, img_height, 1, image, img_width)) {
        fprintf(stderr, "Failed to write image\n");
        free(image);
        return 1;
    }
    free(image);

    printf("Decoded image written to %s (%dx%d)\n", outname, img_width, img_height);
    return 0;
}
