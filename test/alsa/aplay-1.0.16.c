/*
 *  aplay.c - plays and records
 *
 *      CREATIVE LABS CHANNEL-files
 *      Microsoft WAVE-files
 *      SPARC AUDIO .AU-files
 *      Raw Data
 *
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 *  Based on vplay program by Michael Beck
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <asm/byteorder.h>
#include "aconfig.h"
#include "gettext.h"
#include "formats.h"
#include "version.h"

#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif

#define DEFAULT_FORMAT		SND_PCM_FORMAT_U8
#define DEFAULT_SPEED 		8000

#define FORMAT_DEFAULT		-1
#define FORMAT_RAW		0
#define FORMAT_VOC		1
#define FORMAT_WAVE		2
#define FORMAT_AU		3

/* global data */

static snd_pcm_sframes_t (*readi_func)(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writei_func)(snd_pcm_t *handle, const void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*readn_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writen_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);

static char *command;
static snd_pcm_t *handle;
static struct {
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int rate;
} hwparams, rhwparams;
static int timelimit = 0;
static int quiet_mode = 0;
static int file_type = FORMAT_DEFAULT;
static int open_mode = 0;
static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
static int mmap_flag = 0;
static int interleaved = 1;
static int nonblock = 0;
static u_char *audiobuf = NULL;
static snd_pcm_uframes_t chunk_size = 0;
static unsigned period_time = 0;
static unsigned buffer_time = 0;
static snd_pcm_uframes_t period_frames = 0;
static snd_pcm_uframes_t buffer_frames = 0;
static int avail_min = -1;
static int start_delay = 0;
static int stop_delay = 0;
static int verbose = 0;
static int buffer_pos = 0;
static size_t bits_per_sample, bits_per_frame;
static size_t chunk_bytes;
static snd_output_t *log;

static int fd = -1;
static off64_t pbrec_count = LLONG_MAX, fdcount;
static int vocmajor, vocminor;

/* needed prototypes */

static void playback(char *filename);
static void capture(char *filename);
static void playbackv(char **filenames, unsigned int count);
static void capturev(char **filenames, unsigned int count);

static void begin_voc(int fd, size_t count);
static void end_voc(int fd);
static void begin_wave(int fd, size_t count);
static void end_wave(int fd);
static void begin_au(int fd, size_t count);
static void end_au(int fd);

struct fmt_capture {
	void (*start) (int fd, size_t count);
	void (*end) (int fd);
	char *what;
	long long max_filesize;
} fmt_rec_table[] = {
	{	NULL,		NULL,		N_("raw data"),		LLONG_MAX },
	{	begin_voc,	end_voc,	N_("VOC"),		16000000LL },
	/* FIXME: can WAV handle exactly 2GB or less than it? */
	{	begin_wave,	end_wave,	N_("WAVE"),		2147483648LL },
	{	begin_au,	end_au,		N_("Sparc Audio"),	LLONG_MAX }
};

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define error(...) do {\
	fprintf(stderr, "%s: %s:%d: ", command, __FUNCTION__, __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	putc('\n', stderr); \
} while (0)
#else
#define error(args...) do {\
	fprintf(stderr, "%s: %s:%d: ", command, __FUNCTION__, __LINE__); \
	fprintf(stderr, ##args); \
	putc('\n', stderr); \
} while (0)
#endif

static void version(void)
{
	printf("%s: version " SND_UTIL_VERSION_STR " by Jaroslav Kysela <perex@perex.cz>\n", command);
}

int main(int argc, char *argv[])
{
	int option_index;
	char *short_options = "hnlLD:qt:c:f:r:d:MNF:A:R:T:B:vIPC";
	static struct option long_options[] = {
		{"help", 0, 0, 'h'},
		{"version", 0, 0, OPT_VERSION},
		{"list-devnames", 0, 0, 'n'},
		{"list-devices", 0, 0, 'l'},
		{"list-pcms", 0, 0, 'L'},
		{"device", 1, 0, 'D'},
		{"quiet", 0, 0, 'q'},
		{"file-type", 1, 0, 't'},
		{"channels", 1, 0, 'c'},
		{"format", 1, 0, 'f'},
		{"rate", 1, 0, 'r'},
		{"duration", 1, 0 ,'d'},
		{"mmap", 0, 0, 'M'},
		{"nonblock", 0, 0, 'N'},
		{"period-time", 1, 0, 'F'},
		{"period-size", 1, 0, OPT_PERIOD_SIZE},
		{"avail-min", 1, 0, 'A'},
		{"start-delay", 1, 0, 'R'},
		{"stop-delay", 1, 0, 'T'},
		{"buffer-time", 1, 0, 'B'},
		{"buffer-size", 1, 0, OPT_BUFFER_SIZE},
		{"verbose", 0, 0, 'v'},
		{"separate-channels", 0, 0, 'I'},
		{"playback", 0, 0, 'P'},
		{"capture", 0, 0, 'C'},
		{"disable-resample", 0, 0, OPT_DISABLE_RESAMPLE},
		{"disable-channels", 0, 0, OPT_DISABLE_CHANNELS},
		{"disable-format", 0, 0, OPT_DISABLE_FORMAT},
		{"disable-softvol", 0, 0, OPT_DISABLE_SOFTVOL},
		{0, 0, 0, 0}
	};
	char *pcm_name = "default";
	int tmp, err, c;
	int do_device_list = 0, do_pcm_list = 0;
	snd_pcm_info_t *info;

#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	textdomain(PACKAGE);
#endif

	snd_pcm_info_alloca(&info);

	err = snd_output_stdio_attach(&log, stderr, 0);
	assert(err >= 0);

	command = argv[0];
	file_type = FORMAT_DEFAULT;
	if (strstr(argv[0], "arecord")) {
		stream = SND_PCM_STREAM_CAPTURE;
		file_type = FORMAT_WAVE;
		command = "arecord";
		start_delay = 1;
	} else if (strstr(argv[0], "aplay")) {
		stream = SND_PCM_STREAM_PLAYBACK;
		command = "aplay";
	} else {
		error(_("command should be named either arecord or aplay"));
		return 1;
	}

	chunk_size = -1;
	rhwparams.format = DEFAULT_FORMAT;
	rhwparams.rate = DEFAULT_SPEED;
	rhwparams.channels = 1;

	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(command);
			return 0;
		case OPT_VERSION:
			version();
			return 0;
		case 'l':
			do_device_list = 1;
			break;
		case 'L':
			do_pcm_list = 1;
			break;
		case 'D':
			pcm_name = optarg;
			break;
		case 'q':
			quiet_mode = 1;
			break;
		case 't':
			if (strcasecmp(optarg, "raw") == 0)
				file_type = FORMAT_RAW;
			else if (strcasecmp(optarg, "voc") == 0)
				file_type = FORMAT_VOC;
			else if (strcasecmp(optarg, "wav") == 0)
				file_type = FORMAT_WAVE;
			else if (strcasecmp(optarg, "au") == 0 || strcasecmp(optarg, "sparc") == 0)
				file_type = FORMAT_AU;
			else {
				error(_("unrecognized file format %s"), optarg);
				return 1;
			}
			break;
		case 'c':
			rhwparams.channels = strtol(optarg, NULL, 0);
			if (rhwparams.channels < 1 || rhwparams.channels > 32) {
				error(_("value %i for channels is invalid"), rhwparams.channels);
				return 1;
			}
			break;
		case 'f':
			if (strcasecmp(optarg, "cd") == 0 || strcasecmp(optarg, "cdr") == 0) {
				if (strcasecmp(optarg, "cdr") == 0)
					rhwparams.format = SND_PCM_FORMAT_S16_BE;
				else
					rhwparams.format = file_type == FORMAT_AU ? SND_PCM_FORMAT_S16_BE : SND_PCM_FORMAT_S16_LE;
				rhwparams.rate = 44100;
				rhwparams.channels = 2;
			} else if (strcasecmp(optarg, "dat") == 0) {
				rhwparams.format = file_type == FORMAT_AU ? SND_PCM_FORMAT_S16_BE : SND_PCM_FORMAT_S16_LE;
				rhwparams.rate = 48000;
				rhwparams.channels = 2;
			} else {
				rhwparams.format = snd_pcm_format_value(optarg);
				if (rhwparams.format == SND_PCM_FORMAT_UNKNOWN) {
					error(_("wrong extended format '%s'"), optarg);
					exit(EXIT_FAILURE);
				}
			}
			break;
		case 'r':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 300)
				tmp *= 1000;
			rhwparams.rate = tmp;
			if (tmp < 2000 || tmp > 192000) {
				error(_("bad speed value %i"), tmp);
				return 1;
			}
			break;
		case 'd':
			timelimit = strtol(optarg, NULL, 0);
			break;
		case 'N':
			nonblock = 1;
			open_mode |= SND_PCM_NONBLOCK;
			break;
		case 'F':
			period_time = strtol(optarg, NULL, 0);
			break;
		case 'B':
			buffer_time = strtol(optarg, NULL, 0);
			break;
		case OPT_PERIOD_SIZE:
			period_frames = strtol(optarg, NULL, 0);
			break;
		case OPT_BUFFER_SIZE:
			buffer_frames = strtol(optarg, NULL, 0);
			break;
		case 'A':
			avail_min = strtol(optarg, NULL, 0);
			break;
		case 'R':
			start_delay = strtol(optarg, NULL, 0);
			break;
		case 'T':
			stop_delay = strtol(optarg, NULL, 0);
			break;
		case 'v':
			verbose++;
			break;
		case 'M':
			mmap_flag = 1;
			break;
		case 'I':
			interleaved = 0;
			break;
		case 'P':
			stream = SND_PCM_STREAM_PLAYBACK;
			command = "aplay";
			break;
		case 'C':
			stream = SND_PCM_STREAM_CAPTURE;
			command = "arecord";
			start_delay = 1;
			if (file_type == FORMAT_DEFAULT)
				file_type = FORMAT_WAVE;
			break;
		case OPT_DISABLE_RESAMPLE:
			open_mode |= SND_PCM_NO_AUTO_RESAMPLE;
			break;
		case OPT_DISABLE_CHANNELS:
			open_mode |= SND_PCM_NO_AUTO_CHANNELS;
			break;
		case OPT_DISABLE_FORMAT:
			open_mode |= SND_PCM_NO_AUTO_FORMAT;
			break;
		case OPT_DISABLE_SOFTVOL:
			open_mode |= SND_PCM_NO_SOFTVOL;
			break;
		default:
			fprintf(stderr, _("Try `%s --help' for more information.\n"), command);
			return 1;
		}
	}

	if (do_device_list) {
		if (do_pcm_list) pcm_list();
		device_list();
		goto __end;
	} else if (do_pcm_list) {
		pcm_list();
		goto __end;
	}

	err = snd_pcm_open(&handle, pcm_name, stream, open_mode);
	if (err < 0) {
		error(_("audio open error: %s"), snd_strerror(err));
		return 1;
	}

	if ((err = snd_pcm_info(handle, info)) < 0) {
		error(_("info error: %s"), snd_strerror(err));
		return 1;
	}

	if (nonblock) {
		err = snd_pcm_nonblock(handle, 1);
		if (err < 0) {
			error(_("nonblock setting error: %s"), snd_strerror(err));
			return 1;
		}
	}

	chunk_size = 1024;
	hwparams = rhwparams;

	audiobuf = (u_char *)malloc(1024);
	if (audiobuf == NULL) {
		error(_("not enough memory"));
		return 1;
	}

	if (mmap_flag) {
		writei_func = snd_pcm_mmap_writei;
		readi_func = snd_pcm_mmap_readi;
		writen_func = snd_pcm_mmap_writen;
		readn_func = snd_pcm_mmap_readn;
	} else {
		writei_func = snd_pcm_writei;
		readi_func = snd_pcm_readi;
		writen_func = snd_pcm_writen;
		readn_func = snd_pcm_readn;
	}


	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGABRT, signal_handler);
	if (interleaved) {
		if (optind > argc - 1) {
			if (stream == SND_PCM_STREAM_PLAYBACK)
				playback(NULL);
			else
				capture(NULL);
		} else {
			while (optind <= argc - 1) {
				if (stream == SND_PCM_STREAM_PLAYBACK)
					playback(argv[optind++]);
				else
					capture(argv[optind++]);
			}
		}
	} else {
		if (stream == SND_PCM_STREAM_PLAYBACK)
			playbackv(&argv[optind], argc - optind);
		else
			capturev(&argv[optind], argc - optind);
	}
	if (verbose==2)
		putchar('\n');
	snd_pcm_close(handle);
	free(audiobuf);
      __end:
	snd_output_close(log);
	snd_config_update_free_global();
	return EXIT_SUCCESS;
}

/*
 * Safe read (for pipes)
 */

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

/*
 * helper for test_wavefile
 */

size_t test_wavefile_read(int fd, u_char *buffer, size_t *size, size_t reqsize, int line)
{
	if (*size >= reqsize)
		return *size;
	if ((size_t)safe_read(fd, buffer + *size, reqsize - *size) != reqsize - *size) {
		error(_("read error (called from line %i)"), line);
		exit(EXIT_FAILURE);
	}
	return *size = reqsize;
}

#define check_wavefile_space(buffer, len, blimit) \
	if (len > blimit) { \
		blimit = len; \
		if ((buffer = realloc(buffer, blimit)) == NULL) { \
			error(_("not enough memory"));		  \
			exit(EXIT_FAILURE); \
		} \
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
		error(_("unknown length of 'fmt ' chunk (read %u, should be %u at least)"),
		      len, (u_int)sizeof(WaveFmtBody));
		exit(EXIT_FAILURE);
	}
	check_wavefile_space(buffer, len, blimit);
	test_wavefile_read(fd, buffer, &size, len, __LINE__);
	f = (WaveFmtBody*) buffer;
	if (LE_SHORT(f->format) != WAV_PCM_CODE) {
		error(_("can't play not PCM-coded WAVE-files"));
		exit(EXIT_FAILURE);
	}
	if (LE_SHORT(f->modus) < 1) {
		error(_("can't play WAVE-files with %d tracks"), LE_SHORT(f->modus));
		exit(EXIT_FAILURE);
	}
	hwparams.channels = LE_SHORT(f->modus);
	switch (LE_SHORT(f->bit_p_spl)) {
	case 8:
		if (hwparams.format != DEFAULT_FORMAT &&
		    hwparams.format != SND_PCM_FORMAT_U8)
			fprintf(stderr, _("Warning: format is changed to U8\n"));
		hwparams.format = SND_PCM_FORMAT_U8;
		break;
	case 16:
		if (hwparams.format != DEFAULT_FORMAT &&
		    hwparams.format != SND_PCM_FORMAT_S16_LE)
			fprintf(stderr, _("Warning: format is changed to S16_LE\n"));
		hwparams.format = SND_PCM_FORMAT_S16_LE;
		break;
	case 24:
		switch (LE_SHORT(f->byte_p_spl) / hwparams.channels) {
		case 3:
			if (hwparams.format != DEFAULT_FORMAT &&
			    hwparams.format != SND_PCM_FORMAT_S24_3LE)
				fprintf(stderr, _("Warning: format is changed to S24_3LE\n"));
			hwparams.format = SND_PCM_FORMAT_S24_3LE;
			break;
		case 4:
			if (hwparams.format != DEFAULT_FORMAT &&
			    hwparams.format != SND_PCM_FORMAT_S24_LE)
				fprintf(stderr, _("Warning: format is changed to S24_LE\n"));
			hwparams.format = SND_PCM_FORMAT_S24_LE;
			break;
		default:
			error(_(" can't play WAVE-files with sample %d bits in %d bytes wide (%d channels)"),
			      LE_SHORT(f->bit_p_spl), LE_SHORT(f->byte_p_spl), hwparams.channels);
			exit(EXIT_FAILURE);
		}
		break;
	case 32:
		hwparams.format = SND_PCM_FORMAT_S32_LE;
		break;
	default:
		error(_(" can't play WAVE-files with sample %d bits wide"),
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

static void set_params(void)
{
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_uframes_t buffer_size;
	int err;
	size_t n;
	unsigned int rate;
	snd_pcm_uframes_t start_threshold, stop_threshold;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		error(_("Broken configuration for this PCM: no configurations available"));
		exit(EXIT_FAILURE);
	}
	if (mmap_flag) {
		snd_pcm_access_mask_t *mask = alloca(snd_pcm_access_mask_sizeof());
		snd_pcm_access_mask_none(mask);
		snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_INTERLEAVED);
		snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
		snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_COMPLEX);
		err = snd_pcm_hw_params_set_access_mask(handle, params, mask);
	} else if (interleaved)
		err = snd_pcm_hw_params_set_access(handle, params,
						   SND_PCM_ACCESS_RW_INTERLEAVED);
	else
		err = snd_pcm_hw_params_set_access(handle, params,
						   SND_PCM_ACCESS_RW_NONINTERLEAVED);
	if (err < 0) {
		error(_("Access type not available"));
		exit(EXIT_FAILURE);
	}
	err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
	if (err < 0) {
		error(_("Sample format non available"));
		exit(EXIT_FAILURE);
	}
	err = snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);
	if (err < 0) {
		error(_("Channels count non available"));
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
			fprintf(stderr, _("Warning: rate is not accurate (requested = %iHz, got = %iHz)\n"), rate, hwparams.rate);
			if (! pcmname || strchr(snd_pcm_name(handle), ':'))
				*plugex = 0;
			else
				snprintf(plugex, sizeof(plugex), "(-Dplug:%s)",
					 snd_pcm_name(handle));
			fprintf(stderr, _("         please, try the plug plugin %s\n"),
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
		error(_("Unable to install hw params:"));
		snd_pcm_hw_params_dump(params, log);
		exit(EXIT_FAILURE);
	}
	snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	if (chunk_size == buffer_size) {
		error(_("Can't use period equal to buffer size (%lu == %lu)"),
		      chunk_size, buffer_size);
		exit(EXIT_FAILURE);
	}
	snd_pcm_sw_params_current(handle, swparams);
	if (avail_min < 0)
		n = chunk_size;
	else
		n = (double) rate * avail_min / 1000000;
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, n);

	/* round up to closest transfer boundary */
	n = buffer_size;
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

	if (snd_pcm_sw_params(handle, swparams) < 0) {
		error(_("unable to install sw params:"));
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
		error(_("not enough memory"));
		exit(EXIT_FAILURE);
	}
	// fprintf(stderr, "real chunk_size = %i, frags = %i, total = %i\n", chunk_size, setup.buf.block.frags, setup.buf.block.frags * chunk_size);
}

#ifndef timersub
#define	timersub(a, b, result) \
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
		error(_("status error: %s"), snd_strerror(res));
		exit(EXIT_FAILURE);
	}
	if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
		struct timeval now, diff, tstamp;
		gettimeofday(&now, 0);
		snd_pcm_status_get_trigger_tstamp(status, &tstamp);
		timersub(&now, &tstamp, &diff);
		fprintf(stderr, _("%s!!! (at least %.3f ms long)\n"),
			stream == SND_PCM_STREAM_PLAYBACK ? _("underrun") : _("overrun"),
			diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
		if (verbose) {
			fprintf(stderr, _("Status:\n"));
			snd_pcm_status_dump(status, log);
		}
		if ((res = snd_pcm_prepare(handle))<0) {
			error(_("xrun: prepare error: %s"), snd_strerror(res));
			exit(EXIT_FAILURE);
		}
		return;		/* ok, data should be accepted again */
	} if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
		if (verbose) {
			fprintf(stderr, _("Status(DRAINING):\n"));
			snd_pcm_status_dump(status, log);
		}
		if (stream == SND_PCM_STREAM_CAPTURE) {
			fprintf(stderr, _("capture stream format change? attempting recover...\n"));
			if ((res = snd_pcm_prepare(handle))<0) {
				error(_("xrun(DRAINING): prepare error: %s"), snd_strerror(res));
				exit(EXIT_FAILURE);
			}
			return;
		}
	}
	if (verbose) {
		fprintf(stderr, _("Status(R/W):\n"));
		snd_pcm_status_dump(status, log);
	}
	error(_("read/write error, state = %s"), snd_pcm_state_name(snd_pcm_status_get_state(status)));
	exit(EXIT_FAILURE);
}

/* I/O suspend handler */
static void suspend(void)
{
	int res;

	if (!quiet_mode)
		fprintf(stderr, _("Suspended. Trying resume. ")); fflush(stderr);
	while ((res = snd_pcm_resume(handle)) == -EAGAIN)
		sleep(1);	/* wait until suspend flag is released */
	if (res < 0) {
		if (!quiet_mode)
			fprintf(stderr, _("Failed. Restarting stream. ")); fflush(stderr);
		if ((res = snd_pcm_prepare(handle)) < 0) {
			error(_("suspend: prepare error: %s"), snd_strerror(res));
			exit(EXIT_FAILURE);
		}
	}
	if (!quiet_mode)
		fprintf(stderr, _("Done.\n"));
}

/*
 *  write function
 */

static ssize_t pcm_write(u_char *data, size_t count)
{
	ssize_t r;
	ssize_t result = 0;

	if (count < chunk_size) {
		snd_pcm_format_set_silence(hwparams.format, data + count * bits_per_frame / 8, (chunk_size - count) * hwparams.channels);
		count = chunk_size;
	}
	while (count > 0) {
		r = writei_func(handle, data, count);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
			snd_pcm_wait(handle, 1000);
		} else if (r == -EPIPE) {
			xrun();
		} else if (r == -ESTRPIPE) {
			suspend();
		} else if (r < 0) {
			error(_("write error: %s"), snd_strerror(r));
			exit(EXIT_FAILURE);
		}
		if (r > 0) {
			if (verbose > 1)
				compute_max_peak(data, r * hwparams.channels);
			result += r;
			count -= r;
			data += r * bits_per_frame / 8;
		}
	}
	return result;
}

static ssize_t pcm_writev(u_char **data, unsigned int channels, size_t count)
{
	ssize_t r;
	size_t result = 0;

	if (count != chunk_size) {
		unsigned int channel;
		size_t offset = count;
		size_t remaining = chunk_size - count;
		for (channel = 0; channel < channels; channel++)
			snd_pcm_format_set_silence(hwparams.format, data[channel] + offset * bits_per_sample / 8, remaining);
		count = chunk_size;
	}
	while (count > 0) {
		unsigned int channel;
		void *bufs[channels];
		size_t offset = result;
		for (channel = 0; channel < channels; channel++)
			bufs[channel] = data[channel] + offset * bits_per_sample / 8;
		r = writen_func(handle, bufs, count);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
			snd_pcm_wait(handle, 1000);
		} else if (r == -EPIPE) {
			xrun();
		} else if (r == -ESTRPIPE) {
			suspend();
		} else if (r < 0) {
			error(_("writev error: %s"), snd_strerror(r));
			exit(EXIT_FAILURE);
		}
		if (r > 0) {
			if (verbose > 1) {
				for (channel = 0; channel < channels; channel++)
					compute_max_peak(data[channel], r);
			}
			result += r;
			count -= r;
		}
	}
	return result;
}

/* setting the globals for playing raw data */
static void init_raw_data(void)
{
	hwparams = rhwparams;
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

/* playing raw data */

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


/*
 *  let's play or capture it (capture_type says VOC/WAVE/raw)
 */

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
		error(_("read error"));
		exit(EXIT_FAILURE);
	}
	if (test_au(fd, audiobuf) >= 0) {
		rhwparams.format = hwparams.format;
		pbrec_count = calc_count();
		playback_go(fd, 0, pbrec_count, FORMAT_AU, name);
		goto __end;
	}
	dta = sizeof(VocHeader);
	if ((size_t)safe_read(fd, audiobuf + sizeof(AuHeader),
		 dta - sizeof(AuHeader)) != dta - sizeof(AuHeader)) {
		error(_("read error"));
		exit(EXIT_FAILURE);
	}
	if ((ofs = test_vocfile(audiobuf)) >= 0) {
		pbrec_count = calc_count();
		voc_play(fd, ofs, name);
		goto __end;
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
      __end:
	if (fd != 0)
		close(fd);
}

