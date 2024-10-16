"""Processes video and outputs header files."""

from typing import List, Optional
import argparse
import cv2
import pathlib
import re

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
    video_fps = capture.get(cv2.CAP_PROP_FPS)
    video_frame_count = int(capture.get(cv2.CAP_PROP_FRAME_COUNT))
    print(
        f"Grabbing {arguments.frame_count} frames from {video_frame_count} frame video\n"
    )
    video_seconds = int(video_frame_count / video_fps)
    print(f"{video_seconds // 60}:{video_seconds % 60:02d}\n")
    success, image = capture.read()
    height = len(image)
    width = len(image[0])

    def sample_to_int(sample: List[int]) -> int:
        """Converts a sample to int."""
        value = (sample[2] << 16) + (sample[1] << 8) + sample[0]
        return value

    output_name = pathlib.Path(re.sub("\.[^.]+$", ".anim", arguments.video_file))
    if output_name.exists():
        print(f"{output_name} already exists")
        return

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

    print(
        f"video_ratio:{round(video_ratio, 3)} target_ratio:{round(target_ratio, 3)}\n"
    )
    print(f"target_width:{target_width} target_height:{target_height}\n")
    print(f"width:{width} height:{height}\n")
    print(f"w_indexes:{w_indexes} h_indexes:{h_indexes}\n")

    count = 0
    with open(output_name, "wb") as file:
        # First, write info
        file.write(width.to_bytes(1, "little"))
        file.write(height.to_bytes(1, "little"))
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
                    file.write(sample_to_int(image[h_index][w_index].to_bytes(4, "little")))

            count += 1

    print(f"Wrote {count} frames\n")


def make_parser() -> argparse.ArgumentParser:
    """Returns an argument parser."""
    parser = argparse.ArgumentParser(
        description="Video parser. Outputs animation files for display on the vest."
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
