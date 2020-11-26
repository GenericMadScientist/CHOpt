# How fast is CHOpt

CHOpt is now fast enough that I can throw multiple setlists at it, so for fun I
decided to do so and see what results come up. Don't take this too seriously:
for one I only ran this on all these songs once. I did this with CHOpt 1.2.0.

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
* CSC Monthly Packs (June 2018 - November 2020)
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
* Guitar Hero: Guitar Zero DLC (DLC #1 - DLC #9)
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
* Verified Unverified
* Vortex Hero
* Zero Gravity

If there's a setlist or pack you'd like to see on here, let me know.

## Results

### Overall

The combined runtime was 56m35s. Below is a histogram of the runtime for each
song, with the top 1% combined as outliers.

![Runtime histogram](runtime-histogram.svg)

The 25 slowest songs are as follows:

| Song                                             | Setlist                      | Time (s) |
| ------------------------------------------------ | ---------------------------- | -------- |
| CHARTS 2: Endless Setlist                        | CHARTS 2                     | 46.219   |
| CHARTS: The (almost) Endless Setlist             | CHARTS                       | 33.765   |
| Endless Setlist: The Fall of Troy Hero           | The Fall of Troy Hero        | 18.210   |
| Chezy's Ultimate Solo Experience                 | CHARTS 2                     | 11.750   |
| Uranoid                                          | CHARTS                       | 11.500   |
| The Human Equation                               | Marathon Hero                | 8.454    |
| Catch Thirtythree                                | Djent Hero Collection        | 6.912    |
| Periphery IV: Hail Stan (Full Album)             | CHARTS 2                     | 6.375    |
| Eskapist                                         | Marathon Hero                | 6.042    |
| Automata                                         | Marathon Hero                | 5.967    |
| Eskapist                                         | CHARTS                       | 5.962    |
| Extreme Power Metal (Full Album)                 | Guitar Hero: Guitar Zero DLC | 5.954    |
| Live at Dynamo Open Air 1998 [FULL ALBUM]        | CSC Monthly Packs            | 5.604    |
| Coma Witch (Full Album Chart)                    | CHARTS                       | 5.198    |
| Volition (Full Album Chart)                      | CHARTS                       | 5.195    |
| Nothing More [FULL ALBUM]                        | CSC Monthly Packs            | 4.788    |
| The Future in Whose Eyes? (Full Album)           | CHARTS 2                     | 4.549    |
| That's The Spirit (Full Album Chart)             | CHARTS                       | 4.268    |
| Jane Doe (Full Album)                            | Dissonance Hero              | 4.114    |
| Time Will Die And Love Will Bury It (Full Album) | Dissonance Hero              | 3.995    |
| Bloodwork (Full Album Chart)                     | CHARTS                       | 3.986    |
| Low Teens (Full Album Chart)                     | CHARTS                       | 3.972    |
| Passion of the Heist II                          | CHARTS 2 DLC                 | 3.820    |
| Culture Scares (Full Album Chart)                | CHARTS                       | 3.820    |
| In the Unlikely Event                            | The Fall of Troy Hero        | 3.738    |

### By Setlist

| Setlist                                    | Number of songs | Total time (s) | Average time (s) |
| ------------------------------------------ | --------------- | -------------- | ---------------- |
| Angevil Hero II                            | 67              | 28.220         | 0.421            |
| Anti Hero                                  | 402             | 172.086        | 0.428            |
| Anti Hero: Beach Episode                   | 127             | 62.455         | 0.492            |
| Anti Hero: Beach Episode - The Beach Sides | 25              | 10.324         | 0.413            |
| Anti Hero 2                                | 365             | 163.489        | 0.448            |
| Band Hero                                  | 65              | 22.704         | 0.349            |
| Blanket Statement                          | 115             | 45.054         | 0.392            |
| Brand New Hero                             | 83              | 29.247         | 0.352            |
| Carpal Tunnel Hero: Remastered             | 104             | 63.591         | 0.611            |
| Carpal Tunnel Hero 2                       | 309             | 169.476        | 0.548            |
| CHARTS                                     | 646             | 334.850        | 0.518            |
| CHARTS 2                                   | 139             | 122.900        | 0.884            | 
| CHARTS 2 DLC                               | 43              | 20.925         | 0.487            |        
| Circuit Breaker                            | 116             | 48.968         | 0.422            |
| Cow Hero                                   | 71              | 28.209         | 0.397            |
| CSC Monthly Packs                          | 862             | 334.674        | 0.388            |
| DF Discography CH                          | 80              | 48.085         | 0.601            |
| Digitizer                                  | 82              | 32.809         | 0.400            |
| Dissonance Hero                            | 106             | 53.608         | 0.506            |
| DJ Hero                                    | 10              | 2.360          | 0.236            |
| Djent Hero Collection                      | 161             | 82.409         | 0.512            |
| DJMax Packs                                | 58              | 12.858         | 0.222            |
| Facelift                                   | 35              | 13.828         | 0.395            |
| Focal Point                                | 170             | 84.389         | 0.496            |
| Focal Point 2                              | 186             | 90.049         | 0.484            |
| GaMetal Power Pack                         | 54              | 26.936         | 0.499            |
| Green Day Rock Band                        | 43              | 13.950         | 0.324            |
| Guitar Hero                                | 49              | 15.514         | 0.317            |
| Guitar Hero II                             | 74              | 25.254         | 0.341            |
| Guitar Hero II DLC                         | 24              | 8.098          | 0.337            |
| Guitar Hero Encore: Rocks the 80s          | 30              | 10.438         | 0.348            |
| Guitar Hero III: Legends of Rock           | 70              | 25.296         | 0.361            |
| Guitar Hero III: Legends of Rock DLC       | 68              | 29.517         | 0.434            |
| Guitar Hero: Aerosmith                     | 41              | 13.963         | 0.341            |
| Guitar Hero World Tour                     | 84              | 31.814         | 0.379            |
| Guitar Hero World Tour DLC                 | 147             | 58.805         | 0.400            |
| Guitar Hero: Metallica                     | 49              | 26.070         | 0.532            |
| Guitar Hero: Metallica DLC                 | 10              | 6.832          | 0.683            |
| Guitar Hero Smash Hits                     | 48              | 18.456         | 0.385            |
| Guitar Hero: Van Halen                     | 47              | 19.096         | 0.406            |
| Guitar Hero 5                              | 84              | 34.541         | 0.411            |
| Guitar Hero 5 DLC                          | 158             | 56.151         | 0.355            |
| Guitar Hero: Warriors of Rock              | 93              | 34.453         | 0.370            |
| Guitar Hero: Warriors of Rock DLC          | 84              | 34.481         | 0.410            |
| Guitar Hero On Tour                        | 31              | 10.268         | 0.331            |
| Guitar Hero On Tour: Decades               | 36              | 11.186         | 0.311            |
| Guitar Hero On Tour: Modern Hits           | 44              | 13.622         | 0.310            |
| Guitar Hero: Guitar Zero                   | 64              | 23.032         | 0.360            |
| Guitar Hero: Guitar Zero DLC               | 38              | 20.957         | 0.551            |
| Guitar Hero X                              | 127             | 70.060         | 0.552            |
| Koreaboo Hero                              | 51              | 15.569         | 0.305            |
| Koreaboo Hero 2                            | 101             | 30.322         | 0.300            |
| Lego Rock Band                             | 45              | 12.040         | 0.268            |
| Marathon Hero                              | 49              | 89.696         | 1.831            |
| Paradigm                                   | 101             | 51.915         | 0.514            |
| Phase Shift Guitar Project 4               | 162             | 71.817         | 0.443            |
| Redemption Arc                             | 100             | 43.632         | 0.436            |
| Rock Band                                  | 58              | 19.835         | 0.342            |
| Rock Band 2                                | 84              | 30.007         | 0.357            |
| Rock Band 3                                | 83              | 26.943         | 0.325            |
| Rock Band ACDC Live Track Pack             | 18              | 7.775          | 0.432            |
| Rock Band Blitz                            | 25              | 8.095          | 0.324            |
| The Beatles Rock Band                      | 45              | 10.680         | 0.237            |
| The Fall of Troy Hero                      | 70              | 59.884         | 0.855            |
| Trunks252JM's Classic Charts               | 56              | 23.618         | 0.422            |
| Verified Unverified                        | 21              | 9.001          | 0.429            |
| Vortex Hero                                | 222             | 96.101         | 0.433            |
| Zero Gravity                               | 179             | 72.123         | 0.403            |

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

There are 50 songs that cannot be 7 starred. 34 of them do not have any Star
Power; the 16 that do are as follows:

| Song                                    | Setlist                      | Average Multiplier |
| --------------------------------------- | ---------------------------- | ------------------ |
| All                                     | CSC Monthly Packs            | 1.016x             |
| No, All!                                | CSC Monthly Packs            | 1.095x             |
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
