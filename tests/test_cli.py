#!/usr/bin/env python3
"""End-to-end CLI round trips using only the portable PPM interface."""
import pathlib
import random
import subprocess
import sys
import tempfile

encoder, decoder = sys.argv[1:3]


def run(*args, good=True):
    result = subprocess.run(args, capture_output=True, text=True)
    if (result.returncode == 0) != good:
        raise AssertionError(f'Unexpected exit code: {args!r}\n{result.stdout}\n{result.stderr}')


with tempfile.TemporaryDirectory() as tmp:
    tmp = pathlib.Path(tmp)
    rng = random.Random(123)
    frames = []
    for n in range(4):
        # Include literal whitespace and 0xff bytes in pixel data.
        rgb = bytes(rng.randrange(256) for _ in range(11 * 7 * 3)) if n == 0 else bytearray(frames[-1])
        if n > 0:
            rgb[3*n:3*n+3] = bytes((n * 50, 10, 255))
        frames.append(bytes(rgb))
        (tmp / f'{n:04d}.ppm').write_bytes(b'P6\n11 7\n255\n' + frames[-1])
    out = tmp / 'movie.qov'
    run(encoder, str(out), *(str(tmp / f'{i:04d}.ppm') for i in range(4)))
    run(decoder, str(out), str(tmp / 'decoded'))
    for i, expected in enumerate(frames, 1):
        actual = (tmp / 'decoded' / f'frame{i:06d}.ppm').read_bytes()
        assert actual == b'P6\n11 7\n255\n' + expected

    data = out.read_bytes()
    (tmp / 'truncated.qov').write_bytes(data[:-3])
    run(decoder, str(tmp / 'truncated.qov'), str(tmp / 'bad1'), good=False)
    (tmp / 'trailing.qov').write_bytes(data + b'garbage')
    run(decoder, str(tmp / 'trailing.qov'), str(tmp / 'bad2'), good=False)
    (tmp / 'wrongdims.ppm').write_bytes(b'P6\n1 1\n255\n\x00\x00\x00')
    run(encoder, str(tmp / 'bad.qov'), str(tmp / '0000.ppm'),
        str(tmp / 'wrongdims.ppm'), good=False)
    (tmp / 'short.ppm').write_bytes(b'P6\n11 7\n255\n\x01\x02')
    run(encoder, str(tmp / 'short.qov'), str(tmp / 'short.ppm'), good=False)
print('CLI: valid multi-frame roundtrip and malformed input checks passed')
