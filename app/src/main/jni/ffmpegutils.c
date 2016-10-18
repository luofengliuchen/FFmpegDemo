#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/log.h"

#ifdef ANDROID
#include <jni.h>
#include <android/log.h>
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)
#else
#define LOGE(format, ...)  printf("(>_<) " format "\n", ##__VA_ARGS__)
#define LOGI(format, ...)  printf("(^_^) " format "\n", ##__VA_ARGS__)
#endif
/*
 * Class:     luofeng_com_ffmpegdemo_FFmpegUtils
 * Method:    getFFmpegInfo1
 * Signature: ()Ljava/lang/String;
 视频解码器，视频格式转YUV序列
 */
JNIEXPORT jint JNICALL Java_luofeng_com_ffmpegdemo_FFmpegUtils_getFFmpegInfo1
  (JNIEnv *env, jobject obj,jstring input_jstr, jstring output_jstr){

    AVFrame *pFrame, *pFrameYUV;
    		AVFormatContext *pFormatCtx;
    		int             i, videoindex;
    		AVCodecParameters  *pCodecCtx;
    		AVCodec         *pCodec;
    		AVCodecContext *context;
    		uint8_t *out_buffer;
    		AVPacket *packet;
    		int y_size;
    		int ret1,ret2, got_picture;
    		struct SwsContext *img_convert_ctx;
    		FILE *fp_yuv;
    		int frame_cnt;
    		clock_t time_start, time_finish;
    		double  time_duration = 0.0;

    		char input_str[500] = {0};
    		char output_str[500] = {0};
    		char info[1000] = { 0 };

            strcpy(input_str,(*env)->GetStringUTFChars(env,input_jstr, NULL));
            strcpy(output_str,(*env)->GetStringUTFChars(env,output_jstr, NULL));
    		av_register_all();
    		//avformat_network_init();
    		//pFormatCtx = avformat_alloc_context();
    		pFormatCtx = NULL;
    		context = avcodec_alloc_context3(pCodec);
            int state = avformat_open_input(&pFormatCtx, input_str, NULL, NULL);
    		if (state < 0) {
    		char buf[500] = {0};
    		    av_strerror(state, buf, 1024);
    			LOGE("Couldn't open file: %d(%s)", state, buf);
    			return -1;
    		}
    		if (avformat_find_stream_info(pFormatCtx, NULL)<0) {
    			LOGE("Couldn't find stream information.\n");
    			return -1;
    		}
    		videoindex = -1;
    		for (i = 0; i<pFormatCtx->nb_streams; i++)
    			if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
    				videoindex = i;
    				break;
    			}
    		if (videoindex == -1) {
    			LOGE("Couldn't find a video stream.\n");
    			return -1;
    		}
    		pCodecCtx = pFormatCtx->streams[videoindex]->codecpar;
    		pCodec = avcodec_find_decoder(pCodecCtx->codec_id);


    		if (pCodec == NULL) {
    			LOGE("Couldn't find Codec.\n");
    			return -1;
    		}
    		if (avcodec_open2(context, pCodec, NULL)<0) {
    			LOGE("Couldn't open codec.\n");
    			return -1;
    		}

    		pFrame = av_frame_alloc();
    		pFrameYUV = av_frame_alloc();
    		out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
    		av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
    			AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);


    		packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->format,
    			pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);


    		sprintf(info, "[Input     ]%s\n", input_str);
    		sprintf(info, "%s[Output    ]%s\n", info, output_str);
    		sprintf(info, "%s[Format    ]%s\n", info, pFormatCtx->iformat->name);
    		//sprintf(info, "%s[Codec     ]%s\n", info, pCodecCtx->codec->name);
    		sprintf(info, "%s[Resolution]%dx%d\n", info, pCodecCtx->width, pCodecCtx->height);


    		fp_yuv = fopen(output_str, "wb+");
    		if (fp_yuv == NULL) {
    			printf("Cannot open output file.\n");
    			return -1;
    		}

    		frame_cnt = 0;
    		time_start = clock();

    		while (av_read_frame(pFormatCtx, packet) >= 0) {
    		LOGE("to decode");
    			if (packet->stream_index == videoindex) {
    				//avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
    				ret1 = avcodec_send_packet(context,packet);

    				if (ret1 < 0) {
    				    char buf2[500] = {0};
                        av_strerror(ret1, buf2, 1024);
    					LOGE("Decode Error: (%d)(%s)",ret1,buf2);
    					return -1;
    				}
    				ret2 = avcodec_receive_frame(context,pFrame);
    				LOGE("to decode(^=^)+++%d",ret2);
    				if (ret2==0) {
    				LOGE("to decode(^=^)");
    					sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
    						pFrameYUV->data, pFrameYUV->linesize);

    					y_size = pCodecCtx->width*pCodecCtx->height;
    					fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y
    					fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
    					fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
    																		//Output info
    					char pictype_str[10] = { 0 };
    					switch (pFrame->pict_type) {
    					case AV_PICTURE_TYPE_I:sprintf(pictype_str, "I"); break;
    					case AV_PICTURE_TYPE_P:sprintf(pictype_str, "P"); break;
    					case AV_PICTURE_TYPE_B:sprintf(pictype_str, "B"); break;
    					default:sprintf(pictype_str, "Other"); break;
    					}
    					LOGI("Frame Index: %5d. Type:%s", frame_cnt, pictype_str);
    					frame_cnt++;
    				}
    			}
    			av_packet_unref(packet);
    		}
    		//flush decoder
    		//FIX: Flush Frames remained in Codec
    		while (1) {
    			//ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
    			//if (ret < 0)
    			//	break;
    			ret1 = avcodec_send_packet(context,packet);
                if (ret1 < 0) {
                    char buf2[500] = {0};
                    av_strerror(ret1, buf2, 1024);
                    LOGE("Decode Error: (%d)(%s)",ret1,buf2);
                    break;
                }
                ret2 = avcodec_receive_frame(context,pFrame);
    			if (ret2!=0)
    				break;
    			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
    				pFrameYUV->data, pFrameYUV->linesize);
    			int y_size = pCodecCtx->width*pCodecCtx->height;
    			fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y
    			fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
    			fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
    							//Output info
    			char pictype_str[10] = { 0 };
    			switch (pFrame->pict_type) {
    			case AV_PICTURE_TYPE_I:sprintf(pictype_str, "I"); break;
    			case AV_PICTURE_TYPE_P:sprintf(pictype_str, "P"); break;
    			case AV_PICTURE_TYPE_B:sprintf(pictype_str, "B"); break;
    			default:sprintf(pictype_str, "Other"); break;
    			}
    			LOGI("Frame Index: %5d. Type:%s", frame_cnt, pictype_str);
    			frame_cnt++;
    		}
    		time_finish = clock();
    		time_duration = (double)(time_finish - time_start);

    		sprintf(info, "%s[Time      ]%fms\n", info, time_duration);
    		sprintf(info, "%s[Count     ]%d\n", info, frame_cnt);

    		sws_freeContext(img_convert_ctx);

    		fclose(fp_yuv);

    		av_frame_free(&pFrameYUV);
    		av_frame_free(&pFrame);
    		avcodec_close(context);
    		avformat_close_input(&pFormatCtx);

    		return 1;

  };

/*
 * Class:     luofeng_com_ffmpegdemo_FFmpegUtils
 * Method:    getFFmpegInfo2
 * Signature: ()Ljava/lang/String;
 *视频推流器
 */
JNIEXPORT jint JNICALL Java_luofeng_com_ffmpegdemo_FFmpegUtils_getFFmpegInfo2
  (JNIEnv * env, jobject obj,jstring input_jstr, jstring output_jstr){

      AVOutputFormat *ofmt = NULL;
      AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL,*fmt_ctx = NULL;
      AVPacket pkt;
      AVCodec         *pCodec,*outpCodec;
      AVCodecContext *context,*outContext;

      int ret, i;
      char input_str[500]={0};
      char output_str[500]={0};
      char info[1000]={0};
      sprintf(input_str,"%s",(*env)->GetStringUTFChars(env,input_jstr, NULL));
      sprintf(output_str,"%s",(*env)->GetStringUTFChars(env,output_jstr, NULL));

        //复制传入的字符串
        strcpy(input_str,(*env)->GetStringUTFChars(env,input_jstr, NULL));
        strcpy(output_str,(*env)->GetStringUTFChars(env,output_jstr, NULL));

      //input_str  = "cuc_ieschool.flv";
      //output_str = "rtmp://localhost/publishlive/livestream";
      //output_str = "rtp://233.233.233.233:6666";

        //注册组件
      av_register_all();
      //Network
      avformat_network_init();

      //
      if ((ret = avformat_open_input(&ifmt_ctx, input_str, 0, 0)) < 0) {
          LOGE( "Could not open input file.");
          goto end;
      }
      //将基本信息写入ifmt_ctx
      if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
          LOGE( "Failed to retrieve input stream information");
          goto end;
      }

      int videoindex=-1;
      for(i=0; i<ifmt_ctx->nb_streams; i++)
          if(ifmt_ctx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){
              videoindex=i;
              break;
          }

          //av_dump_format(ifmt_ctx, 0, input_str, 0);

      //Output
      avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv",output_str); //RTMP
      //avformat_alloc_output_context2(&ofmt_ctx, NULL, "mpegts", output_str);//UDP

      if (!ofmt_ctx) {
          LOGE( "Could not create output context\n");
          ret = AVERROR_UNKNOWN;
          goto end;
      }

      ofmt = ofmt_ctx->oformat;
      for (i = 0; i < ifmt_ctx->nb_streams; i++) {
            LOGE( "stream_id==%d----%d",ifmt_ctx->nb_streams,i);

            if(i == videoindex){

            //Create output AVStream according to input AVStream
                      AVStream *in_stream = ifmt_ctx->streams[i];
                      //通过
                      pCodec = avcodec_find_decoder(ofmt->video_codec);
                      AVStream *out_stream = avformat_new_stream(ofmt_ctx, pCodec);
                        if (!out_stream) {
                          LOGE( "Failed allocating  output stream\n");
                          ret = AVERROR_UNKNOWN;
                          goto end;
                      }

                      //context = avcodec_alloc_context3(pCodec);
                      //Copy the settings of AVCodecContext
                      //ret = avcodec_parameters_from_context(out_stream->codecpar,context);
                      //avcodec_copy_context(out_stream->codec, in_stream->codec);


                      avcodec_parameters_copy(out_stream->codecpar,in_stream->codecpar);
                      //out_stream->time_base.num = in_stream->time_base.num;
                      //out_stream->time_base.den = in_stream->time_base.den;
                      //outpCodec = avcodec_find_decoder(out_stream->codecpar->codec_id);
                      //outContext = avcodec_alloc_context3(outpCodec);
                      if (ret < 0) {
                            LOGE( "Failed to copy context from input to output stream codec context\n");
                            goto end;
                       }

                      out_stream->codecpar->codec_tag = 0;
                      //ofmt_ctx->oformat->codec_tag = 0;
                      //if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER){
                      //LOGE( "stream_flags_set\n");
                      //  outContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
                       // ofmt_ctx->oformat->flags |= CODEC_FLAG_GLOBAL_HEADER;
                      //}


            }


      }

      //Open output URL
      if (!(ofmt->flags & AVFMT_NOFILE)) {
          ret = avio_open2(&ofmt_ctx->pb, output_str, AVIO_FLAG_WRITE,NULL,NULL);
          if (ret < 0) {
              LOGE( "Could not open output URL '%s'", output_str);
              goto end;
          }
      }
      //Write file header
      //AVOutputFormat
      //ofmt_ctx->oformat

      ret = avformat_write_header(ofmt_ctx, NULL);
      LOGE( "write_header '%d'", ret);
      if (ret < 0) {
         LOGE( "Error occurred when opening output URL%d---%s\n",ret,av_err2str(ret));
          goto end;
      }

      int frame_index=0;

      int64_t start_time=av_gettime();
      while (1) {
          AVStream *in_stream, *out_stream;
          //Get an AVPacket
          ret = av_read_frame(ifmt_ctx, &pkt);
          if (ret < 0){
          LOGE( "av_read_frame_break%d---%s\n",ret,av_err2str(ret));
          break;
          }

          //FIX：No PTS (Example: Raw H.264)
          //Simple Write PTS
          if(pkt.pts==AV_NOPTS_VALUE){
              //Write PTS
              LOGE( "AV_NOPTS_VALUE\n");
              AVRational time_base1=ifmt_ctx->streams[videoindex]->time_base;
              //Duration between 2 frames (us)
              int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
              //Parameters
              pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
              pkt.dts=pkt.pts;
              pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
          }
          //Important:Delay
          if(pkt.stream_index==videoindex){
              AVRational time_base=ifmt_ctx->streams[videoindex]->time_base;
              LOGE( "time_base==%d\n",pkt.stream_index);
              AVRational time_base_q={1,AV_TIME_BASE};
              int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
              int64_t now_time = av_gettime() - start_time;
              LOGE( "new_time%lld-----pts_time%lld\n",now_time,pts_time);
              if (pts_time > now_time)
                  av_usleep(pts_time - now_time);
          }

          in_stream  = ifmt_ctx->streams[pkt.stream_index];
          out_stream = ofmt_ctx->streams[pkt.stream_index];

          LOGE( "out_stream==%d\n",ofmt_ctx->nb_streams);

          /* copy packet */
          //Convert PTS/DTS
          LOGE("--------pkt.pts:%lld----pkt.dts:%lld----pkt.duration:%lld\n",pkt.pts,pkt.dts,pkt.duration);
          pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
          pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
          pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
          pkt.pos = -1;

          LOGE("pkt.pts:%lld----pkt.dts:%lld----pkt.duration:%lld\n",pkt.pts,pkt.dts,pkt.duration);
          //Print to Screen
          if(pkt.stream_index==videoindex){
              LOGE("Send %8d video frames to output URL\n",pkt.stream_index);
              frame_index++;

          ret = av_write_frame(ofmt_ctx, &pkt);
          //ret = av_interleaved_write_frame(ofmt_ctx, &pkt);

          if (ret < 0) {
          LOGE( "Error muxing packet%d---%s\n",ret,av_err2str(ret));
              break;
          }
          }
          av_packet_unref(&pkt);

      }
      //Write file trailer
      av_write_trailer(ofmt_ctx);
  end:
      avformat_close_input(&ifmt_ctx);
      /* close output */
      if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
          avio_close(ofmt_ctx->pb);
      avformat_free_context(ofmt_ctx);
      if (ret < 0 && ret != AVERROR_EOF) {
          LOGE( "Error occurred.\n");
          return -1;
      }
      return 0;

  };



int ffmpegmain(int argc, char **argv);

//Output FFmpeg's av_log()
void custom_log(void *ptr, int level, const char* fmt, va_list vl){

    //To TXT file

    FILE *fp=fopen("/storage/emulated/0/av_log.txt","a+");
    if(fp){
        vfprintf(fp,fmt,vl);
        fflush(fp);
        fclose(fp);
    }
    //To Logcat
    //LOGE(fmt, vl);
}
/*
 * Class:     luofeng_com_ffmpegdemo_FFmpegUtils
 * Method:    getFFmpegInfo3
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_luofeng_com_ffmpegdemo_FFmpegUtils_getFFmpegInfo3
  (JNIEnv *env, jobject obj, jint cmdnum,jobjectArray cmdline){

   /**FFmpeg av_log() callback
    av_log_set_callback(custom_log);

    int argc=cmdnum;
    char** argv=(char**)malloc(sizeof(char*)*argc);

    int i=0;
    for(i=0;i<argc;i++)
    {
      jstring string=(*env)->GetObjectArrayElement(env,cmdline,i);
      const char* tmp=(*env)->GetStringUTFChars(env,string,0);
      argv[i]=(char*)malloc(sizeof(char)*1024);
      strcpy(argv[i],tmp);
    }

    ffmpegmain(argc,argv);

    for(i=0;i<argc;i++){
      free(argv[i]);
    }
    free(argv);
*/
  };

/*
 * Class:     luofeng_com_ffmpegdemo_FFmpegUtils
 * Method:    getFFmpegInfo4
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_luofeng_com_ffmpegdemo_FFmpegUtils_getFFmpegInfo4
  (JNIEnv *, jobject);

/*
 * Class:     luofeng_com_ffmpegdemo_FFmpegUtils
 * Method:    getFFmpegInfo5
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_luofeng_com_ffmpegdemo_FFmpegUtils_getFFmpegInfo5
  (JNIEnv *, jobject);

/*
 * Class:     luofeng_com_ffmpegdemo_FFmpegUtils
 * Method:    getFFmpegInfo6
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_luofeng_com_ffmpegdemo_FFmpegUtils_getFFmpegInfo6
  (JNIEnv *, jobject);
