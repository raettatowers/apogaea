from dataclasses import dataclass


@dataclass
class Item:
    description: str
    title: str
    url: str
    price: float
    quantity: int

def main() -> None:
    """Main."""

    items = [
        Item(
            description="4 way connector x 4",
            title='Golden Valley Tools & Tarps 2pc ~ 1" Flat 4 Way Edge Canopy Fitting/Batting CAGE Fitting (F4B)',
            url='https://www.amazon.com/gp/product/B071HHDW2Z/ref=as_li_tl?ie=UTF8&tag=theplayalab0b-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B071HHDW2Z&linkId=8a7bd7017f6df19635e47d21ccdc79c2',
            price=33.25,
            quantity=1,
        ),
        Item(
            description="foot stool x 4",
            title='Golden Valley Tools & Tarps 4pc - 1" Short Foot PAD Canopy Fitting/Batting CAGE Ground Plate (FPB) - Fits 1" EMT Pipe That has an Outer Diameter of 1 3/16"',
            url="https://www.amazon.com/gp/product/B06XDLSVSX/ref=as_li_tl?ie=UTF8&tag=theplayalab0b-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B06XDLSVSX&linkId=6441181ee14a420d3167b77bcf8ded59",
            price=39.95,
            quantity=1,
        ),
        Item(
            description="3 way connector x 4",
            title='Golden Valley Tools & Tarps 4pc ~ 3 Way (F3B) 1" Flat Corner Canopy Fittings/Batting CAGE Fittings - Fits 1" EMT Pipe That has an Outer Diameter of 1 3/16"',
            url="https://www.amazon.com/gp/product/B06WD8CGRL/ref=as_li_tl?ie=UTF8&tag=theplayalab0b-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B06WD8CGRL&linkId=cace8501e74118a9a164dbfd0b15bf6e",
            price=43.95,
            quantity=1,
        ),
        Item(
            description="tan tarp 10x20",
            title="10 x 20' TAN/Beige Heavy Duty 12 mil Poly TARP w/Grommets (Finished Size Approx. 9'6\" x 19'6\")",
            url='https://www.amazon.com/gp/product/B07115HPM2/ref=as_li_tl?ie=UTF8&tag=theplayalab0b-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B07115HPM2&linkId=92761895af65b9531f9f37c7f85be8a4',
            price=39.95,
            quantity=1,
        ),
        Item(
            description="black mesh tarp 10x20",
            title="MP - Mighty Products Premium 70% Sun Shade",
            url='https://www.amazon.com/gp/product/B00XNYY8OO/ref=as_li_tl?ie=UTF8&tag=theplayalab0b-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B00XNYXUTI&linkId=457ee0acc89cb0565c22566fde437a03&th=1&psc=1',
            price=93.98,
            quantity=1,
        ),
        Item(
            description='lag screws 10" x 25',
            title='(25) Hex Head 3/8 x 10" Lag Bolts Zinc Plate Wood Screws',
            url="https://www.amazon.com/dp/B06XQR4DPQ?psc=1&ref=ppx_yo2ov_dt_b_product_details",
            price=39.99,
            quantity=1,
        ),
        Item(
            description="pipes",
            title="1 in. x 10 ft. Electric Metallic Tube (EMT) Conduit",
            url="https://www.homedepot.com/p/1-in-x-10-ft-Electric-Metallic-Tube-EMT-Conduit-101568/100400409",
            price=19.56,
            quantity=13,
        ),
        Item(
            description="ratchet straps x 4",
            title="Stanley 1 in. x 12 ft. / 1500 lbs. Break Strength Ratchet Straps (4 Pack)",
            url="https://www.homedepot.com/p/Stanley-1-in-x-12-ft-1500-lbs-Break-Strength-Ratchet-Straps-4-Pack-S10004-12/313768373",
            price=19.98,
            quantity=1,
        ),
    ]

    for item in items:
        print(f"{item.description}: {item.price * item.quantity:0.2f}")

    print(f"Total: {sum((item.price * item.quantity for item in items)):0.2f}")


main()
