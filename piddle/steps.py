import sys

if "-h" in sys.argv or len(sys.argv) not in (3, 4):
    print(f"Usage: {sys.argv[0]} <bucket_count> <sample_count> [frequency]")
    sys.exit()

bucket_count = int(sys.argv[1])
sample_count = int(sys.argv[2])
frequency = if len(sys.argv) == 4 : int(sys.argv[3]) else -1

lower = 1
upper = 10

def parts(x):
    return reversed([int(x ** i) for i in range(bucket_count)])

for _ in range(100):
    mid = (lower + upper) / 2
    total = sum(parts(mid))
    if total == sample_count:
        break
    if total < sample_count:
        lower = mid
    else:
        upper = mid

print(mid)
formatted = ', '.join(str(i) for i in parts(mid))
print(f"{{{formatted}}}")

notes = (
    (261, "C4"),
    (523, "C5"),
    (1046, "C6"),
    (2093, "C7"),
    (4186, "C8"),
    (293, "D4"),
    (587, "D5"),
    (1174, "D6"),
    (2349, "D7"),
    (4698, "D8"),
    (329, "E4"),
    (659, "E5"),
    (1318, "E6"),
    (2637, "E7"),
    (5274, "E8"),
    (349, "F4"),
    (698, "F5"),
    (1396, "F6"),
    (2793, "F7"),
    (5587, "F8"),
    (392, "G4"),
    (783, "G5"),
    (1567, "G6"),
    (3135, "G7"),
    (6271, "G8"),
    (440, "A4"),
    (880, "A5"),
    (1760, "A6"),
    (3520, "A7"),
    (7040, "A8"),
    (493, "B4"),
    (987, "B5"),
    (1975, "B6"),
    (3951, "B7"),
    (7902, "B8"),
)
if frequency > 0:
    # TODO: Figure out frequency bucketing
    print("C4 is middle C")
