IMAGES_INPUT=images/1_raw
IMAGES_BNW=images/2_bnw
IMAGES_VORONOI=images/3_voronoi
IMAGES_OUTPUT=images/4_output
EXTRACTED_AUDIO=audio.m4a
VIDEO_IN=video_in.mp4
VIDEO_OUT=video_out.mp4

PYTHON:=python3
CFLAGS=-Wall -Wextra -Werror -Wvla -std=c99 -pedantic -O1
CPPFLAGS=-Isrc

TARGET=voronoi_fitter
OBJ_MAIN=src/main.o
OBJ=src/logger/logger.o \
	src/voronoi/voronoi.o \
	src/image/image.o

.PHONY: all clean

# pipeline steps

all: extract_frames convert_images1 fit_voronoi convert_images2 group_frames

extract_frames:
	[ $(IMAGES_INPUT)/frame0001.png -ot $(VIDEO_IN) ] && ffmpeg -i $(VIDEO_IN) \
		$(IMAGES_INPUT)/frame%04d.png || echo "Skipping frames extraction"
	# also extract audio
	ffmpeg -i $(VIDEO_IN) -vn -acodec copy $(EXTRACTED_AUDIO)

convert_images1:
	$(PYTHON) py/img2bnw.py $(IMAGES_INPUT) $(IMAGES_BNW)

fit_voronoi: $(TARGET)
	./$(TARGET) $(IMAGES_BNW) $(IMAGES_VORONOI)

convert_images2:
	$(PYTHON) py/bnw2img.py $(IMAGES_VORONOI) $(IMAGES_OUTPUT)

group_frames:
	$(RM) $(VIDEO_OUT)
	[ $(IMAGES_OUTPUT)/frame0001.png -nt $(VIDEO_OUT) ] \
		&& ffmpeg -framerate 60 -i $(IMAGES_OUTPUT)/frame%04d.png -c:v libx264 \
		-pix_fmt yuv420p $(VIDEO_OUT) && ffmpeg -i $(VIDEO_OUT) -i \
		$(EXTRACTED_AUDIO) -c:v copy -c:a aac -strict experimental \
		temp_video.mp4 && mv temp_video.mp4 $(VIDEO_OUT) \
		|| echo "Skipping frames grouping"

# Voronoi part of the pipeline, with C code

$(TARGET): $(OBJ_MAIN) $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS) $(LDFLAGS)

dev: CFLAGS+=-fsanitize=address -g
dev: LDLIBS+=-fsanitize=address
dev: all

clean:
	$(RM) $(TARGET) $(OBJ_MAIN) $(OBJ)

clean-cache:
	$(RM) $(IMAGES_INPUT)/* $(IMAGES_BNW)/* $(IMAGES_VORONOI)/* \
		$(IMAGES_OUTPUT)/* $(OUTPUT)
	$(RM) $(VIDEO_OUT) $(EXTRACTED_AUDIO)
	$(RM) -r **/__pycache__
