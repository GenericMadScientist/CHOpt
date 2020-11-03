# How fast is CHOpt

CHOpt is now fast enough that I can throw multiple setlists at it, so for fun I
decided to do so and see what results come up. Don't take this too seriously:
for one I only ran this on all these songs once. I did this with CHOpt 1.1.2.

## The songs

I ran CHOpt on every song from the following setlists and packs with Expert
Lead Guitar, except Classical Thump, Imagine, and Space Cowboy for which I did
Expert Bass.

* Angevil Hero II
* Anti Hero
* Anti Hero: Beach Episode
* Anti Hero: Beach Episode - The Beach Sides
* Anti Hero 2
* Band Hero
* Blanket Statement
* Brand New Hero
* Carpal Tunnel Hero: Remastered
* Carpal Tunnel Hero 2
* CHARTS
* CHARTS 2
* CHARTS 2 DLC (Pack 1)
* Circuit Breaker
* Cow Hero
* CSC Monthly Packs (June 2018 - October 2020)
* DF Discography CH
* Digitizer
* Dissonance Hero
* DJ Hero
* Djent Hero Collection (Pack #1 - Pack #5)
* DJMax Packs (Pack I - Pack II)
* Facelift (Pack 1)
* Focal Point
* Focal Point 2
* GaMetal Power Pack
* Green Day Rock Band
* Guitar Hero
* Guitar Hero II
* Guitar Hero II DLC
* Guitar Hero Encore: Rocks the 80s
* Guitar Hero III: Legends of Rock
* Guitar Hero III: Legends of Rock DLC
* Guitar Hero: Aerosmith
* Guitar Hero World Tour
* Guitar Hero World Tour DLC
* Guitar Hero: Metallica
* Guitar Hero: Metallica DLC
* Guitar Hero Smash Hits
* Guitar Hero: Van Halen
* Guitar Hero 5
* Guitar Hero 5 DLC
* Guitar Hero: Warriors of Rock
* Guitar Hero: Warriors of Rock DLC
* Guitar Hero On Tour
* Guitar Hero On Tour: Decades
* Guitar Hero On Tour: Modern Hits
* Guitar Hero: Guitar Zero
* Guitar Hero: Guitar Zero DLC (DLC #1 to DLC #8)
* Guitar Hero X
* Koreaboo Hero
* Koreaboo Hero 2
* Lego Rock Band
* Marathon Hero
* Paradigm
* Phase Shift Guitar Project 4
* Redemption Arc
* Rock Band
* Rock Band 2
* Rock Band 3
* Rock Band ACDC Live Track Pack
* Rock Band Blitz
* The Beatles Rock Band
* The Fall of Troy Hero
* Trunks252JM's Classic Charts
* Vortex Hero
* Zero Gravity

If there's a setlist or pack you'd like to see on here, let me know.

## Results

### Overall

The combined runtime was 54m41s. Below is a histogram of the runtime for each
song, with the top 1% combined as outliers.

![Runtime histogram](runtime-histogram.svg)

The 25 slowest songs are as follows:

| Song                                             | Setlist                      | Time (s) |
| ------------------------------------------------ | ---------------------------- | -------- |
| CHARTS 2: Endless Setlist                        | CHARTS 2                     | 48.501   |
| CHARTS: The (almost) Endless Setlist             | CHARTS                       | 32.488   |
| Endless Setlist: The Fall of Troy Hero           | The Fall of Troy Hero        | 18.157   |
| Chezy's Ultimate Solo Experience                 | CHARTS 2                     | 12.332   |
| Uranoid                                          | CHARTS                       | 10.968   |
| The Human Equation                               | Marathon Hero                | 8.827    |
| Catch Thirtythree                                | Djent Hero Collection        | 8.106    |
| Periphery IV: Hail Stan (Full Album)             | CHARTS 2                     | 6.626    |
| Eskapist                                         | Marathon Hero                | 6.244    |
| Eskapist                                         | CHARTS                       | 6.222    |
| Automata                                         | Marathon Hero                | 6.096    |
| Extreme Power Metal (Full Album)                 | Guitar Hero: Guitar Zero DLC | 5.910    |
| Live at Dynamo Open Air 1998 [FULL ALBUM]        | CSC Monthly Packs            | 5.665    |
| Coma Witch (Full Album Chart)                    | CHARTS                       | 5.627    |
| Volition (Full Album Chart)                      | CHARTS                       | 5.235    |
| Nothing More [FULL ALBUM]                        | CSC Monthly Packs            | 4.924    |
| The Future in Whose Eyes? (Full Album)           | CHARTS 2                     | 4.775    |
| That's The Spirit (Full Album Chart)             | CHARTS                       | 4.431    |
| Jane Doe (Full Album)                            | Dissonance Hero              | 4.190    |
| Bloodwork (Full Album Chart)                     | CHARTS                       | 4.110    |
| Time Will Die And Love Will Bury It (Full Album) | Dissonance Hero              | 4.078    |
| Low Teens (Full Album Chart)                     | CHARTS                       | 4.014    |
| Passion of the Heist II                          | CHARTS 2 DLC                 | 3.974    |
| Culture Scars (Full Album Chart)                 | CHARTS                       | 3.942    |
| In the Unlikely Event                            | The Fall of Troy Hero        | 3.734    |

### By Setlist

| Setlist                                    | Number of songs | Total time (s) | Average time (s) |
| ------------------------------------------ | --------------- | -------------- | ---------------- |
| Angevil Hero II                            | 67              | 26.985         | 0.403            |
| Anti Hero                                  | 402             | 166.838        | 0.415            |
| Anti Hero: Beach Episode                   | 127             | 60.976         | 0.480            |
| Anti Hero: Beach Episode - The Beach Sides | 25              | 9.754          | 0.390            |
| Anti Hero 2                                | 365             | 157.578        | 0.432            |
| Band Hero                                  | 65              | 21.132         | 0.325            |
| Blanket Statement                          | 115             | 42.856         | 0.373            |
| Brand New Hero                             | 83              | 28.452         | 0.343            |
| Carpal Tunnel Hero: Remastered             | 104             | 67.450         | 0.649            |
| Carpal Tunnel Hero 2                       | 309             | 167.570        | 0.542            |
| CHARTS                                     | 646             | 324.083        | 0.502            |
| CHARTS 2                                   | 139             | 124.441        | 0.895            | 
| CHARTS 2 DLC                               | 43              | 28.948         | 0.673            |        
| Circuit Breaker                            | 116             | 45.772         | 0.395            |
| Cow Hero                                   | 71              | 27.243         | 0.384            |
| CSC Monthly Packs                          | 794             | 311.464        | 0.392            |
| DF Discography CH                          | 80              | 46.519         | 0.581            |
| Digitizer                                  | 82              | 30.431         | 0.371            |
| Dissonance Hero                            | 106             | 52.219         | 0.493            |
| DJ Hero                                    | 10              | 2.406          | 0.241            |
| Djent Hero Collection                      | 161             | 81.715         | 0.508            |
| DJMax Packs                                | 58              | 14.487         | 0.250            |
| Facelift                                   | 35              | 13.077         | 0.374            |
| Focal Point                                | 170             | 81.353         | 0.479            |
| Focal Point 2                              | 186             | 86.678         | 0.466            |
| GaMetal Power Pack                         | 54              | 26.203         | 0.485            |
| Green Day Rock Band                        | 43              | 13.972         | 0.325            |
| Guitar Hero                                | 49              | 15.075         | 0.308            |
| Guitar Hero II                             | 74              | 24.167         | 0.327            |
| Guitar Hero II DLC                         | 24              | 7.693          | 0.321            |
| Guitar Hero Encore: Rocks the 80s          | 30              | 9.779          | 0.326            |
| Guitar Hero III: Legends of Rock           | 70              | 23.419         | 0.335            |
| Guitar Hero III: Legends of Rock DLC       | 68              | 27.821         | 0.409            |
| Guitar Hero: Aerosmith                     | 41              | 13.018         | 0.318            |
| Guitar Hero World Tour                     | 84              | 29.976         | 0.357            |
| Guitar Hero World Tour DLC                 | 147             | 55.909         | 0.380            |
| Guitar Hero: Metallica                     | 49              | 25.199         | 0.514            |
| Guitar Hero: Metallica DLC                 | 10              | 6.751          | 0.675            |
| Guitar Hero Smash Hits                     | 48              | 17.202         | 0.358            |
| Guitar Hero: Van Halen                     | 47              | 18.064         | 0.384            |
| Guitar Hero 5                              | 84              | 32.443         | 0.386            |
| Guitar Hero 5 DLC                          | 158             | 54.262         | 0.343            |
| Guitar Hero: Warriors of Rock              | 93              | 33.024         | 0.355            |
| Guitar Hero: Warriors of Rock DLC          | 84              | 32.192         | 0.383            |
| Guitar Hero On Tour                        | 31              | 9.945          | 0.321            |
| Guitar Hero On Tour: Decades               | 36              | 10.703         | 0.297            |
| Guitar Hero On Tour: Modern Hits           | 44              | 13.016         | 0.296            |
| Guitar Hero: Guitar Zero                   | 64              | 21.201         | 0.331            |
| Guitar Hero: Guitar Zero DLC               | 35              | 19.291         | 0.551            |
| Guitar Hero X                              | 127             | 68.309         | 0.538            |
| Koreaboo Hero                              | 51              | 14.676         | 0.288            |
| Koreaboo Hero 2                            | 101             | 28.828         | 0.285            |
| Lego Rock Band                             | 45              | 12.257         | 0.272            |
| Marathon Hero                              | 49              | 91.887         | 1.875            |
| Paradigm                                   | 101             | 49.883         | 0.494            |
| Phase Shift Guitar Project 4               | 162             | 69.783         | 0.431            |
| Redemption Arc                             | 100             | 41.645         | 0.416            |
| Rock Band                                  | 58              | 19.799         | 0.341            |
| Rock Band 2                                | 84              | 28.773         | 0.343            |
| Rock Band 3                                | 83              | 26.704         | 0.322            |
| Rock Band ACDC Live Track Pack             | 18              | 7.979          | 0.443            |
| Rock Band Blitz                            | 25              | 8.128          | 0.325            |
| The Beatles Rock Band                      | 45              | 10.889         | 0.242            |
| The Fall of Troy Hero                      | 70              | 58.341         | 0.833            |
| Trunks252JM's Classic Charts               | 56              | 22.326         | 0.399            |
| Vortex Hero                                | 222             | 91.668         | 0.413            |
| Zero Gravity                               | 179             | 68.632         | 0.383            |

## Average Multiplier Outliers

I've started gathering more information when I measure performance, including
average multiplier. The 25 songs with the highest optimal average multiplier are
as follows:

| Song                                          | Setlist                          | Average Multiplier |
| --------------------------------------------- | -------------------------------- | ------------------ |
| Sugar Foot Rag                                | Carpal Tunnel Hero 2             | 7.276x             |
| Trojans                                       | Carpal Tunnel Hero 2             | 7.070x             |
| Downfall of Gaia                              | CSC Monthly Packs                | 6.874x             |
| Gee-Wiz                                       | Carpal Tunnel Hero 2             | 6.824x             |
| Star X Speed Story Solo Medley                | Anti Hero: Beach Episode         | 6.784x             |
| Thunder And Lightning                         | Carpal Tunnel Hero 2             | 6.752x             |
| Dithering                                     | Paradigm                         | 6.751x             |
| Black Hole Sun                                | Rock Band                        | 6.696x             |
| lifeisgood                                    | CHARTS 2                         | 6.658x             |
| All Is One                                    | Vortex Hero                      | 6.572x             |
| Solace                                        | Djent Hero Collection            | 6.554x             |
| This Ain't a Scene, It's an Arms Race         | Guitar Hero On Tour: Modern Hits | 6.467x             |
| Away / Poetic Justice                         | Vortex Hero                      | 6.463x             |
| Christmas Time is Here (Vince Guaraldi cover) | CSC Monthly Packs                | 6.455x             |
| ETERNAL NOW                                   | Paradigm                         | 6.433x             |
| Frankenstein                                  | Guitar Hero II DLC               | 6.432x             |
| Frankenstein                                  | Guitar Hero                      | 6.431x             |
| PN35                                          | CHARTS 2                         | 6.427x             |
| PN35                                          | Circuit Breaker                  | 6.427x             |
| Gamer National Anthem (feat. Coey)            | CSC Monthly Packs                | 6.418x             |
| Neonatalimpalionecrophiliation                | Carpal Tunnel Hero 2             | 6.415x             |
| Trippolette                                   | Guitar Hero                      | 6.366x             |
| Oblivion (Rockin' Night Style)                | DJMax Packs                      | 6.366x             |
| Tapping Boogie                                | Paradigm                         | 6.352x             |
| I'm So Sick                                   | Rock Band                        | 6.345x             |

There are 45 songs that cannot be 7 starred. 31 of them do not have any Star
Power; the 14 that do are as follows:

| Song                                    | Setlist                      | Average Multiplier |
| --------------------------------------- | ---------------------------- | ------------------ |
| They Don't Have To Believe...           | CHARTS                       | 3.594x             |
| Uranoid                                 | CHARTS                       | 4.121x             |
| The Betrayal                            | Carpal Tunnel Hero 2         | 4.267x             |
| Bleed                                   | CHARTS                       | 4.311x             |
| Breakout (feat. Scandroid)              | Phase Shift Guitar Project 4 | 4.332x             |
| Luca (Demo)                             | Brand New Hero               | 4.335x             |
| Coffee Mug (Originally by Descendents)  | CHARTS                       | 4.335x             |
| Shards of Scorched Flesh                | Carpal Tunnel Hero 2         | 4.349x             |
| Cascading Failures, Diminishing Returns | Carpal Tunnel Hero 2         | 4.366x             |
| Built This Pool (blink-182 Cover)       | CHARTS                       | 4.368x             |
| The Pretender                           | Paradigm                     | 4.378x             |
| Infinitesimal to the Infinite           | Zero Gravity                 | 4.382x             |
| Get Possessed                           | Anti Hero                    | 4.386x             |
| Monkey Wrench                           | Guitar Hero II               | 4.400x             |

Note the average multiplier for Monkey Wrench is rounding from 4.39995x. Clone
Hero would display the average multiplier as 4.400x, but 5 more points would be
needed for the 7 star.
