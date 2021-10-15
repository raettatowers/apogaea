"""Processes video and outputs header files."""

from typing import List, Optional
import argparse
import cv2
import re
import sys

# TODO: Import these instead of copy/paste
led_counts = [6, 7, 10, 11, 12, 12, 6, 6, 7, 11, 11, 11, 11, 11]
led_counts = led_counts + list(reversed(led_counts))


def process(arguments: argparse.Namespace) -> None:
    """Save outputs."""
    if arguments.center:
        center_index = 8
        target_width = (len(led_counts) // 2 - center_index) * 2
        target_height = max((led_counts[i] for i in range(center_index, len(led_counts) // 2)))
    else:
        target_width = len(led_counts)
        target_height = max(led_counts)

    capture = cv2.VideoCapture(arguments.video_file)
    fps = arguments.fps
    video_fps = capture.get(cv2.CAP_PROP_FPS)
    if fps > video_fps:
        fps = video_fps
    video_frame_count = int(capture.get(cv2.CAP_PROP_FRAME_COUNT))
    sys.stderr.write(
        f"Grabbing {arguments.frame_count} frames from {video_frame_count} frame video\n"
    )
    sys.stderr.write(f"Grabbing {fps} FPS from {round(video_fps, 3)} FPS video\n")
    capture_seconds = int(arguments.frame_count / fps)
    sys.stderr.write(
        f"Capture time: {capture_seconds // 60}:{capture_seconds % 60:02d} of "
    )
    video_seconds = int(video_frame_count / video_fps)
    sys.stderr.write(f"{video_seconds // 60}:{video_seconds % 60:02d}\n")
    success, image = capture.read()
    height = len(image)
    width = len(image[0])

    def sample_to_int(sample: List[int]) -> int:
        """Converts a sample to int."""
        value = (sample[2] << 16) + (sample[1] << 8) + sample[0] 
        return value

    name = re.sub(r"\.[a-zA-Z0-9]+$", "", arguments.video_file)
    name = re.sub("[^a-zA-Z0-9-]", "", name)
    name = name.replace("-", "_").upper()
    while not re.match("^[a-zA-Z]", name) and len(name) > 0:
        name = name[1:]
    if len(name) == 0:
        sys.stderr.write("Invalid name")
        sys.exit(0)

    print(f"const uint16_t {name}_MILLIS_PER_FRAME = {1000 / fps};")
    print(f"const PROGMEM uint32_t {name}[][{target_width * target_height}] = {{")

    video_ratio = width / height
    target_ratio = target_width / target_height

    if video_ratio > target_ratio:
        step = height / target_height
    else:
        step = height / target_height / target_ratio
    h_indexes = [h * step for h in range(target_height)]
    center = (height - h_indexes[-1]) / 2
    h_indexes = [int(h + center) for h in h_indexes]

    if video_ratio > target_ratio:
        step = width / target_width / video_ratio
    else:
        step = width / target_width
    w_indexes = [w * step for w in range(target_width)]
    center = (width - w_indexes[-1]) / 2
    w_indexes = [int(w + center) for w in w_indexes]

    sys.stderr.write(
        f"video_ratio:{round(video_ratio, 3)} target_ratio:{round(target_ratio, 3)}\n"
    )
    sys.stderr.write(f"target_width:{target_width} target_height:{target_height}\n")
    sys.stderr.write(f"width:{width} height:{height}\n")
    sys.stderr.write(f"w_indexes:{w_indexes} h_indexes:{h_indexes}\n")

    millis = 0
    count = 0
    while success:
        success, image = capture.read()
        if not success:
            break
        frame = []
        for h_index in h_indexes:
            for w_index in w_indexes:
                frame.append(sample_to_int(image[h_index][w_index]))

        print(f"{{{', '.join((str(v) for v in frame))}}},")
        count += 1
        millis += 1000 / fps
        if arguments.frame_count is not None and count >= arguments.frame_count:
            break

        while millis > 0:
            success, _ = capture.read()
            millis -= 1000 / video_fps

    print("};")
    sys.stderr.write(f"Wrote {count} frames\n")


def make_parser() -> argparse.ArgumentParser:
    """Returns an argument parser."""
    parser = argparse.ArgumentParser(
        description="Video parser. Creates a C++ header file with information to draw the video on the vest."
    )
    parser.add_argument(
        "--frame-count",
        dest="frame_count",
        type=int,
        help="Number of frames to sample.",
        default=100,
    )
    parser.add_argument(
        "--fps",
        dest="fps",
        type=int,
        help="Frames per second.",
        default=10,
    )
    parser.add_argument(
        "--center",
        action="store_true",
        help="Center and crop the video to the back of the vest.",
        default=False,
    )
    parser.add_argument("video_file", help="The video to read data from.")
    return parser


def main() -> None:
    """Main."""

    parser = make_parser()
    arguments = parser.parse_args()
    process(arguments)


if __name__ == "__main__":
    main()
