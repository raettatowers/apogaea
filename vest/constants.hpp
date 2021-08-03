#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

const int UNUSED_LED = 255;

/*
  items = [6, 7, 10, 11, 12, 12, 0, 0, 7, 11, 11, 11, 11, 11]
  print(f"const int NUM_LEDS = count(items);")
  print(f"const uint8_t LED_STRIPS[{2*len(items)}][{max(items)}] = {{")
  count = 0
  def print_offsets(items):
    global count
    for length in items:
      print(f"{{{', '.join((str(count + i) for i in range(length)))}, {', '.join(('UNUSED_LED_LED' for _ in range(count + length, count + max(items))))}}},")
      count += length
  print_offsets(items), print_offsets(reversed(items)), print("}")
*/
const int NUM_LEDS = 150;
const int LED_STRIPS[28][12] = {
  {0, 1, 2, 3, 4, 5, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED},
  {6, 7, 8, 9, 10, 11, 12, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED},
  {13, 14, 15, 16, 17, 18, 19, 20, 21, 22, UNUSED_LED, UNUSED_LED},
  {23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, UNUSED_LED},
  {34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45},
  {46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57},
  {UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED},
  {UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED},
  {58, 59, 60, 61, 62, 63, 64, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED, UNUSED_LED},
  {65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, UNUSED_LED},
  {76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, UNUSED_LED},
  {87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, UNUSED_LED},
  {98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, UNUSED_LED},
  {109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, UNUSED_LED},
  {120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, UNUSED_LED}
};

#endif
