import subprocess

program = "./build/chopt"
song_dir = "integration_tests/songs"

songs = [
    "amen-brother.chart",
    "cult-of-personality-gh3.mid",
    "cups-and-cakes.mid",
    "eon-break.chart",
    "lysios.chart",
    "me-and-u.chart",
    "raining-blood-sh.mid",
    "satch-boogie-live.chart",
    "soulless.chart",
    "type-03.chart",
    "vampire-in-ghost-town.chart",
]
paths = [
    "2-1-2",
    "2-2(+1)-3(+2)-3",
    "3",
    "2-2-2(+1)-2-2",
    "2-2(+1)-3(+1)-2(+1)-2-2-3-3(+1)-2-2-3-1-2",
    "3-2(+1)-2-2",
    "4(+1)-2-2-1",
    "2-2-2-2-1(+1)-2-4-1",
    "2-3-3-2-4-2",
    "3-2-2(+1)-3(+1)",
    "2(+1)-2(+1)-1(+1)-3(+1)-1-2(+1)-2-3(+1)",
]
scores = [
    (68746, 91294),
    (273394, 383030),
    (50801, 71009),
    (222442, 266442),
    (1131831, 1390531),
    (350735, 472263),
    (314898, 370794),
    (676357, 892633),
    (811014, 933014),
    (412970, 491254),
    (426017, 573577),
]
activations = [
    [(24.7376, 31), (36.375, 40.4329), (72.375, 79)],
    [(15, 19.625), (39.0625, 47), (73.4375, 85.6562), (108.375, 114.656)],
    [(36.625, 43.625)],
    [(35, 39), (59, 63), (106.625, 112.625), (123, 130.25), (155, 159)],
    [
        (52, 60.0358),
        (88.75, 98.625),
        (138.75, 148.5),
        (174.25, 181.812),
        (220.125, 224.625),
        (255.375, 259.812),
        (308, 314.125),
        (378.917, 390),
        (426.312, 431.25),
        (460.5, 464.938),
        (511.417, 518),
        (534.05, 542.9),
        (561.95, 566.95),
    ],
    [(33.9375, 42.3275), (66.6875, 78), (95.75, 99.8125), (109.938, 114)],
    [(80.75, 90.75), (123, 127), (156.667, 160.771), (171.261, 175.942)],
    [
        (37.75, 44.9161),
        (60.7854, 66.1099),
        (74.5, 78.5591),
        (86.9161, 92.6672),
        (101.842, 110.969),
        (116.125, 120.188),
        (160.459, 168.533),
        (174.917, 180.15),
    ],
    [
        (26, 30.1255),
        (85, 91.1255),
        (149, 155.124),
        (192.874, 197),
        (288.875, 297),
        (330.25, 338.374),
    ],
    [(76, 83.4375), (106, 110.003), (159.125, 165.188), (223.571, 232.929)],
    [
        (20.6628, 32.9375),
        (55.6667, 62.25),
        (79.125, 86.4638),
        (113, 122.833),
        (130.876, 135.463),
        (156, 166.062),
        (180.125, 184.697),
        (222, 232.063),
    ],
]

outputs = []
for path, score_pair, acts in zip(paths, scores, activations):
    output_lines = ["Optimising, please wait..."]
    output_lines.append(f"Path: {path}")
    output_lines.append(f"No SP score: {score_pair[0]}")
    output_lines.append(f"Total score: {score_pair[1]}")
    for i, act in enumerate(acts):
        act_range = f"Measure {act[0]} to Measure {act[1]}"
        output_lines.append(f"Activation {i + 1}: {act_range}")
    output_lines.append("")
    outputs.append("\n".join(output_lines).encode("utf-8"))

for song, output in zip(songs, outputs):
    result = subprocess.run([program, "-f", f"{song_dir}/{song}"], capture_output=True)
    result.check_returncode()
    if result.stdout != output:
        raise RuntimeError(f"Song {song} has incorrect output")
