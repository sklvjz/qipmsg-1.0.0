// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QIpMsg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QIpMsg.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef SOUND_H
#define SOUND_H

#include <QString>

#include <alsa/asoundlib.h>

class Sound
{
public :
    Sound(QString path);
    ~Sound() {};

    bool play();

private:
    bool playback_go(int fd, size_t loaded, off64_t count,
                     int rtype, const char *name);
    bool playback(const char *name);
    ssize_t safe_read(int fd, void *buf, size_t count);
    off64_t calc_count(void);
    ssize_t test_wavefile(int fd, u_char *_buffer, size_t size);
    bool set_params(void);
    void init_raw_data(void);
    bool suspend(void);
    bool xrun(void);
    ssize_t pcm_write(u_char *data, size_t count);
    size_t test_wavefile_read(int fd, u_char *buffer, size_t *size,
                              size_t reqsize, int line);
    void header(int rtype, char *name);

    QString m_path;

    const char *device;
    snd_pcm_t *handle;
    int open_mode;
    int verbose;
    int nonblock;
    snd_pcm_uframes_t chunk_size;
    struct {
        snd_pcm_format_t format;
        unsigned int channels;
        unsigned int rate;
    } hwparams, rhwparams;
    u_char *audiobuf;
    unsigned period_time;
    unsigned buffer_time;
    unsigned int sleep_min;
    snd_pcm_uframes_t period_frames;
    snd_pcm_uframes_t buffer_frames;
    off64_t pbrec_count;
    off64_t fdcount;
    int fd;
    snd_pcm_stream_t stream;
    snd_output_t *log;
    int timelimit;
    size_t chunk_bytes;
    size_t bits_per_sample, bits_per_frame;
    int quiet_mode;
    int avail_min;
    int start_delay;
    int stop_delay;
};
#endif // !SOUND_H

