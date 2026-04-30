/*
 * Copyright (c) 2025-2026 Murilo Ijanc' <murilo@ijanc.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>

#define DEFAULT_MODEL	"/usr/local/share/hmf/yunet.onnx"
#define DEFAULT_THRESH	0.6f
#define NMS_THRESH	0.3f
#define TOP_K		5000

extern char 	*__progname;

static void	overlay(cv::Mat &, const cv::Mat &, const cv::Rect &);
static void	usage(void) __attribute__((noreturn));

int
main(int argc, char *argv[])
{
	const char *input, *output, *face_path, *model_path;
	cv::Ptr<cv::FaceDetectorYN> detector;
	cv::Mat img, face, detections;
	float threshold;
	int ch, i, padding, verbose;

	input = output = face_path = NULL;
	model_path = DEFAULT_MODEL;
	threshold = DEFAULT_THRESH;
	padding = 0;
	verbose = 0;

	while ((ch = getopt(argc, argv, "f:i:m:o:p:t:v")) != -1) {
		switch (ch) {
		case 'f':
			face_path = optarg;
			break;
		case 'i':
			input = optarg;
			break;
		case 'm':
			model_path = optarg;
			break;
		case 'o':
			output = optarg;
			break;
		case 'p':
			padding = atoi(optarg);
			if (padding < 0)
				errx(1, "padding must be >= 0");
			break;
		case 't':
			threshold = strtof(optarg, NULL);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 0 || input == NULL || output == NULL || face_path == NULL)
		usage();

	if (access(model_path, R_OK) != 0)
		err(1, "%s", model_path);

	img = cv::imread(input);
	if (img.empty())
		errx(1, "failed to read input image: %s", input);

	face = cv::imread(face_path, cv::IMREAD_UNCHANGED);
	if (face.empty())
		errx(1, "failed to read face image: %s", face_path);

	try {
		detector = cv::FaceDetectorYN::create(model_path, "",
		    cv::Size(img.cols, img.rows),
		    threshold, NMS_THRESH, TOP_K);
	} catch (const cv::Exception &) {
		errx(1, "failed to load model: %s", model_path);
	}

	if (verbose) {
		fprintf(stderr, "model:     %s\n", model_path);
		fprintf(stderr, "threshold: %.2f\n", threshold);
		fprintf(stderr, "input:     %dx%d, %d channel(s)\n",
		    img.cols, img.rows, img.channels());
		fprintf(stderr, "face:      %dx%d, %d channel(s)\n",
		    face.cols, face.rows, face.channels());
	}

	detector->detect(img, detections);

	fprintf(stderr, "found %d face(s)\n", detections.rows);

	for (i = 0; i < detections.rows; i++) {
		cv::Rect roi;
		float score;
		int x, y, w, h, dw, dh;

		x = (int)detections.at<float>(i, 0);
		y = (int)detections.at<float>(i, 1);
		w = (int)detections.at<float>(i, 2);
		h = (int)detections.at<float>(i, 3);
		score = detections.at<float>(i, 14);

		dw = w * padding / 100;
		dh = h * padding / 100;
		roi = cv::Rect(x - dw / 2, y - dh / 2, w + dw, h + dh)
		    & cv::Rect(0, 0, img.cols, img.rows);
		if (roi.width <= 0 || roi.height <= 0)
			continue;
		if (verbose)
			fprintf(stderr,
			    "  face: x=%d y=%d w=%d h=%d score=%.3f\n",
			    roi.x, roi.y, roi.width, roi.height, score);
		overlay(img, face, roi);
	}

	if (!cv::imwrite(output, img))
		errx(1, "failed to write output: %s", output);

	return 0;
}

static void
overlay(cv::Mat &img, const cv::Mat &face, const cv::Rect &roi)
{
	cv::Mat resized;
	int x, y;

	cv::resize(face, resized, roi.size());

	if (resized.channels() != 4) {
		resized.copyTo(img(roi));
		return;
	}

	for (y = 0; y < roi.height; y++) {
		cv::Vec4b *srow;
		cv::Vec3b *drow;

		srow = resized.ptr<cv::Vec4b>(y);
		drow = img.ptr<cv::Vec3b>(roi.y + y) + roi.x;

		for (x = 0; x < roi.width; x++) {
			float a;

			a = srow[x][3] / 255.0f;
			drow[x][0] = cv::saturate_cast<uchar>(
			    srow[x][0] * a + drow[x][0] * (1 - a));
			drow[x][1] = cv::saturate_cast<uchar>(
			    srow[x][1] * a + drow[x][1] * (1 - a));
			drow[x][2] = cv::saturate_cast<uchar>(
			    srow[x][2] * a + drow[x][2] * (1 - a));
		}
	}
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s -i input -o output -f face "
	    "[-m model] [-t threshold] [-p percent] [-v]\n", __progname);
	exit(1);
}
