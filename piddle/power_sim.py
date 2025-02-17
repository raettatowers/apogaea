"""Testing some parameters to see how much the solar panel and batteries can last"""
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
from scipy.stats import norm
from dataclasses import dataclass
import sys


def get_sunlight_percentage(hour: int, minute: int, std_dev: float) -> float:
    """Returns the percent of solar energy the solar panels produce at a time of day."""
    # I made up this std_dev, see
    # https://joshuatauberer.medium.com/solar-numbers-7a7f28d51897
    # for a real curve. It's not perfectly normal, and I fudged the
    # std_dev so that it matches the number of solar hours in southern
    # Colorado in the summer

    mean = 12
    value_at_mean = 1.0

    pdf_at_x = norm.pdf(hour + minute / 60, mean, std_dev)
    pdf_at_mean = norm.pdf(mean, mean, std_dev)

    scaled_value = (pdf_at_x / pdf_at_mean) * value_at_mean
    # There's a minimum amount before the solar panel starts the photovoltaic process
    if scaled_value < 0.05:
        return 0.0
    return scaled_value


DEFAULT_STD_DEV = 2.3
assert get_sunlight_percentage(5, 0, DEFAULT_STD_DEV) < .01
assert get_sunlight_percentage(7, 0, DEFAULT_STD_DEV) < .2
assert get_sunlight_percentage(12, 0, DEFAULT_STD_DEV) == 1
assert get_sunlight_percentage(11, 0, DEFAULT_STD_DEV) == get_sunlight_percentage(13, 0, DEFAULT_STD_DEV)

@dataclass
class Options:
    solar_w: float
    max_battery_wh: float
    min_battery_wh: float
    project_w: float
    std_dev: float


def run_simulation(options: Options) -> None:
    """Runs a simulation."""

    battery_wh = options.max_battery_wh

    days = ("Wed", "Thu", "Fri", "Sat", "Sun")
    day = 0
    hour = 12

    previous_increasing = False
    previous_off = True
    previous_maxed = True
    minute = 0

    def format_message():
        message = f"{days[day]} {hour:02d}:{minute:02d}"
        message += f" {battery_wh:.2f} Wh {battery_wh/options.max_battery_wh*100:.0f}%"
        if maxed:
            message += ' maxed'
        if off:
            message += ' off'
        if not off:
            message += (' in' if increasing else ' de') + "creasing"
        return message

    while day < len(days) - 1 or hour < 18:
        minute += 1
        if minute == 60:
            hour += 1
            minute = 0
        if hour == 24:
            day += 1
            hour = 0

        need_print = False

        previous_battery_wh = battery_wh

        battery_wh -= options.project_w / 60
        battery_wh += get_sunlight_percentage(hour, minute, options.std_dev) * options.solar_w / 60
        off = False
        maxed = False
        if battery_wh < options.min_battery_wh:
            battery_wh = options.min_battery_wh
            off = True
        elif battery_wh > options.max_battery_wh:
            battery_wh = options.max_battery_wh
            maxed = True

        increasing = battery_wh > previous_battery_wh
        if increasing != previous_increasing and not off and not maxed:
            need_print = True
        if off != previous_off:
            need_print = True
        if maxed != previous_maxed:
            need_print = True

        if need_print:
            print(format_message())

        previous_increasing = increasing
        previous_off = off
        previous_maxed = maxed
        previous_hour = hour

    print(format_message())


DEFAULT_W = 2.478 * 2 * 12
IDLE_W = 1.614 * 2 * 12
def make_parser() -> ArgumentParser:
    """Makes a parser."""
    parser = ArgumentParser(prog="power_sim", formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        "--battery-wh",
        "-b",
        type=float,
        help="The max battery capacity in Wh. My battery is 80 Ah @ 12 V = 960 Wh, but it's a few years old, so probably a little less.",
        # 80% because it's a few years old
        default=12 * 80 * 0.8,
    )
    parser.add_argument(
        "--solar-w",
        "-s",
        type=float,
        help="The solar power in W. I have 200 W, but because of Colorado's latitude, they'll likely only produce ~90? percent of their rated power.",
        # 90% because we're not at the equator
        default=200 * .9,
    )
    parser.add_argument(
        "--min-battery",
        type=float,
        help="How low the battery should go before it shuts off, in percent.",
        default=25,
    )
    parser.add_argument(
        "--brightness",
        "-p",
        type=float,
        default=100,
        help="Brightness in percent to run the LEDs at, in percent. Either this or -w may be specified, but not both. You can run more than 100, because my 'max' estimate is based on responding to one song, and other songs may light up more LEDs.",
    )
    parser.add_argument(
        "--project-w",
        "-w",
        type=float,
        help=f"The number of Watts the project uses. From testing, it uses {DEFAULT_W:0.0f} W at full brightness while responding to music and {IDLE_W:0.0f} W at idle.",
        default=None,
    )
    parser.add_argument(
        "--std-dev",
        type=float,
        help="Standard deviation for solar radiation curve.",
        default=DEFAULT_STD_DEV,
    )
    return parser


if __name__ == "__main__":
    parser = make_parser()
    namespace = parser.parse_args()
    if namespace.min_battery < 1 or namespace.min_battery > 100:
        sys.stderr.write(f"Bad battery percentage: {namespace.min_battery}, should be 1 < % < 100\n")
        sys.stderr.flush()
        parser.print_help()
        sys.exit()
    if namespace.project_w and namespace.project_w < IDLE_W:
        sys.stderr.write(f"Warning: project W {namespace.project_w:0.2f} is unrealistically below idle W {IDLE_W:0.2f}\n")
        sys.stderr.flush()
        parser.print_help()
        sys.exit()
    if namespace.project_w is not None and namespace.brightness != 100:
        sys.stderr.write("Can only specify one of project-w and brightness")
        sys.stderr.flush()
        parser.print_help()
        sys.exit()
    # Over 100 is okay, because my "max" is based on 1 measurement from
    # responding to a song, and other songs might make more LEDs light up
    if namespace.brightness < 2:  # Check < 2 in case someone enters .5 instead of 50
        sys.stderr.write(f"Brightness too low: {namespace.brightness}")
        sys.stderr.flush()
        parser.print_help()
        sys.exit()

    if namespace.project_w is not None:
        project_w = namespace.project_w
    else:
        project_w = (DEFAULT_W - IDLE_W) * namespace.brightness / 100 + IDLE_W

    options = Options(
        solar_w=namespace.solar_w,
        max_battery_wh=namespace.battery_wh,
        min_battery_wh=namespace.battery_wh * namespace.min_battery / 100,
        project_w=project_w,
        std_dev=namespace.std_dev,
    )

    # https://www.turbinegenerator.org/solar/colorado/ claims that southern
    # Colorado's peak summer sun hours per day is 5.72
    max_solar_hours = 5.72
    sun_hours = sum((
        get_sunlight_percentage(i, 0, options.std_dev) for i in range(24)
    ))
    print(f"simulated sun hours: {sun_hours:.2f} (max in CO is {max_solar_hours})")
    if sun_hours > max_solar_hours:
        sys.stderr.write("*** Warning! Your std-dev is too high and gives unrealistically high solar hours ***\n")
        sys.stderr.flush()
    print(f"Running simulation with {options}")
    brightness_p = (options.project_w - IDLE_W) / (DEFAULT_W - IDLE_W)
    print(f"Simulated power: {brightness_p * 100:0.0f}%")
    run_simulation(options)
