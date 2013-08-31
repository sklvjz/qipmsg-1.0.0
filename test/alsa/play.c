#define _GNU_SOURCE
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <limits.h>

#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif

#define DEFAULT_FORMAT      SND_PCM_FORMAT_U8
#define DEFAULT_SPEED       8000

#define FORMAT_DEFAULT      -1
#define FORMAT_RAW          0
#define FORMAT_WAVE         1


static snd_pcm_t *handle;
static int open_mode = 0;
static int verbose = 0;
static int nonblock = 0;
static snd_pcm_uframes_t chunk_size = 0;
static struct {
    snd_pcm_format_t format;
    unsigned int channels;
    unsigned int rate;
} hwparams, rhwparams;
static u_char *audiobuf = NULL;
static unsigned period_time = 0;
static unsigned buffer_time = 0;
static unsigned int sleep_min = 0;
static snd_pcm_uframes_t period_frames = 0;
static snd_pcm_uframes_t buffer_frames = 0;
static off64_t pbrec_count = LLONG_MAX, fdcount;
static int fd = -1;
static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
static snd_output_t *log;
static int timelimit = 0;
static size_t chunk_bytes;
static size_t bits_per_sample, bits_per_frame;
static int quiet_mode = 0;
static int avail_min = -1;
static int start_delay = 0;
static int stop_delay = 0;

typedef struct au_header {
    u_int magic;        /* '.snd' */
    u_int hdr_size;     /* size of header (min 24) */
    u_int data_size;    /* size of data */
    u_int encoding;     /* see to AU_FMT_XXXX */
    u_int sample_rate;  /* sample rate */
    u_int channels;     /* number of channels (voices) */
} AuHeader;

/* it's in chunks like .voc and AMIGA iff, but my source say there
   are in only in this combination, so I combined them in one header;
   it works on all WAVE-file I have
   */
typedef struct {
    u_int magic;        /* 'RIFF' */
    u_int length;       /* filelen */
    u_int type;         /* 'WAVE' */
} WaveHeader;

typedef struct {
    u_short format;     /* should be 1 for PCM-code */
    u_short modus;      /* 1 Mono, 2 Stereo */
    u_int sample_fq;    /* frequence of sample */
    u_int byte_p_sec;
    u_short byte_p_spl; /* samplesize; 1 or 2 bytes */
    u_short bit_p_spl;  /* 8, 12 or 16 bit */
} WaveFmtBody;

typedef struct {
    u_int type;         /* 'data' */
    u_int length;       /* samplecount */
} WaveChunkHeader;

struct fmt {
    char *what;
} fmt_table[] = {
    "raw data",
    "WAVE"
};

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define COMPOSE_ID(a,b,c,d) ((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#define LE_SHORT(v)     (v)
#define LE_INT(v)       (v)
#define BE_SHORT(v)     bswap_16(v)
#define BE_INT(v)       bswap_32(v)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define COMPOSE_ID(a,b,c,d) ((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#define LE_SHORT(v)     bswap_16(v)
#define LE_INT(v)       bswap_32(v)
#define BE_SHORT(v)     (v)
#define BE_INT(v)       (v)
#else
#error "Wrong endian"
#endif

#define WAV_RIFF        COMPOSE_ID('R','I','F','F')
#define WAV_WAVE        COMPOSE_ID('W','A','V','E')
#define WAV_FMT         COMPOSE_ID('f','m','t',' ')
#define WAV_DATA        COMPOSE_ID('d','a','t','a')
#define WAV_PCM_CODE    1

/* needed prototypes */
static void playback(char *filename);

#define check_wavefile_space(buffer, len, blimit) \
    if (len > blimit) { \
        blimit = len; \
        if ((buffer = realloc(buffer, blimit)) == NULL) { \
            printf(("not enough memory"));        \
            exit(EXIT_FAILURE); \
        } \
    }

/* setting the globals for playing raw data */
static void init_raw_data(void)
{
    hwparams = rhwparams;
}

static void set_params(void)
{
    snd_pcm_hw_params_t *params;
    snd_pcm_sw_params_t *swparams;
    snd_pcm_uframes_t buffer_size;
	snd_pcm_uframes_t xfer_align;
    int err;
    size_t n;
    unsigned int rate;
    snd_pcm_uframes_t start_threshold, stop_threshold;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_sw_params_alloca(&swparams);
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
        printf(("Broken configuration for this PCM: no configurations available"));
        exit(EXIT_FAILURE);
    }
    err = snd_pcm_hw_params_set_access(handle, params,
                                       SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        printf(("Access type not available"));
        exit(EXIT_FAILURE);
    }
    err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
    if (err < 0) {
        printf(("Sample format non available"));
        exit(EXIT_FAILURE);
    }
    err = snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);
    if (err < 0) {
        printf(("Channels count non available"));
        exit(EXIT_FAILURE);
    }

#if 0
    err = snd_pcm_hw_params_set_periods_min(handle, params, 2);
    assert(err >= 0);
#endif
    rate = hwparams.rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &hwparams.rate, 0);
    assert(err >= 0);
    if ((float)rate * 1.05 < hwparams.rate || (float)rate * 0.95 > hwparams.rate) {
        if (!quiet_mode) {
            char plugex[64];
            const char *pcmname = snd_pcm_name(handle);
            fprintf(stderr, ("Warning: rate is not accurate (requested = %iHz, got = %iHz)\n"), rate, hwparams.rate);
            if (! pcmname || strchr(snd_pcm_name(handle), ':'))
                *plugex = 0;
            else
                snprintf(plugex, sizeof(plugex), "(-Dplug:%s)",
                         snd_pcm_name(handle));
            fprintf(stderr, ("         please, try the plug plugin %s\n"),
                    plugex);
        }
    }
    rate = hwparams.rate;
    if (buffer_time == 0 && buffer_frames == 0) {
        err = snd_pcm_hw_params_get_buffer_time_max(params,
                                                    &buffer_time, 0);
        assert(err >= 0);
        if (buffer_time > 500000)
            buffer_time = 500000;
    }
    if (period_time == 0 && period_frames == 0) {
        if (buffer_time > 0)
            period_time = buffer_time / 4;
        else
            period_frames = buffer_frames / 4;
    }
    if (period_time > 0)
        err = snd_pcm_hw_params_set_period_time_near(handle, params,
                                                     &period_time, 0);
    else
        err = snd_pcm_hw_params_set_period_size_near(handle, params,
                                                     &period_frames, 0);
    assert(err >= 0);
    if (buffer_time > 0) {
        err = snd_pcm_hw_params_set_buffer_time_near(handle, params,
                                                     &buffer_time, 0);
    } else {
        err = snd_pcm_hw_params_set_buffer_size_near(handle, params,
                                                     &buffer_frames);
    }
    assert(err >= 0);
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        printf(("Unable to install hw params:"));
        snd_pcm_hw_params_dump(params, log);
        exit(EXIT_FAILURE);
    }
    snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
    if (chunk_size == buffer_size) {
        printf(("Can't use period equal to buffer size (%lu == %lu)"),
              chunk_size, buffer_size);
        exit(EXIT_FAILURE);
    }
    snd_pcm_sw_params_current(handle, swparams);

    if (strcmp(snd_asoundlib_version(), "1.0.15") == 0) {
        err = snd_pcm_sw_params_get_xfer_align(swparams, &xfer_align);
        if (err < 0) {
            printf(("Unable to obtain xfer align\n"));
            exit(EXIT_FAILURE);
        }
        if (sleep_min)
            xfer_align = 1;
        err = snd_pcm_sw_params_set_sleep_min(handle, swparams,
                                              sleep_min);
        assert(err >= 0);
    }
    if (avail_min < 0)
        n = chunk_size;
    else
        n = (double) rate * avail_min / 1000000;
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, n);

    /* round up to closest transfer boundary */
    if (strcmp(snd_asoundlib_version(), "1.0.15") == 0) {
        n = (buffer_size / xfer_align) * xfer_align;
    } else {
        n = buffer_size;
    }
    if (start_delay <= 0) {
        start_threshold = n + (double) rate * start_delay / 1000000;
    } else
        start_threshold = (double) rate * start_delay / 1000000;
    if (start_threshold < 1)
        start_threshold = 1;
    if (start_threshold > n)
        start_threshold = n;
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);
    assert(err >= 0);
    if (stop_delay <= 0)
        stop_threshold = buffer_size + (double) rate * stop_delay / 1000000;
    else
        stop_threshold = (double) rate * stop_delay / 1000000;
    err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold);
    assert(err >= 0);

    if (strcmp(snd_asoundlib_version(), "1.0.15") == 0) {
        err = snd_pcm_sw_params_set_xfer_align(handle, swparams, xfer_align);
        assert(err >= 0);
    }

    if (snd_pcm_sw_params(handle, swparams) < 0) {
        printf(("unable to install sw params:"));
        snd_pcm_sw_params_dump(swparams, log);
        exit(EXIT_FAILURE);
    }

    if (verbose)
        snd_pcm_dump(handle, log);

    bits_per_sample = snd_pcm_format_physical_width(hwparams.format);
    bits_per_frame = bits_per_sample * hwparams.channels;
    chunk_bytes = chunk_size * bits_per_frame / 8;
    audiobuf = realloc(audiobuf, chunk_bytes);
    if (audiobuf == NULL) {
        printf(("not enough memory"));
        exit(EXIT_FAILURE);
    }
    // fprintf(stderr, "real chunk_size = %i, frags = %i, total = %i\n", chunk_size, setup.buf.block.frags, setup.buf.block.frags * chunk_size);
}

/* I/O suspend handler */
static void suspend(void)
{
    int res;

    if (!quiet_mode)
        fprintf(stderr, ("Suspended. Trying resume. ")); fflush(stderr);
    while ((res = snd_pcm_resume(handle)) == -EAGAIN)
        sleep(1);   /* wait until suspend flag is released */
    if (res < 0) {
        if (!quiet_mode)
            fprintf(stderr, ("Failed. Restarting stream. ")); fflush(stderr);
        if ((res = snd_pcm_prepare(handle)) < 0) {
            printf(("suspend: prepare error: %s"), snd_strerror(res));
            exit(EXIT_FAILURE);
        }
    }
    if (!quiet_mode)
        fprintf(stderr, ("Done.\n"));
}

#ifndef timersub
#define timersub(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) { \
            --(result)->tv_sec; \
            (result)->tv_usec += 1000000; \
        } \
    } while (0)
#endif

/* I/O error handler */
static void xrun(void)
{
    snd_pcm_status_t *status;
    int res;

    snd_pcm_status_alloca(&status);
    if ((res = snd_pcm_status(handle, status))<0) {
        printf(("status error: %s"), snd_strerror(res));
        exit(EXIT_FAILURE);
    }
    if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
        struct timeval now, diff, tstamp;
        gettimeofday(&now, 0);
        snd_pcm_status_get_trigger_tstamp(status, &tstamp);
        timersub(&now, &tstamp, &diff);
        fprintf(stderr, ("%s!!! (at least %.3f ms long)\n"),
                stream == SND_PCM_STREAM_PLAYBACK ? ("underrun") : ("overrun"),
                diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
        if (verbose) {
            fprintf(stderr, ("Status:\n"));
            snd_pcm_status_dump(status, log);
        }
        if ((res = snd_pcm_prepare(handle))<0) {
            printf(("xrun: prepare error: %s"), snd_strerror(res));
            exit(EXIT_FAILURE);
        }
        return;     /* ok, data should be accepted again */
    } if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
        if (verbose) {
            fprintf(stderr, ("Status(DRAINING):\n"));
            snd_pcm_status_dump(status, log);
        }
        if (stream == SND_PCM_STREAM_CAPTURE) {
            fprintf(stderr, ("capture stream format change? attempting recover...\n"));
            if ((res = snd_pcm_prepare(handle))<0) {
                printf(("xrun(DRAINING): prepare error: %s"), snd_strerror(res));
                exit(EXIT_FAILURE);
            }
            return;
        }
    }
    if (verbose) {
        fprintf(stderr, ("Status(R/W):\n"));
        snd_pcm_status_dump(status, log);
    }
    printf(("read/write error, state = %s"), snd_pcm_state_name(snd_pcm_status_get_state(status)));
    exit(EXIT_FAILURE);
}

static ssize_t pcm_write(u_char *data, size_t count)
{
    ssize_t r;
    ssize_t result = 0;

    if (strcmp(snd_asoundlib_version(), "1.0.15") == 0) {
        if (sleep_min == 0 &&
            count < chunk_size) {
            snd_pcm_format_set_silence(hwparams.format, data + count * bits_per_frame / 8, (chunk_size - count) * hwparams.channels);
            count = chunk_size;
        }
    } else {
        if (count < chunk_size) {
            snd_pcm_format_set_silence(hwparams.format, data + count * bits_per_frame / 8, (chunk_size - count) * hwparams.channels);
            count = chunk_size;
        }
    }
    while (count > 0) {
        r = snd_pcm_writei(handle, data, count);
        if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
            snd_pcm_wait(handle, 1000);
        } else if (r == -EPIPE) {
            xrun();
        } else if (r == -ESTRPIPE) {
            suspend();
        } else if (r < 0) {
            printf(("write error: %s"), snd_strerror(r));
            exit(EXIT_FAILURE);
        }
        if (r > 0) {
#if 0
            if (verbose > 1)
                compute_max_peak(data, r * hwparams.channels);
#endif
            result += r;
            count -= r;
            data += r * bits_per_frame / 8;
        }
    }
    return result;
}

size_t test_wavefile_read(int fd, u_char *buffer, size_t *size, size_t reqsize, int line)
{
    if (*size >= reqsize)
        return *size;
    if ((size_t)safe_read(fd, buffer + *size, reqsize - *size) != reqsize - *size) {
        printf(("read error (called from line %i)"), line);
        exit(EXIT_FAILURE);
    }
    return *size = reqsize;
}

/* calculate the data count to read from/to dsp */
static off64_t calc_count(void)
{
    off64_t count;

    if (timelimit == 0) {
        count = pbrec_count;
    } else {
        count = snd_pcm_format_size(hwparams.format, hwparams.rate * hwparams.channels);
        count *= (off64_t)timelimit;
    }
    return count < pbrec_count ? count : pbrec_count;
}

/*
 * test, if it's a .WAV file, > 0 if ok (and set the speed, stereo etc.)
 *                            == 0 if not
 * Value returned is bytes to be discarded.
 */
static ssize_t test_wavefile(int fd, u_char *_buffer, size_t size)
{
    WaveHeader *h = (WaveHeader *)_buffer;
    u_char *buffer = NULL;
    size_t blimit = 0;
    WaveFmtBody *f;
    WaveChunkHeader *c;
    u_int type, len;

    if (size < sizeof(WaveHeader))
        return -1;
    if (h->magic != WAV_RIFF || h->type != WAV_WAVE)
        return -1;
    if (size > sizeof(WaveHeader)) {
        check_wavefile_space(buffer, size - sizeof(WaveHeader), blimit);
        memcpy(buffer, _buffer + sizeof(WaveHeader), size - sizeof(WaveHeader));
    }
    size -= sizeof(WaveHeader);
    while (1) {
        check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
        test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = LE_INT(c->length);
        len += len % 2;
        if (size > sizeof(WaveChunkHeader))
            memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
        size -= sizeof(WaveChunkHeader);
        if (type == WAV_FMT)
            break;
        check_wavefile_space(buffer, len, blimit);
        test_wavefile_read(fd, buffer, &size, len, __LINE__);
        if (size > len)
            memmove(buffer, buffer + len, size - len);
        size -= len;
    }

    if (len < sizeof(WaveFmtBody)) {
        printf(("unknown length of 'fmt ' chunk (read %u, should be %u at least)"),
              len, (u_int)sizeof(WaveFmtBody));
        exit(EXIT_FAILURE);
    }
    check_wavefile_space(buffer, len, blimit);
    test_wavefile_read(fd, buffer, &size, len, __LINE__);
    f = (WaveFmtBody*) buffer;
    if (LE_SHORT(f->format) != WAV_PCM_CODE) {
        printf(("can't play not PCM-coded WAVE-files"));
        exit(EXIT_FAILURE);
    }
    if (LE_SHORT(f->modus) < 1) {
        printf(("can't play WAVE-files with %d tracks"), LE_SHORT(f->modus));
        exit(EXIT_FAILURE);
    }
    hwparams.channels = LE_SHORT(f->modus);
    switch (LE_SHORT(f->bit_p_spl)) {
    case 8:
        if (hwparams.format != DEFAULT_FORMAT &&
            hwparams.format != SND_PCM_FORMAT_U8)
            fprintf(stderr, ("Warning: format is changed to U8\n"));
        hwparams.format = SND_PCM_FORMAT_U8;
        break;
    case 16:
        if (hwparams.format != DEFAULT_FORMAT &&
            hwparams.format != SND_PCM_FORMAT_S16_LE)
            fprintf(stderr, ("Warning: format is changed to S16_LE\n"));
        hwparams.format = SND_PCM_FORMAT_S16_LE;
        break;
    case 24:
        switch (LE_SHORT(f->byte_p_spl) / hwparams.channels) {
        case 3:
            if (hwparams.format != DEFAULT_FORMAT &&
                hwparams.format != SND_PCM_FORMAT_S24_3LE)
                fprintf(stderr, ("Warning: format is changed to S24_3LE\n"));
            hwparams.format = SND_PCM_FORMAT_S24_3LE;
            break;
        case 4:
            if (hwparams.format != DEFAULT_FORMAT &&
                hwparams.format != SND_PCM_FORMAT_S24_LE)
                fprintf(stderr, ("Warning: format is changed to S24_LE\n"));
            hwparams.format = SND_PCM_FORMAT_S24_LE;
            break;
        default:
            printf((" can't play WAVE-files with sample %d bits in %d bytes wide (%d channels)"),
                  LE_SHORT(f->bit_p_spl), LE_SHORT(f->byte_p_spl), hwparams.channels);
            exit(EXIT_FAILURE);
        }
        break;
    case 32:
        hwparams.format = SND_PCM_FORMAT_S32_LE;
        break;
    default:
        printf((" can't play WAVE-files with sample %d bits wide"),
              LE_SHORT(f->bit_p_spl));
        exit(EXIT_FAILURE);
    }
    hwparams.rate = LE_INT(f->sample_fq);

    if (size > len)
        memmove(buffer, buffer + len, size - len);
    size -= len;

    while (1) {
        u_int type, len;

        check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
        test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = LE_INT(c->length);
        if (size > sizeof(WaveChunkHeader))
            memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
        size -= sizeof(WaveChunkHeader);
        if (type == WAV_DATA) {
            if (len < pbrec_count && len < 0x7ffffffe)
                pbrec_count = len;
            if (size > 0)
                memcpy(_buffer, buffer, size);
            free(buffer);
            return size;
        }
        len += len % 2;
        check_wavefile_space(buffer, len, blimit);
        test_wavefile_read(fd, buffer, &size, len, __LINE__);
        if (size > len)
            memmove(buffer, buffer + len, size - len);
        size -= len;
    }

    /* shouldn't be reached */
    return -1;
}

ssize_t safe_read(int fd, void *buf, size_t count)
{
    ssize_t result = 0, res;

    while (count > 0) {
        if ((res = read(fd, buf, count)) == 0)
            break;
        if (res < 0)
            return result > 0 ? result : res;
        count -= res;
        result += res;
        buf = (char *)buf + res;
    }
    return result;
}

static void header(int rtype, char *name)
{
    if (!quiet_mode) {
        if (! name)
            name = (stream == SND_PCM_STREAM_PLAYBACK) ? "stdout" : "stdin";
        fprintf(stderr, "%s %s '%s' : ",
                (stream == SND_PCM_STREAM_PLAYBACK) ? ("Playing") : ("Recording"),
                fmt_table[rtype].what,
                name);
        fprintf(stderr, "%s, ", snd_pcm_format_description(hwparams.format));
        fprintf(stderr, ("Rate %d Hz, "), hwparams.rate);
        if (hwparams.channels == 1)
            fprintf(stderr, ("Mono"));
        else if (hwparams.channels == 2)
            fprintf(stderr, ("Stereo"));
        else
            fprintf(stderr, ("Channels %i"), hwparams.channels);
        fprintf(stderr, "\n");
    }
}

void playback_go(int fd, size_t loaded, off64_t count, int rtype, char *name)
{
    int l, r;
    off64_t written = 0;
    off64_t c;

    header(rtype, name);
    set_params();

    while (loaded > chunk_bytes && written < count) {
        if (pcm_write(audiobuf + written, chunk_size) <= 0)
            return;
        written += chunk_bytes;
        loaded -= chunk_bytes;
    }
    if (written > 0 && loaded > 0)
        memmove(audiobuf, audiobuf + written, loaded);

    l = loaded;
    while (written < count) {
        do {
            c = count - written;
            if (c > chunk_bytes)
                c = chunk_bytes;
            c -= l;

            if (c == 0)
                break;
            r = safe_read(fd, audiobuf + l, c);
            if (r < 0) {
                perror(name);
                exit(EXIT_FAILURE);
            }
            fdcount += r;
            if (r == 0)
                break;
            l += r;
        } while ((size_t)l < chunk_bytes);
        l = l * 8 / bits_per_frame;
        r = pcm_write(audiobuf, l);
        if (r != l)
            break;
        r = r * bits_per_frame / 8;
        written += r;
        l = 0;
    }
    snd_pcm_nonblock(handle, 0);
    snd_pcm_drain(handle);
    snd_pcm_nonblock(handle, nonblock);
}

static void playback(char *name)
{
    int ofs;
    size_t dta;
    ssize_t dtawave;

    pbrec_count = LLONG_MAX;
    fdcount = 0;
    if (!name || !strcmp(name, "-")) {
        fd = fileno(stdin);
        name = "stdin";
    } else {
        if ((fd = open64(name, O_RDONLY, 0)) == -1) {
            perror(name);
            exit(EXIT_FAILURE);
        }
    }
    /* read the file header */
    dta = sizeof(AuHeader);
    if ((size_t)safe_read(fd, audiobuf, dta) != dta) {
        printf(("read error"));
        exit(EXIT_FAILURE);
    }
    /* read bytes for WAVE-header */
    if ((dtawave = test_wavefile(fd, audiobuf, dta)) >= 0) {
        pbrec_count = calc_count();
        playback_go(fd, dtawave, pbrec_count, FORMAT_WAVE, name);
    } else {
        /* should be raw data */
        init_raw_data();
        pbrec_count = calc_count();
        playback_go(fd, dta, pbrec_count, FORMAT_RAW, name);
    }
    if (fd != 0)
        close(fd);
}

int main(int argc, char **argv)
{
    char *pcm_name = "default";
    stream = SND_PCM_STREAM_PLAYBACK;
    int err;
    snd_pcm_info_t *info;

    snd_pcm_info_alloca(&info);

    err = snd_output_stdio_attach(&log, stderr, 0);
    assert(err >= 0);

    err = snd_pcm_open(&handle, pcm_name, stream, open_mode);
    if (err < 0) {
        printf(("audio open error: %s"), snd_strerror(err));
        return 1;
    }

    if ((err = snd_pcm_info(handle, info)) < 0) {
        printf(("info error: %s"), snd_strerror(err));
        return 1;
    }

    if (nonblock) {
        err = snd_pcm_nonblock(handle, 1);
        if (err < 0) {
            printf(("nonblock setting error: %s"), snd_strerror(err));
            return 1;
        }
    }

    chunk_size = 1024;
    hwparams = rhwparams;

    audiobuf = (u_char *)malloc(1024);
    if (audiobuf == NULL) {
        printf(("not enough memory"));
        return 1;
    }

    playback(argv[1]);

    snd_pcm_close(handle);
    free(audiobuf);
    snd_output_close(log);
    snd_config_update_free_global();

    return EXIT_SUCCESS;
}

