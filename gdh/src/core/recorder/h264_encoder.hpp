#pragma once
#include <Geode/Geode.hpp>
#include <chrono>
#include "log_overlay.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class H264Encoder {
public:
    static void log_ffmpeg(const std::string& text, bool geode_log = true) {
        if (geode_log) geode::log::debug("{}", text);
        LogOverlay::get().add_log(text);
    }

    H264Encoder(int width, int height, int fps, const std::string& bitrate_str, const std::string& preset = "medium")
        : width_(width), height_(height), fps_(fps), pts_(0), audio_pts_(0),
          enc_ctx_(nullptr), audio_enc_ctx_(nullptr), fmt_ctx_(nullptr), 
          stream_(nullptr), audio_stream_(nullptr), yuv_frame_(nullptr), 
          rgb_frame_(nullptr), audio_frame_(nullptr), pkt_(nullptr), sws_ctx_(nullptr),
          is_valid_(false), header_written_(false), total_bytes_written_(0)
    {
        const AVCodec* codec = avcodec_find_encoder_by_name("libx264");
        if (!codec) {
            log_ffmpeg("[ffmpeg] [error] failed to find libx264 encoder");
            cleanup();
            return;
        }

        enc_ctx_ = avcodec_alloc_context3(codec);
        if (!enc_ctx_) {
            log_ffmpeg("[ffmpeg] [error] failed to allocate video codec context");
            cleanup();
            return;
        }

        int64_t bitrate = 25000000; 
        if (!bitrate_str.empty()) {
            std::string str_copy = bitrate_str;
            char unit = str_copy.back();
            int64_t multiplier = 1;

            if (unit == 'M' || unit == 'm') {
                multiplier = 1000000ll;
                str_copy.pop_back();
            } else if (unit == 'K' || unit == 'k') {
                multiplier = 1000ll;
                str_copy.pop_back();
            }

            auto parsed_val = geode::utils::numFromString<int64_t>(str_copy);
            if (!parsed_val.isErr()) {
                bitrate = parsed_val.unwrap() * multiplier;
            }
        }

        enc_ctx_->bit_rate = bitrate;
        enc_ctx_->width = width_;
        enc_ctx_->height = height_;
        enc_ctx_->time_base = {1, fps_};
        enc_ctx_->framerate = {fps_, 1};
        enc_ctx_->gop_size = 10;
        enc_ctx_->max_b_frames = 0;
        enc_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;
        enc_ctx_->colorspace = AVCOL_SPC_BT709;
        enc_ctx_->color_primaries = AVCOL_PRI_BT709;
        enc_ctx_->color_trc = AVCOL_TRC_BT709;
        enc_ctx_->color_range = AVCOL_RANGE_MPEG;

        av_opt_set(enc_ctx_->priv_data, "preset", preset.c_str(), 0);

        sws_ctx_ = sws_getContext(width_, height_, AV_PIX_FMT_RGBA,
                                  width_, height_, AV_PIX_FMT_YUV420P,
                                  SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
        if (!sws_ctx_) {
            log_ffmpeg("[ffmpeg] [error] failed to initialize sws context");
            cleanup();
            return;
        }
        
        const int *inv_table = sws_getCoefficients(SWS_CS_ITU601);
        const int *table     = sws_getCoefficients(SWS_CS_ITU709);
        sws_setColorspaceDetails(sws_ctx_, inv_table, 1, table, 0, 0, 1 << 16, 1 << 16);

        if (avcodec_open2(enc_ctx_, codec, nullptr) < 0) {
            log_ffmpeg("[ffmpeg] [error] failed to open video codec");
            cleanup();
            return;
        }

        const AVCodec* audio_codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        if (audio_codec) {
            audio_enc_ctx_ = avcodec_alloc_context3(audio_codec);
            if (audio_enc_ctx_) {
                int sampleRate = 0, channels = 0;
                FMODAudioEngine::sharedEngine()->m_system->getSoftwareFormat(&sampleRate, nullptr, &channels);
                
                if (sampleRate <= 0) sampleRate = 44100;

                audio_enc_ctx_->bit_rate = 320000;
                audio_enc_ctx_->sample_fmt = AV_SAMPLE_FMT_FLTP;
                audio_enc_ctx_->sample_rate = sampleRate;
                audio_enc_ctx_->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
                audio_enc_ctx_->time_base = {1, audio_enc_ctx_->sample_rate};
                if (avcodec_open2(audio_enc_ctx_, audio_codec, nullptr) < 0) {
                    log_ffmpeg("[ffmpeg] [warning] failed to open audio codec");
                    avcodec_free_context(&audio_enc_ctx_);
                }
            }
        } else {
            log_ffmpeg("[ffmpeg] [warning] aac audio encoder not found");
        }

        pkt_ = av_packet_alloc();
        if (!pkt_) {
            log_ffmpeg("[ffmpeg] [error] failed to allocate packet");
            cleanup();
            return;
        }
        
        yuv_frame_ = av_frame_alloc();
        if (!yuv_frame_) {
            log_ffmpeg("[ffmpeg] [error] failed to allocate yuv frame");
            cleanup();
            return;
        }
        yuv_frame_->format = enc_ctx_->pix_fmt;
        yuv_frame_->width = width_;
        yuv_frame_->height = height_;
        if (av_frame_get_buffer(yuv_frame_, 0) < 0) {
            log_ffmpeg("[ffmpeg] [error] failed to get yuv frame buffer");
            cleanup();
            return;
        }

        rgb_frame_ = av_frame_alloc();
        if (!rgb_frame_) {
            log_ffmpeg("[ffmpeg] [error] failed to allocate rgb frame");
            cleanup();
            return;
        }
        rgb_frame_->format = AV_PIX_FMT_RGBA;
        rgb_frame_->width = width_;
        rgb_frame_->height = height_;
        if (av_frame_get_buffer(rgb_frame_, 0) < 0) {
            log_ffmpeg("[ffmpeg] [error] failed to get rgb frame buffer");
            cleanup();
            return;
        }

        is_valid_ = true;
        log_ffmpeg(fmt::format("[ffmpeg] encoder initialized: {}x{} @ {} fps", width_, height_, fps_));
    }

    ~H264Encoder() {
        stop();
        cleanup();
    }

    bool is_valid() const { return is_valid_; }

    bool start(const std::string& filename) {
        if (!is_valid_) {
            log_ffmpeg("[ffmpeg] [error] cannot start encoder, state is invalid");
            return false;
        }
        if (fmt_ctx_) stop();

        header_written_ = false;
        total_bytes_written_ = 0;

        int ret = avformat_alloc_output_context2(&fmt_ctx_, nullptr, nullptr, filename.c_str());
        if (ret < 0 || !fmt_ctx_) {
            log_ffmpeg(fmt::format("[ffmpeg] [error] failed to alloc output context for file: {}", filename));
            return false;
        }

        stream_ = avformat_new_stream(fmt_ctx_, nullptr);
        if (!stream_) { 
            log_ffmpeg("[ffmpeg] [error] failed to create video stream");
            cleanup_format_context(); 
            return false; 
        }
        avcodec_parameters_from_context(stream_->codecpar, enc_ctx_);
        stream_->time_base = enc_ctx_->time_base;

        if (audio_enc_ctx_) {
            audio_stream_ = avformat_new_stream(fmt_ctx_, nullptr);
            if (audio_stream_) {
                avcodec_parameters_from_context(audio_stream_->codecpar, audio_enc_ctx_);
                audio_stream_->time_base = audio_enc_ctx_->time_base;
            } else {
                log_ffmpeg("[ffmpeg] [warning] failed to create audio stream");
            }
        }

        if (!(fmt_ctx_->oformat->flags & AVFMT_NOFILE)) {
            ret = avio_open(&fmt_ctx_->pb, filename.c_str(), AVIO_FLAG_WRITE);
            if (ret < 0) { 
                log_ffmpeg(fmt::format("[ffmpeg] [error] failed to open output file io: {}", filename));
                cleanup_format_context(); 
                return false; 
            }
        }

        if (avformat_write_header(fmt_ctx_, nullptr) < 0) {
            log_ffmpeg("[ffmpeg] [error] failed to write file header");
            cleanup_format_context();
            return false;
        }

        header_written_ = true;
        pts_ = 0;
        audio_pts_ = 0;
        start_time_ = std::chrono::steady_clock::now();
        log_ffmpeg(fmt::format("[ffmpeg] Output #0, to '{}':", filename));
        return true;
    }

    void encode_frame(const std::vector<uint8_t>& rgb_data) {
        if (!is_valid_ || !fmt_ctx_ || !header_written_ || rgb_frame_ == nullptr) return;

        av_image_fill_arrays(rgb_frame_->data, rgb_frame_->linesize, 
                             rgb_data.data(), AV_PIX_FMT_RGBA, width_, height_, 1);

        uint8_t* flipped_data[4] = { rgb_frame_->data[0] + rgb_frame_->linesize[0] * (height_ - 1), 0, 0, 0 };
        int flipped_linesize[4] = { -rgb_frame_->linesize[0], 0, 0, 0 };

        av_frame_make_writable(yuv_frame_);
        sws_scale(sws_ctx_, flipped_data, flipped_linesize, 0, height_, 
                  yuv_frame_->data, yuv_frame_->linesize);

        yuv_frame_->pts = pts_++;
        encode_internal(enc_ctx_, stream_, yuv_frame_);

        double time_sec = static_cast<double>(pts_) / fps_;
        int hours = static_cast<int>(time_sec) / 3600;
        int mins = (static_cast<int>(time_sec) % 3600) / 60;
        double secs = time_sec - (hours * 3600 + mins * 60);
        
        double size_kb = static_cast<double>(total_bytes_written_) / 1024.0;
        double bitrate_kbits = (time_sec > 0) ? ((total_bytes_written_ * 8.0) / 1000.0) / time_sec : 0.0;

        if (fps_ > 0 && (pts_ % fps_ == 0)) {
            auto now = std::chrono::steady_clock::now();
            double elapsed_sec = std::chrono::duration<double>(now - start_time_).count();
            double speed = (elapsed_sec > 0) ? (time_sec / elapsed_sec) : 0.0;

            log_ffmpeg(fmt::format("frame={:5d} fps={:2d} size={:7.0f}kB time={:02d}:{:02d}:{:05.2f} bitrate={:6.1f}kbits/s speed={:4.2f}x", 
                pts_, fps_, size_kb, hours, mins, secs, bitrate_kbits, speed), false);
        }
    }

    void finalize_with_audio(std::vector<float> samples) {
        if (!is_valid_ || !fmt_ctx_ || !header_written_ || !audio_enc_ctx_ || !audio_stream_ || samples.empty()) {
            log_ffmpeg("[ffmpeg] [warning] skipping audio finalization");
            return;
        }

        int sample_rate = audio_enc_ctx_->sample_rate;
        int channels = audio_enc_ctx_->ch_layout.nb_channels;

        if (channels <= 0) return;

        double video_duration = static_cast<double>(pts_) / fps_;
        size_t max_samples = static_cast<size_t>(video_duration * sample_rate);
        if (samples.size() / channels > max_samples) {
            samples.resize(max_samples * channels);
        }

        audio_frame_ = av_frame_alloc();
        if (!audio_frame_) {
            log_ffmpeg("[ffmpeg] [error] failed to alloc audio frame");
            return;
        }

        audio_frame_->nb_samples = audio_enc_ctx_->frame_size;
        audio_frame_->format = audio_enc_ctx_->sample_fmt;
        audio_frame_->ch_layout = audio_enc_ctx_->ch_layout;
        if (av_frame_get_buffer(audio_frame_, 0) < 0) {
            log_ffmpeg("[ffmpeg] [error] failed to get audio frame buffer");
            av_frame_free(&audio_frame_);
            return;
        }

        size_t sample_idx = 0;
        int frame_size = audio_enc_ctx_->frame_size;

        while (sample_idx + frame_size * channels <= samples.size()) {
            av_frame_make_writable(audio_frame_);
            
            float* l_channel = reinterpret_cast<float*>(audio_frame_->data[0]);
            float* r_channel = reinterpret_cast<float*>(audio_frame_->data[1]);
            
            for (int i = 0; i < frame_size; ++i) {
                l_channel[i] = samples[sample_idx + i * channels];
                r_channel[i] = samples[sample_idx + i * channels + 1];
            }
            
            audio_frame_->pts = audio_pts_;
            audio_pts_ += frame_size;
            sample_idx += frame_size * channels;

            encode_internal(audio_enc_ctx_, audio_stream_, audio_frame_);
        }

        encode_internal(audio_enc_ctx_, audio_stream_, nullptr);
        av_frame_free(&audio_frame_);
        log_ffmpeg("[ffmpeg] audio muxing complete");
    }

    void stop() {
        if (!fmt_ctx_) return;

        if (header_written_) {
            if (enc_ctx_ && stream_) encode_internal(enc_ctx_, stream_, nullptr);
            if (audio_enc_ctx_ && audio_stream_) encode_internal(audio_enc_ctx_, audio_stream_, nullptr);
            
            int ret = av_write_trailer(fmt_ctx_);
            if (ret < 0) {
                log_ffmpeg(fmt::format("[ffmpeg] [error] av_write_trailer failed with code {}", ret));
            } else {
                log_ffmpeg("[ffmpeg] video process finished successfully");
            }
            header_written_ = false;
        }

        cleanup_format_context();
        log_ffmpeg("[ffmpeg] encoder stopped");
    }

private:
    void cleanup_format_context() {
        if (!fmt_ctx_) return;

        if (!(fmt_ctx_->oformat->flags & AVFMT_NOFILE) && fmt_ctx_->pb) {
            avio_closep(&fmt_ctx_->pb);
        }
        avformat_free_context(fmt_ctx_);
        fmt_ctx_ = nullptr;
        stream_ = nullptr;
        audio_stream_ = nullptr;
    }

    void cleanup() {
        stop();
        if (enc_ctx_) avcodec_free_context(&enc_ctx_);
        if (audio_enc_ctx_) avcodec_free_context(&audio_enc_ctx_);
        if (yuv_frame_) av_frame_free(&yuv_frame_);
        if (rgb_frame_) av_frame_free(&rgb_frame_);
        if (pkt_) av_packet_free(&pkt_);
        if (sws_ctx_) sws_freeContext(sws_ctx_);
        is_valid_ = false;
    }

    void encode_internal(AVCodecContext* ctx, AVStream* st, AVFrame* frame) {
        if (!ctx || !st || !fmt_ctx_) return;
        int ret = avcodec_send_frame(ctx, frame);
        if (ret < 0) {
            log_ffmpeg(fmt::format("[ffmpeg] [error] error sending frame to encoder: {}", ret));
            return;
        }

        while (ret >= 0) {
            ret = avcodec_receive_packet(ctx, pkt_);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return;
            if (ret < 0) {
                log_ffmpeg(fmt::format("[ffmpeg] [error] error receiving packet from encoder: {}", ret));
                return;
            }
            
            pkt_->stream_index = st->index;
            av_packet_rescale_ts(pkt_, ctx->time_base, st->time_base);
            total_bytes_written_ += pkt_->size;
            av_interleaved_write_frame(fmt_ctx_, pkt_);
            av_packet_unref(pkt_);
        }
    }

    int width_, height_, fps_;
    int64_t pts_;
    int64_t audio_pts_;
    int64_t total_bytes_written_;
    std::chrono::steady_clock::time_point start_time_;
    AVCodecContext *enc_ctx_, *audio_enc_ctx_;
    AVFormatContext* fmt_ctx_;
    AVStream *stream_, *audio_stream_;
    AVFrame *yuv_frame_, *rgb_frame_, *audio_frame_;
    AVPacket* pkt_;
    SwsContext* sws_ctx_;
    bool is_valid_;
    bool header_written_;
};