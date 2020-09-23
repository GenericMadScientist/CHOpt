# How fast is CHOpt

CHOpt is now fast enough that I can throw multiple setlists at it, so for fun I
decided to do so and see what results come up. Don't take this too seriously:
for one I only ran this on all these songs once. I did this with CHOpt 1.0.3.

## The songs

I ran CHOpt on every song from the following setlists/packs that had Expert
Lead Guitar charted.

* Angevil Hero II
* Anti Hero
* Anti Hero: Beach Episode
* Anti Hero 2
* Blanket Statement
* Brand New Hero
* Carpal Tunnel Hero: Remastered
* Carpal Tunnel Hero 2
* CHARTS
* CHARTS 2
* Circuit Breaker
* Cow Hero
* CSC Monthly Packs (June 2018 - September 2020)
* DF Discography CH
* Digitizer
* Dissonance Hero
* Djent Hero Collection (Pack #1 - Pack #5)
* Facelift (Pack 1)
* Focal Point
* GaMetal Power Pack
* Guitar Hero: Guitar Zero
* Guitar Hero X
* Koreaboo Hero
* Koreaboo Hero 2
* Marathon Hero
* Paradigm
* Phase Shift Guitar Project 4
* Redemption Arc
* The Fall of Troy Hero
* Vortex Hero
* Zero Gravity

If there's a setlist or pack you'd like to see on here, let me know.

## Results

### Overall

The combined runtime was 52m31s. Below is a histogram of the runtime for each
song, with the top 1% combined as outliers.

![Runtime histogram](runtime-histogram.svg)

The 25 slowest songs are as follows:

| Song                                                               | Setlist                        | Time (s) |
| ------------------------------------------------------------------ | ------------------------------ | -------- |
| CHARTS 2: Endless Setlist                                          | CHARTS 2                       | 72.022   |
| Uranoid                                                            | CHARTS                         | 64.427   |
| CHARTS: The (almost) Endless Setlist                               | CHARTS                         | 36.294   |
| Catch Thirtythree                                                  | Djent Hero Collection          | 23.649   |
| Endless Setlist: The Fall of Troy Hero                             | The Fall of Troy Hero          | 18.791   |
| The Human Equation                                                 | Marathon Hero                  | 14.677   |
| Chezy's Ultimate Solo Experience                                   | CHARTS 2                       | 14.225   |
| Oh Holy Night                                                      | Brand New Hero                 | 12.831   |
| Darkbloom                                                          | Djent Hero Collection          | 11.299   |
| Eskapist                                                           | Marathon Hero                  | 11.250   |
| Eskapist                                                           | CHARTS                         | 11.151   |
| Solace                                                             | Djent Hero Collection          | 10.768   |
| The Source/Language I & II                                         | Marathon Hero                  | 10.638   |
| Warp                                                               | CSC Monthly Packs              | 9.797    |
| Coma Witch (Full Album Chart)                                      | CHARTS                         | 9.412    |
| The Future in Whose Eyes? (Full Album)                             | CHARTS 2                       | 9.284    |
| Periphery IV: Hail Stan (Full Album)                               | CHARTS 2                       | 8.724    |
| Good to Know That If I Ever Need Attention All I Have to Do Is Die | Brand New Hero                 | 8.453    |
| Automata                                                           | Marathon Hero                  | 7.711    |
| Nothing More [FULL ALBUM]                                          | CSC Monthly Packs              | 7.414    |
| Time Traveler                                                      | Circuit Breaker                | 6.847    |
| 8                                                                  | Marathon Hero                  | 6.740    |
| Volition (Full Album Chart)                                        | CHARTS                         | 6.588    |
| Art of Life (***)                                                  | Carpal Tunnel Hero 2           | 6.344    |
| The Baying of the Hounds                                           | Carpal Tunnel Hero: Remastered | 6.237    |

### By Setlist

| Setlist                        | Number of songs | Total time (s) | Average time (s) |
| ------------------------------ | --------------- | -------------- | ---------------- |
| Angevil Hero II                | 67              | 31.151         | 0.465            |
| Anti Hero                      | 402             | 194.813        | 0.485            |
| Anti Hero: Beach Episode       | 127             | 75.851         | 0.597            |
| Anti Hero 2                    | 365             | 185.672        | 0.509            |
| Blanket Statement              | 115             | 49.157         | 0.427            |
| Brand New Hero                 | 83              | 54.528         | 0.657            |
| Carpal Tunnel Hero: Remastered | 103             | 75.450         | 0.733            |
| Carpal Tunnel Hero 2           | 309             | 208.255        | 0.674            |
| CHARTS                         | 646             | 448.952        | 0.695            |
| CHARTS 2                       | 139             | 174.787        | 1.257            |         
| Circuit Breaker                | 116             | 59.822         | 0.516            |
| Cow Hero                       | 71              | 31.596         | 0.445            |
| CSC Monthly Packs              | 762             | 375.070        | 0.492            |
| DF Discography CH              | 80              | 63.750         | 0.797            |
| Digitizer                      | 82              | 42.777         | 0.522            |
| Dissonance Hero                | 106             | 65.805         | 0.621            |
| Djent Hero Collection          | 161             | 140.130        | 0.870            |
| Facelift                       | 35              | 14.068         | 0.402            |
| Focal Point                    | 170             | 91.550         | 0.539            |
| GaMetal Power Pack             | 54              | 30.594         | 0.567            |
| Guitar Hero: Guitar Zero       | 64              | 26.232         | 0.410            |
| Guitar Hero X                  | 127             | 85.436         | 0.673            |
| Koreaboo Hero                  | 51              | 16.871         | 0.331            |
| Koreaboo Hero 2                | 101             | 32.404         | 0.321            |
| Marathon Hero                  | 49              | 140.034        | 2.858            |
| Paradigm                       | 101             | 56.268         | 0.557            |
| Phase Shift Guitar Project 4   | 162             | 78.000         | 0.481            |
| Redemption Arc                 | 100             | 45.863         | 0.459            |
| The Fall of Troy Hero          | 70              | 63.346         | 0.905            |
| Vortex Hero                    | 222             | 108.285        | 0.488            |
| Zero Gravity                   | 178             | 84.564         | 0.475            |
