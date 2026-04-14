from pathlib import Path

from PIL import Image

CIRCLE_TEMPLATE = [
    [0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0],
    [0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0],
    [0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0],
    [1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1],
    [1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1],
    [1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1],
    [1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1],
    [1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1],
    [0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0],
    [0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0],
    [0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0],
]

OPEN_TEMPLATE = (
    [[0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0]]
    + [[0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0]] * 65
    + [[0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0]]
)

STAR_TEMPLATE = [
    [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0],
    [0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0],
    [1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1],
    [0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0],
    [0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0],
    [0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0],
    [0, 0, 0, 1, 2, 1, 2, 1, 0, 0, 0],
    [0, 0, 1, 2, 1, 0, 1, 2, 1, 0, 0],
    [0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0],
    [0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0],
]

TRIANGLE_TEMPLATE = [
    [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0],
    [0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0],
    [0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0],
    [0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0],
    [0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0],
    [0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0],
    [0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0],
    [0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0],
    [1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1],
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
]

SQUARE_TEMPLATE = [[1] * 11] * 6 + [[1] + [2] * 9 + [1]] * 4 + [[1] * 11]


def circle_image(index, orientation):
    img = Image.new("RGBA", (11, 71), "#00000000")

    colour_map = [
        [(0, 0, 0), (0, 255, 0)],
        [(0, 0, 0), (255, 0, 0)],
        [(0, 0, 0), (255, 255, 0)],
        [(0, 0, 0), (0, 0, 255)],
        [(0, 0, 0), (255, 165, 0)],
        [(0, 0, 0), (128, 0, 128, 128)],
    ]
    for colour in reversed(range(6)):
        template = OPEN_TEMPLATE if colour == 5 else CIRCLE_TEMPLATE
        if index & (1 << colour):
            for i, row in enumerate(template):
                if colour == 5:
                    y_offset = 2
                elif orientation == "lefty":
                    y_offset = 15 * (4 - colour)
                else:
                    y_offset = 15 * colour

                for j, pixel in enumerate(row):
                    if pixel != 0:
                        img.putpixel((j, y_offset + i), colour_map[colour][pixel - 1])

    return img


def star_image(index, orientation):
    img = Image.new("RGBA", (11, 73), "#00000000")

    colour_map = [
        [(0, 0, 0), (0, 255, 0)],
        [(0, 0, 0), (255, 0, 0)],
        [(0, 0, 0), (255, 255, 0)],
        [(0, 0, 0), (0, 0, 255)],
        [(0, 0, 0), (255, 165, 0)],
        [(0, 0, 0), (128, 0, 128, 128)],
    ]
    for colour in reversed(range(6)):
        template = OPEN_TEMPLATE if colour == 5 else STAR_TEMPLATE
        if index & (1 << colour):
            for i, row in enumerate(template):
                if colour == 5:
                    y_offset = 3
                elif orientation == "lefty":
                    y_offset = 15 * (4 - colour)
                else:
                    y_offset = 15 * colour

                for j, pixel in enumerate(row):
                    if pixel != 0:
                        img.putpixel((j, y_offset + i), colour_map[colour][pixel - 1])

    return img


def tom_image(index, orientation):
    img = Image.new("RGBA", (11, 71), "#00000000")

    colour_map = [
        [(0, 0, 0), (255, 0, 0)],
        [(0, 0, 0), (255, 255, 0)],
        [(0, 0, 0), (0, 0, 255)],
        [(0, 0, 0), (0, 255, 0)],
        [(0, 0, 0), (255, 165, 0, 128)],
        [(0, 0, 0), (255, 165, 0, 128)],
    ]
    template = OPEN_TEMPLATE if index >= 4 else CIRCLE_TEMPLATE
    for i, row in enumerate(template):
        if index >= 4:
            y_offset = 2
        elif orientation == "lefty":
            y_offset = 20 * (3 - index)
        else:
            y_offset = 20 * index

        for j, pixel in enumerate(row):
            if pixel != 0:
                img.putpixel((j, y_offset + i), colour_map[index][pixel - 1])

    return img


def cymbal_image(index, orientation):
    img = Image.new("RGBA", (11, 71), "#00000000")

    colour_map = [
        [(0, 0, 0), (255, 0, 0)],
        [(0, 0, 0), (255, 255, 0)],
        [(0, 0, 0), (0, 0, 255)],
        [(0, 0, 0), (0, 255, 0)],
        [(0, 0, 0), (255, 165, 0, 128)],
        [(0, 0, 0), (255, 165, 0, 128)],
    ]
    template = OPEN_TEMPLATE if index >= 4 else TRIANGLE_TEMPLATE
    for i, row in enumerate(template):
        if index >= 4:
            y_offset = 2
        elif orientation == "lefty":
            y_offset = 20 * (3 - index)
        else:
            y_offset = 20 * index

        for j, pixel in enumerate(row):
            if pixel != 0:
                img.putpixel((j, y_offset + i), colour_map[index][pixel - 1])

    return img


def ghl_image(index, orientation):
    img = Image.new("RGBA", (11, 71), "#00000000")

    white_colours = [(0, 0, 0), (255, 255, 255)]
    black_colours = [(0, 0, 0), (0, 0, 0)]
    open_colours = [(0, 0, 0), (255, 255, 255, 128)]
    if index & (1 << 6):
        for i, row in enumerate(OPEN_TEMPLATE):
            for j, pixel in enumerate(row):
                if pixel != 0:
                    img.putpixel((j, 2 + i), open_colours[pixel - 1])

    for lane in range(3):
        mask = 0b1001
        lane_colours = (index >> lane) & mask
        if lane_colours == 0:
            continue
        template = SQUARE_TEMPLATE if lane_colours == mask else CIRCLE_TEMPLATE
        colours = black_colours if lane_colours == 0b1000 else white_colours

        for i, row in enumerate(template):
            if orientation == "lefty":
                y_offset = 30 * (2 - lane)
            else:
                y_offset = 30 * lane

            for j, pixel in enumerate(row):
                if pixel != 0:
                    img.putpixel((j, y_offset + i), colours[pixel - 1])

    return img


sprites_dir = Path("resources/sprites")
sprites_dir.mkdir(exist_ok=True)

for orientation in ["lefty", "righty"]:
    circle_dir = sprites_dir.joinpath(orientation, "circles")
    circle_dir.mkdir(parents=True, exist_ok=True)
    star_dir = sprites_dir.joinpath(orientation, "stars")
    star_dir.mkdir(parents=True, exist_ok=True)
    for i in range(1, 33):
        img = circle_image(i, orientation)
        img.save(circle_dir.joinpath(f"{i}.png"), optimize=True)
        img = star_image(i, orientation)
        img.save(star_dir.joinpath(f"{i}.png"), optimize=True)

    tom_dir = sprites_dir.joinpath(orientation, "drums")
    tom_dir.mkdir(parents=True, exist_ok=True)
    cymbal_dir = sprites_dir.joinpath(orientation, "cymbals")
    cymbal_dir.mkdir(parents=True, exist_ok=True)
    for i in range(6):
        img = tom_image(i, orientation)
        img.save(tom_dir.joinpath(f"{1 << i}.png"), optimize=True)
        img = cymbal_image(i, orientation)
        img.save(cymbal_dir.joinpath(f"{1 << i}.png"), optimize=True)

    ghl_dir = sprites_dir.joinpath(orientation, "ghl")
    ghl_dir.mkdir(parents=True, exist_ok=True)
    for i in range(1, 65):
        img = ghl_image(i, orientation)
        img.save(ghl_dir.joinpath(f"{i}.png"), optimize=True)


resources = [
    str(path).removeprefix("resources/") for path in sprites_dir.rglob("*.png")
]
resources.sort()
resources.insert(0, "icon.png")

with open("resources/resources.qrc", "w", encoding="utf-8", newline="\n") as f:
    f.write("<!DOCTYPE RCC>\n")
    f.write('<RCC version="1.0">\n')
    f.write("  <qresource>\n")
    for r in resources:
        f.write(f"    <file>{r}</file>\n")
    f.write("  </qresource>\n")
    f.write("</RCC>\n")
