# How fast is CHOpt

CHOpt is now fast enough that I can throw multiple setlists at it, so for fun I
decided to do so and see what results come up. Don't take this too seriously:
for one I only ran this on all these songs once. I did this with CHOpt 1.4.0.

## The songs

I ran CHOpt on every song from the following setlists and packs with Expert
Lead Guitar, except for songs lacking them in which case I did Expert Bass or
Keys.

* Angevil Hero I
* Angevil Hero II
* Anti Hero
* Anti Hero 2
* Anti Hero 2 Charity Drive
* Anti Hero: Beach Episode
* Anti Hero: Beach Episode - The Beach Sides
* Band Hero
* Bitcrusher
* Blanket Statement
* Brand New Hero
* Carpal Tunnel Hero: Remastered
* Carpal Tunnel Hero 2
* CHARTS
* CHARTS 2
* CHARTS 2 DLC (Pack 1 - Pack 2)
* Circuit Breaker
* Community Track Packs (Pack 1 - Pack 11)
* Cow Hero
* CSC Monthly Packs (June 2018 - December 2021)
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
* Guitar Hero: Guitar Zero DLC (DLC #1 - DLC #12)
* Guitar Hero: Guitar Zero 2
* Guitar Hero: Guitar Zero 2 DLC (DLC #1 - DLC #7)
* Guitar Hero X
* Guitar Hero X-II
* Harmony Hero
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
* Symphonic Effect
* Symphony X Discography Setlist
* Technical Difficulties Setlist (Pack 1)
* The Beatles Rock Band
* The Fall of Troy Hero
* Trunks252JM's Classic Charts
* Verified Unverified
* Vortex Hero
* WCC Monthly Packs (January 2019 - November 2021)
* Xroniàl Xéro
* Zero Gravity
* Zero Gravity: Space Battle

If there's a setlist or pack you'd like to see on here, let me know.

## Results

### Overall

The combined runtime was 103m17s. Below is a histogram of the runtime for each
song, with the top 1% combined as outliers.

![Runtime histogram](runtime-histogram.svg)

The 25 slowest songs are as follows:

| Song                                    | Setlist                        | Time (s) |
| --------------------------------------- | ------------------------------ | -------- |
| The Forever Saga                        | Marathon Hero 2                | 62.726   |
| CHARTS 2: Endless Setlist               | CHARTS 2                       | 48.987   |
| CHARTS: The (almost) Endless Setlist    | CHARTS                         | 32.175   |
| CTP5 Overture                           | Community Track Packs          | 27.341   |
| Way Too Damn Much Anti Hero 2           | Anti Hero 2 Charity Drive      | 21.736   |
| Endless Setlist: The Fall of Troy Hero  | The Fall of Troy Hero          | 19.268   |
| FIRECORE 4: Burned To Ashes             | Community Track Packs          | 13.895   |
| Vector/Virus                            | Marathon Hero 2                | 13.575   |
| Uranoid                                 | CHARTS                         | 12.933   |
| Chezy's Ultimate Solo Experience        | CHARTS 2                       | 12.152   |
| The Dark Tower (Full Album)             | Marathon Hero 2                | 9.082    |
| The Human Equation                      | Marathon Hero                  | 9.054    |
| The Olympian                            | Community Track Packs          | 8.945    |
| The Tale of Iso Subject 5               | Marathon Hero 2                | 8.151    |
| Reincarnation (Full Album)              | Marathon Hero 2                | 7.926    |
| Iconoclast (Full Album)                 | Symphony X Discography Setlist | 7.900    |
| Catch Thirtythree                       | Djent Hero Collection          | 7.800    |
| Operation: Mindcrime (Full Album)       | Marathon Hero 2                | 7.619    |
| Under the Force of Courage [FULL ALBUM] | Anti Hero 2 Charity Drive      | 7.570    |
| The Odyssey (Full Album)                | Symphony X Discography Setlist | 7.308    |
| Ascendancy [FULL ALBUM]                 | Anti Hero 2 Charity Drive      | 6.957    |
| Stillworld (Full Album)                 | Djent Hero Collection          | 6.830    |
| Eskapist                                | CHARTS                         | 6.776    |
| Eskapist                                | Marathon Hero                  | 6.598    |
| Death of a Dead Day (Full Album)        | Djent Hero Collection          | 6.578    |

### By Setlist

| Setlist                                           | Number of songs | Total time (s) | Average time (s) |
| ------------------------------------------------- | --------------- | -------------- | ---------------- |
| Angevil Hero I                                    | 80              | 34.227         | 0.428            |
| Angevil Hero II                                   | 67              | 28.214         | 0.421            |
| Anti Hero                                         | 402             | 184.862        | 0.460            |
| Anti Hero 2                                       | 365             | 174.853        | 0.479            |
| Anti Hero 2 Charity Drive                         | 9               | 56.574         | 6.286            |
| Anti Hero: Beach Episode                          | 127             | 66.891         | 0.527            |
| Anti Hero: Beach Episode - The Beach Sides        | 25              | 10.920         | 0.437            |
| Band Hero                                         | 65              | 27.442         | 0.422            |
| Bitcrusher                                        | 74              | 31.363         | 0.424            |
| Blanket Statement                                 | 115             | 47.039         | 0.409            |
| Brand New Hero                                    | 83              | 31.898         | 0.384            |
| Carpal Tunnel Hero: Remastered                    | 104             | 66.663         | 0.641            |
| Carpal Tunnel Hero 2                              | 309             | 180.814        | 0.585            |
| CHARTS                                            | 646             | 355.201        | 0.550            |
| CHARTS 2                                          | 139             | 129.660        | 0.933            |
| CHARTS 2 DLC                                      | 94              | 49.866         | 0.530            |
| Circuit Breaker                                   | 116             | 50.436         | 0.435            |
| Community Track Packs                             | 625             | 375.875        | 0.601            |
| Cow Hero                                          | 71              | 30.449         | 0.429            |
| CSC Monthly Packs                                 | 1296            | 541.872        | 0.418            |
| DF Discography CH                                 | 80              | 51.866         | 0.648            |
| Digitizer                                         | 82              | 34.354         | 0.419            |
| Dissonance Hero                                   | 106             | 57.333         | 0.541            |
| Dissonance Hero Free-LC                           | 22              | 8.820          | 0.401            |
| DJ Hero                                           | 10              | 2.606          | 0.261            |
| Djent Hero Collection                             | 237             | 156.182        | 0.659            |
| DJMax Packs                                       | 58              | 15.628         | 0.269            |
| Facelift                                          | 86              | 38.223         | 0.444            |
| Focal Point                                       | 170             | 88.452         | 0.520            |
| Focal Point 2                                     | 186             | 94.606         | 0.509            |
| Game Changer                                      | 63              | 35.821         | 0.569            |
| GaMetal Power Pack                                | 54              | 28.887         | 0.535            |
| Glitter Hero                                      | 26              | 8.923          | 0.343            |
| Green Day Rock Band                               | 43              | 16.502         | 0.384            |
| Guitar Hero                                       | 49              | 16.699         | 0.341            |
| Guitar Hero II                                    | 74              | 27.035         | 0.365            |
| Guitar Hero II DLC                                | 24              | 8.564          | 0.357            |
| Guitar Hero Encore: Rocks the 80s                 | 30              | 10.780         | 0.359            |
| Guitar Hero III: Legends of Rock                  | 70              | 26.347         | 0.376            |
| Guitar Hero III: Legends of Rock DLC              | 68              | 31.503         | 0.463            |
| Guitar Hero: Aerosmith                            | 41              | 14.600         | 0.356            |
| Guitar Hero World Tour                            | 84              | 38.390         | 0.457            |
| Guitar Hero World Tour DLC                        | 147             | 69.474         | 0.473            |
| Guitar Hero: Metallica                            | 49              | 31.686         | 0.647            |
| Guitar Hero: Metallica DLC                        | 10              | 8.873          | 0.887            |
| Guitar Hero Smash Hits                            | 48              | 21.514         | 0.448            |
| Guitar Hero: Van Halen                            | 47              | 22.946         | 0.488            |
| Guitar Hero 5                                     | 84              | 42.230         | 0.503            |
| Guitar Hero 5 DLC                                 | 158             | 67.849         | 0.429            |
| Guitar Hero: Warriors of Rock                     | 93              | 43.931         | 0.472            |
| Guitar Hero: Warriors of Rock DLC                 | 84              | 40.158         | 0.478            |
| Guitar Hero On Tour                               | 31              | 10.954         | 0.353            |
| Guitar Hero On Tour: Decades                      | 36              | 11.813         | 0.328            |
| Guitar Hero On Tour: Modern Hits                  | 44              | 14.630         | 0.333            |
| Guitar Hero: Guitar Zero                          | 64              | 23.966         | 0.374            |
| Guitar Hero: Guitar Zero DLC                      | 49              | 24.496         | 0.500            |
| Guitar Hero: Guitar Zero 2                        | 134             | 66.297         | 0.495            |
| Guitar Hero: Guitar Zero 2 DLC                    | 50              | 21.079         | 0.422            |
| Guitar Hero X                                     | 127             | 76.251         | 0.600            |
| Guitar Hero X-II                                  | 157             | 100.130        | 0.638            |
| Harmony Hero                                      | 49              | 22.850         | 0.466            |
| Holiday Overcharts                                | 33              | 10.310         | 0.312            |
| Koreaboo Hero                                     | 51              | 16.467         | 0.323            |
| Koreaboo Hero 2                                   | 101             | 32.495         | 0.322            |
| Lego Rock Band                                    | 45              | 14.654         | 0.326            |
| Marathon Hero                                     | 49              | 97.179         | 1.983            |
| Marathon Hero 2                                   | 152             | 301.040        | 1.981            |
| Marathon Hero 2 DLC                               | 216             | 122.515        | 0.567            |
| Miscellaneous Packs                               | 53              | 28.759         | 0.543            |
| Paradigm                                          | 101             | 49.738         | 0.492            |
| Phase Shift Guitar Project 4                      | 162             | 77.402         | 0.478            |
| Plastic Shred Hero: Legends of Apathetic Charting | 169             | 84.839         | 0.502            |
| Project Strandberger                              | 100             | 50.866         | 0.509            |
| Redemption Arc                                    | 100             | 45.440         | 0.454            |
| Rock Band                                         | 58              | 22.755         | 0.392            |
| Rock Band 2                                       | 84              | 32.151         | 0.383            |
| Rock Band 3                                       | 83              | 31.781         | 0.383            |
| Rock Band ACDC Live Track Pack                    | 18              | 8.790          | 0.488            |
| Rock Band Blitz                                   | 25              | 10.009         | 0.400            |
| Symphonic Effect                                  | 150             | 107.387        | 0.716            |
| Symphony X Discography Setlist                    | 101             | 111.706        | 1.106            |
| Technical Difficulties Setlist                    | 55              | 26.546         | 0.483            |
| The Beatles Rock Band                             | 45              | 13.490         | 0.300            |
| The Fall of Troy Hero                             | 70              | 63.596         | 0.909            |
| Trunks252JM's Classic Charts                      | 56              | 25.250         | 0.451            |
| Verified Unverified                               | 21              | 9.923          | 0.473            |
| Vortex Hero                                       | 222             | 102.032        | 0.460            |
| WCC Monthly Packs                                 | 1239            | 486.305        | 0.392            |
| Xroniàl Xéro                                      | 17              | 9.859          | 0.580            |
| Zero Gravity                                      | 179             | 76.569         | 0.428            |
| Zero Gravity: Space Battle                        | 236             | 118.569        | 0.502            |

## Average Multiplier Outliers

I've started gathering more information when I measure performance, including
average multiplier. Excluding Community Track Pack songs, the 25 songs with the
highest optimal average multiplier are as follows:

| Song                              | Setlist                                           | Average Multiplier |
| --------------------------------- | ------------------------------------------------- | ------------------ |
| Sugar Foot Rag                    | Carpal Tunnel Hero 2                              | 7.276x             |
| SLOW DANCING IN THE DARK          | Glitter Hero                                      | 7.165x             |
| Trojans                           | Carpal Tunnel Hero 2                              | 7.070x             |
| No Buses                          | WCC Monthly Packs                                 | 6.978x             |
| Trepak                            | Holiday Overcharts                                | 6.888x             |
| Downfall of Gaia                  | CSC Monthly Packs                                 | 6.874x             |
| Shards of Scorched Flesh          | Plastic Shred Hero: Legends of Apathetic Charting | 6.835x             |
| Gee-Wiz                           | Carpal Tunnel Hero 2                              | 6.824x             |
| Star X Speed Story Solo Medley    | Anti Hero: Beach Episode                          | 6.784x             |
| Thunder And Lightning             | Carpal Tunnel Hero 2                              | 6.752x             |
| Dithering                         | Paradigm                                          | 6.751x             |
| The Office Chiptune Madness       | WCC Monthly Packs                                 | 6.747x             |
| Callaita (feat. Tainy)            | WCC Monthly Packs                                 | 6.743x             |
| Trumpet Christmas                 | Holiday Overcharts                                | 6.733x             |
| Existence                         | WCC Monthly Packs                                 | 6.729x             |
| Black Hole Sun                    | Rock Band                                         | 6.696x             |
| Fat Refund                        | Plastic Shred Hero: Legends of Apathetic Charting | 6.683x             |
| lifeisgood                        | CHARTS 2                                          | 6.658x             |
| Megalovania (Metal Cover)         | Plastic Shred Hero: Legends of Apathetic Charting | 6.642x             |
| Hoy Lo Siento (feat. Tony Dize)   | WCC Monthly Packs                                 | 6.603x             |
| Incessant Mace                    | Game Changer                                      | 6.589x             |
| Computers                         | WCC Monthly Packs                                 | 6.583x             |
| He Then Turned the Gun on Himself | Plastic Shred Hero: Legends of Apathetic Charting | 6.578x             |
| Blunt Instrument                  | Plastic Shred Hero: Legends of Apathetic Charting | 6.576x             |
| All Is One                        | Vortex Hero                                       | 6.572x             |

There are 165 songs that cannot be 7 starred. 110 of them do not have any Star
Power; the 55 that do are as follows:

| Song                                                       | Setlist                                           | Average Multiplier |
| ---------------------------------------------------------- | ------------------------------------------------- | ------------------ |
| You Suffer                                                 | Plastic Shred Hero: Legends of Apathetic Charting | 1.000x             |
| All                                                        | CSC Monthly Packs                                 | 1.016x             |
| No, All!                                                   | CSC Monthly Packs                                 | 1.095x             |
| You Suffer (S3RL Remix)                                    | CSC Monthly Packs                                 | 1.333x             |
| March of the Machines                                      | Marathon Hero 2 DLC                               | 1.664x             |
| Curtain                                                    | Marathon Hero 2 DLC                               | 2.217x             |
| Any                                                        | WCC Monthly Packs                                 | 2.552x             |
| Daniel's Vision                                            | Marathon Hero 2 DLC                               | 3.039x             |
| Transcendence (Segue)                                      | Symphony X Discography Setlist                    | 3.081x             |
| Más Artista Que El Artista                                 | WCC Monthly Packs                                 | 3.207x             |
| Fuck the Kids I & II                                       | Game Changer                                      | 3.268x             |
| They Don't Have To Believe...                              | CHARTS                                            | 3.594x             |
| Hillside                                                   | CHARTS 2 DLC                                      | 3.819x             |
| Sacred Omen                                                | WCC Monthly Packs                                 | 3.850x             |
| Day One: Vigil                                             | Marathon Hero 2 DLC                               | 3.885x             |
| Beaver Moon                                                | Project Strandberger                              | 4.030x             |
| Folklorhiem                                                | WCC Monthly Packs                                 | 4.103x             |
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
| Bad Bad Things                                             | WCC Monthly Packs                                 | 4.284x             |
| A Place Where Bugs Live -Crawling Yet Deeper-              | Xroniàl Xéro                                      | 4.299x             |
| ～Call of the Genealogy～Eiyuu no keifu                      | WCC Monthly Packs                                 | 4.308x             |
| Bleed                                                      | CHARTS                                            | 4.311x             |
| Prologue ~The Final Decisive Battle~                       | WCC Monthly Packs                                 | 4.315x             |
| Yofo                                                       | WCC Monthly Packs                                 | 4.325x             |
| Breakout (feat. Scandroid)                                 | Phase Shift Guitar Project 4                      | 4.332x             |
| Coffee Mug (Originally by Descendents)                     | CHARTS                                            | 4.335x             |
| Luca (Demo)                                                | Brand New Hero                                    | 4.335x             |
| Isle Of Flightless Birds                                   | WCC Monthly Packs                                 | 4.340x             |
| MADD BLEEPP                                                | Xroniàl Xéro                                      | 4.345x             |
| Shards of Scorched Flesh                                   | Carpal Tunnel Hero 2                              | 4.349x             |
| Musical Lobster                                            | Community Track Packs                             | 4.353x             |
| ƒiиorzᾶ                                                     | Xroniàl Xéro                                      | 4.355x             |
| Introduction - Xursed divinitiY                            | Xroniàl Xéro                                      | 4.361x             |
| Xronièr ("genèXe" Long ver.)                               | Xroniàl Xéro                                      | 4.361x             |
| Cascading Failures, Diminishing Returns                    | Carpal Tunnel Hero 2                              | 4.366x             |
| Built This Pool (blink-182 Cover)                          | CHARTS                                            | 4.368x             |
| The Pretender                                              | Paradigm                                          | 4.378x             |
| Moniker                                                    | Community Track Packs                             | 4.381x             |
| Infinitesimal to the Infinite                              | Zero Gravity                                      | 4.382x             |
| Get Possessed                                              | Anti Hero                                         | 4.386x             |
| =El=Dorado=                                                | Xroniàl Xéro                                      | 4.396x             |
| Ivory Shores                                               | WCC Monthly Packs                                 | 4.400x             |
| Monkey Wrench                                              | Guitar Hero II                                    | 4.400x             |

Note the average multiplier for Monkey Wrench is rounding from 4.39995x. Clone
Hero would display the average multiplier as 4.400x, but 5 more points would be
needed for the 7 star. Similarly, the average multiplier for Ivory Shores is
rounding from 4.3998x and 3 more points would be needed to 7 star.
