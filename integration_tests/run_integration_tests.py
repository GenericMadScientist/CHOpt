import subprocess

program = "./build/chopt"
song_dir = "integration_tests/songs"

songs = [
    "cups-and-cakes.mid",
    "me-and-u.chart",
    "raining-blood-sh.mid",
    "satch-boogie-live.chart",
    "soulless.chart",
    "vampire-in-ghost-town.chart",
]
paths = [
    "3",
    "3-2(+1)-2-2",
    "4(+1)-2-2-1",
    "2-2-2-2-1(+1)-2-4-1",
    "2-3-3-2-4-1(+1)",
    "2(+1)-2(+1)-1(+1)-3(+1)-1-2(+1)-2-3(+1)",
]
scores = [
    (50801, 71009),
    (350735, 472263),
    (314898, 370794),
    (676357, 892633),
    (811014, 933218),
    (426017, 573617),
]
activations = [
    [(36.625, 43.625)],
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
        (328.5, 337),
    ],
    [
        (20.6628, 32.9375),
        (55.6667, 62.25),
        (78.875, 86.25),
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
