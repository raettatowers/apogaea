import sys


def main() -> None:
    notes = get_notes()
    print_buckets(notes)


def get_notes():
    # From https://pages.mtu.edu/~suits/notefreqs.html
    # Some examples
    # (41.20, "E1"),  # This is the lowest note with a 4-string bass guitar
    # (1174.66, "D6"),  # This is the highest a trumpet goes
    # (2093.00, "C7"),  # This is the highest a flute goes
    name_formats = ("A", "A+", "B", "C", "C+", "D", "E", "E+", "F", "F+", "G", "G+")
    assert(len(name_formats) == 12)
    start = -41

    def frequency(semitones):
        # Semitones above or below a4
        a4 = 440
        return 2 ** (semitones / 12) * a4

    # Start at E1
    assert(41 < frequency(start) < 42)
    index = start
    notes = []
    while index < 40:
        # Start at E1
        note_index = (index + 6 - start + (10 * len(name_formats))) % len(name_formats)
        note_count = (index + 6 - start + len(name_formats)) // len(name_formats)
        note = f"{name_formats[note_index]}{note_count}"
        notes.append((frequency(index), note))
        index += 1
    assert(notes[0][1] == "E1")
    return notes


def print_buckets(notes) -> None:
    if "-h" in sys.argv or len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <bucket_count> <frequency>")
        sys.exit()

    bucket_count = int(sys.argv[1])
    if bucket_count not in (2**n for n in range(20)):
        print(f"bucket_count ({bucket_count}) must be a power of 2")
        sys.exit()
    frequency = int(sys.argv[2])

    step_size = frequency / bucket_count
    lower = 0
    empty = []
    skips = []
    for index in range(bucket_count):
        # Find which notes
        upper = lower + step_size
        found = []
        for note in notes:
            if lower <= note[0] <= upper:
                found.append(note[1])

        if upper < notes[0][0]:
            print(f"Bucket {index} has no notes")

        if len(found) == 0:
            empty.append(index)
        else:
            if len(empty) > 0:
                #print(f"Buckets {' '.join((str(b) for b in empty))} were empty")
                empty = []
            skips.append(index)
            print(f"Bucket {index} {lower:0.1f}-{upper:0.1f} has {' '.join(found)}")
            if "C4" in found:
                c4 = index

        lower = upper
    print("Remaining buckets are empty because they're above note range")
    print(f"{{{', '.join((str(s) for s in skips))}}}")
    print(f"const int c4Index = {skips.index(c4)};")
    print(f"Length = {len(skips)}")


if __name__ == "__main__":
    main()
