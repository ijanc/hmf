hmf - hide my face
==================
hmf is a small command-line tool that detects faces in an image and
replaces each detection with a user-supplied image.
It is written in C++ and uses OpenCV's YuNet DNN face detector, which
handles tilted, partial, profile and infant faces far more reliably
than the legacy Haar cascades.

![hmf overlaying a mask on a detected face](anonymised.jpg)


Requirements
------------
- OpenCV 4.5.4+

The YuNet ONNX model is shipped in this repository as `yunet.onnx`, so
no extra download is needed to build and run. If you prefer to fetch a
fresh copy from the OpenCV Model Zoo, `make fetch-model` does that --
it requires `curl`.


Installation
------------
Edit Makefile to match your local setup (hmf is installed into
the /usr/local/bin namespace by default).

Build:

    make

Then install the binary, the manual page and the model:

    sudo make install

This places:

    /usr/local/bin/hmf
    /usr/local/share/man/man1/hmf.1
    /usr/local/share/hmf/yunet.onnx


Running hmf
-----------
Replace every detected face in `group.jpg` with `mask.png`, using the
installed model:

    hmf -i group.jpg -o anonymised.jpg -f mask.png

Run from the build tree (no install required) with verbose detection
output:

    ./hmf -v -m ./yunet.onnx -i baby.jpg -o out.jpg -f mask.png

Lower the confidence threshold to recover marginal detections (default
is 0.6):

    hmf -t 0.4 -i hard.jpg -o out.jpg -f mask.png

Grow the detected bounding box by 20% before overlaying, so the mask
also covers the forehead and chin:

    hmf -p 20 -i baby.jpg -o out.jpg -f mask.png


Options
-------
    -i input      source image (any OpenCV-readable format)
    -o output     destination image (encoder picked from extension)
    -f face       replacement image; PNG with alpha is blended properly
    -m model      path to the YuNet ONNX model
                  (default /usr/local/share/hmf/yunet.onnx)
    -t threshold  confidence threshold in [0.0, 1.0] (default 0.6)
    -p percent    grow each detected bounding box by N% before overlay,
                  keeping it centred (default 0; try 10-30 for tight
                  detectors like YuNet on baby faces)
    -v            verbose: print model path, image sizes and per-face
                  bounding box + score on stderr

Exit status is 0 on success and 1 on any I/O or argument error. A run
that detects zero faces still exits 0; the output is written unchanged.


Caveats
-------
Heavy occlusion, motion blur or very low resolution can still cause
missed detections. Lowering `-t` recovers marginal detections at the
cost of false positives, especially on busy backgrounds.


License
-------
ISC - see LICENSE.
