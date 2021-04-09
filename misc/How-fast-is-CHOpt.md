# How fast is CHOpt

CHOpt is now fast enough that I can throw multiple setlists at it, so for fun I
decided to do so and see what results come up. Don't take this too seriously:
for one I only ran this on all these songs once. I did this with CHOpt 1.3.0.

## The songs

I ran CHOpt on every song from the following setlists and packs with Expert
Lead Guitar, except Classical Thump, Dawn Patrol, Imagine, and Space Cowboy for
which I did Expert Bass.

* Angevil Hero II
* Anti Hero
* Anti Hero: Beach Episode
* Anti Hero: Beach Episode - The Beach Sides
* Anti Hero 2
* Anti Hero 2 Charity Drive
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
* CSC Monthly Packs (June 2018 - April 2021)
* DF Discography CH
* Digitizer
* Dissonance Hero
* Dissonance Hero Free-LC (Pack 1)
* DJ Hero
* Djent Hero Collection (Pack #1 - Pack #6)
* DJMax Packs (Pack I - Pack II)
* Facelift (Pack 1 - Pack 2)
* Focal Point
* Focal Point 2
* Game Changer
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
* Marathon Hero 2
* Marathon Hero 2 DLC
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
* Xroniàl Xéro
* Zero Gravity

If there's a setlist or pack you'd like to see on here, let me know.

## Results

### Overall

The combined runtime was 64m19s. Below is a histogram of the runtime for each
song, with the top 1% combined as outliers.

![Runtime histogram](runtime-histogram.svg)

The 25 slowest songs are as follows:

| Song                                             | Setlist                      | Time (s) |
| ------------------------------------------------ | ---------------------------- | -------- |
| The Forever Saga                                 | Marathon Hero 2              | 55.919   |
| CHARTS 2: Endless Setlist                        | CHARTS 2                     | 45.690   |
| CHARTS: The (almost) Endless Setlist             | CHARTS                       | 29.965   |
| Way Too Damn Much Anti Hero 2                    | Anti Hero 2 Charity Drive    | 19.006   |
| Endless Setlist: The Fall of Troy Hero           | The Fall of Troy Hero        | 18.309   |
| Vector/Virus                                     | Marathon Hero 2              | 12.441   |
| Uranoid                                          | CHARTS                       | 11.484   |
| Chezy's Ultimate Solo Experience                 | CHARTS 2                     | 11.235   |
| The Human Equation                               | Marathon Hero                | 8.272    |
| The Dark Tower (Full Album)                      | Marathon Hero 2              | 8.220    |
| The Tale of Iso Subject 5                        | Marathon Hero 2              | 7.495    |
| Reincarnation (Full Album)                       | Marathon Hero 2              | 7.299    |
| Catch Thirtythree                                | Djent Hero Collection        | 6.917    |
| Under the Force of Courage [FULL ALBUM]          | Anti Hero 2 Charity Drive    | 6.913    |
| Operation: Mindcrime (Full Album)                | Marathon Hero 2              | 6.759    |
| Ascendancy [FULL ALBUM]                          | Anti Hero 2 Charity Drive    | 6.501    |
| Eskapist                                         | Marathon Hero                | 5.973    |
| Eskapist                                         | CHARTS                       | 5.965    |
| Automata                                         | Marathon Hero                | 5.792    |
| Tragic Separation (Full Album)                   | Marathon Hero 2              | 5.679    |
| St. Anger (Full Album)                           | Marathon Hero 2              | 5.463    |
| Periphery IV: Hail Stan (Full Album)             | CHARTS 2                     | 5.345    |
| Volition (Full Album Chart)                      | CHARTS                       | 5.184    |
| Coma Witch (Full Album Chart)                    | CHARTS                       | 5.176    |
| Frankenstein                                     | Marathon Hero 2              | 4.915    |

### By Setlist

| Setlist                                    | Number of songs | Total time (s) | Average time (s) |
| ------------------------------------------ | --------------- | -------------- | ---------------- |
| Angevil Hero II                            | 67              | 28.183         | 0.421            |
| Anti Hero                                  | 402             | 173.394        | 0.431            |
| Anti Hero: Beach Episode                   | 127             | 62.850         | 0.495            |
| Anti Hero: Beach Episode - The Beach Sides | 25              | 10.235         | 0.409            |
| Anti Hero 2                                | 365             | 163.347        | 0.448            |
| Anti Hero 2 Charity Drive                  | 9               | 51.561         | 5.729            |
| Band Hero                                  | 65              | 21.602         | 0.332            |
| Blanket Statement                          | 115             | 44.198         | 0.384            |
| Brand New Hero                             | 83              | 29.347         | 0.354            |
| Carpal Tunnel Hero: Remastered             | 104             | 62.126         | 0.597            |
| Carpal Tunnel Hero 2                       | 309             | 165.174        | 0.535            |
| CHARTS                                     | 646             | 320.394        | 0.496            |
| CHARTS 2                                   | 139             | 117.419        | 0.845            |
| CHARTS 2 DLC                               | 43              | 20.092         | 0.467            |
| Circuit Breaker                            | 116             | 44.146         | 0.381            |
| Cow Hero                                   | 71              | 27.325         | 0.385            |
| CSC Monthly Packs                          | 1044            | 379.262        | 0.363            |
| DF Discography CH                          | 80              | 46.296         | 0.579            |
| Digitizer                                  | 82              | 29.958         | 0.365            |
| Dissonance Hero                            | 106             | 51.532         | 0.486            |
| Dissonance Hero Free-LC                    | 22              | 7.829          | 0.356            |
| DJ Hero                                    | 10              | 2.397          | 0.240            |
| Djent Hero Collection                      | 200             | 96.327         | 0.482            |
| DJMax Packs                                | 58              | 12.720         | 0.219            |
| Facelift                                   | 86              | 34.166         | 0.397            |
| Focal Point                                | 170             | 80.707         | 0.475            |
| Focal Point 2                              | 186             | 86.246         | 0.464            |
| Game Changer                               | 63              | 31.671         | 0.503            |
| GaMetal Power Pack                         | 54              | 26.331         | 0.488            |
| Green Day Rock Band                        | 43              | 14.009         | 0.326            |
| Guitar Hero                                | 49              | 15.363         | 0.314            |
| Guitar Hero II                             | 74              | 25.171         | 0.340            |
| Guitar Hero II DLC                         | 24              | 7.860          | 0.327            |
| Guitar Hero Encore: Rocks the 80s          | 30              | 10.035         | 0.335            |
| Guitar Hero III: Legends of Rock           | 70              | 24.377         | 0.348            |
| Guitar Hero III: Legends of Rock DLC       | 68              | 28.543         | 0.420            |
| Guitar Hero: Aerosmith                     | 41              | 13.640         | 0.333            |
| Guitar Hero World Tour                     | 84              | 30.369         | 0.362            |
| Guitar Hero World Tour DLC                 | 147             | 59.072         | 0.402            |
| Guitar Hero: Metallica                     | 49              | 25.352         | 0.517            |
| Guitar Hero: Metallica DLC                 | 10              | 6.814          | 0.681            |
| Guitar Hero Smash Hits                     | 48              | 17.500         | 0.365            |
| Guitar Hero: Van Halen                     | 47              | 18.365         | 0.391            |
| Guitar Hero 5                              | 84              | 32.992         | 0.393            |
| Guitar Hero 5 DLC                          | 158             | 54.581         | 0.345            |
| Guitar Hero: Warriors of Rock              | 93              | 33.380         | 0.359            |
| Guitar Hero: Warriors of Rock DLC          | 84              | 31.686         | 0.377            |
| Guitar Hero On Tour                        | 31              | 10.082         | 0.325            |
| Guitar Hero On Tour: Decades               | 36              | 10.877         | 0.302            |
| Guitar Hero On Tour: Modern Hits           | 44              | 13.281         | 0.302            |
| Guitar Hero: Guitar Zero                   | 64              | 20.778         | 0.325            |
| Guitar Hero: Guitar Zero DLC               | 38              | 18.661         | 0.491            |
| Guitar Hero X                              | 127             | 66.728         | 0.525            |
| Koreaboo Hero                              | 51              | 14.827         | 0.291            |
| Koreaboo Hero 2                            | 101             | 28.897         | 0.286            |
| Lego Rock Band                             | 45              | 12.741         | 0.283            |
| Marathon Hero                              | 49              | 87.785         | 1.792            |
| Marathon Hero 2                            | 152             | 273.501        | 1.799            |
| Marathon Hero 2 DLC                        | 216             | 107.978        | 0.500            |
| Paradigm                                   | 101             | 44.525         | 0.441            |
| Phase Shift Guitar Project 4               | 162             | 69.975         | 0.432            |
| Redemption Arc                             | 100             | 41.439         | 0.414            |
| Rock Band                                  | 58              | 20.024         | 0.345            |
| Rock Band 2                                | 84              | 27.644         | 0.329            |
| Rock Band 3                                | 83              | 27.273         | 0.329            |
| Rock Band ACDC Live Track Pack             | 18              | 7.832          | 0.435            |
| Rock Band Blitz                            | 25              | 8.221          | 0.329            |
| The Beatles Rock Band                      | 45              | 10.945         | 0.243            |
| The Fall of Troy Hero                      | 70              | 58.323         | 0.833            |
| Trunks252JM's Classic Charts               | 56              | 22.236         | 0.397            |
| Verified Unverified                        | 21              | 9.076          | 0.432            |
| Vortex Hero                                | 222             | 91.371         | 0.412            |
| Xroniàl Xéro                               | 17              | 8.703          | 0.512            |
| Zero Gravity                               | 179             | 68.919         | 0.385            |

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
| Incessant Mace                                | Game Changer                     | 6.589x             |
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

There are 73 songs that cannot be 7 starred. 37 of them do not have any Star
Power; the 36 that do are as follows:

| Song                                                       | Setlist                      | Average Multiplier |
| ---------------------------------------------------------- | ---------------------------- | ------------------ |
| All                                                        | CSC Monthly Packs            | 1.016x             |
| No, All!                                                   | CSC Monthly Packs            | 1.095x             |
| March of the Machines                                      | Marathon Hero 2 DLC          | 1.664x             |
| Curtain                                                    | Marathon Hero 2 DLC          | 2.217x             |
| Daniel's Vision                                            | Marathon Hero 2 DLC          | 3.039x             |
| Fuck the Kids I & II                                       | Game Changer                 | 3.268x             |
| They Don't Have To Believe...                              | CHARTS                       | 3.594x             |
| Day One: Vigil                                             | Marathon Hero 2 DLC          | 3.885x             |
| Uranoid                                                    | CHARTS                       | 4.121x             |
| Tathagata                                                  | Marathon Hero 2 DLC          | 4.131x             |
| Z:iRNiTRA                                                  | Xroniàl Xéro                 | 4.155x             |
| Xroniàl Xéro                                               | Xroniàl Xéro                 | 4.168x             |
| Completeness Under Incompleteness ("true prooF" Long ver.) | Xroniàl Xéro                 | 4.202x             |
| The Mirror Cluster Genesis Theory                          | Xroniàl Xéro                 | 4.217x             |
| Lowermost revolt ("Jerermiad" Long ver.)                   | Xroniàl Xéro                 | 4.241x             |
| Xéroa ("préconnaiXance" Long ver.)                         | Xroniàl Xéro                 | 4.261x             |
| The Betrayal                                               | Carpal Tunnel Hero 2         | 4.267x             |
| BANGER A.F., BROOOO!!!                                     | Xroniàl Xéro                 | 4.277x             |
| Diastrophism ("Anamorphism" Long ver.)                     | Xroniàl Xéro                 | 4.279x             |
| A Place Where Bugs Live -Crawling Yet Deeper-              | Xroniàl Xéro                 | 4.299x             |
| Bleed                                                      | CHARTS                       | 4.311x             |
| Breakout (feat. Scandroid)                                 | Phase Shift Guitar Project 4 | 4.332x             |
| Luca (Demo)                                                | Brand New Hero               | 4.335x             |
| Coffee Mug (Originally by Descendents)                     | CHARTS                       | 4.335x             |
| MADD BLEEPP                                                | Xroniàl Xéro                 | 4.345x             |
| Shards of Scorched Flesh                                   | Carpal Tunnel Hero 2         | 4.349x             |
| ƒi?orz?                                                    | Xroniàl Xéro                 | 4.355x             |
| Introduction - Xursed divinitiY                            | Xroniàl Xéro                 | 4.361x             |
| Xronièr ("genèXe" Long ver.)                               | Xroniàl Xéro                 | 4.361x             |
| Cascading Failures, Diminishing Returns                    | Carpal Tunnel Hero 2         | 4.366x             |
| Built This Pool (blink-182 Cover)                          | CHARTS                       | 4.368x             |
| The Pretender                                              | Paradigm                     | 4.378x             |
| Infinitesimal to the Infinite                              | Zero Gravity                 | 4.382x             |
| Get Possessed                                              | Anti Hero                    | 4.386x             |
| =El=Dorado=                                                | Xroniàl Xéro                 | 4.396x             |
| Monkey Wrench                                              | Guitar Hero II               | 4.400x             |

Note the average multiplier for Monkey Wrench is rounding from 4.39995x. Clone
Hero would display the average multiplier as 4.400x, but 5 more points would be
needed for the 7 star.
