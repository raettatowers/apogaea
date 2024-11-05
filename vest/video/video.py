"""Processes video and outputs header files."""

from typing import List, Optional
import argparse
import cv2
import numpy
import pathlib
import re

# TODO: Import these instead of copy/paste?
LED_COLUMN_COUNT = 32
LED_ROW_COUNT = 15


def debug_print(s: str) -> None:
    """Debug print."""
    print(s)


def process(arguments: argparse.Namespace) -> None:
    """Save outputs."""
    output_file_name_str = arguments.out if arguments.out else re.sub("\.[^.]+$", ".anim", arguments.video_file)
    output_name = pathlib.Path(output_file_name_str)
    if output_name.exists():
        debug_print(f"{output_name} already exists")
        return

    debug_print(f"Writing to {output_name}")
    try:
        process_inner(arguments, output_name)
    except Exception as exc:
        output_name.unlink()
        raise


def process_inner(arguments: argparse.Namespace, output_name: pathlib.Path) -> None:
    """Save outputs."""
    if arguments.center:
        center_index = 8
        target_width = (LED_COLUMN_COUNT // 2 - center_index) * 2
        target_height = LED_ROW_COUNT
    else:
        target_width = LED_COLUMN_COUNT
        target_height = LED_ROW_COUNT

    capture = cv2.VideoCapture(arguments.video_file)
    video_fps = capture.get(cv2.CAP_PROP_FPS)
    video_frame_count = int(capture.get(cv2.CAP_PROP_FRAME_COUNT))
    debug_print(
        f"Grabbing frames from {video_frame_count} frame video"
    )
    video_seconds = int(video_frame_count / video_fps)
    debug_print(f"{video_seconds // 60}:{video_seconds % 60:02d}")
    success, image = capture.read()
    height = len(image)
    width = len(image[0])

    def sample_to_int(sample: List[numpy.uint8]) -> int:
        """Converts a sample to int."""
        value = (sample[2] << 16) + (sample[1] << 8) + sample[0]
        return int(value)


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

    debug_print(
        f"video_ratio:{round(video_ratio, 3)} target_ratio:{round(target_ratio, 3)}"
    )
    debug_print(f"target_width:{target_width} target_height:{target_height}")
    debug_print(f"width:{width} height:{height}")
    debug_print(f"w_indexes:{w_indexes} h_indexes:{h_indexes}")

    count = 0
    with open(output_name, "wb") as file:
        # First, write info
        file.write(target_width.to_bytes(1, "little"))
        file.write(target_height.to_bytes(1, "little"))
        # Then milliseonds per frame. This probably won't ever exceed 255, but
        # use 2 bytes just in case.
        file.write(int(1000 / video_fps).to_bytes(2, "little"))

        while success:
            success, image = capture.read()
            if not success:
                break
            for h_index in h_indexes:
                for w_index in w_indexes:
                    # Write 4-byte samples? I don't know if it's worth trying to do 3
                    sample_int = sample_to_int(image[h_index][w_index])
                    file.write(sample_int.to_bytes(4, "little"))

            count += 1

    debug_print(f"Wrote {count} frames")


def make_parser() -> argparse.ArgumentParser:
    """Returns an argument parser."""
    parser = argparse.ArgumentParser(
        description="Video parser. Outputs animation files for display on the vest."
    )
    parser.add_argument(
        "-c",
        "--center",
        action="store_true",
        help="Center and crop the video to the back of the vest.",
        default=False,
    )
    parser.add_argument(
        "-o",
        "--out",
        type=str,
        help="Output file name",
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
