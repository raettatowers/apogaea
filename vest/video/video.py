"""Processes video and outputs header files."""

from typing import List
import cv2
import re
import sys

# TODO: Import these instead of copy/paste
led_counts = [6, 7, 10, 11, 12, 12, 6, 6, 7, 11, 11, 11, 11, 11]
led_counts = led_counts + list(reversed(led_counts))


def process(video_file_name: str, frameskip: int) -> None:
    """Save outputs."""
    center_index = 8
    target_width = (len(led_counts) // 2 - center_index) * 2
    target_height = max((led_counts[i] for i in range(center_index, len(led_counts) // 2)))

    capture = cv2.VideoCapture(video_file_name)
    success, image = capture.read()
    height = len(image)
    width = len(image[0])

    def sample_to_int(sample: List[int]) -> int:
        """Converts a sample to int."""
        value = (sample[2] << 16) + (sample[1] << 8) + sample[0] 
        return value

    frames = []
    while success:
        success, image = capture.read()
        if not success:
            break
        frames.append([])
        for h in range(target_height):
            h_index = int(height / target_height * (h + 0.5))
            for w in range(target_width):
                w_index = int(width / target_width * (w + 0.5))
                frames[-1].append(sample_to_int(image[h_index][w_index]))

        for _ in range(frameskip):
            success, _ = capture.read()

    name = re.sub(r"\.[a-zA-Z0-9]+$", "", video_file_name)
    name = re.sub("[^a-zA-Z0-9-]", "", name)
    name = name.replace("-", "_").upper()
    while not re.match("^[a-zA-Z]", name) and len(name) > 0:
        name = name[1:]
    if len(name) == 0:
        print("Invalid name")
        sys.exit(0)
    print(f"const uint32_t {name}[{len(frames)}][{len(frames[0])}] = {{")
    for frame in frames:
        print(f"{{{', '.join((str(v) for v in frame))}}},")
    print("};")


def main() -> None:
    """Main."""
    if len(sys.argv) != 2:
        print("Usage: python <movie-file>")
        sys.exit(1)
    process(sys.argv[1], 2)


if __name__ == "__main__":
    main()
