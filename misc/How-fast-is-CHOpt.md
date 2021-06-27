# How fast is CHOpt

CHOpt is now fast enough that I can throw multiple setlists at it, so for fun I
decided to do so and see what results come up. Don't take this too seriously:
for one I only ran this on all these songs once. I did this with CHOpt 1.3.2.

## The songs

I ran CHOpt on every song from the following setlists and packs with Expert
Lead Guitar, except Classical Thump, Dawn Patrol, Imagine, and Space Cowboy for
which I did Expert Bass.

* Angevil Hero I
* Angevil Hero II
* Anti Hero
* Anti Hero 2
* Anti Hero 2 Charity Drive
* Anti Hero: Beach Episode
* Anti Hero: Beach Episode - The Beach Sides
* Band Hero
* Blanket Statement
* Brand New Hero
* Carpal Tunnel Hero: Remastered
* Carpal Tunnel Hero 2
* CHARTS
* CHARTS 2
* CHARTS 2 DLC (Pack 1 - Pack 2)
* Circuit Breaker
* Community Track Packs (Pack 6 - Pack X)
* Cow Hero
* CSC Monthly Packs (June 2018 - June 2021)
* DF Discography CH
* Digitizer
* Dissonance Hero
* Dissonance Hero Free-LC (Pack 1)
* DJ Hero
* Djent Hero Collection (Pack #1 - Pack #7)
* DJMax Packs (Pack I - Pack II)
* Facelift (Pack 1 - Pack 2)
* Focal Point
* Focal Point 2
* Game Changer
* GaMetal Power Pack
* Glitter Hero
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
* Guitar Hero: Guitar Zero 2
* Guitar Hero: Guitar Zero 2 DLC (DLC #1)
* Guitar Hero X
* Holiday Overcharts (2017 - 2020)
* Koreaboo Hero
* Koreaboo Hero 2
* Lego Rock Band
* Marathon Hero
* Marathon Hero 2
* Marathon Hero 2 DLC
* Miscellaneous Packs (Pack #1)
* Paradigm
* Phase Shift Guitar Project 4
* Plastic Shred Hero: Legends of Apathetic Charting
* Project Strandberger
* Redemption Arc
* Rock Band
* Rock Band 2
* Rock Band 3
* Rock Band ACDC Live Track Pack
* Rock Band Blitz
* Symphony X Discography Setlist
* The Beatles Rock Band
* The Fall of Troy Hero
* Trunks252JM's Classic Charts
* Verified Unverified
* Vortex Hero
* Xroniàl Xéro
* Zero Gravity
* Zero Gravity: Space Battle

If there's a setlist or pack you'd like to see on here, let me know.

## Results

### Overall

The combined runtime was 81m43s. Below is a histogram of the runtime for each
song, with the top 1% combined as outliers.

![Runtime histogram](runtime-histogram.svg)

The 25 slowest songs are as follows:

| Song                                     | Setlist                        | Time (s) |
| ---------------------------------------- | ------------------------------ | -------- |
| The Forever Saga                         | Marathon Hero 2                | 59.918   |
| CHARTS 2: Endless Setlist                | CHARTS 2                       | 46.375   |
| CHARTS: The (almost) Endless Setlist     | CHARTS                         | 32.529   |
| Way Too Damn Much Anti Hero 2            | Anti Hero 2 Charity Drive      | 18.791   |
| Endless Setlist: The Fall of Troy Hero   | The Fall of Troy Hero          | 18.327   |
| Vector/Virus                             | Marathon Hero 2                | 13.310   |
| Chezy's Ultimate Solo Experience         | CHARTS 2                       | 12.121   |
| Uranoid                                  | CHARTS                         | 11.995   |
| The Human Equation                       | Marathon Hero                  | 8.527    |
| The Dark Tower (Full Album)              | Marathon Hero 2                | 8.497    |
| The Olympian                             | Community Track Packs          | 8.377    |
| The Tale of Iso Subject 5                | Marathon Hero 2                | 7.593    |
| Under the Force of Courage [FULL ALBUM]  | Anti Hero 2 Charity Drive      | 7.556    |
| Reincarnation (Full Album)               | Marathon Hero 2                | 7.520    |
| Iconoclast (Full Album)                  | Symphony X Discography Setlist | 7.431    |
| Catch Thirtythree                        | Djent Hero Collection          | 7.283    |
| Operation: Mindcrime (Full Album)        | Marathon Hero 2                | 7.148    |
| The Odyssey (Full Album)                 | Symphony X Discography Setlist | 6.870    |
| Periphery IV: Hail Stan (Full Album)     | CHARTS 2                       | 6.550    |
| Ascendancy [FULL ALBUM]                  | Anti Hero 2 Charity Drive      | 6.497    |
| Death of a Dead Day (Full Album)         | Djent Hero Collection          | 6.456    |
| Eskapist                                 | CHARTS                         | 6.153    |
| Eskapist                                 | Marathon Hero                  | 6.138    |
| V: The New Mythology Suite (Full Album)  | Symphony X Discography Setlist | 6.000    |
| The Divine Wings of Tragedy (Full Album) | Symphony X Discography Setlist | 5.967    |

### By Setlist

| Setlist                                           | Number of songs | Total time (s) | Average time (s) |
| ------------------------------------------------- | --------------- | -------------- | ---------------- |
| Angevil Hero I                                    | 80              | 31.665         | 0.396            |
| Angevil Hero II                                   | 67              | 39.278         | 0.586            |
| Anti Hero                                         | 402             | 235.054        | 0.585            |
| Anti Hero 2                                       | 365             | 182.078        | 0.499            |
| Anti Hero 2 Charity Drive                         | 9               | 51.902         | 5.767            |
| Anti Hero: Beach Episode                          | 127             | 94.101         | 0.741            |
| Anti Hero: Beach Episode - The Beach Sides        | 25              | 16.767         | 0.671            |
| Band Hero                                         | 65              | 22.487         | 0.346            |
| Blanket Statement                                 | 115             | 44.128         | 0.384            |
| Brand New Hero                                    | 83              | 29.080         | 0.350            |
| CHARTS                                            | 646             | 331.889        | 0.514            |
| CHARTS 2                                          | 139             | 122.781        | 0.883            |
| CHARTS 2 DLC                                      | 94              | 47.077         | 0.501            |
| CSC Monthly Packs                                 | 1124            | 434.048        | 0.386            |
| Carpal Tunnel Hero 2                              | 309             | 166.762        | 0.540            |
| Carpal Tunnel Hero: Remastered                    | 104             | 62.347         | 0.599            |
| Circuit Breaker                                   | 116             | 46.651         | 0.402            |
| Community Track Packs                             | 337             | 194.342        | 0.577            |
| Cow Hero                                          | 71              | 27.914         | 0.393            |
| DF Discography CH                                 | 80              | 47.814         | 0.598            |
| DJ Hero                                           | 10              | 2.529          | 0.253            |
| DJMax Packs                                       | 58              | 13.983         | 0.241            |
| Digitizer                                         | 82              | 31.339         | 0.382            |
| Dissonance Hero                                   | 106             | 52.921         | 0.499            |
| Dissonance Hero Free-LC                           | 22              | 8.217          | 0.373            |
| Djent Hero Collection                             | 237             | 145.519        | 0.614            |
| Facelift                                          | 86              | 35.765         | 0.416            |
| Focal Point                                       | 170             | 82.878         | 0.488            |
| Focal Point 2                                     | 186             | 88.996         | 0.478            |
| GaMetal Power Pack                                | 54              | 26.855         | 0.497            |
| Game Changer                                      | 63              | 33.340         | 0.529            |
| Glitter Hero                                      | 26              | 8.395          | 0.323            |
| Green Day Rock Band                               | 43              | 14.911         | 0.347            |
| Guitar Hero                                       | 49              | 15.582         | 0.318            |
| Guitar Hero 5                                     | 84              | 33.883         | 0.403            |
| Guitar Hero 5 DLC                                 | 158             | 56.527         | 0.358            |
| Guitar Hero Encore: Rocks the 80s                 | 30              | 10.118         | 0.337            |
| Guitar Hero II                                    | 74              | 25.162         | 0.340            |
| Guitar Hero II DLC                                | 24              | 7.923          | 0.330            |
| Guitar Hero III: Legends of Rock                  | 70              | 24.559         | 0.351            |
| Guitar Hero III: Legends of Rock DLC              | 68              | 28.971         | 0.426            |
| Guitar Hero On Tour                               | 31              | 10.243         | 0.330            |
| Guitar Hero On Tour: Decades                      | 36              | 11.131         | 0.309            |
| Guitar Hero On Tour: Modern Hits                  | 44              | 13.614         | 0.309            |
| Guitar Hero Smash Hits                            | 48              | 17.882         | 0.373            |
| Guitar Hero World Tour                            | 84              | 31.800         | 0.379            |
| Guitar Hero World Tour DLC                        | 147             | 62.201         | 0.423            |
| Guitar Hero X                                     | 127             | 70.313         | 0.554            |
| Guitar Hero: Aerosmith                            | 41              | 13.731         | 0.335            |
| Guitar Hero: Guitar Zero                          | 64              | 21.902         | 0.342            |
| Guitar Hero: Guitar Zero 2                        | 134             | 62.312         | 0.465            |
| Guitar Hero: Guitar Zero 2 DLC                    | 21              | 8.578          | 0.408            |
| Guitar Hero: Guitar Zero DLC                      | 38              | 20.616         | 0.543            |
| Guitar Hero: Metallica                            | 49              | 26.025         | 0.531            |
| Guitar Hero: Metallica DLC                        | 10              | 6.916          | 0.692            |
| Guitar Hero: Van Halen                            | 47              | 18.797         | 0.400            |
| Guitar Hero: Warriors of Rock                     | 93              | 34.432         | 0.370            |
| Guitar Hero: Warriors of Rock DLC                 | 84              | 33.842         | 0.403            |
| Holiday Overcharts                                | 33              | 9.599          | 0.291            |
| Koreaboo Hero                                     | 51              | 15.478         | 0.303            |
| Koreaboo Hero 2                                   | 101             | 30.300         | 0.300            |
| Lego Rock Band                                    | 45              | 12.853         | 0.286            |
| Marathon Hero                                     | 49              | 93.526         | 1.909            |
| Marathon Hero 2                                   | 152             | 281.407        | 1.851            |
| Marathon Hero 2 DLC                               | 216             | 113.856        | 0.527            |
| Miscellaneous Packs                               | 53              | 26.116         | 0.493            |
| Paradigm                                          | 101             | 50.952         | 0.504            |
| Phase Shift Guitar Project 4                      | 162             | 71.182         | 0.439            |
| Plastic Shred Hero: Legends of Apathetic Charting | 169             | 78.994         | 0.467            |
| Project Strandberger                              | 100             | 47.732         | 0.477            |
| Redemption Arc                                    | 100             | 42.715         | 0.427            |
| Rock Band                                         | 58              | 20.763         | 0.358            |
| Rock Band 2                                       | 84              | 28.721         | 0.342            |
| Rock Band 3                                       | 83              | 28.574         | 0.344            |
| Rock Band ACDC Live Track Pack                    | 18              | 8.124          | 0.451            |
| Rock Band Blitz                                   | 25              | 8.529          | 0.341            |
| Symphony X Discography Setlist                    | 101             | 104.036        | 1.030            |
| The Beatles Rock Band                             | 45              | 11.721         | 0.260            |
| The Fall of Troy Hero                             | 70              | 59.277         | 0.847            |
| Trunks252JM's Classic Charts                      | 56              | 23.144         | 0.413            |
| Verified Unverified                               | 21              | 9.275          | 0.442            |
| Vortex Hero                                       | 222             | 94.215         | 0.424            |
| Xroniàl Xéro                                      | 17              | 9.166          | 0.539            |
| Zero Gravity                                      | 179             | 71.162         | 0.398            |
| Zero Gravity: Space Battle                        | 236             | 110.972        | 0.470            |

## Average Multiplier Outliers

I've started gathering more information when I measure performance, including
average multiplier. The 25 songs with the highest optimal average multiplier are
as follows:

| Song                                          | Setlist                                           | Average Multiplier |
| --------------------------------------------- | ------------------------------------------------- | ------------------ |
| Sin of Gluttony                               | Community Track Packs                             | 7.556x             |
| Sugar Foot Rag                                | Carpal Tunnel Hero 2                              | 7.276x             |
| Κούνια                                         | Community Track Packs                             | 7.237x             |
| This Went Too Far                             | Community Track Packs                             | 7.204x             |
| SLOW DANCING IN THE DARK                      | Glitter Hero                                      | 7.165x             |
| Ode To THR3A                                  | Community Track Packs                             | 7.128x             |
| Trojans                                       | Carpal Tunnel Hero 2                              | 7.070x             |
| Nocturnal Keys 3                              | Community Track Packs                             | 7.030x             |
| Minute of what?                               | Community Track Packs                             | 7.029x             |
| VIP Party 2                                   | Community Track Packs                             | 6.980x             |
| Shiver vCH                                    | Community Track Packs                             | 6.936x             |
| Mozart and Scatology                          | Community Track Packs                             | 6.917x             |
| Fyighfreak's Solo 2                           | Community Track Packs                             | 6.903x             |
| Trepak                                        | Holiday Overcharts                                | 6.888x             |
| Icicles                                       | Community Track Packs                             | 6.879x             |
| Downfall of Gaia                              | CSC Monthly Packs                                 | 6.874x             |
| Shards of Scorched Flesh                      | Plastic Shred Hero: Legends of Apathetic Charting | 6.835x             |
| Gee-Wiz                                       | Carpal Tunnel Hero 2                              | 6.824x             |
| Star X Speed Story Solo Medley                | Anti Hero: Beach Episode                          | 6.784x             |
| Thunder And Lightning                         | Carpal Tunnel Hero 2                              | 6.752x             |
| Dithering                                     | Paradigm                                          | 6.751x             |
| Bytes                                         | Community Track Packs                             | 6.738x             |
| Trumpet Christmas                             | Holiday Overcharts                                | 6.733x             |
| Black Hole Sun                                | Rock Band                                         | 6.696x             |
| Fat Refund                                    | Plastic Shred Hero: Legends of Apathetic Charting | 6.683x             |

There are 118 songs that cannot be 7 starred. 73 of them do not have any Star
Power; the 45 that do are as follows:

| Song                                                       | Setlist                                           | Average Multiplier |
| ---------------------------------------------------------- | ------------------------------------------------- | ------------------ |
| You Suffer                                                 | Plastic Shred Hero: Legends of Apathetic Charting | 1.000x             |
| All                                                        | CSC Monthly Packs                                 | 1.016x             |
| No, All!                                                   | CSC Monthly Packs                                 | 1.095x             |
| You Suffer (S3RL Remix)                                    | CSC Monthly Packs                                 | 1.333x             |
| March of the Machines                                      | Marathon Hero 2 DLC                               | 1.664x             |
| Curtain                                                    | Marathon Hero 2 DLC                               | 2.217x             |
| Daniel's Vision                                            | Marathon Hero 2 DLC                               | 3.039x             |
| Transcendence (Segue)                                      | Symphony X Discography Setlist                    | 3.081x             |
| Fuck the Kids I & II                                       | Game Changer                                      | 3.268x             |
| They Don't Have To Believe...                              | CHARTS                                            | 3.594x             |
| Hillside                                                   | CHARTS 2 DLC                                      | 3.819x             |
| Day One: Vigil                                             | Marathon Hero 2 DLC                               | 3.885x             |
| Beaver Moon                                                | Project Strandberger                              | 4.030x             |
| Uranoid                                                    | CHARTS                                            | 4.121x             |
| Tathagata                                                  | Marathon Hero 2 DLC                               | 4.131x             |
| Z:iRNiTRA                                                  | Xroniàl Xéro                                      | 4.155x             |
| Xroniàl Xéro                                               | Xroniàl Xéro                                      | 4.168x             |
| Completeness Under Incompleteness ("true prooF" Long ver.) | Xroniàl Xéro                                      | 4.202x             |
| The Mirror Cluster Genesis Theory                          | Xroniàl Xéro                                      | 4.217x             |
| Lowermost revolt ("Jerermiad" Long ver.)                   | Xroniàl Xéro                                      | 4.241x             |
| Xéroa ("préconnaiXance" Long ver.)                         | Xroniàl Xéro                                      | 4.261x             |
| Scanlines                                                  | Plastic Shred Hero: Legends of Apathetic Charting | 4.265x             |
| The Betrayal                                               | Carpal Tunnel Hero 2                              | 4.267x             |
| BANGER A.F., BROOOO!!!                                     | Xroniàl Xéro                                      | 4.277x             |
| Diastrophism ("Anamorphism" Long ver.)                     | Xroniàl Xéro                                      | 4.279x             |
| Potential for Anything                                     | Plastic Shred Hero: Legends of Apathetic Charting | 4.282x             |
| A Place Where Bugs Live -Crawling Yet Deeper-              | Xroniàl Xéro                                      | 4.299x             |
| Bleed                                                      | CHARTS                                            | 4.311x             |
| Breakout (feat. Scandroid)                                 | Phase Shift Guitar Project 4                      | 4.332x             |
| Luca (Demo)                                                | Brand New Hero                                    | 4.335x             |
| Coffee Mug (Originally by Descendents)                     | CHARTS                                            | 4.335x             |
| MADD BLEEPP                                                | Xroniàl Xéro                                      | 4.345x             |
| Shards of Scorched Flesh                                   | Carpal Tunnel Hero 2                              | 4.349x             |
| Musical Lobster                                            | Community Track Packs                             | 4.353x             |
| ƒi?orz?                                                    | Xroniàl Xéro                                      | 4.355x             |
| Introduction - Xursed divinitiY                            | Xroniàl Xéro                                      | 4.361x             |
| Xronièr ("genèXe" Long ver.)                               | Xroniàl Xéro                                      | 4.361x             |
| Cascading Failures, Diminishing Returns                    | Carpal Tunnel Hero 2                              | 4.366x             |
| Built This Pool (blink-182 Cover)                          | CHARTS                                            | 4.368x             |
| The Pretender                                              | Paradigm                                          | 4.378x             |
| Moniker                                                    | Community Track Packs                             | 4.381x             |
| Infinitesimal to the Infinite                              | Zero Gravity                                      | 4.382x             |
| Get Possessed                                              | Anti Hero                                         | 4.386x             |
| =El=Dorado=                                                | Xroniàl Xéro                                      | 4.396x             |
| Monkey Wrench                                              | Guitar Hero II                                    | 4.400x             |

Note the average multiplier for Monkey Wrench is rounding from 4.39995x. Clone
Hero would display the average multiplier as 4.400x, but 5 more points would be
needed for the 7 star.
