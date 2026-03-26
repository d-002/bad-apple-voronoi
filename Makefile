IMAGES_INPUT=images/1_raw
IMAGES_BNW=images/2_bnw
IMAGES_VORONOI=images/3_voronoi
IMAGES_OUTPUT=images/4_output
EXTRACTED_AUDIO=audio.m4a
VIDEO_IN=video_in.mp4
VIDEO_OUT=video_out.mp4

PYTHON:=python3
CFLAGS=-Wall -Wextra -Werror -Wvla -std=c99 -pedantic -O2
CPPFLAGS=-Isrc
LDLIBS=-lm

TARGET=voronoi_fitter
OBJ_MAIN=src/main.o
OBJ=src/image/image.o \
	src/files/files.o \
	src/logger/logger.o \
	src/signals/signals.o \
	src/utils/now.o src/utils/thread_conf.o \
	src/voronoi/apply.o src/voronoi/cost.o src/voronoi/fit.o src/voronoi/shared_data.o src/voronoi/voronoi.o

.PHONY: all clean

# pipeline steps

all: $(TARGET) extract_frames convert_images1 fit_voronoi convert_images2 group_frames

extract_frames:
	@echo -e "\n##### 1/5 -- FRAMES AND VIDEO EXTRACTION"
	@[ $(IMAGES_INPUT)/frame0001.png -ot $(VIDEO_IN) ] && ffmpeg -i \
		$(VIDEO_IN) $(IMAGES_INPUT)/frame%04d.png -y -hide_banner \
		-loglevel warning && ffmpeg -i $(VIDEO_IN) -vn -acodec copy \
		$(EXTRACTED_AUDIO) -y -hide_banner -loglevel warning \
		|| echo "Skipping frames extraction"

convert_images1:
	@echo -e "\n##### 2/5 -- IMAGES CONVERSION 1"
	@$(PYTHON) py/img2bnw.py $(IMAGES_INPUT) $(IMAGES_BNW)

fit_voronoi:
	@echo -e "\n##### 3/5 -- VORONOI FITTING"
	@./$(TARGET) $(IMAGES_BNW) $(IMAGES_VORONOI)

convert_images2:
	@echo -e "\n##### 4/5 -- IMAGES CONVERSION 2"
	@$(PYTHON) py/bnw2img.py $(IMAGES_VORONOI) $(IMAGES_OUTPUT)

group_frames:
	@echo -e "\n##### 5/5 -- FRAMES AND AUDIO GROUPING"
	@ffmpeg -framerate 60 -i $(IMAGES_OUTPUT)/frame%04d.png -c:v libx264 \
		-pix_fmt yuv420p $(VIDEO_OUT) -y -hide_banner -loglevel warning \
		&& ffmpeg -i $(VIDEO_OUT) -i $(EXTRACTED_AUDIO) -c:v copy -c:a aac \
		-strict experimental temp_video.mp4 -y -hide_banner -loglevel warning \
		&& mv temp_video.mp4 $(VIDEO_OUT) || echo "Skipping frames grouping"

# Voronoi part of the pipeline, with C code

$(TARGET): $(OBJ_MAIN) $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS) $(LDFLAGS)

dev: CFLAGS+=-fsanitize=address -g
dev: LDLIBS+=-fsanitize=address
dev: all

clean:
	$(RM) $(TARGET) $(OBJ_MAIN) $(OBJ)

clean-full: clean
	$(RM) $(IMAGES_INPUT)/* $(IMAGES_BNW)/* $(IMAGES_VORONOI)/* \
		$(IMAGES_OUTPUT)/* $(OUTPUT)
	$(RM) $(VIDEO_OUT) $(EXTRACTED_AUDIO)
	$(RM) -r **/__pycache__ saved_state.bin
