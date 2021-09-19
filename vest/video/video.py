"""Processes video and outputs header files."""

from typing import List, Optional
import cv2
import re
import sys

# TODO: Import these instead of copy/paste
led_counts = [6, 7, 10, 11, 12, 12, 6, 6, 7, 11, 11, 11, 11, 11]
led_counts = led_counts + list(reversed(led_counts))


def process(video_file_name: str, frame_count: Optional[int], fps: float) -> None:
    """Save outputs."""
    center_index = 8
    target_width = (len(led_counts) // 2 - center_index) * 2
    target_height = max((led_counts[i] for i in range(center_index, len(led_counts) // 2)))

    capture = cv2.VideoCapture(video_file_name)
    video_fps = capture.get(cv2.CAP_PROP_FPS)
    if fps > video_fps:
        fps = video_fps
    success, image = capture.read()
    height = len(image)
    width = len(image[0])

    def sample_to_int(sample: List[int]) -> int:
        """Converts a sample to int."""
        value = (sample[2] << 16) + (sample[1] << 8) + sample[0] 
        return value

    name = re.sub(r"\.[a-zA-Z0-9]+$", "", video_file_name)
    name = re.sub("[^a-zA-Z0-9-]", "", name)
    name = name.replace("-", "_").upper()
    while not re.match("^[a-zA-Z]", name) and len(name) > 0:
        name = name[1:]
    if len(name) == 0:
        print("Invalid name")
        sys.exit(0)
    print(f"const PROGMEM uint32_t {name}[][{target_width * target_height}] = {{")

    millis = 0
    count = 0
    while success:
        success, image = capture.read()
        if not success:
            break
        frame = []
        for h in range(target_height):
            h_index = int(height / target_height * (h + 0.5))
            for w in range(target_width):
                w_index = int(width / target_width * (w + 0.5))
                frame.append(sample_to_int(image[h_index][w_index]))

        print(f"{{{', '.join((str(v) for v in frame))}}},")
        count += 1
        millis += 1000 / fps - 1000 / video_fps
        if frame_count is not None and count >= frame_count:
            break

        while millis > 0:
            success, _ = capture.read()
            millis -= 1000 / video_fps

    print("};")

    print(f"const uint16_t {name}_MILLIS_PER_FRAME = {1000 / fps};")


def main() -> None:
    """Main."""
    if not 2 <= len(sys.argv) <= 4:
        print("Usage: python <movie-file> [frame count] [fps]")
        print("It will sample a number of frames to get the correct FPS.")
        print("For example, if you have a 60 FPS video and specify 15,")
        print("It will only sample every 4 frames.")
        sys.exit(1)

    frame_count = None if len(sys.argv) < 3 else int(sys.argv[2])
    fps = 4 if len(sys.argv) < 4 else float(sys.argv[3])

    process(sys.argv[1], frame_count, fps)


if __name__ == "__main__":
    main()
